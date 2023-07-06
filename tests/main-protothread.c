#include <stdarg.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <termios.h>
#include <unistd.h>
#include <stdint.h>

#define PT_USE_SETJMP 1

#include "pt.h"

static volatile atomic_int event = ATOMIC_VAR_INIT(0);

#define async_begin(name)                                                      \
    static void coroutine_##name(struct pt* pt)                                \
    {                                                                          \
        pt_begin(pt);                                                          \
        while (true)                                                           \
        {

#define async_end() \
        }                                                                      \
    pt_end(pt);                                                                \
    }

#define async(name)     coroutine_##name

typedef void (*coroutine_t)(struct pt* pt);

enum EVENT_TYPES
{
    EVENT_NONE = 0,
    EVENT_TIMER,
    EVENT_BEGIN,
    EVENT_1 = EVENT_BEGIN,
    EVENT_2,
    EVENT_3,
    EVENT_END = EVENT_3,
    MAX_EVENTS
};

typedef enum async_status_t
{
    ASYNC_STATUS_UNINITIALIZED = 0,
    ASYNC_STATUS_RUNNING,
    ASYNC_STATUS_EXIT,
} async_status_t;

struct context_t
{
    int            event;
    uint64_t       period;
    uint64_t       t_next;
    async_status_t status;
    struct pt      c_io, c_events[MAX_EVENTS];
    coroutine_t    cor_io, coroutines[MAX_EVENTS];
};

static struct context_t ctx = { 0 };

void
async_init(coroutine_t io, int n, ...)
{
    memset(&ctx, 0, sizeof(struct context_t));
    ctx.cor_io = io;

    va_list args;
    va_start(args, n);
    for (int i = EVENT_BEGIN; i < n + EVENT_BEGIN; i++)
    {
        ctx.coroutines[i] = va_arg(args, coroutine_t);
    }
    va_end(args);
}

void
async_periodic(uint64_t period, coroutine_t co)
{
    ctx.period = period;
    ctx.t_next = 0;
    ctx.coroutines[EVENT_TIMER] = co;
}

uint64_t get_time()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000000000 + ts.tv_nsec;
}

void
async_spin_once()
{
    uint64_t t = get_time();

    if (ctx.period > 0 && t >= ctx.t_next)
    {
        ctx.t_next = t + ctx.period;
        ctx.coroutines[EVENT_TIMER](&ctx.c_events[EVENT_TIMER]);
    }
    ctx.cor_io(&ctx.c_io);

    if (ctx.event == EVENT_NONE)
    {
        return;
    }

    coroutine_t co = ctx.coroutines[ctx.event];
    printf("event %d coroutine %p\n", ctx.event - EVENT_BEGIN + 1, co);
    if (co != NULL)
    {
        co(&ctx.c_events[ctx.event]);
    }
}

void
async_spin()
{
    while (ctx.status != ASYNC_STATUS_EXIT)
    {
        async_spin_once();
        nanosleep(&(struct timespec){ .tv_nsec = 1000 }, NULL);
    }
}

async_begin(poll)
{
    atomic_int e = atomic_load(&event);
    ctx.event    = e;
    atomic_store(&event, EVENT_NONE);
    pt_yield(pt);
}
async_end();

static bool has_events = false;

async_begin(timeout)
{
    if (!has_events)
    {
        printf("no events received in during the last 1 second\n");
    }
    else
    {
        has_events = false;
    }
    pt_yield(pt);
}
async_end();

async_begin(event_1)
{
    printf("handle event 1\n");
    has_events = true;
    pt_yield(pt);
}
async_end();

async_begin(event_2)
{
    printf("handle event 2\n");
    has_events = true;
    pt_yield(pt);
}
async_end();

async_begin(event_3)
{
    printf("handle event 3\n");
    has_events = true;
    pt_yield(pt);
}
async_end();


void keyboard_listener()
{
    struct termios term;
    tcgetattr(0, &term);
    term.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(0, TCSANOW, &term);

    char c = 0;
    while (c != 'q')
    {
        c = getchar();
        int x = c - '1' + EVENT_BEGIN;
        if (x <= EVENT_END && x >= EVENT_BEGIN)
        {
            printf("send: %d\n", x - EVENT_BEGIN + 1);
            atomic_store(&event, x);
        }
        else
        {
            printf("invalid event %d\n", x);
        }
    }
}

int
main(int argc, char** argv)
{
    async_init(async(poll), 3,
               async(event_1),
               async(event_2),
               async(event_3));
    async_periodic(1000000000, async(timeout));

    pthread_t tid;
    pthread_create(&tid, NULL, (void*) keyboard_listener, NULL);
    async_spin();

    pthread_join(tid, NULL);

    return 0;
}
