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

#ifndef PRIM_INCLUDE_MOD_SHELL_H
#define PRIM_INCLUDE_MOD_SHELL_H
#ifdef PRIM_WITH_SHELL //Depends on non-portable system headers.

namespace PRIM_NAMESPACE
{
  ///Launching processes and piping information in and out.
  class Shell
  {
    public:

    ///Returns the current shell path.
    static List<String> Path();

    ///Launches a process, pipes data to its stdin and out of its stdout.
    static count PipeInOut(const String& ProcessName, const String& Input,
      String& Output, String& Error, String Arg1 = "", String Arg2 = "",
      String Arg3 = "", String Arg4 = "", String Arg5 = "", String Arg6 = "",
      String Arg7 = "", String Arg8 = "", String Arg9 = "", String Arg10 = "",
      String Arg11 = "", String Arg12 = "", String Arg13 = "",
      String Arg14 = "", String Arg15 = "", String Arg16 = "",
      String Arg17 = "", String Arg18 = "", String Arg19 = "",
      String Arg20 = "");

    /**Looks for a process on the path and returns the full path name. If it
    could not find the process on the path, then it returns an empty string.*/
    static String GetProcessOnPath(String ProcessName)
    {
      List<String> PathList = Path();
      for(count i = 0; i < PathList.n(); i++)
      {
        String Filename = PathList[i];
        Filename << ProcessName;
        if(File::Length(Filename) > 0)
          return Filename;
      }
      return "";
    }
  };

  /**An atomic unit of work meant to run as its own process. The job will
  inherit a copy of the memory of the process running the job manager. This
  allows the parent process to share writeable global data (though immutable
  from the perspective of the parent process and other jobs).*/
  class Job
  {
    friend class JobManager;

    static const int PipeEnds  = 2;
    static const count MaxSharedDataSize = 16 * 1024 * 1024;
    count Index;
    int PipeOut[2];
    int PipeError[2];
    integer ProcessID;
    ascii* SharedMemory;
    String StandardError;
    String StandardOutput;

    count Status;
    Timer Stopwatch;
    number Timeout;

    void MapSharedData();
    void UnmapSharedData();
    void SyncSharedData(count Length);

    public:

    Job() : SharedMemory(0), Status(0), Timeout(0) {MapSharedData();}

    virtual ~Job();

    ///Aborts the process.
    static void Abort();

    ///Crashes the process.
    static void Crash();

    ///Returns the amount of elapsed time from process start to stop.
    number ElapsedTime() {return Stopwatch.Elapsed();}

    ///Returns whether the process failed.
    bool   Failed()      {return Signal();}

    ///Gets the value of the shared memory.
    Value GetSharedData()
    {
      Value Result;
      if(SharedMemory)
        JSON::ImportResult(String(SharedMemory), Result);
      return Result;
    }

    ///Returns the job's standard error.
    String GetStandardError() {return StandardError;}

    ///Returns the job's standard output.
    String GetStandardOutput() {return StandardOutput;}

    ///Returns whether the process exceeded its timeout and was killed.
    bool   Hung()        {return Signal() == 9;}

    ///Returns the process result code.
    count  Result()      {return count(Status >> 8);}

    /**Sets the value of the shared memory. If the serialized JSON string
    exceeds the preallocated size of the buffer (currently 16 MB), then the data
    will not be set and subsequent calls to GetSharedData() will return a nil
    value.*/
    void SetSharedData(Value v)
    {
      String s = JSON::ExportResult(v);
      if(s.n() < MaxSharedDataSize)
      {
        Memory::MemCopy(SharedMemory, s.Merge(), s.n() + 1);
        SyncSharedData(s.n() + 1);
      }
    }

    ///Returns the process signal code if it failed and zero otherwise.
    count  Signal()      {return count(Status & 255);}

    ///Returns whether the process exited normally.
    bool   Succeeded()   {return !Failed();}

    ///Process entry-point to implement.
    virtual count Run() = 0;
  };

