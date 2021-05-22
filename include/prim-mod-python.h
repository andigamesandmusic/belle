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

#ifndef PRIM_INCLUDE_MOD_PYTHON_H
#define PRIM_INCLUDE_MOD_PYTHON_H
#ifdef PRIM_WITH_PYTHON //Depends on ModShell.

namespace PRIM_NAMESPACE
{
  /**Wrapper for shelling expressions out to Python. This requires that the user
  has installed Python and that it is available on their shell path.*/
  class Python
  {
    public:

    /**Returns the full path to the Python shell on the current path. If there
    is no Python installation available then an empty string is returned.*/
    static String GetPythonShell()
    {
      return Shell::GetProcessOnPath("python");
    }

    /**Returns whether Python is installed and working. A simple expression is
    tested to check that Python is actually functional.*/
    static bool IsPythonInstalled()
    {
      String In, Out, Error;
      //Do a simple calculation to make sure someone is inside.
      In << "print(11*17)";
      Shell::PipeInOut(GetPythonShell(), In, Out, Error);
      Out.Trim();
      return Out == "187";
    }

    ///Checks the current version of Python.
    static String GetVersion()
    {
      String In, Out, Version;
      Shell::PipeInOut(GetPythonShell(), In, Out, Version, "--version");
      Version.Replace("\r\n", "\n");
      while(Version.EndsWith("\n"))
        Version.EraseLast();
      Version.Replace("Python ", "");
      Version.Replace("python ", "");
      return Version;
    }

    /**Checks whether the version is at least a certain one. The format is in
    x.y.z where the letters are all single digit numbers.*/
    static bool IsVersionAtLeast(String VersionCheck)
    {
      return GetVersion() >= VersionCheck;
    }

    ///Prints information about the known state of the Python installation.
    static void PrintInfo()
    {
      C::Out()++;
      C::Out() >> "-----------------------------------------------------------";
      C::Out() >> "                    Python Information";
      C::Out() >> "-----------------------------------------------------------";
      C::Out() >> "Python Shell:     " << Python::GetPythonShell();
      C::Out() >> "Python Installed: " << Python::IsPythonInstalled();
      C::Out() >> "Python Version:   " << Python::GetVersion();
      C::Out() >> "Python >= 2.7.0:  " << Python::IsVersionAtLeast("2.7.0");
      C::Out() >> "Python >= 3.0.0:  " << Python::IsVersionAtLeast("3.0.0");
      C::Out()++;
      C::Out() >> "Get all subsets of [1, 2, 3]...";
      C::Out() >> Python::Evaluate(
        "f = lambda l: reduce(lambda z, x: z + [y + [x] for y in z], l, [[]])"
        "\nprint(f([1,2,3]))");
      C::Out()++;
      C::Out() >> "Testing INVALID expression reporting to stderr...";
      C::Out() >> Python::Evaluate("bogus");
      C::Out()++;
      C::Out() >> "Testing bignum expression 2^1000 / 3^500:";
      C::Out() >> Python::Evaluate("print((2**1000)/(3**500))");
      C::Out() >> "-----------------------------------------------------------";
      C::Out()++;
    }

    /**Evaluates an expression in Python and returns the result. If there was an
    error, then the error is reported instead prefixed and suffixed by "$$$".*/
    static String Evaluate(String Expression)
    {
      if(not IsPythonInstalled())
        return "$$$Python is not installed$$$";

      String In, Out, Error;
      Shell::PipeInOut(GetPythonShell(), Expression, Out, Error);
      Out.Trim();
      Error.Trim();
      if(Error)
      {
        Error.Prepend("$$$");
        Error.Append("$$$");
        return Error;
      }
      return Out;
    }
  };
}
#endif
#endif
