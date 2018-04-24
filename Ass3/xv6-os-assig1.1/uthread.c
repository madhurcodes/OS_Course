#include "uthread.h"

thread_t all_thread[MAX_THREAD];
thread_p  current_thread;
thread_p  next_thread;

int getLength(char *a)
{
  int len=0;
  while(a[len]!='\0')
    len++;
  return len;
}

void init_lock(spinlock *lk, char *lock_name){

    char * temp = (char *) malloc(getLength(lock_name)+1);
    int  k = 0;
    while(lock_name[k]!='\0'){
      temp[k] = lock_name[k];
      k++;
    }
    temp[k] = lock_name[k];

    lk->lock_name = temp;
    lk->locked = 0;
    lk->thr = 0;
    for(int i=0;i<MAX_THREAD;i++)
      lk->waiting_threads[i]=NULL;
}

void lock_busy_wait_acquire(spinlock *lk){

  while((lk->locked) && (lk->thr!=current_thread))
  {
    // if((lk->locked) && (lk->thr!=current_thread)){
      thread_yield();    
  }

  lk->locked = 1;
  lk->thr = current_thread;
}

void lock_non_busy_wait_acquire(spinlock *lk){
  int pos=-1;

  while((lk->locked) && (lk->thr!=current_thread))
  {
      current_thread->state = WAITING;
      if(pos==-1)
      {
        for(int i=0;i<MAX_THREAD;i++)
        {
          if(lk->waiting_threads[i]==NULL)
          {
            pos=i;
            break;
          }
        }
        lk->waiting_threads[pos]=current_thread;
      }
      thread_yield();    
  }

  lk->locked = 1;
  for(int i=0;i<MAX_THREAD;i++)
  {
    if(lk->waiting_threads[i]==current_thread)
      lk->waiting_threads[i]=NULL;
  }
  lk->thr = current_thread;

}

void lock_non_busy_wait_acquire_donation(spinlock *lk){
  int pos=-1;
  int i;
  while((lk->locked) && (lk->thr!=current_thread))
  {
      current_thread->state = WAITING;
      if(pos==-1)
      {
        for(i=0;i<MAX_THREAD;i++)
        {
          if(lk->waiting_threads[i]==NULL)
          {
            pos=i;
            break;
          }
        }
        lk->waiting_threads[pos]=current_thread;
      }

      int to_donate=current_thread->priority;
      int can_receive=(PRIORITY_LIM)-(lk->thr)->priority;
      int final_donation=0;
      if(to_donate>can_receive)
        final_donation=can_receive;
      else
        final_donation=to_donate;
      
      if(final_donation!=0)
      {
        current_thread->priority-=final_donation;
        lk->thr->priority += final_donation;
        for(i=0;i<MAX_THREAD;i++)
        {
          if(lk->thr->donating_thread[i]==NULL)
          {
            lk->thr->donating_thread[i]=current_thread;
            lk->thr->recieved_priority[i]=final_donation;
          }
        }
        
      }

      thread_yield();
  }

  lk->locked = 1;
  for(int i=0;i<MAX_THREAD;i++)
  {
    if(lk->waiting_threads[i]==current_thread)
      lk->waiting_threads[i]=NULL;
  }
  lk->thr = current_thread;

}

void lock_release(spinlock *lk){
  if((lk->locked) && (lk->thr==current_thread)){
    lk->locked = 0;
    int i = 0;
    for(i=0;i<MAX_THREAD;i++)
    {
      if(lk->waiting_threads[i]==NULL)
        continue;
      (lk->waiting_threads[i])->state=RUNNABLE;
      lk->waiting_threads[i]=NULL;
    }
    // thread
    for(i = 0;i<MAX_THREAD;i++){
      int p = 0;
      if(current_thread->donating_thread[i] != NULL){
        p = current_thread->recieved_priority[i];
        current_thread->priority -= p;
        current_thread->donating_thread[i]->priority += p; 
        current_thread->donating_thread[i] = NULL;
        current_thread->recieved_priority[i] = 0;
      }
    }
    lk->thr = 0; // okay?
  }
}



void 
thread_init(void)
{
  current_thread = &all_thread[0];
  current_thread->state = RUNNING;
}

static void clear_one_waiting_thread(void){
  thread_p t;
  for (t = all_thread; t < all_thread + MAX_THREAD; t++) {
    if (t->state == WAITING && t != current_thread) {
      t->state = RUNNABLE;
      break;
    }
  }
}