  ///Runs jobs as process children and handles timeouts, results, and signals.
  class JobManager
  {
    static const int PipeRead  = 0;
    static const int PipeWrite = 1;
    static const int StdIn  = 0;
    static const int StdOut = 1;
    static const int StdErr = 2;

    List<Pointer<Job> > Active, Finished, Queued;

    static count PlatformAvailableCores();
    static void PlatformCloseParentPipes(Pointer<Job> Job_);
    static void PlatformCreatePipes(Pointer<Job> Job_);
    static void PlatformExit(count Result, bool AllowExit = true);
    static integer PlatformFork();
    static void PlatformInstallSignalHandler();
    static void PlatformKill(integer ProcessID);
    static void PlatformPollingWait(integer ProcessID, count& Result,
      count& Status);
    static void PlatformReadPipes(Pointer<Job> Job_, bool NonBlocking);
    static void PlatformSetupChildPipes(Pointer<Job> Job_);
    static void PlatformSetupParentPipes(Pointer<Job> Job_);
    static void PlatformSignalHandler(int SignalCode);

    static number PollTime() {return 0.01f;}

    void CheckForJobStateChange()
    {
      for(count i = 0; i < Active.n(); i++)
      {
        Pointer<Job> Current = Active[i];
        if(Current->Timeout > 0.f and
          Current->ElapsedTime() > Current->Timeout)
        {
          PlatformCloseParentPipes(Current);
          PlatformKill(Current->ProcessID);
          Current->Stopwatch.Stop();
          Current->Status = 9;
          Finished.Push(Current);
          Active.Remove(i--);
          {
            count Total = Queued.n() + Active.n() + Finished.n();
            count CurrentIndex = Finished.z()->Index;
            C::Out() >> "Finished job " << CurrentIndex << " of " << Total;
          }
        }
        else
        {
          count Result, Status;
          PlatformPollingWait(Current->ProcessID, Result, Status);
          PlatformReadPipes(Current, true);
          if(Result > 0)
          {
            PlatformReadPipes(Current, false);
            PlatformCloseParentPipes(Current);
            Current->Stopwatch.Stop();
            Current->Status = count(Status);
            Finished.Push(Current);
            Active.Remove(i--);
            {
              count Total = Queued.n() + Active.n() + Finished.n();
              count CurrentIndex = Finished.z()->Index;
              C::Out() >> "Finished job " << CurrentIndex << " of " << Total;
            }
          }
        }
      }
    }

    void QueueJobs(count MaximumConcurrentJobs)
    {
      while(Queued.n() and Active.n() < MaximumConcurrentJobs)
      {
        count Current = Active.n() + Finished.n() + 1;
        count Total = Queued.n() + Active.n() + Finished.n();
        C::Out() >> "Starting job " << Current << " of " << Total;
        Active.Push(Queued.Pop());
        Active.z()->Stopwatch.Start();
        Active.z()->Index = Current;
        PlatformCreatePipes(Active.z());
        if(integer ProcessID = PlatformFork())
        {
          Active.z()->ProcessID = ProcessID;
          PlatformSetupParentPipes(Active.z());
        }
        else
        {
          PlatformSetupChildPipes(Active.z());
          PlatformInstallSignalHandler();
          PlatformExit(Active.z()->Run());
        }
      }
    }

    public:

    ///Sleeps this process thread for the given number of seconds.
    static void Sleep(number Seconds);

    ///Adds a job to the queue with an optional timeout.
    void Add(Pointer<Job> JobToEnqueue, number SecondsTimeout = 0.f)
    {
      JobToEnqueue->Timeout = SecondsTimeout;
      Queued.Prepend(JobToEnqueue);
    }

    /**Processes the enqueued jobs with an optional number of concurrent jobs.
    If the argument is not a positive number, then the available cores will be
    taken as the number of concurrent jobs.*/
    void Process(count MaximumConcurrentJobs = 0)
    {
      if(MaximumConcurrentJobs <= 0) MaximumConcurrentJobs =
        PlatformAvailableCores();

      while(Queued.n() or Active.n())
      {
        QueueJobs(MaximumConcurrentJobs);
        CheckForJobStateChange();
        Sleep(PollTime());
      }
    }

