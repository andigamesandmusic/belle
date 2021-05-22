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

#ifndef PRIM_INCLUDE_MOD_THREAD_H
#define PRIM_INCLUDE_MOD_THREAD_H
#ifdef PRIM_WITH_THREAD //Depends on non-portable system headers.

#ifndef PRIM_LIBRARY
#error This file can not be included individually. Include prim.h instead.
#endif

/*
Module Configuration
================================================================================
Thread support was introduced in C++11 via std::thread. They can be enabled
using:

//#define PRIM_THREAD_USE_STD_THREAD

The std::thread implementation is known to work with the following:

Clang: clang++ -stdlib=libc++ -std=c++11 -o BinaryName SourceFile1.cpp ...

GCC: g++ -std=c++11 -pthread -o BinaryName SourceFile1.cpp ...

Otherwise:
1) If you are on a Windows system then Windows API threads will be used.
2) If you are on a Unix-like system then Pthreads will be used. You may need to
   compile with the -pthread or -pthreads compiler option.
================================================================================
End Module Configuration
*/

#ifndef PRIM_THREAD_USE_STD_THREAD
#ifdef PRIM_ENVIRONMENT_WINDOWS
//Use thread API from <windows.h>
#define PRIM_THREAD_USE_WINDOWS
#elif defined(PRIM_ENVIRONMENT_UNIX_LIKE)
//Use thread API from <pthread.h>
#define PRIM_THREAD_USE_PTHREAD
#endif
#endif

namespace PRIM_NAMESPACE
{
  ///Recursive thread mutex maintaining a lock and unlock count.
  class Mutex
  {
    ///The mutex object. Using void* avoids including other headers here.
    void* RecursiveMutex;

    public:

    ///Initializes the mutex with a lock count of zero.
    Mutex();

    ///Releases the mutex. The lock count must be zero at this point.
    ~Mutex();

    ///Blocks until a lock is established, incrementing the lock count.
    void Lock();

    ///Attempts to establish a lock but returns false if unsuccessful.
    bool TryLock();

    /**Decrements the lock count. If the lock count is greater than the unlock
    count, then the lock is still in effect (since it is a recursive mutex).*/
    void Unlock();
  };

  ///Locks a given mutex for the lifetime of the object.
  class Lock
  {
    Mutex& ScopedMutex;

    public:

    /**Locks the given mutex using Mutex::Lock(). Object instantiation is
    blocking, so the constructor does not return until a lock has been
    achieved.*/
    Lock(Mutex& ScopedMutex_) : ScopedMutex(ScopedMutex_)
    {
      ScopedMutex.Lock();
    }

    ///Deleted assignment operator. Do not use.
    Lock& operator = (const Lock&) PRIM_11_DELETE_DEFAULT;

    ///Unlocks the mutex using Mutex::Unlock().
    ~Lock()
    {
      ScopedMutex.Unlock();
    }
  };

  ///Base class from which threads inherit and implement their run loops.
  class Thread
  {
    ///The platform thread object.
    void* ThreadObject; //(void*) avoids including thread headers here.

    ///Thread state signal
    Mutex InternalMutex;

    //Thread state flags
    bool Running; PRIM_PAD(bool)
    bool Ending; PRIM_PAD(bool)

  #ifdef PRIM_THREAD_USE_PTHREAD
    static void* BeginThread(void* VoidThreadPointer);
  #elif defined(PRIM_THREAD_USE_WINDOWS)
    static unsigned long __stdcall BeginThread(void* VoidThreadPointer);
  #else
    static void BeginThread(Thread* ThreadPointer);
  #endif

    protected:

    /**Overload and place thread contents inside. If the thread can be
    interrupted, then make sure to call IsEnding() frequently to see whether or
    not the method should return. You can signal a thread to end by calling
    SignalEnd().*/
    virtual void Run() = 0;

    public:

    ///Initializes but does not begin the thread.
    Thread();

