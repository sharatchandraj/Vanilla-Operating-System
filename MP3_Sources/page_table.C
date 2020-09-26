#include "assert.H"
#include "exceptions.H"
#include "console.H"
#include "paging_low.H"
#include "page_table.H"

PageTable * PageTable::current_page_table = NULL;
unsigned int PageTable::paging_enabled = 0;
ContFramePool * PageTable::kernel_mem_pool = NULL;
ContFramePool * PageTable::process_mem_pool = NULL;
unsigned long PageTable::shared_size = 0;



void PageTable::init_paging(ContFramePool * _kernel_mem_pool,
                            ContFramePool * _process_mem_pool,
                            const unsigned long _shared_size)
{
    kernel_mem_pool = _kernel_mem_pool;
    process_mem_pool = _process_mem_pool;
    shared_size = _shared_size;
    Console::puts("Initialized Paging System!\n");
}

PageTable::PageTable()
{
    page_directory = (unsigned long *)(kernel_mem_pool -> get_frames(1)*PAGE_SIZE);
    if(page_directory==0){
        Console::puts("Unable to allocate frame to page directory\n");
    }


    ////////////// DIRECTLY MAPPED REGION/////////////////////////////
    unsigned long *page_table = (unsigned long *)(kernel_mem_pool -> get_frames(1)*PAGE_SIZE);
    if(page_table==0){
        Console::puts("Unable to allocate frame to page table\n");
    }
    unsigned long mask =0;

    //Marking the enries as valid
    //there are 1024 entries per page
    for(int i=0;i<1024;i++){
        page_table[i]=mask|3; //Setting bit 0 and bit 1. Note : These pages are also marked as kernel level pages (bit 2 = 0)
        mask=mask+PAGE_SIZE; //moving onto the next page
    }

    ////////////////////////////////////////////////////////////////////

    //Pointing the page directory to the page table
    //Set the first entry of directory to the address of page table and mark the entry as valid
    page_directory[0] = (unsigned long)page_table|3;

    //Mark all the remaining entries as invalid
    mask = 0;
    for(int i=1;i<1024;i++){
        page_directory[i]=mask | 2; //Setting bit1
    }
    Console::puts("Constructed Page Table object!\n");
}


void PageTable::load()
{
    current_page_table = this;
    write_cr3((unsigned long)this->page_directory); // put that page directory address into CR3
    Console::puts("Loaded page table!\n");
}

void PageTable::enable_paging()
{
    paging_enabled = 1;
    write_cr0(read_cr0() | 0x80000000); // set the paging bit in CR0 to 1
    Console::puts("Enabled paging!\n");
}

void PageTable::handle_fault(REGS * _r)
{

   //Console::puts("handle_fault() called\n");

    unsigned int err_code = (_r->err_code);

    if((err_code&7)==7){ //checking the lowest 3 bits for a protection fault
        Console::puts("Protection fault\n");
    }

    unsigned long fault_address = (unsigned long)read_cr2();
    unsigned long *current_page_directory = current_page_table->page_directory;
    unsigned long pda = fault_address>>22; //index of entry in Page Directory
    unsigned long pta = (fault_address>>12)&0x3FF; //index of entry in Page Table

    unsigned long *page_table;

    if((err_code&1)==0){  //checking if the error code is because of an invalid entry

        if((current_page_directory[pda] & 1) == 0){  //checking if the page directory does not have the page table entry i.e fault in page directory

            current_page_directory[pda]=(unsigned long)(kernel_mem_pool->get_frames(1)*PAGE_SIZE)|3;
            page_table = (unsigned long *) (current_page_directory[pda]&0xFFFFF000); //page_table points to the first entry in the frame that has just been allocated as a page table


            for(int i=0;i<1024;i++){
                page_table[i]=4; //marking the page as a user level page
            }

            page_table[pta]=(unsigned long)(process_mem_pool->get_frames(1)*PAGE_SIZE)|3;
        }

        else { //fault is in page table
            page_table = (unsigned long *) (current_page_directory[pda]&0xFFFFF000);
            page_table[pta]=(unsigned long)(process_mem_pool->get_frames(1)*PAGE_SIZE)|3;
        }

    }

    Console::puts("handled page fault\n");
}

