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

#ifndef PRIM_INCLUDE_MOD_DIRECTORY_H
#define PRIM_INCLUDE_MOD_DIRECTORY_H
#ifdef PRIM_WITH_DIRECTORY //Depends on non-portable system headers.

#ifndef PRIM_LIBRARY
#error This file can not be included individually. Include prim.h instead.
#endif

namespace PRIM_NAMESPACE
{
  ///Lists files in a directory.
  class Directory
  {
    static String CanonicalSeparators(String Path)
    {
      Path.Replace("\\", "/");
      return Path;
    }

    public:

    ///Display modes for files and directories.
    enum DisplayMode
    {
      JustFiles,
      JustDirectories,
      FilesAndDirectories
    };

    /**Lists the files at a given path. Returns whether or not the listing was
    successful. On POSIX, tilde expansion is also done via lookup of the HOME
    environment variable.*/
    static bool Files(const ascii* Path, Array<String>& Files,
      DisplayMode Mode = JustFiles, bool ShowHidden = false,
      const ascii* Extension = "");

    ///Returns the current working directory.
    static String Working();
  };
}

#ifdef PRIM_COMPILE_INLINE
#ifdef PRIM_ENVIRONMENT_UNIX_LIKE
#include <dirent.h>
#include <sys/stat.h>
#else
#include <windows.h>
#include <direct.h>
#include <tchar.h>
#include <strsafe.h>
#endif

namespace PRIM_NAMESPACE
{
  bool Directory::Files(const ascii* Path, Array<String>& Files,
    DisplayMode Mode, bool ShowHidden, const ascii* Extension)
  {
    //Immediately clear the output array.
    Files.Clear();

    //Canonicalize the path.
    String CanonicalPath = CanonicalSeparators(Path);

    //Guarantee a trailing slash.
    while(CanonicalPath.EraseEnding("/")) {}
    CanonicalPath << "/";

    //Go through each entry and add it to the array.
    Sortable::Array<String> Output;
    String ExtensionName = ".";
    ExtensionName << Extension;

#ifdef PRIM_ENVIRONMENT_UNIX_LIKE
    //Open the directory for reading.
    DIR* Directory = opendir(CanonicalPath.Merge());
    if(not Directory)
      return false;
    dirent* Entry = 0;
    while((Entry = readdir(Directory)))
    {
      String Name = Entry->d_name;
      if(Name == "." or Name == "..") continue;
      if(Name[0] == '.' and not ShowHidden) continue;
      if(Entry->d_type == DT_DIR and Mode == JustFiles) continue;
      if(Entry->d_type == DT_REG and Mode == JustDirectories) continue;
      if(ExtensionName.n() > 1 and not Name.EndsWith(ExtensionName)) continue;

      //Concatenate the path and the filename.
      Name.Prepend(CanonicalPath);

      //Create the absolute path from the concatenated path.
      String AbsolutePath;
      if(Name.StartsWith("/"))
      {
        //Path is already absolute, so take it at face value.
        AbsolutePath = Name;
      }
      else
      {
        /*Get the full path using realpath. Note that older systems used
        PATH_MAX with a preallocated buffer; however, on new systems PATH_MAX
        is no longer defined.*/
        if(char* Buffer = realpath(Name, 0))
        {
          count BufferLength = String::LengthOf(Buffer);
          AbsolutePath.Append(reinterpret_cast<byte*>(Buffer), BufferLength);
          Memory::Clear(Buffer, BufferLength);
          free(Buffer);
        }
      }

      //If it was a directory, then add a trailing slash.
      if(Entry->d_type == DT_DIR and not AbsolutePath.EndsWith("/"))
        AbsolutePath << "/";

      //Add the file to the output list.
      Output.Add() = AbsolutePath;
    }
    //Close the directory to reading.
    closedir(Directory);
#else
    CanonicalPath.Replace("/", "\\");
    WIN32_FIND_DATA FindData;
    TCHAR DirectoryString[MAX_PATH];
    StringCchCopy(DirectoryString, MAX_PATH, CanonicalPath + "*");
    HANDLE FindHandle = FindFirstFile(DirectoryString, &FindData);
    if(FindHandle != INVALID_HANDLE_VALUE)
    {
      do
      {
        bool IsDirectory = FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
        bool IsHidden = FindData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN;
        String Name = FindData.cFileName;
        if(IsHidden && !ShowHidden) continue;
        if(Name == "." || Name == "..") continue;
        if(IsDirectory && Mode == JustFiles) continue;
        if(!IsDirectory && Mode == JustDirectories) continue;
        if(ExtensionName.n() > 1 && !Name.EndsWith(ExtensionName)) continue;
        TCHAR FullName[2048] = TEXT("");
        Name.Prepend(CanonicalPath);
        if(GetFullPathName(Name, sizeof(FullName), FullName, 0))
          Name = FullName;
        Output.Add() = Name + (IsDirectory ? "\\" : "");
      }
      while(FindNextFile(FindHandle, &FindData));
      FindClose(FindHandle);
    }
#endif

    //Sort the output.
    Output.Sort();

    //Copy the output to the files array.
    Files = Output;

    //Return success.
    return true;
  }

  String Directory::Working()
  {
    char temp[2048];
#ifdef PRIM_ENVIRONMENT_UNIX_LIKE
    return String(getcwd(temp, sizeof(temp)));
#else
    return String(_getcwd(temp, sizeof(temp)));
#endif
  }
}
#endif
#endif
#endif
