/*
     File        : blocking_disk.c

     Author      :
     Modified    :

     Description :

*/

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "assert.H"
#include "utils.H"
#include "console.H"
#include "blocking_disk.H"
#include "scheduler.H"
#include "thread.H"

extern Scheduler * SYSTEM_SCHEDULER;
/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR */
/*--------------------------------------------------------------------------*/

BlockingDisk::BlockingDisk(DISK_ID _disk_id, unsigned int _size)
  : SimpleDisk(_disk_id, _size) {

    bt_q_size = 0;
    this->bt_queue =  new Queue();
}

/*--------------------------------------------------------------------------*/
/* SIMPLE_DISK FUNCTIONS */
/*--------------------------------------------------------------------------*/


void BlockingDisk::wait_until_ready() {

    if (!SimpleDisk::is_ready()) {
        Thread *current_thread = Thread::CurrentThread();
        this->disk_enqueue(current_thread);
        SYSTEM_SCHEDULER->yield();
    }

}


void BlockingDisk::disk_enqueue(Thread *thread) {
    this->bt_queue->enqueue(thread);
    bt_q_size++;
}

//Since is ready is protected in SimpleDisk
bool BlockingDisk::is_ready() {
    return SimpleDisk::is_ready();
}

void BlockingDisk::read(unsigned long _block_no, unsigned char * _buf) {

    SimpleDisk::read(_block_no, _buf);
    Console::puts("END OF READ CALL!!!!!!!!!!!!!!!!!! \n");
}


void BlockingDisk::write(unsigned long _block_no, unsigned char * _buf) {
    SimpleDisk::write(_block_no, _buf);
    Console::puts("END OF WRITE CALL!!!!!!!!!!!!!!!!!! \n");
}