    /**Releases the thread. WaitToEnd() must be called before this time or
    else a crash may occur due to members in the subclass not being available.*/
    virtual ~Thread();

    /**Begins the thread. If the thread is already running, then it will wait
    for the current thread instance to end before running the new instance.*/
    bool Begin();

    /**Determines whether the end-state was signaled by SignalEnd(). A thread
    that receives true from this method should clean-up and end Run() as quickly
    as possible.*/
    bool IsEnding()
    {
      Lock Lock(InternalMutex);
      return Ending;
    }

    /**Determines whether or not the thread is still running. Note that a thread
    is considered to be running until WaitToEnd() is called. Even if the Run()
    method has already returned, the thread is still considered active.*/
    bool IsRunning()
    {
      Lock Lock(InternalMutex);
      return Running;
    }

    /**Indicates to the thread that it should end as soon as possible. This is
    achieved by periodically checking IsEnding() in the Run() method and exiting
    if necessary.*/
    void SignalEnd()
    {
      Lock Lock(InternalMutex);
      Ending = true;
    }

    ///Busy sleeps current thread by a certain amount.
    static void Sleep(number Seconds);

    /**The proper way to clean up a thread. It automatically signals the thread
    to exit using SignalEnd(), and it will block the caller thread until this
    thread exits.*/
    void WaitToEnd();

    ///Hints the thread scheduler to allow other threads to run.
    void Yield();
  };

  ///Mechanism for signaling and waiting on a signal.
  class Event
  {
    bool Signaled; PRIM_PAD(bool)
    bool ManuallyClearSignal; PRIM_PAD(bool)
    count Waiting;
    count ActualWaiting;
    void* SignalMutex;
    void* ProducerCondition;
    void* ConsumerCondition;

    void Initialize();
    void Cleanup();

    public:

    Event(bool StartSignaled, bool ManuallyClearSignal_) : Signaled(false),
      ManuallyClearSignal(ManuallyClearSignal_), Waiting(0), ActualWaiting(0),
      SignalMutex(0), ProducerCondition(0), ConsumerCondition(0)
    {
      Initialize();
      if(StartSignaled)
        Signal();
    }

    Event() : Signaled(false), ManuallyClearSignal(false), Waiting(0),
      ActualWaiting(0), SignalMutex(0), ProducerCondition(0),
      ConsumerCondition(0)
    {
      Initialize();
    }

    ~Event()
    {
      Cleanup();
    }

    void Wait();
    void WaitAtMost(number Seconds);
    void Signal();
    void Clear();
  };
}

#ifdef PRIM_COMPILE_INLINE

#ifdef PRIM_THREAD_USE_PTHREAD
#include <pthread.h> //Pthreads
#include <time.h>
#include <sys/time.h>
#include <errno.h>
#elif defined(PRIM_THREAD_USE_WINDOWS)
#include <windows.h> //Windows API threads
#else
#include <thread>    //std::thread
#endif

namespace PRIM_NAMESPACE
{
  void Event::Initialize()
  {
#ifdef PRIM_THREAD_USE_PTHREAD
    pthread_cond_t* PTProducerCondition = new pthread_cond_t;
    pthread_cond_t* PTConsumerCondition = new pthread_cond_t;
    pthread_mutex_t* PTMutex = new pthread_mutex_t;
    pthread_cond_init(PTProducerCondition, 0);
    pthread_cond_init(PTConsumerCondition, 0);
    pthread_mutex_init(PTMutex, 0);
    ProducerCondition = PTProducerCondition;
    ConsumerCondition = PTConsumerCondition;
    SignalMutex = PTMutex;
    ActualWaiting = Waiting = 0;
    Signaled = false;
#elif defined(PRIM_THREAD_USE_WINDOWS)
#else
#endif
  }

