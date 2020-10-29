/*
 File: scheduler.C

 Author:
 Date  :

 */

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "scheduler.H"
#include "thread.H"
#include "console.H"
#include "utils.H"
#include "assert.H"
#include "simple_keyboard.H"

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* CONSTANTS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* FORWARDS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* METHODS FOR CLASS   S c h e d u l e r  */
/*--------------------------------------------------------------------------*/

Scheduler::Scheduler() {


    head = NULL;
    tail = NULL;

  Console::puts("Constructed Scheduler.\n");
}

void Scheduler::yield() {

    Machine::disable_interrupts();

    if(head!=NULL){
        readyQ *thread_TBR = head; //thread To Be Run

        if(tail == thread_TBR)  //only one thread in queue
            tail = NULL;

        Thread *thread_TCB = thread_TBR->TCB;

        head=head->next;

        delete thread_TBR;

        Thread::dispatch_to(thread_TCB);

    }

    Machine::enable_interrupts();

}

void Scheduler::resume(Thread * _thread) {

    Machine::disable_interrupts();

    readyQ *thread = new readyQ;

    thread->TCB = _thread;

    thread->next = NULL;

    if(head!=NULL){
        tail->next=thread;
        tail=tail->next;
    } else{
        head = thread;
        tail = thread;
    }

    Machine::enable_interrupts();

}

void Scheduler::add(Thread * _thread) {

    resume(_thread);

}

void Scheduler::terminate(Thread * _thread) {

    readyQ *thread_TBR = head; // thread To Be Run
    readyQ *thread_CE = new readyQ; //thread Currently Executing


    thread_CE->TCB = NULL;
    thread_CE->next = thread_TBR;


    while(thread_TBR!=NULL){ //loop to traverse queue
        if(thread_TBR->TCB == _thread){ //thread found
            thread_CE->next = thread_TBR->next;
            delete thread_TBR;

            Machine::disable_interrupts();

            thread_TBR = thread_CE->next;
            continue;
        }
        thread_CE = thread_TBR;
        thread_TBR = thread_TBR->next;
    }


    Machine::enable_interrupts();
}
