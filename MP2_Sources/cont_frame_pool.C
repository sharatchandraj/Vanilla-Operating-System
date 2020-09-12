/*
 File: ContFramePool.C
 
 Author: Sharat Chandra Janapareddy	
 Date  : 8/6/20
 
 */

/*--------------------------------------------------------------------------*/
/* 
 POSSIBLE IMPLEMENTATION
 -----------------------

 The class SimpleFramePool in file "simple_frame_pool.H/C" describes an
 incomplete vanilla implementation of a frame pool that allocates 
 *single* frames at a time. Because it does allocate one frame at a time, 
 it does not guarantee that a sequence of frames is allocated contiguously.
 This can cause problems.
 
 The class ContFramePool has the ability to allocate either single frames,
 or sequences of contiguous frames. This affects how we manage the
 free frames. In SimpleFramePool it is sufficient to maintain the free 
 frames.
 In ContFramePool we need to maintain free *sequences* of frames.
 
 This can be done in many ways, ranging from extensions to bitmaps to 
 free-lists of frames etc.
 
 IMPLEMENTATION:
 
 One simple way to manage sequences of free frames is to add a minor
 extension to the bitmap idea of SimpleFramePool: Instead of maintaining
 whether a frame is FREE or ALLOCATED, which requires one bit per frame, 
 we maintain whether the frame is FREE, or ALLOCATED, or HEAD-OF-SEQUENCE.
 The meaning of FREE is the same as in SimpleFramePool. 
 If a frame is marked as HEAD-OF-SEQUENCE, this means that it is allocated
 and that it is the first such frame in a sequence of frames. Allocated
 frames that are not first in a sequence are marked as ALLOCATED.
 
 NOTE: If we use this scheme to allocate only single frames, then all 
 frames are marked as either FREE or HEAD-OF-SEQUENCE.
 
 NOTE: In SimpleFramePool we needed only one bit to store the state of 
 each frame. Now we need two bits. In a first implementation you can choose
 to use one char per frame. This will allow you to check for a given status
 without having to do bit manipulations. Once you get this to work, 
 revisit the implementation and change it to using two bits. You will get 
 an efficiency penalty if you use one char (i.e., 8 bits) per frame when
 two bits do the trick.
 
 DETAILED IMPLEMENTATION:
 
 How can we use the HEAD-OF-SEQUENCE state to implement a contiguous
 allocator? Let's look a the individual functions:
 
 Constructor: Initialize all frames to FREE, except for any frames that you 
 need for the management of the frame pool, if any.
 
 get_frames(_n_frames): Traverse the "bitmap" of states and look for a 
 sequence of at least _n_frames entries that are FREE. If you find one, 
 mark the first one as HEAD-OF-SEQUENCE and the remaining _n_frames-1 as
 ALLOCATED.

 release_frames(_first_frame_no): Check whether the first frame is marked as
 HEAD-OF-SEQUENCE. If not, something went wrong. If it is, mark it as FREE.
 Traverse the subsequent frames until you reach one that is FREE or 
 HEAD-OF-SEQUENCE. Until then, mark the frames that you traverse as FREE.
 
 mark_inaccessible(_base_frame_no, _n_frames): This is no different than
 get_frames, without having to search for the free sequence. You tell the
 allocator exactly which frame to mark as HEAD-OF-SEQUENCE and how many
 frames after that to mark as ALLOCATED.
 
 needed_info_frames(_n_frames): This depends on how many bits you need 
 to store the state of each frame. If you use a char to represent the state
 of a frame, then you need one info frame for each FRAME_SIZE frames.
 
 A WORD ABOUT RELEASE_FRAMES():
 
 When we releae a frame, we only know its frame number. At the time
 of a frame's release, we don't know necessarily which pool it came
 from. Therefore, the function "release_frame" is static, i.e., 
 not associated with a particular frame pool.
 
 This problem is related to the lack of a so-called "placement delete" in
 C++. For a discussion of this see Stroustrup's FAQ:
 http://www.stroustrup.com/bs_faq2.html#placement-delete
 
 */
/*--------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "cont_frame_pool.H"
#include "console.H"
#include "utils.H"
#include "assert.H"

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
/* METHODS FOR CLASS   C o n t F r a m e P o o l */
/*--------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------*/
/* POINTERS FOR FRAME POOL LINKED-LIST */
/*--------------------------------------------------------------------------*/
ContFramePool *ContFramePool::head;
ContFramePool *ContFramePool::last;

