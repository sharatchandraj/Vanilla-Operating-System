/*
     File        : file.C

     Author      : Riccardo Bettati
     Modified    : 2017/05/01

     Description : Implementation of simple File class, with support for
                   sequential read/write operations.
*/

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "assert.H"
#include "console.H"
#include "file.H"

/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR */
/*--------------------------------------------------------------------------*/

File::File(Inode* _inode) {
    /* We will need some arguments for the constructor, maybe pointer to disk
     block with file management and allocation data. */
    Console::puts("In file constructor.\n");


    inode = (Inode*) new Inode();

    memcpy((unsigned char*)inode,(unsigned char*)_inode,sizeof(Inode));

    file_block_index = 0;

    //Console::puts("End of file constructor.\n");

}

/*--------------------------------------------------------------------------*/
/* FILE FUNCTIONS */
/*--------------------------------------------------------------------------*/

int File::Read(unsigned int _n, char * _buf) {
    Console::puts("reading from file\n");


    unsigned int  chars_read = 0;
    char data_buffer[512];
    memset(data_buffer,0,512);

    while(!EoF() && chars_read < _n)
    {
        FILE_SYSTEM->disk->read(inode->blocks[file_block_index++],(unsigned char*)data_buffer);

        int remaining =_n-chars_read;

        if(remaining>512)
        {
            memcpy(_buf+chars_read,data_buffer,512);
            chars_read +=  512;
        }
        else
        {
            memcpy(_buf+chars_read,(char*)data_buffer,remaining);
            chars_read += remaining;
        }
    }

    Console::puts("No. of chars read: ");Console::puti(chars_read);Console::puts("\n");
    return chars_read;
}


void File::Write(unsigned int _n, const char * _buf) {
    Console::puts("writing to file\n");


    unsigned int chars_written = 0;

    while(chars_written<_n)
    {
        int new_block = FILE_SYSTEM->get_empty_block();

        FILE_SYSTEM->mark_used(new_block);

        inode->blocks[inode->size++] = new_block;

        char data_buffer[512];
        memset(data_buffer,0,512);

        int remaining = _n-chars_written;

        if(remaining > 512)
        {
            memcpy(data_buffer,_buf,512);
            FILE_SYSTEM->disk->write(new_block,(unsigned char*)data_buffer);
            chars_written += 512;
        }
        else
        {
            memcpy(data_buffer,_buf,remaining);
            FILE_SYSTEM->disk->write(new_block,(unsigned char*)data_buffer);
            chars_written += remaining;
        }
        file_block_index++;
    }

    FILE_SYSTEM->disk->write(inode->unique_id,(unsigned char*)inode);

    //Console::puts("Finished writing to file\n");
}

void File::Reset() {
    Console::puts("reset current position in file\n");

    file_block_index = 0;

}

void File::Rewrite() {
    Console::puts("erase content of file\n");

    file_block_index = 0;

    for(int i=0;i<inode->size;i++)
    {
        FILE_SYSTEM->mark_free(inode->blocks[i]);
    }

    inode->size = 0;
}


bool File::EoF() {
    Console::puts("testing end-of-file condition\n");

    return (inode->size == 0 || ( inode->size < file_block_index));
}