/*Default Thread Scheduler*/
/****************************************************************/
static void 
thread_schedule_normal(void)
{
  thread_p t;
  /* Find another runnable thread. */
  clear_one_waiting_thread();

  for (t = all_thread; t < all_thread + MAX_THREAD; t++) {
    if (t->state == RUNNABLE && t != current_thread) {
      next_thread = t;
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
    
    // printf(2, "Starting Switch\n");
    thread_switch();
    // printf(2, "Exiting Switch\n");    
  } 
  else
  {
    next_thread = 0;
  }
}

/* Aging */
/****************************************************************/
static void
AGING(void)
{
  thread_p temp;
  for(temp=all_thread;temp<all_thread+MAX_THREAD;temp++)
  {
    if(temp->state==RUNNABLE && temp!=next_thread)
    {
      temp->counter=temp->counter+1;
      if(temp->counter==(COUNTER_LIM))
      {
        temp->counter=0;
        if(temp->priority<(PRIORITY_LIM))
          temp->priority=temp->priority+1;
      }
    }
  }
}

/* Priority Based Thread Scheduler */
/****************************************************************/
static void 
thread_schedule_priority(void)
{
  thread_p t=current_thread;
  /* It was declared static which helps ensure RoundRobin nature in cases of ties */ 
 
  clear_one_waiting_thread();
  t++;

  thread_p temp=NULL;

  /* Find runnable thread with Maximum Priority. */
  for (; t < all_thread + MAX_THREAD; t++) {
    if (t->state == RUNNABLE) {
      if(temp==NULL){
        temp=t;
      }
      else if(t->priority > temp->priority){
        temp=t;
      }
    }
  }

  for (t=all_thread;t<=current_thread;t++)
  {
    if (t->state == RUNNABLE) {
      if(temp==NULL){
        temp=t;
      }
      else if(t->priority > temp->priority){
        temp=t;
      }
    }
  }

  /*Move loop pointer to next thread*/
  // t=temp;


  if(temp!=NULL)
  {
    next_thread=temp;
    next_thread->state=RUNNING;
    next_thread->counter = 0;

    AGING();
    thread_switch();
  }

  else
  {
    printf(2, "thread_schedule: no runnable threads; deadlock\n");
    exit();    
  }
}


int
thread_create(char *thr_name, void (*func)(),int priority)
{
  thread_p t;

  char * temp = (char *) malloc(getLength(thr_name)+1);
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
  t->priority = priority;
  t->thr_name = temp;
  t->counter = 0;
  int i;
  for(i=0;i<MAX_THREAD;i++){
    t->recieved_priority[i] = 0;
    t->donating_thread[i] = NULL;
  }
  return 1;
}

void 
thread_yield(void)
{
  current_thread->state = RUNNABLE;
  printf(1, "Yielded \n");

  (*thread_schedule)();
}

/* Testing */
/***********************************************************/
int shared = 0;
spinlock myl;

static void 
test_part_one_internals(void)
{
  int i;
  printf(1, "my thread running\n");
  for (i = 0; i < 100; i++) {
    printf(1, "my thread 0x%x, name is %s\n", (int) current_thread, current_thread->thr_name);
    thread_yield();
  }
  printf(1, "my thread: exit\n");
  current_thread->state = FREE;
  (*thread_schedule)();
}

static void 
test_part_one(void){
  thread_create("fff",test_part_one_internals,1);
  thread_create("fun",test_part_one_internals,1);
  thread_create("donk",test_part_one_internals,1);  
  (*thread_schedule)();
}


static void 
test_part_two_internals(void){
  (*lock_acquire)(&myl);
  int i;
  for (i = 0; i < 10; i++) {
    printf(1, "my thread 0x%x, name is %s, and s is %d\n", (int) current_thread, current_thread->thr_name,shared);
    printf(1,"%s is holding lock\n",myl.thr->thr_name);
    shared++;
    thread_yield();
  }
  lock_release(&myl);
  printf(1, "my thread: exit\n");
  current_thread->state = FREE;
  (*thread_schedule)();
}
static void 
test_part_two(void){
  init_lock(&myl,"l11");
  thread_create("fff",test_part_two_internals,1);
  thread_create("fun",test_part_two_internals,1);
  thread_create("donk",test_part_two_internals,10);  
  (*thread_schedule)();
}

static void 
test_starvation_and_aging_internals(void){
    int i =0;
    for(i=0;i<90;i++){
      printf(1, "my thread name is %s, and priority is %d\n", current_thread->thr_name,current_thread->priority);
      thread_yield();
    }
      printf(1, "my thread: exit\n");
  current_thread->state = FREE;
  (*thread_schedule)();
}

static void 
test_starvation_and_aging(void){
  thread_create("low_prior",test_starvation_and_aging_internals,2);
  thread_create("high_priority",test_starvation_and_aging_internals,4);  
  (*thread_schedule)();
}

static void 
simple_prod(void){
  (*lock_acquire)(&myl);
  int i;
  for (i = 0; i < 10; i++) {
    printf(1, "my thread 0x%x, name is %s, and s is %d\n", (int) current_thread, current_thread->thr_name,shared);
    printf(1,"%s is holding lock\n",myl.thr->thr_name);
    shared*=2;
    thread_yield();
  }
  lock_release(&myl);
  printf(1, "my thread: exit\n");
  current_thread->state = FREE;
  (*thread_schedule)();
}

static void 
simple_sum(void){
  (*lock_acquire)(&myl);
  thread_create("high_prior",simple_prod,5);  
  int i;
  for (i = 0; i < 10; i++) {
    printf(1, "my thread 0x%x, name is %s, and s is %d\n", (int) current_thread, current_thread->thr_name,shared);
    printf(1,"%s is holding lock\n",myl.thr->thr_name);
    shared++;
    thread_yield();
  }
  lock_release(&myl);
  printf(1, "my thread: exit\n");
  current_thread->state = FREE;
  (*thread_schedule)();
}

static void 
test_priority_inv_problem(void){
  init_lock(&myl,"l11");
  thread_create("low_prior",simple_sum,2);
  (*thread_schedule)();
}

int 
main(int argc, char *argv[]) 
{
  if (PRIORITY_SCHEDULE){
    thread_schedule = &thread_schedule_priority;
  }
  else{
    thread_schedule = &thread_schedule_normal;
  }
  if(BUSY_WAITING){
    lock_acquire = &lock_busy_wait_acquire;
  }
  else if(PRIORITY_INVERSION){
    lock_acquire = &lock_non_busy_wait_acquire_donation;    
  }
  else{
    lock_acquire = &lock_non_busy_wait_acquire;
  }
  test = &test_part_one;
  test = &test_part_two;
  test = &test_starvation_and_aging;
  test = &test_priority_inv_problem;

  thread_init();
  (*test)();
  printf(1, "Exiting Program\n");  
  exit();
}