ContFramePool::ContFramePool(unsigned long _base_frame_no,
                             unsigned long _n_frames,
                             unsigned long _info_frame_no,
                             unsigned long _n_info_frames)
{
     
	Console::puts("Initializing frame pool\n");
    // Bitmap must fit in a single frame! Since we are using two bits to hold the state of one frame. Max number of frames that can be managed by a pool is less than equal to 4K*4. 
	//As each byte can keep track of 4 frames
    //assert(_n_frames<=FRAME_SIZE*4); 

    base_frame_no = _base_frame_no;
    nframes = _n_frames;
    nFreeFrames = _n_frames;
    info_frame_no = _info_frame_no;

    // If _info_frame_no is zero then we keep management info in the first
    //frame, else we use the provided frame to keep management info
    if(info_frame_no == 0) {
        bitmap = (unsigned char *) (base_frame_no * FRAME_SIZE);
    } else {
        bitmap = (unsigned char *) (info_frame_no * FRAME_SIZE);
    }

    // Number of frames must be "fill" the bitmap!
    assert ((nframes % 8 ) == 0);

    // Everything ok. Proceed to mark all frames as free
    for(int i=0; i *8< _n_frames*2; i++) {
        bitmap[i] = 0x00;
    }

    // Mark the first frame as being used if it is being used
    if(_info_frame_no == 0) {
        bitmap[0] = 0x40;
        nFreeFrames--;
    }

    if(ContFramePool::head==NULL){
    	ContFramePool::head=this;
    	ContFramePool::last=this;
    }
    else{
    	ContFramePool::last->next = this;
    	ContFramePool::last=this;
    }

    next=NULL;

    Console::puts("Frame Pool initialized with ");Console::puti(nframes);Console::puts(" frames\n");
	Console::puts("Frame Pool has ");Console::puti(nFreeFrames);Console::puts(" free frames left\n");
}

unsigned long ContFramePool::get_frames(unsigned int _n_frames)
{

	Console::puts("get_frames(): Number of free frames before allocation: ");Console::puti(nFreeFrames);Console::puts("\n");
	int count=0;
	int byte_pos; //holds the index of byte with head-of-sequence 
	int pos_hos; //holds index of head-of-sequence within a byte
	bool flag_hos = false;
	bool flag_framesFound = false;
	unsigned int frame_no=base_frame_no;

	for(int i=0;i<nframes/4;i++){
		unsigned char mask = 0xC0;
		
		for(int j=0;j<4;j++){
			if((bitmap[i]&mask)==0){
				if(flag_hos){
					count++;
				}
				else{
					flag_hos=true;
					frame_no = frame_no + (i*4+j);
					byte_pos=i;
					pos_hos=j;
					count++;
				}
			}
			else{
				if(flag_hos){
					frame_no=base_frame_no;
					pos_hos=0;
					byte_pos=0;
					count=0;
					flag_hos=false;
				}
			}

			mask=mask>>2;

			if(count==_n_frames){
				flag_framesFound=true;
				break;
			}
		}
		if(count == _n_frames){
			flag_framesFound=true;
			break;
		}		
	}

	if(flag_framesFound ==false){
		Console::puts("No empty / free frames were found for length: ");
        Console::puti(_n_frames);
		Console::puts("\n");
		return 0;
	}

	//Mark the allocated frames
	
	//Mark Head-of-sequence
	unsigned char head_mask = 0x40; //Mask to set head-of-sequence
	unsigned char alloc_mask = 0xC0; //Mask to mark frames as allocated
	unsigned char filter_mask = 0xC0;
	count = _n_frames;
	head_mask = head_mask>>(pos_hos*2);
	filter_mask = filter_mask>>(pos_hos*2);
	bitmap[byte_pos] = (bitmap[byte_pos] & ~filter_mask)| head_mask;
	
	pos_hos++;
	count--;

	//Mark the frames after the head of sequence
	alloc_mask = alloc_mask>>(pos_hos*2);

	while(count>0 && pos_hos<4){
		bitmap[byte_pos] = bitmap[byte_pos] | alloc_mask;
		alloc_mask=alloc_mask>>2;
		pos_hos++;
		count--;
	}

	for(int i = byte_pos+1;i<nframes/4;i++){
		alloc_mask=0xC0;
		for(int j=0;j<4;j++){

			if(count==0) break;

			bitmap[i] = bitmap[i] | alloc_mask;
			alloc_mask=alloc_mask>>2;
			count--;
		}
		if(count==0) break;
	}
	
	
	//Reduce number of free frames
	if(flag_hos){
		nFreeFrames = nFreeFrames - _n_frames;
		return frame_no;
	}
	else{
		Console::puts("No empty / free frames were found for length: ");
        Console::puti(_n_frames);
		Console::puts("\n");
		return 0;
	}

}

