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

#ifdef PRIM_WITH_QUEUE
#ifndef PRIM_INCLUDE_MOD_QUEUE_H
#define PRIM_INCLUDE_MOD_QUEUE_H

#ifndef PRIM_LIBRARY
#error This file can not be included individually. Include prim.h instead.
#endif

namespace PRIM_NAMESPACE
{
#ifdef PRIM_WITH_THREAD
/// Queue for processing tasks in a separate thread.

template<class TaskType, class ReturnType>
class SerialQueue : protected Thread
{
  Mutex ProcessStackMutex;
  Mutex ExecuteMutex;
  Mutex WaitWarningMutex;
  List<TaskType> ProcessStack;
  List<Timer> ProcessTimerStack;
  Event ProcessStackWait;
  bool Exiting; PRIM_PAD(bool)
  number WaitWarningThreshold;

  void CheckForLongTime(number Duration, String Activity)
  {
    if(Duration > GetWaitWarningThreshold())
    {
      C::Out()++;
      C::Out()++;
      C::Out() >> "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!";
      C::Out() >> "Warning: task spent " << Duration << " seconds " << Activity;
      C::Out() >> "This may cause thread blocking in real-time applications";
      C::Out() >> "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!";
      C::Out()++;
      C::Out()++;
    }
  }

  bool MustExit()
  {
    bool IsExiting;
    {
      Lock L(ProcessStackMutex);
      IsExiting = Exiting;
    }
    return IsExiting;
  }

  void SetMustExit()
  {
    Lock L(ProcessStackMutex);
    Exiting = true;
  }

  void Run()
  {
    bool HasTasksToProcess = true;
    while((not IsEnding() or HasTasksToProcess) and not MustExit())
    {
      TaskType CurrentTaskToExecute;
      Timer TaskTimer;
      bool PoppedTask = false;
      {
        Lock StackLock(ProcessStackMutex);
        if(ProcessStack.n())
        {
          CurrentTaskToExecute = ProcessStack.Pop();
          TaskTimer = ProcessTimerStack.Pop();
          PoppedTask = true;
        }
        HasTasksToProcess = ProcessStack.n();
      }
      if(PoppedTask)
      {
        Lock ExecuteLock(ExecuteMutex);
        CheckForLongTime(TaskTimer.Stop(), "asynchronously waiting");
        TaskTimer.Start();
        Process(CurrentTaskToExecute);
        CheckForLongTime(TaskTimer.Stop(), "asynchronously processing");
      }
      if(not IsEnding() and not HasTasksToProcess and not MustExit())
        ProcessStackWait.Wait();
    }
  }

  public:

  /**Adds task to serial queue and processes after enqueued tasks finish. The
  serial queue thread is used.*/
  void ExecuteAsynchronously(const TaskType& Task)
  {
    if(IsRunning())
    {
      Lock StackLock(ProcessStackMutex);
      ProcessStack.Prepend(Task);
      Timer ProcessTimer;
      ProcessTimer.Start();
      ProcessTimerStack.Prepend(ProcessTimer);
      ProcessStackWait.Signal();
    }
  }

  ///Processes task immediately in the thread of the callee.
  ReturnType ExecuteSynchronously(const TaskType& Task)
  {
    //Now execute the synchronous task and return the result.
    if(IsRunning())
    {
      Timer TaskTimer;
      TaskTimer.Start();
      Lock ExecuteLock(ExecuteMutex);
      CheckForLongTime(TaskTimer.Stop(), "synchronously waiting");
      TaskTimer.Start();
      ReturnType ReturnValue = Process(Task);
      CheckForLongTime(TaskTimer.Stop(), "synchronously processing");
      return ReturnValue;
    }
    return ReturnType();
  }

  /**Processes task if number of tasks waiting is no more than threshold.
  Returns whether the task was executed.*/
  bool TrySynchronously(const TaskType& Task, count MaxWaitingTasks)
  {
    bool WillExecute;
    {
      Lock StackLock(ProcessStackMutex);
      WillExecute = ProcessStack.n() <= MaxWaitingTasks;
    }

    if(WillExecute)
      ExecuteSynchronously(Task);

    return WillExecute;
  }

  /**Processes task if number of tasks waiting is no more than threshold.
  Returns whether the task was executed.*/
  bool TryAsynchronously(const TaskType& Task, count MaxWaitingTasks)
  {
    bool WillExecute;
    {
      Lock StackLock(ProcessStackMutex);
      WillExecute = ProcessStack.n() <= MaxWaitingTasks;
    }

    if(WillExecute)
      ExecuteAsynchronously(Task);

    return WillExecute;
  }

  ///Sets maximum amount of time a process can wait before triggering a warning.
  void SetWaitWarningThreshold(number WaitWarningThreshold_)
  {
    if(WaitWarningThreshold_ <= 0.f)
      WaitWarningThreshold_ = Limits<number>::Infinity();
    Lock WaitWarningLock(WaitWarningMutex);
    WaitWarningThreshold = WaitWarningThreshold_;
  }

  ///Gets maximum amount of time a process can wait before triggering a warning.
  number GetWaitWarningThreshold()
  {
    Lock WaitWarningLock(WaitWarningMutex);
    number WaitWarningThreshold_ = WaitWarningThreshold;
    return WaitWarningThreshold_;
  }

  ///Starts the serial queue thread.
  void Start()
  {
    if(not IsRunning())
      Exiting = false, Begin();
  }

  ///Stops and waits for the serial queue thread to finish.
  void Stop(bool DoNotFinishTasks = false)
  {
    if(IsRunning())
    {
      if(DoNotFinishTasks)
      {
        Lock StackLock(ProcessStackMutex);
        ProcessStack.RemoveAll();
        ProcessTimerStack.RemoveAll();
      }
      SetMustExit();
      ProcessStackWait.Signal();
      WaitToEnd();
    }
  }

  SerialQueue() : Exiting(false),
    WaitWarningThreshold(Limits<number>::Infinity()) {}

  virtual ~SerialQueue()
  {
    //Technically, it is too late to stop, but try and prevent a crash anyway.
    if(IsRunning())
    {
      C::Out()++;
      C::Red();
      C::Error() >> "Warning: serial queue not explicitly stopped";
      C::Reset();
      C::Out()++;
      Stop(true);
    }
  }

  ///Override this function to process the task.
  virtual ReturnType Process(const TaskType& Task) = 0;
};
#endif
}
#endif
#endif
