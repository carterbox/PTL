//
// MIT License
// Copyright (c) 2018 Jonathan R. Madsen
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software. THE SOFTWARE IS PROVIDED
// "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
// LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
// PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
// HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
// ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
// class description:
//   This is a class for run control in Tasking for multi-threaded runs
//   It extends RunManager re-implementing multi-threaded behavior in
//   key methods. See documentation for RunManager
//   Users initializes an instance of this class instead of RunManager
//   to start a multi-threaded simulation.

#pragma once

#include "PTL/TBBTaskGroup.hh"
#include "PTL/TaskGroup.hh"
#include "PTL/ThreadPool.hh"
#include "PTL/Threading.hh"
#include "PTL/VUserTaskQueue.hh"

#include <list>
#include <map>

class ThreadPool;
class TaskManager;

//======================================================================================//

class TaskRunManager
{
public:
    typedef TaskGroup<void>    RunTaskGroup;
    typedef TBBTaskGroup<void> RunTaskGroupTBB;

public:
    // Parameters:
    //      taskQueue: provide a custom task queue
    //      useTBB: only relevant if PTL_USE_TBB defined
    //      grainsize:  0 = auto
    TaskRunManager(bool useTBB = false);
    virtual ~TaskRunManager();

public:
    virtual int GetNumberOfThreads() const
    {
        return (threadPool) ? threadPool->size() : 0;
    }
    virtual size_t GetNumberActiveThreads() const
    {
        return (threadPool) ? threadPool->size() : 0;
    }

public:
    // Inherited methods to re-implement for MT case
    virtual void Initialize(uint64_t n = std::thread::hardware_concurrency());
    virtual void Terminate();
    virtual void Wait();
    ThreadPool*  GetThreadPool() const { return threadPool; }
    TaskManager* GetTaskManager() const { return taskManager; }
    bool         IsInitialized() const { return isInitialized; }
    int          GetVerbose() const { return verbose; }
    void         SetVerbose(int val) { verbose = val; }

private:
    // grainsize

public:  // with description
    // Singleton implementing master thread behavior
    static TaskRunManager*  GetInstance(bool useTBB = false);
    static TaskRunManager*& GetMasterRunManager(bool useTBB = false);

private:
    static TaskRunManager*& GetPrivateMasterRunManager(bool init, bool useTBB = false);

protected:
    // Barriers: synch points between master and workers
    bool             isInitialized;
    int              verbose;
    uint64_t         nworkers;
    VUserTaskQueue*  taskQueue;
    ThreadPool*      threadPool;
    TaskManager*     taskManager;
    RunTaskGroup*    workTaskGroup;
    RunTaskGroupTBB* workTaskGroupTBB;

public:
    virtual void TiMemoryReport(std::string fname = "", bool echo_stdout = true) const;
};
