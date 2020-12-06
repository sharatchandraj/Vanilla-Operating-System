/*
    File: file_system.H

    Author: R. Bettati
            Department of Computer Science
            Texas A&M University
    Date  : 10/04/05

    Description: Simple File System.


*/

#ifndef _FILE_SYSTEM_H_                   // include file only once
#define _FILE_SYSTEM_H_

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "file.H"
#include "simple_disk.H"

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */
/*--------------------------------------------------------------------------*/

struct Inode
{
    unsigned int unique_id;
    unsigned int size;
    unsigned int blocks[16];
};


struct inode_file
{
    int unique_id;
    int file_id;
};

/*--------------------------------------------------------------------------*/
/* FORWARD DECLARATIONS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* F i l e S y s t e m  */
/*--------------------------------------------------------------------------*/
class File;

class FileSystem {

friend class File; /* -- not sure if we need this; feel free to delete */

private:
     /* -- DEFINE YOUR FILE SYSTEM DATA STRUCTURES HERE. */

     SimpleDisk * disk;
     static unsigned int size;

     inode_file inode_file_maps[10];
     int inode_file_maps_index;

     unsigned int max_block_count;
     unsigned char block_bit_map[512];


     int file_count;

public:

    FileSystem();
    /* Just initializes local data structures. Does not connect to disk yet. */

    bool Mount(SimpleDisk * _disk);
    /* Associates this file system with a disk. Limit to at most one file system per disk.
     Returns true if operation successful (i.e. there is indeed a file system on the disk.) */

    static bool Format(SimpleDisk * _disk, unsigned int _size);
    /* Wipes any file system from the disk and installs an empty file system of given size. */

    File * LookupFile(int _file_id);
    /* Find file with given id in file system. If found, return the initialized
     file object. Otherwise, return null. */

    bool CreateFile(int _file_id);
    /* Create file with given id in the file system. If file exists already,
     abort and return false. Otherwise, return true. */

    bool DeleteFile(int _file_id);
    /* Delete file with given id in the file system; free any disk block occupied by the file. */


    void mark_used(unsigned int block_num);

    void mark_free(unsigned int block_num);

    int get_empty_block();

};
#endif
