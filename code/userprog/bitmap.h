// bitmap.h
//      Data structures defining a bitmap -- an array of bits each of which
//      can be either on or off.
//
//      Represented as an array of unsigned integers, on which we do
//      modulo arithmetic to find the bit we are interested in.
//
//      The bitmap can be parameterized with with the number of bits being
//      managed.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#ifndef BITMAP_H
#define BITMAP_H

#include "copyright.h"
#include "openfile.h"
#include "utility.h"
// #include "synch.h"

// Definitions helpful for representing a bitmap as an array of integers
#define BitsInByte 8
#define BitsInWord 32

#define PagePerThread 2 // fixing the pages allowed per thread in the user-program - might find a dynamic way to set later

// The following class defines a "bitmap" -- an array of bits,
// each of which can be independently set, cleared, and tested.
//
// Most useful for managing the allocation of the elements of an array --
// for instance, disk sectors, or main memory pages.
// Each bit represents whether the corresponding sector or page is
// in use or free.

// class Semaphore;

class BitMap {
  public:
    BitMap(int nitems); // Initialize a bitmap, with "nitems" bits
                        // initially, all bits are cleared.
    ~BitMap();          // De-allocate bitmap

    void Mark(int which);  // Set the "nth" bit
    void Clear(int which); // Clear the "nth" bit
    bool Test(int which);  // Is the "nth" bit set?
    int Find();            // Return the # of a clear bit, and as a side
    // effect, set the bit.
    // If no bits are clear, return -1.
    int NumClear(); // Return the number of clear bits

    void Print(); // Print contents of bitmap

    // These aren't needed until FILESYS, when we will need to read and
    // write the bitmap to a file
    void FetchFrom(OpenFile *file); // fetch contents from disk
    void WriteBack(OpenFile *file); // write contents to disk

    int NumThreads();

  private:
    int numBits;  // number of bits in the bitmap
    int numWords; // number of words of bitmap storage
    // (rounded up if numBits is not a
    //  multiple of the number of bits in
    //  a word)
    unsigned int *map; // bit storage
    // Semaphore *findSem;
    // Semaphore *modifSem;
};

#endif // BITMAP_H
