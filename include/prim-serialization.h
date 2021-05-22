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

#ifndef PRIM_INCLUDE_SERIALIZATION_H
#define PRIM_INCLUDE_SERIALIZATION_H

#ifndef PRIM_LIBRARY
#error This file can not be included individually. Include prim.h instead.
#endif

namespace PRIM_NAMESPACE
{
  //This class is essentially deprecated by Value, JSON, and Encoding::Base64.

  /**Byte array that can read and write various higher-level data types. One
  important thing to note is that data is always stored as the widest type. This
  is to eliminate any confusion over storage sizes on different platforms.
  Effectively this means: (u)int8/16/32/64 as well as the bool type are all
  stored internally as int64, and float32/64 are both stored as float64. String
  data is stored in a Pascal-like format of 64-bit length + byte data. All other
  types use the above primitive types. The first 32 bytes is reserved for a MD5
  checksum of the remaining data in plain-text lowercase hexadecimal format.*/
  class Serial : public Array<byte>
  {
    ///Stores the position of the read head during reading.
    count ReadHead;

    public:

    ///Enumeration of generic serial modes.
    enum Modes
    {
      Reading,
      Writing,
      CheckVersion,
      CheckID
    };

    ///Callback-style interface for objects that require serialization.
    class Object
    {
      public:

      /**Requests the object to read, write, or provide information. VersionOrID
      is used as follows: in Reading mode it contains the version number of the
      string; in CheckVersion mode the method should provide a version (default
      is 0); in CheckID mode the method should provide a unique ID that
      identifies the class; and, in Writing mode it has no purpose.*/
      virtual void Serialize(Serial& s, Serial::Modes Mode,
        UUIDv4& VersionOrID) = 0;

      ///Virtual destructor
      virtual ~Object();
    };

    /**Restores an object from an ID. The method is intended to be overloaded
    in any system that has objects that need to be serialized and allows the
    system to handle specific memory new operations (possibly with non-default
    constructors or other logic). Note that the ID refers to the ID assigned
    to the class type in Object::Serialize during the CheckID mode.*/
    virtual Serial::Object* RestoreObject(UUIDv4 ID) {(void)ID; return 0;}

    ///Virtual destructor
    virtual ~Serial();

    ///Resets the read head to after the checksum.
    void StartFromBeginning() {ReadHead = 32;}

    ///Writes a 64-bit signed integer.
    void Write(int64 Value)
    {
      count WriteHead = n();
      n(WriteHead + count(sizeof(int64)));
      int64 ValueToWrite = Value;
      Endian::ConvertToLittleEndian(ValueToWrite);

      /*Memory::Copy is word-alignment safe. This is important for mobile
      platforms that do not allow dereferencing on arbitrary byte locations.*/
      Memory::Copy(reinterpret_cast<int64&>(ith(WriteHead)), ValueToWrite);
    }

    ///Takes in a uint64 but always writes a 64-bit signed integer.
    void Write(uint64 Value)
    {
      Write(int64(Value));
    }

    ///Takes in an int32 but always writes a 64-bit signed integer.
    void Write(int32 Value)
    {
      Write(int64(Value));
    }

    ///Takes in a uint32 but always writes a 64-bit signed integer.
    void Write(uint32 Value)
    {
      Write(int64(Value));
    }

    ///Takes in an int16 but always writes a 64-bit signed integer.
    void Write(int16 Value)
    {
      Write(int64(Value));
    }

    ///Takes in a uint16 but always writes a 64-bit signed integer.
    void Write(uint16 Value)
    {
      Write(int64(Value));
    }

    ///Takes in an int8 but always writes a 64-bit signed integer.
    void Write(int8 Value)
    {
      Write(int64(Value));
    }

    ///Takes in a uint8 but always writes a 64-bit signed integer.
    void Write(uint8 Value)
    {
      Write(int64(Value));
    }

    /**Writes a bool as a 64-bit integer. The encoded value is -1 for true
    (all bits on) and 0 for false (all bits off).*/
    void Write(bool Value)
    {
      Write(Value ? int64(-1) : int64(0));
    }

