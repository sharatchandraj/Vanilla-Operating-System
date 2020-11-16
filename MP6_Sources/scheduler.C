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


    q_size = 0;
    this->blocking_disk = NULL;
  Console::puts("Constructed Scheduler.\n");
}

void Scheduler::yield() {

    if(blocking_disk !=NULL && blocking_disk->is_ready() && blocking_disk->bt_q_size != 0) {
        Thread *disk_thread_tbe = blocking_disk->bt_queue->dequeue();
        blocking_disk->bt_q_size--;
        Thread::dispatch_to(disk_thread_tbe);
    } else {

        if (q_size == 0) {
            Console::puts("Queue is empty \n");
        } else {
            q_size--;
            Thread* new_thread = readyQ.dequeue();
            Thread::dispatch_to(new_thread);
        }
    }
}

void Scheduler::resume(Thread * _thread) {

    readyQ.enqueue(_thread);
    q_size++;

}

void Scheduler::add(Thread * _thread) {

    readyQ.enqueue(_thread);
    q_size++;
}

void Scheduler::terminate(Thread * _thread) {
    for (int i = 0; i < q_size; i++) {
        Thread * thread_tbe = readyQ.dequeue();

        if (_thread->ThreadId() == thread_tbe->ThreadId()) {
            q_size--;
        } else {
            readyQ.enqueue(thread_tbe);
        }
    }
}


void Scheduler::add_disk(BlockingDisk * disk) {
    blocking_disk = disk;
}