  void Event::Cleanup()
  {
    pthread_cond_t* PTProducerCondition = reinterpret_cast<pthread_cond_t*>(
      ProducerCondition);
    pthread_cond_t* PTConsumerCondition = reinterpret_cast<pthread_cond_t*>(
      ConsumerCondition);
    pthread_mutex_t* PTMutex = reinterpret_cast<pthread_mutex_t*>(SignalMutex);

    pthread_mutex_lock(PTMutex);
    if(Waiting)
    {
      std::cerr <<
        "\nError: Threads are waiting on deallocating event." <<
        "\nThis behavior is undefined and will likely cause a crash.";
      std::cerr.flush();
    }
    pthread_mutex_unlock(PTMutex);

    pthread_cond_destroy(PTProducerCondition);
    pthread_cond_destroy(PTConsumerCondition);
    pthread_mutex_destroy(PTMutex);
    delete PTProducerCondition;
    delete PTConsumerCondition;
    delete PTMutex;
  }

  void Event::Wait()
  {
    pthread_cond_t* PTProducerCondition =
      reinterpret_cast<pthread_cond_t*>(ProducerCondition);
    pthread_cond_t* PTConsumerCondition =
      reinterpret_cast<pthread_cond_t*>(ConsumerCondition);
    pthread_mutex_t* PTMutex =
      reinterpret_cast<pthread_mutex_t*>(SignalMutex);

    pthread_mutex_lock(PTMutex);
    if(Signaled)
      Signaled = ManuallyClearSignal;
    else
    {
      Waiting++;
      while(not Signaled)
        pthread_cond_wait(PTConsumerCondition, PTMutex);
      Waiting--;
      if(not --ActualWaiting)
      {
        Signaled = ManuallyClearSignal;
        pthread_cond_signal(PTProducerCondition);
      }
    }
    pthread_mutex_unlock(PTMutex);
  }

  void Event::WaitAtMost(number Seconds)
  {
    struct timespec AbsoluteExpiration;
    {
      struct timeval tv;
      gettimeofday(&tv, 0);
      number AbsoluteTime = number(tv.tv_sec) + number(tv.tv_usec) / 1.E6f;
      AbsoluteTime += Seconds;
      AbsoluteExpiration.tv_sec = time_t(Floor(AbsoluteTime));
      AbsoluteExpiration.tv_nsec = long(FractionalPart(AbsoluteTime) * 1.E9f);
    }

    pthread_cond_t* PTProducerCondition =
      reinterpret_cast<pthread_cond_t*>(ProducerCondition);
    pthread_cond_t* PTConsumerCondition =
      reinterpret_cast<pthread_cond_t*>(ConsumerCondition);
    pthread_mutex_t* PTMutex =
      reinterpret_cast<pthread_mutex_t*>(SignalMutex);

    pthread_mutex_lock(PTMutex);
    if(Signaled)
      Signaled = ManuallyClearSignal;
    else
    {
      Waiting++;
      bool WasTimeout = false;
      while(not Signaled)
      {
        if(pthread_cond_timedwait(PTConsumerCondition, PTMutex,
          &AbsoluteExpiration) == ETIMEDOUT)
        {
          WasTimeout = true;
          break;
        }
      }
      Waiting--;
      if(not WasTimeout and not --ActualWaiting)
      {
        Signaled = ManuallyClearSignal;
        pthread_cond_signal(PTProducerCondition);
      }
    }
    pthread_mutex_unlock(PTMutex);
  }

  void Event::Signal()
  {
    pthread_cond_t* PTProducerCondition =
      reinterpret_cast<pthread_cond_t*>(ProducerCondition);
    pthread_cond_t* PTConsumerCondition =
      reinterpret_cast<pthread_cond_t*>(ConsumerCondition);
    pthread_mutex_t* PTMutex =
      reinterpret_cast<pthread_mutex_t*>(SignalMutex);

    pthread_mutex_lock(PTMutex);
    Signaled = true;
    if(Waiting)
    {
      ActualWaiting = Waiting;
      pthread_cond_broadcast(PTConsumerCondition);
      while(ActualWaiting)
        pthread_cond_wait(PTProducerCondition, PTMutex);
    }
    pthread_mutex_unlock(PTMutex);
  }

