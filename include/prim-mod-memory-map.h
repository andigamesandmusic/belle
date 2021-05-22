/*
  ==============================================================================

  Copyright 2007-2013, 2017 William Andrew Burnson
  Copyright 2013-2016 Robert Taub

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice,
       this list of conditions and the following disclaimer.

    2. Redistributions in binary form must reproduce the above copyright notice,
       this list of conditions and the following disclaimer in the documentation
       and/or other materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.

  ==============================================================================
*/

#ifndef PRIM_INCLUDE_MOD_MEMORY_MAP_H
#define PRIM_INCLUDE_MOD_MEMORY_MAP_H
#ifdef PRIM_WITH_MEMORY_MAP //Depends on non-portable system headers.

#ifndef PRIM_LIBRARY
#error This file can not be included individually. Include prim.h instead.
#endif

namespace PRIM_NAMESPACE
{
  /**Memory maps a file for reading. This allows a file to be read on demand as
  though it were memory which is useful for things that need access to large
  files but not necessarily all at once.*/
  class MemoryMap
  {
    ///Length of the file
    count Length;

    ///Length of the memory map (padded with zeros)
    count MappedLength;

    ///Platform-dependent file handle
    void* Handle;

    ///Mapped memory
    void* Mapped;

    ///Initializes all of the fields.
    void Initialize()
    {
      Length = MappedLength = 0;
      Mapped = Handle = 0;
    }

    public:

    ///Initializes the memory map.
    MemoryMap() {Initialize();}

    ///Constructor to open file as memory map.
    MemoryMap(const ascii* Filename)
    {
      Initialize();
      Open(Filename);
    }

    ///Destructor closes memory map if it is open.
    ~MemoryMap() {Close();}

    ///Opens a file as a memory map.
    bool Open(const ascii* Filename);

    ///Closes the memory map if it was open.
    void Close();

    ///Returns whether the memory map is open.
    bool IsOpen() const {return Mapped;}

    /**Gets the length of the data. Note that there is guaranteed to be at least
    eight bytes of zero-padding past the end of the map for null-termination
    purposes.*/
    count n() const {return Length;}

    ///Returns the beginning address of the map or null if it is not open.
    const void* a() const {return Mapped;}
  };
}

#ifdef PRIM_COMPILE_INLINE

#ifdef PRIM_ENVIRONMENT_UNIX_LIKE
#include <sys/types.h>
#include <sys/mman.h>
#endif

namespace PRIM_NAMESPACE
{
  bool MemoryMap::Open(const ascii* Filename)
  {
    //Close any existing file opened by this class.
    Close();

    //Read the length of the file.
    Length = File::Length(Filename);

#ifdef PRIM_ENVIRONMENT_UNIX_LIKE
    //Attempt to open the file.
    Handle = fopen(Filename, "r");

    //Return if for some reason the file could not be opened.
    if(not Handle)
    {
      Initialize();
      return false;
    }

    //Attempt to make a memory map of the file.
    MappedLength = Length + 8;
    Mapped = mmap(0, size_t(MappedLength), PROT_READ, MAP_FILE | MAP_PRIVATE,
      fileno(reinterpret_cast<FILE*>(Handle)), 0);

    //Failed to map the file, so close the file handle.
    if(Mapped == MAP_FAILED)
    {
      C::Error() >> "Error: could not make memory map from file.";
      fclose(reinterpret_cast<FILE*>(Handle));
      Initialize();
      return false;
    }

    return true;
#else
    return false;
#endif
  }

  void MemoryMap::Close()
  {
    //Close the memory map and file handle.
    if(IsOpen())
    {
#ifdef PRIM_ENVIRONMENT_UNIX_LIKE
      munmap(Mapped, size_t(MappedLength));
      fclose(reinterpret_cast<FILE*>(Handle));
#endif
    }

    //Reset the fields.
    Initialize();
  }
}
#endif
#endif
#endif