    ///Writes a number. The encoded value is of float64 type.
    void Write(float64 Value)
    {
      count WriteHead = n();
      n(WriteHead + count(sizeof(float64)));
      float64 ValueToWrite = Value;
      Endian::ConvertToLittleEndian(ValueToWrite);

      /*Memory::Copy is word-alignment safe. This is important for mobile
      platforms that do not allow dereferencing on arbitrary byte locations.*/
      Memory::Copy(reinterpret_cast<float64&>(ith(WriteHead)), ValueToWrite);
    }

    ///Takes in a float32 but always writes a float64.
    void Write(float32 Value)
    {
      Write(float64(Value));
    }

    /**Takes in a float80 but always writes a float64. Note that since float80
    is platform-specific, it is not safe to write it out directly.*/
    void Write(float80 Value)
    {
      Write(float64(Value));
    }

    ///Writes a integer vector to the serial.
    void Write(const VectorInt& Value)
    {
      Write(Value.x);
      Write(Value.y);
    }

    ///Writes a number vector to the serial.
    void Write(const Vector& Value)
    {
      Write(number(Value.x));
      Write(number(Value.y));
    }

    ///Writes an integer rectangle to the serial.
    void Write(const BoxInt& Value)
    {
      Write(Value.a);
      Write(Value.b);
    }

    ///Writes a rectangle to the serial.
    void Write(const Box& Value)
    {
      Write(Value.a);
      Write(Value.b);
    }

    ///Writes a ratio to the serial.
    void Write(const Ratio& Value)
    {
      Write(Value.Numerator());
      Write(Value.Denominator());
    }

    ///Writes a Pascal string (length plus data array) to the serial.
    void Write(const String& Value)
    {
      //Writes a string in PASCAL format (length + data)
      Write(Value.n());
      count WriteHead = n();
      n(WriteHead + Value.n());
      Memory::Copy(reinterpret_cast<ascii*>(&ith(WriteHead)),
        Value.Merge(), Value.n());
      WriteHead = n();
      n(WriteHead);
    }

    ///Writes a UUIDv4 to the serial.
    void Write(const UUIDv4& Value)
    {
      Write(Value.High());
      Write(Value.Low());
    }

    ///Writes a serial source to the serial.
    void Write(Serial::Object& Value)
    {
      UUIDv4 Version(0, 0);
      Value.Serialize(*this, CheckVersion, Version);
      Write(Version);
      Value.Serialize(*this, Writing, Version);
    }

    ///Reads a 64-bit signed integer.
    void Read(int64& Value)
    {
      int64 ReadValue = 0;

      /*Memory::Copy is word-alignment safe. This is important for mobile
      platforms that do not allow dereferencing on arbitrary byte locations.*/
      Memory::Copy(ReadValue, reinterpret_cast<int64&>(ith(ReadHead)));
      Endian::ConvertToLittleEndian(ReadValue);
      Value = ReadValue;
      ReadHead += count(sizeof(int64));
    }

    ///Reads in a uint64 from a 64-bit signed integer.
    void Read(uint64& Value)
    {
      int64 v = 0;
      Read(v);
      Value = uint64(v);
    }

    ///Reads in an int32 from a 64-bit signed integer.
    void Read(int32& Value)
    {
      int64 v = 0;
      Read(v);
      Value = int32(v);
    }

    ///Reads in an uint32 from a 64-bit signed integer.
    void Read(uint32& Value)
    {
      int64 v = 0;
      Read(v);
      Value = uint32(v);
    }

    ///Reads in an int16 from a 64-bit signed integer.
    void Read(int16& Value)
    {
      int64 v = 0;
      Read(v);
      Value = int16(v);
    }

    ///Reads in an uint16 from a 64-bit signed integer.
    void Read(uint16& Value)
    {
      int64 v = 0;
      Read(v);
      Value = uint16(v);
    }

