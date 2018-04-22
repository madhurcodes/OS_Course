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

typedef struct thread thread_t, *thread_p;
typedef struct mutex mutex_t, *mutex_p;

struct thread {
  int        sp;                /* curent stack pointer */
  char stack[STACK_SIZE];       /* the thread's stack */
  int        state;             /* running, runnable, waiting */
  char *thr_name;
  };

static thread_t all_thread[MAX_THREAD];
thread_p  current_thread;
thread_p  next_thread;
extern void thread_switch(void);

void 
thread_init(void)
{
  current_thread = &all_thread[0];
  current_thread->state = RUNNING;
}

static void 
thread_schedule(void)
{
  thread_p t;
  printf(2, "Entering Thread scheduler\n");
  /* Find another runnable thread. */
  for (t = all_thread; t < all_thread + MAX_THREAD; t++) {
    if (t->state == RUNNABLE && t != current_thread) {
      next_thread = t;
        // if (next_thread == 0) {
        // printf(2, "is 0 still\n");
        // }
      break;
    }
  }

  if (t >= all_thread + MAX_THREAD && current_thread->state == RUNNABLE) {
    /* The current thread is the only runnable thread; run it. */
    next_thread = current_thread;
    printf(2, "Current thread is the only runnable thread\n");
  }

  if (next_thread == 0) {
    printf(2, "thread_schedule: no runnable threads; deadlock\n");
    exit();
  }

  if (current_thread != next_thread) {         /* switch threads?  */
    next_thread->state = RUNNING;

    // current_thread->state = RUNNABLE;
    printf(2, "Starting Switch\n");
    thread_switch();
    printf(2, "Exiting Switch\n");    
  } 
  else
  {
    next_thread = 0;
  }
  printf(2, "Exiting Thread scheduler\n");
}

int
thread_create( const char *thr_name, void (*func)())
{
  thread_p t;

  char * temp = (char *) malloc(100);
  // Allocate some space for thread name in heap and store pointer to it in thread
  int  k = 0;
  while(thr_name[k]!='\0'){
    temp[k] = thr_name[k];
    k++;
  }
  temp[k] = thr_name[k];

  int is_free = 0;
  for (t = all_thread; t < all_thread + MAX_THREAD; t++) {
    if (t->state == FREE){ 
      is_free = 1;
      break;
    }
  }
  if(!is_free){
    return 0;
  }

  t->sp = (int) (t->stack + STACK_SIZE);   // set sp to the top of the stack
  t->sp -= 4;                              // space for return address
  * (int *) (t->sp) = (int)func;           // push return address on stack
  t->sp -= 32;                             // space for registers that thread_switch will push
  t->state = RUNNABLE;

  t->thr_name = temp;
  return 1;
}

void 
thread_yield(void)
{
  current_thread->state = RUNNABLE;
  printf(1, "Yielded \n");
  thread_schedule();
}

static void 
mythread(void)
{
  int i;
  printf(1, "my thread running\n");
  for (i = 0; i < 100; i++) {
    printf(1, "my thread 0x%x\n", (int) current_thread);
    thread_yield();
  }
  printf(1, "my thread: exit\n");
  current_thread->state = FREE;
  thread_schedule();
}


int 
main(int argc, char *argv[]) 
{
  thread_init();
  
  thread_create("go",mythread);
  thread_create("d",mythread);
  thread_create("donk",mythread);  
  thread_schedule();
  printf(1, "Exiting Program\n");  
  exit();
}
