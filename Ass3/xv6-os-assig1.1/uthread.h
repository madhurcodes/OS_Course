#ifndef UTHREAD_HEADER
#define UTHREAD_HEADER

#include "types.h"
#include "stat.h"
#include "user.h"

/* Possible states of a thread; */
#define FREE        0x0
#define RUNNING     0x1
#define RUNNABLE    0x2
#define WAITING     0x3

#define STACK_SIZE  8192
#define MAX_THREAD  8

#define COUNTER_LIM       30
#define PRIORITY_LIM      10  
#define PRIORITY_SCHEDULE 1

#define PRIORITY_INVERSION 1
#define BUSY_WAITING 1

#define NULL 0

typedef struct thread thread_t, *thread_p;
typedef struct spinlock_s spinlock;

struct thread {
  int        sp;                /* curent stack pointer */
  char stack[STACK_SIZE];       /* the thread's stack */
  int        state;             /* running, runnable, waiting */
  char *thr_name;
  int priority;                 /* Thresholded at a Limit */
  int counter;                  /* counter indicating number of misses the process undergoes */
  int recieved_priority[MAX_THREAD]; // 
  thread_p donating_thread[MAX_THREAD];
  spinlock *donating_thread_waiting_lock[MAX_THREAD];

  };

extern thread_t all_thread[MAX_THREAD];
extern thread_p  current_thread;
extern thread_p  next_thread;


struct spinlock_s {
  uint locked;       // Is the lock held?
  char *lock_name;        // Name of lock.
  thread_t *thr;   // The thread holding the lock.
  thread_p waiting_threads[MAX_THREAD];
};



extern void thread_switch(void);
static void thread_yield(void);
static void  thread_schedule_priority(void);
static void  thread_schedule_normal(void);
static void lock_non_busy_wait_acquire(spinlock *lk);
static void lock_non_busy_wait_acquire_donation(spinlock *lk);
static void lock_busy_wait_acquire(spinlock *lk);


static void (*lock_acquire)(spinlock *lk);
static void (*thread_schedule)(void);
static void (*test)(void);


#endif