    ///Reads in an int8 from a 64-bit signed integer.
    void Read(int8& Value)
    {
      int64 v = 0;
      Read(v);
      Value = int8(v);
    }

    ///Reads in an uint8 from a 64-bit signed integer.
    void Read(uint8& Value)
    {
      int64 v = 0;
      Read(v);
      Value = uint8(v);
    }

    ///Reads in a boolean from the serial.
    void Read(bool& Value)
    {
      integer ReadValue = 0;
      Read(ReadValue);
      Value = (ReadValue != 0);
    }

    ///Reads in a number from the serial.
    void Read(float64& Value)
    {
      float64 ReadValue = 0.0;

      /*Memory::Copy is word-alignment safe. This is important for ARM platforms
      which do not allow dereferencing on arbitrary byte locations.*/
      Memory::Copy(ReadValue, reinterpret_cast<float64&>(ith(ReadHead)));
      Endian::ConvertToLittleEndian(ReadValue);
      Value = ReadValue;
      ReadHead += count(sizeof(float64));
    }

    ///Reads in a float32 from a float64.
    void Read(float32& Value)
    {
      float64 v = 0;
      Read(v);
      Value = float32(v);
    }

    /**Reads in a float80 from a float64. Note that since float80 is
    platform-specific, it is not safe to serialize directly.*/
    void Read(float80& Value)
    {
      float64 v = 0;
      Read(v);
      Value = float80(v);
    }

    ///Reads a integer vector from the serial.
    void Read(VectorInt& Value)
    {
      Read(Value.x);
      Read(Value.y);
    }

    ///Reads a number vector from the serial.
    void Read(Vector& Value)
    {
      Read(Value.x);
      Read(Value.y);
    }

    ///Reads an integer rectangle from the serial.
    void Read(BoxInt& Value)
    {
      Read(Value.a);
      Read(Value.b);
    }

    ///Reads a rectangle from the serial.
    void Read(Box& Value)
    {
      Read(Value.a);
      Read(Value.b);
    }

    ///Reads a ratio to the serial.
    void Read(Ratio& Value)
    {
      int64 n, d;
      Read(n);
      Read(d);
      Value = Ratio(n, d);
    }

    ///Reads in a string from the serial.
    void Read(String& Value)
    {
      integer NumberOfCharacters = 0;
      Read(NumberOfCharacters);
      Value.Clear();
      Value.Append(&ith(ReadHead), count(NumberOfCharacters));
      ReadHead += count(NumberOfCharacters);
    }

    ///Reads in a UUIDv4 from the serial.
    void Read(UUIDv4& Value)
    {
      uint64 h = 0, l = 0;
      Read(h);
      Read(l);
      Value.High(h);
      Value.Low(l);
    }

    ///Reads in a serial source from the serial.
    void Read(Serial::Object& Value)
    {
      //Reads the serial source by checking version and then reading the data.
      UUIDv4 Version;
      Read(Version);
      Value.Serialize(*this, Reading, Version);
    }

    ///Writes a list of items to the serial.
    template <class T>
    void Write(List<T>& ListOfItems)
    {
      Write(ListOfItems.n());
      for(count i = 0; i < ListOfItems.n(); i++)
        Write(ListOfItems[i]);
    }

    ///Reads a list of items from the serial.
    template <class T>
    void Read(List<T>& ListOfItems)
    {
      count NumberOfItems = 0;
      Read(NumberOfItems);
      ListOfItems.RemoveAll();
      for(count i = 0; i < NumberOfItems; i++)
        Read(ListOfItems.Add());
    }

    ///Writes an array of items to the serial.
    template <class T>
    void Write(Array<T>& ArrayOfItems)
    {
      Write(ArrayOfItems.n());
      for(count i = 0; i < ArrayOfItems.n(); i++)
        Write(ArrayOfItems[i]);
    }

