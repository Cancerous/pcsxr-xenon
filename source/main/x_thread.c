#include <xenon_soc/xenon_power.h>
#include <ppc/atomic.h>
#include <xetypes.h>
#include <ppc/register.h>
#include <ppc/xenonsprs.h>

/**
 * States =>
 * 0 => Nothing on thread
 * 1 => Thread Running
 */
typedef struct thread_s{
    unsigned int lock  __attribute__ ((aligned (128)));
    unsigned int states;
    int (*func)(void);
}thread_t;

static unsigned char thread_stack[6][0x10000];
static thread_t thread_states[6];

int xenon_run_thread_task(int thread, void *stack, void *task);

void * taskrunner(){
    int thread_id = mfspr(pir); // PIR
    thread_t * thread = &thread_states[thread_id];
    
    // Set thread states to running
    lock(&thread->lock);
    thread->states = 1;
    unlock(&thread->lock);
    
    // Run function
    if(thread->func)
        thread->func();
    
    // Set thread states to stopped
    lock(&thread->lock);
    thread->states = 0;
    unlock(&thread->lock);
    
    return NULL;
}


int x_thread_wait(int thread){
    int wait = 1;
    while(wait){
        lock(&thread_states[thread].lock);
        if(thread_states[thread].states==0){
            wait = 0;
        }
        unlock(&thread_states[thread].lock);
    }
    return 0;
}

int x_thread_cancel(int thread){
    
    // put thread to sleep
    xenon_sleep_thread(thread);
    
    // release states on thread
    lock(&thread_states[thread].lock);
    thread_states[thread].states = 0;
    thread_states[thread].func = NULL;
    unlock(&thread_states[thread].lock);
    
    return 0;
}

int x_thread_create(int thread,void *task){
    thread_states[thread].func = task;
    thread_states[thread].states = 0;

    return xenon_run_thread_task(thread,&thread_stack[thread][sizeof(thread_stack[thread])-0x100],taskrunner);
}