  void Event::Clear()
  {
    pthread_mutex_t* PTMutex =
      reinterpret_cast<pthread_mutex_t*>(SignalMutex);

    pthread_mutex_lock(PTMutex);
    if(ManuallyClearSignal)
      Signaled = false;
    pthread_mutex_unlock(PTMutex);
  }

  Mutex::Mutex()
  {
#ifdef PRIM_THREAD_USE_PTHREAD
    //Create the mutex attributes object for a recursive mutex.
    pthread_mutexattr_t MutexAttributes;
    pthread_mutexattr_init(&MutexAttributes);
    pthread_mutexattr_settype(&MutexAttributes, PTHREAD_MUTEX_RECURSIVE);

    //Initialize the mutex using the attributes object.
    RecursiveMutex = new pthread_mutex_t;
    pthread_mutex_init(reinterpret_cast<pthread_mutex_t*>(RecursiveMutex),
      &MutexAttributes);
#elif defined(PRIM_THREAD_USE_WINDOWS)
    RecursiveMutex = new HANDLE;
    *reinterpret_cast<HANDLE*>(RecursiveMutex) = CreateMutex(0, FALSE, 0);
#else
    RecursiveMutex = new std::recursive_mutex;
#endif
  }

  Mutex::~Mutex()
  {
#ifdef PRIM_THREAD_USE_PTHREAD
    delete reinterpret_cast<pthread_mutex_t*>(RecursiveMutex);
#elif defined(PRIM_THREAD_USE_WINDOWS)
    CloseHandle(*reinterpret_cast<HANDLE*>(RecursiveMutex));
#else
    delete reinterpret_cast<std::recursive_mutex*>(RecursiveMutex);
#endif
  }

  void Mutex::Lock()
  {
#ifdef PRIM_THREAD_USE_PTHREAD
    pthread_mutex_lock(reinterpret_cast<pthread_mutex_t*>(RecursiveMutex));
#elif defined(PRIM_THREAD_USE_WINDOWS)
    WaitForSingleObject(*reinterpret_cast<HANDLE*>(RecursiveMutex), INFINITE);
#else
    reinterpret_cast<std::recursive_mutex*>(RecursiveMutex)->lock();
#endif
  }

  bool Mutex::TryLock()
  {
#ifdef PRIM_THREAD_USE_PTHREAD
    return pthread_mutex_trylock(
      reinterpret_cast<pthread_mutex_t*>(RecursiveMutex)) == 0;
#elif defined(PRIM_THREAD_USE_WINDOWS)
    return WaitForSingleObject(
      *reinterpret_cast<HANDLE*>(RecursiveMutex), 0) == WAIT_OBJECT_0;
#else
    return reinterpret_cast<std::recursive_mutex*>(RecursiveMutex)->try_lock();
#endif
  }

  void Mutex::Unlock()
  {
#ifdef PRIM_THREAD_USE_PTHREAD
    pthread_mutex_unlock(reinterpret_cast<pthread_mutex_t*>(RecursiveMutex));
#elif defined(PRIM_THREAD_USE_WINDOWS)
    ReleaseMutex(*reinterpret_cast<HANDLE*>(RecursiveMutex));
#else
    reinterpret_cast<std::recursive_mutex*>(RecursiveMutex)->unlock();
#endif
  }

#ifdef PRIM_THREAD_USE_PTHREAD
  void* Thread::BeginThread(void* VoidThreadPointer)
  {
    Thread* ThreadPointer = reinterpret_cast<Thread*>(VoidThreadPointer);
    {Lock(ThreadPointer->InternalMutex);} //Wait for initialization
    ThreadPointer->Run();
    return 0;
  }
#elif defined(PRIM_THREAD_USE_WINDOWS)
  unsigned long __stdcall Thread::BeginThread(void* VoidThreadPointer)
  {
    Thread* ThreadPointer = reinterpret_cast<Thread*>(VoidThreadPointer);
    {Lock(ThreadPointer->InternalMutex);} //Wait for initialization
    ThreadPointer->Run();
    return 0;
  }
#else
  void Thread::BeginThread(Thread* ThreadPointer)
  {
    {Lock(ThreadPointer->InternalMutex);} //Wait for initialization
    ThreadPointer->Run();
  }
#endif