    ///Summarizes the status of each job process.
    String Summarize() const
    {
      String s;
      for(count i = 0; i < Finished.n(); i++)
      {
        Pointer<Job> J = Finished[i];
        s >> J->GetStandardOutput();
        s >> J->GetStandardError();
        s >> J->GetSharedData();
        s >> "Process " << J->ProcessID;
        if(J->Hung()) s << " hung";
        else if(J->Failed()) s << " failed with signal " << J->Signal();
        else s << " finished with result " << J->Result();
        s << " in " << J->ElapsedTime() << " seconds";
      }
      return s;
    }
  };
}

#ifdef PRIM_COMPILE_INLINE

#ifdef PRIM_ENVIRONMENT_UNIX_LIKE
#ifndef PRIM_ENVIRONMENT_ANDROID
#include <execinfo.h>
#endif
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

namespace PRIM_NAMESPACE
{
  List<String> Shell::Path()
  {
    //Retrieve the path.
    String FullPath;
#ifdef PRIM_ENVIRONMENT_UNIX_LIKE
    FullPath = getenv("PATH"); //String can handle null pointer case.
#endif
    //Canonicalize for parsing.
    String Delimiter = Environment::UnixLike() ? ":" : ";";
    List<String> Result = FullPath.Tokenize(Delimiter, true);

    //Add the path separators.
    for(count i = 0; i < Result.n(); i++)
    {
      String s = Result[i];
      if(Environment::UnixLike())
      {
        s.EraseEnding("/");
        s << "/";
      }
      else
      {
        s.EraseEnding("\\");
        s << "\\";
      }
      Result[i] = s;
    }

    //Return the list of paths.
    return Result;
  }