    ///Reads an array of items from the serial.
    template <class T>
    void Read(Array<T>& ArrayOfItems)
    {
      count NumberOfItems = 0;
      Read(NumberOfItems);
      ArrayOfItems.Clear();
      for(count i = 0; i < NumberOfItems; i++)
        Read(ArrayOfItems.Add());
    }

    ///Writes a list of objects to the serial.
    template <class T>
    void WriteObjects(List<T>& ListOfItems)
    {
      Write(ListOfItems.n());
      for(count i = 0; i < ListOfItems.n(); i++)
      {
        UUIDv4 x;
        ListOfItems[i]->Serialize(*this, CheckID, x);
        Write(x);
        Write(*ListOfItems[i]);
      }
    }

    ///Writes an array of objects to the serial.
    template <class T>
    void WriteObjects(Array<T>& ArrayOfItems)
    {
      Write(ArrayOfItems.n());
      for(count i = 0; i < ArrayOfItems.n(); i++)
      {
        UUIDv4 x;
        ArrayOfItems[i]->Serialize(*this, CheckID, x);
        Write(x);
        Write(*ArrayOfItems[i]);
      }
    }

    ///Reads a list of objects from the serial.
    template <class T>
    void ReadObjects(List<T*>& ListOfItems)
    {
      count NumberOfItems = 0;
      Read(NumberOfItems);
      ListOfItems.RemoveAll();
      for(count i = 0; i < NumberOfItems; i++)
      {
        UUIDv4 x;
        Read(x);
        Read(*(ListOfItems.Add() = dynamic_cast<T*>(RestoreObject(x))));
      }
    }

    ///Reads an array of objects from the serial.
    template <class T>
    void ReadObjects(Array<T*>& ArrayOfItems)
    {
      count NumberOfItems = 0;
      Read(NumberOfItems);
      ArrayOfItems.ClearAndDeleteAll();
      for(count i = 0; i < NumberOfItems; i++)
      {
        UUIDv4 x;
        Read(x);
        Read(*(ArrayOfItems.Add() = dynamic_cast<T*>(RestoreObject(x))));
      }
    }

    ///Reads or writes an array of objects to the serial depending on the mode.
    template <class T>
    void DoObjects(T& Value, Modes Mode)
    {
      if(Mode == Reading)
        ReadObjects(Value);
      else if(Mode == Writing)
        WriteObjects(Value);
    }

    ///Reads or writes a value to the serial depending on the mode of operation.
    template <class T>
    void Do(T& Value, Modes Mode)
    {
      if(Mode == Reading)
        Read(Value);
      else if(Mode == Writing)
        Write(Value);
    }

    ///Constructor to create checksum header and initialize serial reading.
    Serial()
    {
      n(32);
      StartFromBeginning();
    }

    /**Either reads a stored checksum or calculates the current checksum. When
    calculating the checksum the last 32 bytes are ignored and considered to be
    the storage area for the checksum.*/
    String ReadChecksum(bool ReadStoredChecksum)
    {
      //If no space for checksum, serial is invalid.
      if(n() < 32)
        return "";

      if(ReadStoredChecksum)
        return String(&a(), 32);
      else
        return MD5::Hex(&ith(32), n() - 32, false);
    }

    ///Writes a checksum to the serial.
    void WriteChecksum()
    {
      String Checksum = ReadChecksum(false);
      for(count i = 0; i < 32; i++)
        ith(i) = byte(Checksum[i]);
    }

    /**Determines whether the end of the serial contains a valid checksum. This
    method can be used regardless of whether or not it is known that a checksum
    was actually saved. The method will only return true if a checksum was
    written and it matches the data checksum.*/
    bool ChecksumValid()
    {
      String StoredChecksum = ReadChecksum(true);
      String ActualChecksum = ReadChecksum(false);
      return StoredChecksum.n() == 32 and StoredChecksum == ActualChecksum;
    }
  };
#ifdef PRIM_COMPILE_INLINE
  //Avoid weak-vtable warning by putting at a virtual method out-of-line.
  Serial::~Serial() {}
  Serial::Object::~Object() {}
#endif
}
#endif
