#include "system.h"
#include "frameprovider.h"
#include "bitmap.h"
#include "synch.h"
#include "time.h"
#include <stdio.h>


//--------------------------------------------------------------------------
// FrameProvider::FrameProvider
//			Initialize a new FrameProvider to manage physical memory frames.
//
//			This constructor creates and initiliazes a bitmap 'frameBitmap' 
//			to track the availability of each physical frames. It sets the
//			initial total number of frames and a lock 'frameBitmapLock'
//
//			'numFrames' represents the number of physical frames available
//---------------------------------------------------------------------------

FrameProvider::FrameProvider(int numFrames)
{
	srand(time(NULL)) ;

	framesBitmap = new BitMap(numFrames) ;
	framesBitmap->Mark(0) ;

	framesBitmapLock = new Lock("FrameProvider bitmap lock") ;
	nb_frames = numFrames ;
}

//--------------------------------------------------------------------------
// FrameProvider::~FrameProvider
//			Clean up the resources used by the FrameProvider.
//
//			this destructor deletes the bitmap and the lock associated 
//---------------------------------------------------------------------------

FrameProvider::~FrameProvider()
{
	delete framesBitmap ;
	delete framesBitmapLock ;
}

//--------------------------------------------------------------------------
// FrameProvider::GetEmptyFrame
//			Allocate an empty frame of physical memory to a process.
//
//			This function loops through all frames and retrieves the free ones
//			inside a temporary table, then apply a random policy to choose 
//			among those free frames a frame to allocate. The function returns
//			the chosen index, and frees the corresponding physical space. This
// 			access is protected by the 'frameBitmapLock'.
//
//			if no frames are available (isFull stays true), the function returns 
//			-1 to indicate an error
//
//			returns:
//				The index of the allocated frame or -1
//---------------------------------------------------------------------------

int FrameProvider::GetEmptyFrame()
{
	int selectedFrame = -1;

	framesBitmapLock->Acquire() ;

	int emptyFramesCount = 0;
	for (int i = 0 ; i < nb_frames ; i ++)
	{
		if (! framesBitmap->Test(i))
		{
			emptyFramesCount++;
			if(rand() % emptyFramesCount == 0) {
				selectedFrame = i;
			}
		}
	}

	if(selectedFrame == -1){
		framesBitmapLock->Release();
		return -1;
	}
	
	framesBitmap->Mark(selectedFrame) ;
	bzero(&(machine->mainMemory[selectedFrame * PageSize]), PageSize) ;
	framesBitmapLock->Release() ;

	return selectedFrame ;
}

//--------------------------------------------------------------------------
// FrameProvider::ReleaseFrame
//			Mark a given frame as available.
//
//			This function clears the status of a specific frame in the bitmap
//			for future allocations.
//
//			arg:
//				frame: the index of the frame to mark
//---------------------------------------------------------------------------

void FrameProvider::ReleaseFrame(int frame)
{
	framesBitmapLock->Acquire() ;
	framesBitmap->Clear(frame) ;
	framesBitmapLock->Release() ;
}

//--------------------------------------------------------------------------
// FrameProvider::NumAvailFrame
//			Return the number of available frames in the system.
//
//			This function uses the bitmap class function to calculta the
//			number of frames that available.
//
//			return:
//				The total number of available frames
//---------------------------------------------------------------------------

unsigned int FrameProvider::NumAvailFrame()
{
	framesBitmapLock->Acquire() ;
	int numFrames = framesBitmap->NumClear();
	framesBitmapLock->Release() ;

    return numFrames;
}

//--------------------------------------------------------------------------
// FrameProvider::IsFrameAvail
//			Checks if any frame is available.
//
//			This function determines whether there are any free frames by 
//			checking if the number of available frames is positive.
//
//			return:
//				true if at least one frame is available, false otherwise
//---------------------------------------------------------------------------

bool FrameProvider::IsFrameAvail()
{
	return NumAvailFrame() > 0 ;
}
