/*
 File: vm_pool.C

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

#include "vm_pool.H"
#include "console.H"
#include "utils.H"
#include "assert.H"
#include "simple_keyboard.H"

#include "page_table.H"

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
/* METHODS FOR CLASS   V M P o o l */
/*--------------------------------------------------------------------------*/

VMPool::VMPool(unsigned long  _base_address,
               unsigned long  _size,
               ContFramePool *_frame_pool,
               PageTable     *_page_table) {

    base_address=_base_address;
    size=_size;
    frame_pool = _frame_pool;
    page_table=_page_table;

    last_mem_region = 0;

    mem_region_list = (mem_region *)base_address; //pointing the memory region list to the base_address of the virtual memory pool

    page_table->register_pool(this);



    Console::puts("Constructed VMPool object.\n");
}

unsigned long VMPool::allocate(unsigned long _size) {

    unsigned long size = _size/Machine::PAGE_SIZE;
    unsigned long return_address;
    if ((_size%Machine::PAGE_SIZE) > 0){
        size += 1;
    }

    if(last_mem_region==0){
        mem_region_list[0].region_base_address=base_address+Machine::PAGE_SIZE;
        mem_region_list[0].region_size = size;

        return_address = mem_region_list[0].region_base_address;
    }else{
        mem_region_list[last_mem_region].region_base_address = mem_region_list[last_mem_region-1].region_base_address + mem_region_list[last_mem_region-1].region_size;
        mem_region_list[last_mem_region].region_size=size;

        mem_region_list[0].region_base_address = mem_region_list[last_mem_region].region_base_address;
    }

    last_mem_region++;

    Console::puts("Allocated region of memory.\n");

    return return_address;
}

void VMPool::release(unsigned long _start_address) {

    int pos;
    bool found=false;

    //traverse the mem_region_list to look for the right region to free
    for(pos=0;pos<last_mem_region;pos++){
        if(mem_region_list[pos].region_base_address == _start_address){
            found=true;
            break;
        }
    }

    if(!found){
        Console::puts("Region cannot be released because start address is not in this region\n");
        return;
    }

    //compute number of pages to be freed
    int num = mem_region_list[pos].region_size/Machine::PAGE_SIZE;

    for(int i=0;i<num;i++){
        page_table->free_page(_start_address);
        _start_address+=Machine::PAGE_SIZE; //increment address of the page
    }

    //shift elements in the list to the left by 1 region index
    for(int i= pos;i<last_mem_region-1;i++){
        mem_region_list[i]=mem_region_list[i+1];
    }

    last_mem_region--;

    page_table->load(); //TLB flush

    Console::puts("Released region of memory.\n");
}

bool VMPool::is_legitimate(unsigned long _address) {

    if((_address>=base_address)&&(_address<=(base_address+size)))   return true;
    else return false;
}

