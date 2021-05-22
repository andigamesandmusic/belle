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

#ifndef PRIM_INCLUDE_FILE_H
#define PRIM_INCLUDE_FILE_H

#ifndef PRIM_LIBRARY
#error This file can not be included individually. Include prim.h instead.
#endif

namespace PRIM_NAMESPACE
{
  ///Static methods for reading and writing to files.
  class File
  {
    ///Internal method for reading from file. Fails if file could not be opened.
    static bool Read(const ascii* Filename, byte* ByteData, count Bytes);

    ///Internal method for writing to file. Fails if all bytes are not written.
    static bool Write(const ascii* Filename, const byte* ByteData, count Bytes);

    /**Internal method for appending to file. Fails if all bytes are not
    appended.*/
    static bool Append(const ascii* Filename, const byte* ByteData,
      count Bytes);

  public:
    /**Returns the length of the file in bytes. If the file does not exist, this
    will return 0.*/
    static count Length(const ascii* Filename);

    /**Reads a file into an array and returns whether it was successful. If the
    file length is not a multiple of the object length, then the trailing bytes
    are ignored.*/
    template <class T>
    static bool Read(const ascii* Filename, Array<T>& SomeArray)
    {
      count LengthOfFile = Length(Filename);
      SomeArray.n(LengthOfFile / count(sizeof(T)));
      bool Result = Read(Filename, reinterpret_cast<byte*>(&SomeArray[0]),
        LengthOfFile);
      if(not Result)
        SomeArray.n(0);
      return Result;
    }

    ///Reads the file as a string and returns whether it was successful.
    static bool Read(const ascii* Filename, String& Destination)
    {
      Array<byte> ByteArray;
      Destination.Clear();

      if(not Read(Filename, ByteArray))
        return false;

      Destination.Append(&ByteArray[0], ByteArray.n());
      return true;
    }

    ///Reads a file and passes back a string containing its contents.
    static String Read(const ascii* Filename)
    {
      String Contents;
      Read(Filename, Contents);
      return Contents;
    }

    ///Writes a new file from an array and returns whether it was successful.
    template <class T>
    static bool Write(const ascii* Filename, const Array<T>& SomeArray)
    {
      return Write(Filename, reinterpret_cast<const byte*>(&SomeArray[0]),
        SomeArray.n() * count(sizeof(T)));
    }

    ///Writes a new file from a string and returns whether it was successful.
    static bool Write(const ascii* Filename, const String& SomeString)
    {
      return Write(Filename, reinterpret_cast<const byte*>(SomeString.Merge()),
        SomeString.n());
    }

    /**Appends array data to a file and returns whether it was successful. The
    file is created if it does not exist.*/
    template <class T>
    static bool Append(const ascii* Filename, const Array<T>& SomeArray)
    {
      return Append(Filename, reinterpret_cast<const byte*>(&SomeArray[0]),
        SomeArray.n() * count(sizeof(T)));
    }

    /**Appends a string to an existing file. The file is created if it does not
    exist.*/
    static bool Append(const ascii* Filename, const String& SomeString)
    {
      return Append(Filename, reinterpret_cast<const byte*>(SomeString.Merge()),
        SomeString.n());
    }
  };

#ifdef PRIM_COMPILE_INLINE
  bool File::Read(const ascii* Filename, byte* ByteData, count Bytes)
  {
    //Create an input file stream.
    std::ifstream FileStream;

    //Open the file, but do not truncate it.
    FileStream.open(Filename, std::ios::in | std::ios::binary);

    //Get out of here if for some reason the file could not be opened.
    if(not FileStream.is_open())
      return false;

    //Seek to the beginning of the file and read into the byte array.
    FileStream.seekg(0, std::ios_base::beg);
    FileStream.read(reinterpret_cast<ascii*>(ByteData), Bytes);

    //Return the length of the file.
    return true;
  }

  bool File::Write(const ascii* Filename, const byte* ByteData, count Bytes)
  {
    //Create an input file stream.
    std::ofstream FileStream;

    //Open the file and truncate it.
    FileStream.open(Filename, std::ios::out | std::ios::trunc |
      std::ios::binary);

    //Get out of here if for some reason the file could not be opened.
    if(not FileStream.is_open())
      return false;

    //Write the byte array to the file.
    FileStream.write(reinterpret_cast<const ascii*>(ByteData), Bytes);

    //Close the file.
    FileStream.close();

    //Return success.
    return true;
  }

  bool File::Append(const ascii* Filename, const byte* ByteData, count Bytes)
  {
    //Create an input file stream.
    std::ofstream FileStream;

    //Open the file and append to it.
    FileStream.open(Filename, std::ios::out | std::ios::app |
      std::ios::binary);

    //Get out of here if for some reason the file could not be opened.
    if(not FileStream.is_open())
      return false;

    //Write the byte array to the file.
    FileStream.write(reinterpret_cast<const ascii*>(ByteData), Bytes);

    //Close the file.
    FileStream.close();

    //Return success.
    return true;
  }

  count File::Length(const ascii* Filename)
  {
    //Create an input file stream.
    std::ifstream FileStream;

    //Open the file, but do not truncate it.
    FileStream.open(Filename, std::ios::in | std::ios::binary);

    //Get out of here if for some reason the file could not be opened.
    if(not FileStream.is_open())
      return 0;

    //Seek to the end of the file to get the length.
    FileStream.seekg(0, std::ios_base::end);
    return count(FileStream.tellg()); //tellg() returns aggregate data
  }
#endif
}
#endif