  count Shell::PipeInOut(const String& ProcessName, const String& Input,
    String& Output, String& Error, String Arg1, String Arg2, String Arg3,
    String Arg4, String Arg5, String Arg6, String Arg7, String Arg8,
    String Arg9, String Arg10, String Arg11, String Arg12, String Arg13,
    String Arg14, String Arg15, String Arg16, String Arg17, String Arg18,
    String Arg19, String Arg20)
  {
#ifdef PRIM_ENVIRONMENT_UNIX_LIKE
    //Max length of process name
    const count MaxProcessNameLength = 1024;

    //Amount to buffer by
    const count BufferAmount = 1024;

    //Name some of the magic numbers to make things clear.
    const int PipeRead  = 0;
    const int PipeWrite = 1;
    const int PipeEnds  = 2;
    const int StdIn  = 0;
    const int StdOut = 1;
    const int StdErr = 2;

    //Clear the output and error.
    Output.Clear();
    Error.Clear();

    //First check to make sure the process name is of a reasonable length.
    if(ProcessName.n() >= MaxProcessNameLength)
      return -1;

    //Create the pipes.
    int ChildIn[PipeEnds];
    int ChildOut[PipeEnds];
    int ChildError[PipeEnds];
    pipe(ChildIn);
    pipe(ChildOut);
    pipe(ChildError);

    //Fork the process.
    pid_t ProcessId = fork();

    if(ProcessId == 0) //Child--------------------------------------------------
    {
      //Set stdin to child in pipe read.
      dup2(ChildIn[PipeRead], StdIn);

      //Close child in pipe write since child does not write to stdin.
      close(ChildIn[PipeWrite]);

      //Close child out pipe read since child does not read to stdout.
      close(ChildOut[PipeRead]);

      //Set stdout to child out pipe write.
      dup2(ChildOut[PipeWrite], StdOut);

      //Close child error pipe read since child does not read to stderr.
      close(ChildError[PipeRead]);

      //Set stderr to child error pipe write.
      dup2(ChildError[PipeWrite], StdErr);

      //Copy process name to char buffer.
      char ProcessNameBuffer[MaxProcessNameLength];
      memset(ProcessNameBuffer, 0, size_t(MaxProcessNameLength));
      memcpy(ProcessNameBuffer, ProcessName.Merge(),
        size_t(ProcessName.n()) + 1);

      //Send with no extra arguments.
      char* ProcessArguments[22] = {ProcessNameBuffer, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

      if(Arg1) ProcessArguments[1] = const_cast<char*>(Arg1.Merge());
      if(Arg2) ProcessArguments[2] = const_cast<char*>(Arg2.Merge());
      if(Arg3) ProcessArguments[3] = const_cast<char*>(Arg3.Merge());
      if(Arg4) ProcessArguments[4] = const_cast<char*>(Arg4.Merge());
      if(Arg5) ProcessArguments[5] = const_cast<char*>(Arg5.Merge());
      if(Arg6) ProcessArguments[6] = const_cast<char*>(Arg6.Merge());
      if(Arg7) ProcessArguments[7] = const_cast<char*>(Arg7.Merge());
      if(Arg8) ProcessArguments[8] = const_cast<char*>(Arg8.Merge());
      if(Arg9) ProcessArguments[9] = const_cast<char*>(Arg9.Merge());
      if(Arg10) ProcessArguments[10] = const_cast<char*>(Arg10.Merge());
      if(Arg11) ProcessArguments[11] = const_cast<char*>(Arg11.Merge());
      if(Arg12) ProcessArguments[12] = const_cast<char*>(Arg12.Merge());
      if(Arg13) ProcessArguments[13] = const_cast<char*>(Arg13.Merge());
      if(Arg14) ProcessArguments[14] = const_cast<char*>(Arg14.Merge());
      if(Arg15) ProcessArguments[15] = const_cast<char*>(Arg15.Merge());
      if(Arg16) ProcessArguments[16] = const_cast<char*>(Arg16.Merge());
      if(Arg17) ProcessArguments[17] = const_cast<char*>(Arg17.Merge());
      if(Arg18) ProcessArguments[18] = const_cast<char*>(Arg18.Merge());
      if(Arg19) ProcessArguments[19] = const_cast<char*>(Arg19.Merge());
      if(Arg20) ProcessArguments[20] = const_cast<char*>(Arg20.Merge());

      //If all goes well the following function does not return.
      execv(ProcessNameBuffer, ProcessArguments);

      //Halt execution immediately as child process never launched.
      exit(1);
    }
    else if(ProcessId > 0) //Parent---------------------------------------------
    {
      //Child does the writing to stdout and stderr.
      close(ChildOut[PipeWrite]);
      close(ChildError[PipeWrite]);

      //Child does the reading from stdin.
      close(ChildIn[PipeRead]);

      //It appears that write will automatically buffer.
      for(count i = 0; i < Input.n(); i += BufferAmount)
      {
        count BytesToSend = Min(BufferAmount, Input.n() - i);
        write(ChildIn[PipeWrite], &Input.Merge()[i], size_t(BytesToSend));
      }

      //Send EOF so the child process can start working with the input.
      close(ChildIn[PipeWrite]);

      //Buffer the child output.
      {
        Array<byte> Buffer;
        Buffer.n(BufferAmount);
        for(;;)
        {
          ssize_t BytesRead = read(ChildOut[PipeRead], &Buffer.a(),
            size_t(Buffer.n()));
          if(BytesRead <= 0) break;
          Output.Append(&Buffer.a(), count(BytesRead));
        }
      }

      //Buffer the child error.
      {
        Array<byte> Buffer;
        Buffer.n(BufferAmount);
        for(;;)
        {
          ssize_t BytesRead = read(ChildError[PipeRead], &Buffer.a(),
            size_t(Buffer.n()));
          if(BytesRead <= 0) break;
          Error.Append(&Buffer.a(), count(BytesRead));
        }
      }

      //Close the last pipe ends and return.
      close(ChildOut[PipeRead]);
      close(ChildError[PipeRead]);

      //Wait for the child to terminate so that it cleans up.
      int Status;
      waitpid(ProcessId, &Status, 0);
      return WIFEXITED(Status) ? count(WEXITSTATUS(Status)) : -1;
    }

    //Fork failed somehow.
    return -1;
#else
    (void)Error; (void)Output; (void)Input; (void)ProcessName;
    return -1;
#endif
  }

  void Job::Abort()
  {
#ifdef PRIM_ENVIRONMENT_UNIX_LIKE
    raise(SIGABRT);
#endif
  }

  void Job::Crash()
  {
#ifdef PRIM_ENVIRONMENT_UNIX_LIKE
    raise(SIGSEGV);
#endif
  }

  void Job::MapSharedData()
  {
#ifdef PRIM_ENVIRONMENT_UNIX_LIKE
    SharedMemory = reinterpret_cast<ascii*>(
      mmap(NULL, size_t(MaxSharedDataSize),
      PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANON, -1, 0));
    SetSharedData(Value());
#endif
  }

  void Job::SyncSharedData(count Length)
  {
#ifdef PRIM_ENVIRONMENT_UNIX_LIKE
    msync(SharedMemory, size_t(Length), MS_SYNC);
#else
    (void)Length;
#endif
  }

  void Job::UnmapSharedData()
  {
#ifdef PRIM_ENVIRONMENT_UNIX_LIKE
    munmap(SharedMemory, size_t(MaxSharedDataSize));
#endif
  }

  Job::~Job() {UnmapSharedData();}

  count JobManager::PlatformAvailableCores()
  {
#ifdef PRIM_ENVIRONMENT_UNIX_LIKE
    return Min(Max(count(sysconf(_SC_NPROCESSORS_ONLN)),
      count(1)), count(64));
#else
    return 1;
#endif
  }

  void JobManager::PlatformCloseParentPipes(Pointer<Job> Job_)
  {
#ifdef PRIM_ENVIRONMENT_UNIX_LIKE
    //Completely close stdout pipe.
    close(Job_->PipeOut[PipeRead]);

    //Completely close stderr pipe.
    close(Job_->PipeError[PipeRead]);
#endif
  }

  void JobManager::PlatformCreatePipes(Pointer<Job> Job_)
  {
#ifdef PRIM_ENVIRONMENT_UNIX_LIKE
    pipe(Job_->PipeOut);
    pipe(Job_->PipeError);
    fcntl(Job_->PipeOut[PipeRead], F_SETFL,
      fcntl(Job_->PipeOut[PipeRead], F_GETFL, 0) | O_NONBLOCK);
    fcntl(Job_->PipeOut[PipeWrite], F_SETFL,
      fcntl(Job_->PipeOut[PipeWrite], F_GETFL, 0) | O_NONBLOCK);
    fcntl(Job_->PipeError[PipeRead], F_SETFL,
      fcntl(Job_->PipeError[PipeRead], F_GETFL, 0) | O_NONBLOCK);
    fcntl(Job_->PipeError[PipeWrite], F_SETFL,
      fcntl(Job_->PipeError[PipeWrite], F_GETFL, 0) | O_NONBLOCK);
#endif
  }

  void JobManager::PlatformExit(count Result, bool AllowExit)
  {
#ifdef PRIM_ENVIRONMENT_UNIX_LIKE
    if(AllowExit)
      exit(int(Result));
#else
    (void)Result;
    (void)AllowExit;
#endif
  }

  integer JobManager::PlatformFork()
  {
#ifdef PRIM_ENVIRONMENT_UNIX_LIKE
    return integer(fork());
#else
    return 0;
#endif
  }

  void JobManager::PlatformInstallSignalHandler()
  {
#ifdef PRIM_ENVIRONMENT_UNIX_LIKE
    signal(SIGBUS, PlatformSignalHandler);
    signal(SIGFPE, PlatformSignalHandler);
    signal(SIGPIPE, PlatformSignalHandler);
    signal(SIGSEGV, PlatformSignalHandler);
#endif
  }

  void JobManager::PlatformKill(integer ProcessID)
  {
#ifdef PRIM_ENVIRONMENT_UNIX_LIKE
    kill(pid_t(ProcessID), SIGKILL);
#else
    (void)ProcessID;
#endif
  }

  void JobManager::PlatformPollingWait(integer ProcessID, count& Result,
    count& Status)
  {
#ifdef PRIM_ENVIRONMENT_UNIX_LIKE
    int StatusRef = 0;
    Result = waitpid(pid_t(ProcessID), &StatusRef, WNOHANG);
    Status = count(StatusRef);
#else
    (void)ProcessID;
    (void)Result;
    (void)Status;
#endif
  }

  void JobManager::PlatformReadPipes(Pointer<Job> Job_, bool NonBlocking)
  {
#ifdef PRIM_ENVIRONMENT_UNIX_LIKE
    const count BufferAmount = 1024;
    const count MaxBuffersToRead = 64;
    Array<byte> Buffer;
    Buffer.n(BufferAmount);
    for(count i = 0; (NonBlocking and i < MaxBuffersToRead) or
      not NonBlocking; i++)
    {
      Buffer.Zero();
      ssize_t BytesRead = read(Job_->PipeOut[PipeRead], &Buffer.a(),
        size_t(Buffer.n()));
      if(BytesRead <= 0) break;
      Job_->StandardOutput.Append(&Buffer.a(), count(BytesRead));
    }
    for(count i = 0; (NonBlocking and i < MaxBuffersToRead) or
      not NonBlocking; i++)
    {
      Buffer.Zero();
      ssize_t BytesRead = read(Job_->PipeError[PipeRead], &Buffer.a(),
        size_t(Buffer.n()));
      if(BytesRead <= 0) break;
      Job_->StandardError.Append(&Buffer.a(), count(BytesRead));
    }
#else
    (void)NonBlocking;
#endif
  }

  void JobManager::PlatformSetupChildPipes(Pointer<Job> Job_)
  {
#ifdef PRIM_ENVIRONMENT_UNIX_LIKE
    //Close child out pipe read since child does not read to stdout.
    close(Job_->PipeOut[PipeRead]);

    //Set stdout to child out pipe write.
    dup2(Job_->PipeOut[PipeWrite], StdOut);

    //Close child error pipe read since child does not read to stderr.
    close(Job_->PipeError[PipeRead]);

    //Set stderr to child error pipe write.
    dup2(Job_->PipeError[PipeWrite], StdErr);
#endif
  }

  void JobManager::PlatformSetupParentPipes(Pointer<Job> Job_)
  {
#ifdef PRIM_ENVIRONMENT_UNIX_LIKE
    //Close parent out pipe write since parent does not write to stdout.
    close(Job_->PipeOut[PipeWrite]);

    //Close parent error pipe write since parent does not write to stderr.
    close(Job_->PipeError[PipeWrite]);
#endif
  }

  void JobManager::PlatformSignalHandler(int SignalCode)
  {
#ifdef PRIM_ENVIRONMENT_UNIX_LIKE
    fprintf(stderr, "Job caught fatal error: signal %d:\n", SignalCode);
#ifndef PRIM_ENVIRONMENT_ANDROID
    const int MaxItems = 1024;
    void* Items[MaxItems];
    backtrace_symbols_fd(Items, backtrace(Items, MaxItems), STDERR_FILENO);
#endif
    Job::Abort();
#else
    (void)SignalCode;
#endif
  }

  void JobManager::Sleep(number Seconds)
  {
#ifdef PRIM_ENVIRONMENT_UNIX_LIKE
    timespec SleepTime =
      {time_t(Floor(Seconds)), long(FractionalPart(Seconds) * 1.e9f)};
    nanosleep(&SleepTime, 0);
#else
    (void)Seconds;
#endif
  }
}
#endif
#endif
#endif