void ContFramePool::mark_inaccessible(unsigned long _base_frame_no,
                                      unsigned long _n_frames)
{
	if(_base_frame_no<base_frame_no || base_frame_no + nframes < _base_frame_no + _n_frames){
		Console::puts("Frame index out of range");
		return;
	}

	int byte_pos = (_base_frame_no-base_frame_no)/4;
	int pos_firstFrame = (_base_frame_no-base_frame_no)%4;
	int count = _n_frames;

	//Mark the frames as inaccessible
	int index = pos_firstFrame;
	unsigned char inacc_mask = 0x80;
	inacc_mask = inacc_mask>>(pos_firstFrame*2);
	

	while(count>0 && index<4){
		bitmap[byte_pos] = bitmap[byte_pos] | inacc_mask;
		inacc_mask>>2;
		index++;
		count--;
	}

	for(int i = byte_pos+1;i<nframes/4;i++){
		inacc_mask=0x80;
		for(int j=0;j<4;j++){

			if(count==0) break;

			bitmap[i] = bitmap[i] | inacc_mask;
			inacc_mask>>2;
			count--;
		}
		if(count==0) break;
	}

	nFreeFrames = nFreeFrames -_n_frames;
	Console::puts("Number of free frames left after marking some as inaccessible ");Console::puti(nFreeFrames);Console::puts("\n");
}

void ContFramePool::release_frames(unsigned long _first_frame_no)
{
	Console::puts("Frame to be released ");Console::puti(_first_frame_no);Console::puts("\n");
    ContFramePool *current = ContFramePool::head;
	Console::puts("Number of free frames before realeasing frames ");Console::puti(current->nFreeFrames);Console::puts("\n");
	while (current->base_frame_no > _first_frame_no || current->base_frame_no + current->nframes <= _first_frame_no){
        if (current->next == NULL){
            Console::puts("Frame was not found in this pool, can't release... \n");
            return;
        }
        else{
            current = current->next;
        }
    }

	unsigned char *bitmap_pointer = current->bitmap;
	int difference = (_first_frame_no - current->base_frame_no)*2;
    int byte_pos = difference/8; 
	int pos_firstFrame = (difference % 8)/2;
    

	unsigned char head_mask = 0x80;
	unsigned char filter_mask = 0xC0;

	head_mask = head_mask>>(pos_firstFrame*2);
	filter_mask = filter_mask>>(pos_firstFrame*2);   
 
	//Console::puts("Byte from bitmap of first frame ");Console::puti(bitmap_pointer[byte_pos]);Console::puts("\n");
	//Console::puts("Filter mask ");Console::puti(filter_mask);Console::puts("\n");
	//Console::puts("head mask ");Console::puti(head_mask);Console::puts("\n");

	if (((bitmap_pointer[byte_pos]^head_mask)&filter_mask ) == filter_mask){
        bitmap_pointer[byte_pos] = bitmap_pointer[byte_pos] & (~filter_mask); //free head-of-sequence frame
        current->nFreeFrames++;
		filter_mask=filter_mask>>2;
		pos_firstFrame++;

		while(pos_firstFrame<4){
			if((bitmap_pointer[byte_pos] & filter_mask) == filter_mask){
				bitmap_pointer[byte_pos] = bitmap_pointer[byte_pos] & (~filter_mask);
				filter_mask=filter_mask>>2;
				current->nFreeFrames++;
				pos_firstFrame++;
			}
			else{
				Console::puts("Frames released successfully \n");
				Console::puts("No. of free frames after realeasing frames ");Console::puti(current->nFreeFrames);Console::puts("\n");
				return;
			}

		}


		for (int i = byte_pos+1; i < (current->base_frame_no+current->nframes)/4; i++)
	    {
			filter_mask = 0xC0;
			for(int j =0 ;j<4;j++){
				if((bitmap_pointer[i] & filter_mask) == filter_mask){
					bitmap_pointer[i] = bitmap_pointer[i] & (~filter_mask);
					filter_mask=filter_mask>>2;
					current->nFreeFrames++;
				}
		        else{
					Console::puts("Frames released successfully \n");
					Console::puts("Number of free frames after realeasing frames ");Console::puti(current->nFreeFrames);Console::puts("\n");
			        return;
	        	}
			}

	    }	
		
	}
	else{
		Console::puts("Error: _first_frame_no is not HEAD-OF-SEQUENCE\n");
	}
	
    
}

unsigned long ContFramePool::needed_info_frames(unsigned long _n_frames)
{
    return  _n_frames / 16384 + (_n_frames % 16384 > 0 ? 1 : 0);
}
