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

#ifndef PRIM_INCLUDE_CONSOLE_H
#define PRIM_INCLUDE_CONSOLE_H

#ifndef PRIM_LIBRARY
#error This file can not be included individually. Include prim.h instead.
#endif

namespace PRIM_NAMESPACE
{
  /**Interface to console standard streams. Do not create an instance of this
  object directly. The static calls will automatically create a singleton of
  this class. To clean up memory associated with this object call the following
  at the end of the program:
  \code
  AutoRelease<Console>();
  \endcode
  or to combine it with the int return for int main():
  \code
  return AutoRelease<Console>();
  \endcode
  */
  class Console
  {
    ///Remembers whether ANSI effects were used.
    bool UsedANSI; PRIM_PAD(bool)

    ///Stores the output and error streams.
    String Stream;

    ///Gets the singleton console stream.
    static String& GetStream()
    {
      return Singleton<Console>::Instance().Stream;
    }

    ///Sets the ANSI color condition.
    static void SetANSI()
    {
      Singleton<Console>::Instance().UsedANSI = true;
    }

    static String ANSIColorCommand(String Command)
    {
#ifdef PRIM_ENVIRONMENT_WINDOWS
      return "";
#else
      return String() + "\033[" + Command;
#endif
    }

    public:

    //-------//
    //Default//
    //-------//

    ///Resets the color back to normal.
    static void Reset() {GetStream() << ANSIColorCommand("0m"); SetANSI();}

    //----------//
    //Text Style//
    //----------//

    ///Changes text to bold.
    static void Bold() {GetStream() << ANSIColorCommand("1m"); SetANSI();}

    ///Changes text to underline.
    static void Underline() {GetStream() << ANSIColorCommand("4m"); SetANSI();}

    //----------//
    //Text Color//
    //----------//

    ///Changes text color to black.
    static void Black() {GetStream() << ANSIColorCommand("30m"); SetANSI();}

    ///Changes text color to red.
    static void Red() {GetStream() << ANSIColorCommand("31m"); SetANSI();}

    ///Changes text color to green.
    static void Green() {GetStream() << ANSIColorCommand("32m"); SetANSI();}

    ///Changes text color to yellow.
    static void Yellow() {GetStream() << ANSIColorCommand("33m"); SetANSI();}

    ///Changes text color to blue.
    static void Blue() {GetStream() << ANSIColorCommand("34m"); SetANSI();}

    ///Changes text color to magenta.
    static void Magenta() {GetStream() << ANSIColorCommand("35m"); SetANSI();}

    ///Changes text color to cyan.
    static void Cyan() {GetStream() << ANSIColorCommand("36m"); SetANSI();}

    ///Changes text color to white.
    static void White() {GetStream() << ANSIColorCommand("37m"); SetANSI();}

    //----------------//
    //Background Color//
    //----------------//

    ///Changes background color to black.
    static void BlackBackground() {GetStream() <<
      ANSIColorCommand("40m"); SetANSI();}

    ///Changes background color to red.
    static void RedBackground() {GetStream() <<
      ANSIColorCommand("41m"); SetANSI();}

    ///Changes background color to green.
    static void GreenBackground() {GetStream() <<
      ANSIColorCommand("42m"); SetANSI();}

    ///Changes background color to yellow.
    static void YellowBackground() {GetStream() <<
      ANSIColorCommand("43m"); SetANSI();}

    ///Changes background color to blue.
    static void BlueBackground() {GetStream() <<
      ANSIColorCommand("44m"); SetANSI();}

    ///Changes background color to magenta.
    static void MagentaBackground() {GetStream() <<
      ANSIColorCommand("45m"); SetANSI();}

    ///Changes background color to cyan.
    static void CyanBackground() {GetStream() <<
      ANSIColorCommand("46m"); SetANSI();}

    ///Changes background color to white.
    static void WhiteBackground() {GetStream() <<
      ANSIColorCommand("47m"); SetANSI();}

    //-------//
    //Display//
    //-------//

    ///Attempts to calculate the terminal width in columns.
    static count Columns();

    ///Attempts to calculate the terminal height in rows.
    static count Rows();

    ///Clears the display.
    static void ClearDisplay()
    {
      for(count i = 0, n = Rows(); i < n; i++)
        GetStream() << "\n";
    }

    public:

    /**Returns a string retrieved from standard input. Note this will block
    until the client enters a string of text on the keyboard or from standard
    in.*/
    static String Input()
    {
      String s(String::StandardInput);
      return s;
    }

    /**Returns all the standard input lines. Note this will block if nothing has
    been piped into standard input.*/
    static Array<String> InputLines();

    ///Waits for the user to press a key and return.
    void Wait(String Message = "")
    {
      Output() >> Message;
      Input();
    }

    ///Returns a string stream attached to standard output.
    static String& Output()
    {
      GetStream().Attach(String::StandardOutput);
      return GetStream();
    }

    ///Returns a string stream attached to standard output.
    static String& Out()
    {
      return Output();
    }

    ///Returns a string stream attached to standard error.
    static String& Error()
    {
      GetStream().Attach(String::StandardError);
      return GetStream();
    }

    ///Returns a string stream attached to standard error.
    static String& Err()
    {
      return Error();
    }

    ///Returns a string stream attached to none of the standard streams.
    static String& Null()
    {
      GetStream().Attach(String::NotAttached);
      return GetStream();
    }

    ///Initializes console.
    Console() : UsedANSI(false) {}

    ///Appends a trailing line and resets ANSI colors if necessary.
    ~Console()
    {
      Output()++;
      if(UsedANSI)
        Reset();
    }
  };

  ///Short-form alias C for Console.
  typedef Console C;
}

#ifdef PRIM_COMPILE_INLINE
#if defined(PRIM_ENVIRONMENT_ANDROID) || defined(PRIM_ENVIRONMENT_WINDOWS)
#else
#include <unistd.h>
#include <sys/ioctl.h>
#endif

namespace PRIM_NAMESPACE
{
  count Console::Columns()
  {
#if defined(PRIM_ENVIRONMENT_ANDROID) || defined(PRIM_ENVIRONMENT_WINDOWS)
    count ColumnCount = 80;
#else
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    count ColumnCount = count(w.ws_col);
#endif

    //If the column count looks unreasonable, return a reasonable number.
    if(ColumnCount < 10 or ColumnCount > 1000)
      ColumnCount = 80;

    return ColumnCount;
  }

  count Console::Rows()
  {
#if defined(PRIM_ENVIRONMENT_ANDROID) || defined(PRIM_ENVIRONMENT_WINDOWS)
    count RowCount = 25;
#else
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    count RowCount = count(w.ws_row);
#endif

    //If the row count look unreasonable, return a reasonable number.
    if(RowCount < 10 or RowCount > 1000)
      RowCount = 25;

    return RowCount;
  }

  Array<String> Console::InputLines()
  {
    Array<String> Lines;
    while(!std::cin.eof())
    {
      std::string Line;
      getline(std::cin, Line);
      if(std::cin.fail())
        break;
      Lines.Add() = Line.c_str();
    }
    return Lines;
  }
}
#endif

#endif