  Thread::Thread() : Running(false), Ending(false)
  {
#ifdef PRIM_THREAD_USE_PTHREAD
    ThreadObject = new pthread_t;
#elif defined(PRIM_THREAD_USE_WINDOWS)
    ThreadObject = new HANDLE;
#else
    ThreadObject = new std::thread;
#endif
  }

  Thread::~Thread()
  {
    /*This is to prevent a crash, but all threads should be ended before they
    go out of scope or are deleted. It is really too late here to end the thread
    since the subclass has had member data deleted already.*/
    WaitToEnd();
#ifdef PRIM_THREAD_USE_PTHREAD
    delete reinterpret_cast<pthread_t*>(ThreadObject);
#elif defined(PRIM_THREAD_USE_WINDOWS)
    delete reinterpret_cast<HANDLE*>(ThreadObject);
#else
    delete reinterpret_cast<std::thread*>(ThreadObject);
#endif
  }

  bool Thread::Begin()
  {
    WaitToEnd();
    {
      Lock Lock(InternalMutex);
      Running = true;
#ifdef PRIM_THREAD_USE_PTHREAD
      pthread_attr_t ThreadAttributes;
      pthread_attr_init(&ThreadAttributes);
      pthread_attr_setdetachstate(&ThreadAttributes, PTHREAD_CREATE_JOINABLE);
      pthread_create(reinterpret_cast<pthread_t*>(ThreadObject),
        &ThreadAttributes, BeginThread, reinterpret_cast<void*>(this));
#elif defined(PRIM_THREAD_USE_WINDOWS)
      DWORD ThreadID = 0;
      *reinterpret_cast<HANDLE*>(ThreadObject) = CreateThread(NULL, 0,
      reinterpret_cast<LPTHREAD_START_ROUTINE>(Thread::BeginThread),
      reinterpret_cast<LPVOID>(this), 0, &ThreadID);
#else
      *reinterpret_cast<std::thread*>(ThreadObject) =
      std::thread(BeginThread, this);
#endif
      return true;
    }
  }

  void Thread::WaitToEnd()
  {
    if(not IsRunning())
      return;
    SignalEnd();
#ifdef PRIM_THREAD_USE_PTHREAD
    pthread_join(*reinterpret_cast<pthread_t*>(ThreadObject), 0);
#elif defined(PRIM_THREAD_USE_WINDOWS)
    WaitForSingleObject(*reinterpret_cast<HANDLE*>(ThreadObject), INFINITE);
#else
    reinterpret_cast<std::thread*>(ThreadObject)->join();
#endif
    {
      Lock Lock(InternalMutex);
      Ending = false;
      Running = false;
    }
  }

  void Thread::Sleep(number Seconds)
  {
    (void)Seconds;
#ifdef PRIM_THREAD_USE_PTHREAD
    timespec SleepTime;
    SleepTime.tv_sec = time_t(Floor(Seconds));
    SleepTime.tv_nsec = long(FractionalPart(Seconds) * 1000000000.f);
    nanosleep(&SleepTime, 0);
#endif
  }

  void Thread::Yield()
  {
#ifdef PRIM_THREAD_USE_PTHREAD
#ifdef PRIM_ENVIRONMENT_APPLE
    pthread_yield_np();
#endif
#elif defined(PRIM_THREAD_USE_WINDOWS)
    Sleep(0);
#else
    std::this_thread::yield();
#endif
  }
}
#endif

//Undefine implementation-specific macros.
#undef PRIM_THREAD_USE_STD_THREAD
#undef PRIM_THREAD_USE_WINDOWS
#undef PRIM_THREAD_USE_PTHREAD
#endif
#endif
