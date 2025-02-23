#ifndef FRAMEPROVIDER_H
#define FRAMEPROVIDER_H


#include "bitmap.h"
#include "synch.h"


class FrameProvider 
{
	public :

		FrameProvider(int numFrame) ;		// initializes the FrameProvider the given number of frames
		~FrameProvider() ;					// cleans up resources used by the frameprovider

		int GetEmptyFrame() ;				// allocates and returns an empty frame
		void ReleaseFrame(int frame) ;		// marks a given frame as free
		unsigned int NumAvailFrame() ;		// returns the number of available frames
		bool IsFrameAvail() ;				// checks if at least one frame is available

	private :

		int nb_frames ;					// total number of frames managed by the provider
		BitMap *framesBitmap ;				// bitmap to tracks the status of whether user or available of each frame
		Lock *framesBitmapLock ;  			// lock to synchronise access to the bitmap
} ;


#endif
