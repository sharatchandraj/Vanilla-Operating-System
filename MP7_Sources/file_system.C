/*
     File        : file_system.C

     Author      : Riccardo Bettati
     Modified    : 2017/05/01

     Description : Implementation of simple File System class.
                   Has support for numerical file identifiers.
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
#include "file_system.H"


/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR */
/*--------------------------------------------------------------------------*/

unsigned int FileSystem::size;

FileSystem::FileSystem()
{
    Console::puts("In file system constructor.\n");
    disk = NULL;
    max_block_count=0;
}

/*--------------------------------------------------------------------------*/
/* FILE SYSTEM FUNCTIONS */
/*--------------------------------------------------------------------------*/

bool FileSystem::Mount(SimpleDisk * _disk)
{
    Console::puts("mounting file system form disk\n");
    disk = _disk;
    max_block_count = (size/512);

    //mark all the blocks
    for(int i=0; i<10; i++)
    {
        inode_file_maps[i].file_id = -1;
        inode_file_maps[i].unique_id = -1;
    }

    file_count = 0;
    inode_file_maps_index = -1;
    memset(block_bit_map,0,512);
    return true;
}

bool FileSystem::Format(SimpleDisk * _disk, unsigned int _size)
{
    Console::puts("formatting disk\n");

    char data_buffer[512];
    memset(data_buffer,0,512);

    for(int i=0; i<512; i++)
    {
        _disk->write(i,(unsigned char*)data_buffer);
    }

    FileSystem::size = _size;

    return true;
}

File * FileSystem::LookupFile(int _file_id)
{
    Console::puts("looking up file\n");


    for(int i=0; i<10; i++)
    {
        if(inode_file_maps[i].file_id == _file_id)
        {
            int block_num = inode_file_maps[i].unique_id;

            unsigned char data_buffer[512];
            disk->read(block_num,data_buffer);

            File* file = (File*) new File((Inode*)data_buffer);

            return file;
        }
    }
    return NULL;
}

bool FileSystem::CreateFile(int _file_id)
{
    Console::puts("creating file\n");

    if(LookupFile(_file_id)!=NULL)
        return false;

    int new_block = get_empty_block();
    unsigned char temp[512];

    Inode* new_inode = (Inode*)temp;
    new_inode->unique_id = new_block;
    new_inode->size = 0;

    mark_used(new_block);

    disk->write(new_block,temp);

    inode_file_maps_index = (++inode_file_maps_index)%10;
    inode_file_maps[inode_file_maps_index].file_id = _file_id;
    inode_file_maps[inode_file_maps_index].unique_id = new_block;
    file_count++;

    Console::puts("Created File!! \n");

    return true;
}

bool FileSystem::DeleteFile(int _file_id)
{
    Console::puts("deleting file\n");

    File* f = LookupFile(_file_id);

    if(f==NULL)
        return false;


    int block_num = f->inode->unique_id;

    for(int i=0; i<f->inode->size; i++)
    {
        mark_free(f->inode->blocks[i]);
    }

    mark_free(block_num);

    for(int i=0; i<10; i++)
    {
        if(inode_file_maps[i].file_id == _file_id)
        {
            inode_file_maps[i].file_id = -1;
            inode_file_maps[i].unique_id = -1;
        }
    }

    file_count--;

    if(file_count==0)
        inode_file_maps_index =-1;

    Console::puts("Deleted File!! \n");

    return true;
}

void FileSystem::mark_free(unsigned int block_num)
{
    block_bit_map[block_num/8] &= ~(1<<(block_num%8));
}

void FileSystem::mark_used(unsigned int block_num)
{
    block_bit_map[block_num/8] |= (1<<(block_num%8));
}

int FileSystem::get_empty_block()
{
    for(int index=0; index<max_block_count/8; index++)
    {
        for(int bit_no=0; bit_no<8; bit_no++)
        {
            if((block_bit_map[index] & (1<<bit_no)) == 0)
            {
                return (index*8+bit_no);
            }
        }
    }

    return -1;
}
