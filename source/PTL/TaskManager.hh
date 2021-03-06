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
// ---------------------------------------------------------------
// Tasking class header file
//
// Class Description:
//
// This file creates a class for handling the wrapping of functions
// into task objects and submitting to thread pool
//
// ---------------------------------------------------------------
// Author: Jonathan Madsen (Feb 13th 2018)
// ---------------------------------------------------------------

#pragma once

#include "PTL/TBBTaskGroup.hh"
#include "PTL/Task.hh"
#include "PTL/TaskGroup.hh"
#include "PTL/ThreadPool.hh"
#include "PTL/Threading.hh"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <iomanip>

//======================================================================================//

class TaskManager
{
public:
    typedef TaskManager           this_type;
    typedef ThreadPool::size_type size_type;

    template <bool _Bp, typename _Tp = void>
    using enable_if_t = typename std::enable_if<_Bp, _Tp>::type;

public:
    // Constructor and Destructors
    explicit TaskManager(ThreadPool*);
    virtual ~TaskManager();

    TaskManager(const this_type&) = delete;
    TaskManager(this_type&&)      = default;
    this_type& operator=(const this_type&) = delete;
    this_type& operator=(this_type&&) = default;

public:
    /// get the singleton pointer
    static TaskManager* GetInstance();
    static TaskManager* GetInstanceIfExists();
    static unsigned     ncores() { return std::thread::hardware_concurrency(); }

public:
    //------------------------------------------------------------------------//
    // return the thread pool
    inline ThreadPool* thread_pool() const { return m_pool; }

    //------------------------------------------------------------------------//
    // return the number of threads in the thread pool
    inline size_type size() const { return m_pool->size(); }

    //------------------------------------------------------------------------//
    // kill all the threads
    inline void finalize() { m_pool->destroy_threadpool(); }
    //------------------------------------------------------------------------//

public:
    //------------------------------------------------------------------------//
    // direct insertion of a task
    //------------------------------------------------------------------------//
    template <typename... _Args>
    void exec(Task<_Args...>* _task)
    {
        typedef Task<_Args...>             task_type;
        typedef std::shared_ptr<task_type> task_pointer;

        m_pool->add_task(task_pointer(_task));
    }

    //------------------------------------------------------------------------//
    // direct insertion of a packaged_task
    //------------------------------------------------------------------------//
    template <typename _Ret, typename _Func, typename... _Args>
    std::future<_Ret> async(const _Func& func, _Args... args)
    {
        typedef PackagedTask<_Ret, _Ret, _Args...> task_type;
        typedef std::shared_ptr<task_type>         task_pointer;

        task_pointer      _ptask(new task_type(func, std::forward<_Args>(args)...));
        std::future<_Ret> _f = _ptask->get_future();
        m_pool->add_task(_ptask);
        return _f;
    }
    //------------------------------------------------------------------------//
    template <typename _Ret, typename _Func>
    std::future<_Ret> async(const _Func& func)
    {
        typedef PackagedTask<_Ret, _Ret>   task_type;
        typedef std::shared_ptr<task_type> task_pointer;

        task_pointer      _ptask(new task_type(func));
        std::future<_Ret> _f = _ptask->get_future();
        m_pool->add_task(_ptask);
        return _f;
    }
    //------------------------------------------------------------------------//

public:
    //------------------------------------------------------------------------//
    // public wrap functions
    //------------------------------------------------------------------------//
    template <typename _Ret, typename _Arg, typename _Func, typename... _Args>
    std::shared_ptr<Task<_Ret, _Arg, _Args...>> wrap(TaskGroup<_Ret, _Arg>& tg,
                                                     const _Func& func, _Args... args)
    {
        typedef Task<_Ret, _Arg, _Args...> task_type;
        typedef std::shared_ptr<task_type> task_pointer;

        return tg.store(
            task_pointer(new task_type(tg, func, std::forward<_Args>(args)...)));
    }
    //------------------------------------------------------------------------//
    template <typename _Ret, typename _Arg, typename _Func>
    std::shared_ptr<Task<_Ret, _Arg>> wrap(TaskGroup<_Ret, _Arg>& tg, const _Func& func)
    {
        typedef Task<_Ret, _Arg>           task_type;
        typedef std::shared_ptr<task_type> task_pointer;

        return tg.store(task_pointer(new task_type(tg, func)));
    }

public:
    //------------------------------------------------------------------------//
    // public exec functions
    //------------------------------------------------------------------------//
    template <typename _Ret, typename _Arg, typename _Func, typename... _Args>
    void exec(TaskGroup<_Ret, _Arg>& tg, const _Func& func, _Args... args)
    {
        typedef Task<_Ret, _Arg, _Args...> task_type;
        typedef std::shared_ptr<task_type> task_pointer;

        m_pool->add_task(tg.store(
            task_pointer(new task_type(tg, func, std::forward<_Args>(args)...))));
    }
    //------------------------------------------------------------------------//
    template <typename _Ret, typename _Arg, typename _Func>
    void exec(TaskGroup<_Ret, _Arg>& tg, const _Func& func)
    {
        typedef Task<_Ret, _Arg>           task_type;
        typedef std::shared_ptr<task_type> task_pointer;

        m_pool->add_task(tg.store(task_pointer(new task_type(tg, func))));
    }
    //------------------------------------------------------------------------//
    template <typename _Ret, typename _Arg, typename _Func, typename... _Args>
    void rexec(TaskGroup<_Ret, _Arg>& tg, const _Func& func, _Args... args)
    {
        typedef Task<_Ret, _Arg, _Args...> task_type;
        typedef std::shared_ptr<task_type> task_pointer;

        if(m_pool->query_create_task())
            m_pool->add_task(tg.store(
                task_pointer(new task_type(tg, func, std::forward<_Args>(args)...))));
        else
        {
            PackagedTask<_Ret, _Arg, _Args...> _ptask(func, std::forward<_Args>(args)...);
            auto                               _f = _ptask.get_future();
            tg.add(std::move(_f));
            _ptask();
        }
    }
    //------------------------------------------------------------------------//
    template <typename _Ret, typename _Arg, typename _Func>
    void rexec(TaskGroup<_Ret, _Arg>& tg, const _Func& func)
    {
        typedef Task<_Ret, _Arg>           task_type;
        typedef std::shared_ptr<task_type> task_pointer;

        if(m_pool->query_create_task())
            m_pool->add_task(tg.store(task_pointer(new task_type(tg, func))));
        else
            tg.add(async<_Arg>(func));
    }
    //------------------------------------------------------------------------//
    // public exec functions (void specializations)
    //------------------------------------------------------------------------//
    template <typename _Func, typename... _Args>
    void rexec(TaskGroup<void, void>& tg, const _Func& func, _Args... args)
    {
        typedef Task<void, void, _Args...> task_type;
        typedef std::shared_ptr<task_type> task_pointer;

        if(m_pool->query_create_task())
            m_pool->add_task(tg.store(
                task_pointer(new task_type(tg, func, std::forward<_Args>(args)...))));
        else
            func(std::forward<_Args>(args)...);
    }
    //------------------------------------------------------------------------//
    template <typename _Func>
    void rexec(TaskGroup<void, void>& tg, const _Func& func)
    {
        typedef Task<void, void>           task_type;
        typedef std::shared_ptr<task_type> task_pointer;

        if(m_pool->query_create_task())
            m_pool->add_task(tg.store(task_pointer(new task_type(tg, func))));
        else
            func();
    }
    //------------------------------------------------------------------------//

#if defined(PTL_USE_TBB)
    //------------------------------------------------------------------------//
    // public wrap functions using TBB tasks
    //------------------------------------------------------------------------//
    template <typename _Ret, typename _Arg, typename _Func, typename... _Args>
    std::shared_ptr<Task<_Ret, _Arg, _Args...>> wrap(TBBTaskGroup<_Ret, _Arg>& tg,
                                                     const _Func& func, _Args... args)
    {
        typedef Task<_Ret, _Arg, _Args...> task_type;
        typedef std::shared_ptr<task_type> task_pointer;

        return tg.store(
            task_pointer(new task_type(tg, func, std::forward<_Args>(args)...)));
    }
    //------------------------------------------------------------------------//
    template <typename _Ret, typename _Arg, typename _Func>
    std::shared_ptr<Task<_Ret, _Arg>> wrap(TBBTaskGroup<_Ret, _Arg>& tg,
                                           const _Func&              func)
    {
        typedef Task<_Ret, _Arg>           task_type;
        typedef std::shared_ptr<task_type> task_pointer;

        return tg.store(task_pointer(new task_type(tg, func)));
    }

    //------------------------------------------------------------------------//
    // public exec functions using TBB tasks
    //------------------------------------------------------------------------//
    template <typename _Ret, typename _Arg, typename _Func, typename... _Args>
    void exec(TBBTaskGroup<_Ret, _Arg>& tg, const _Func& func, _Args... args)
    {
        typedef Task<_Ret, _Arg, _Args...> task_type;
        typedef std::shared_ptr<task_type> task_pointer;

        m_pool->add_task(tg.store(
            task_pointer(new task_type(tg, func, std::forward<_Args>(args)...))));
    }
    //------------------------------------------------------------------------//
    template <typename _Ret, typename _Arg, typename _Func>
    void exec(TBBTaskGroup<_Ret, _Arg>& tg, const _Func& func)
    {
        typedef Task<_Ret, _Arg>           task_type;
        typedef std::shared_ptr<task_type> task_pointer;

        m_pool->add_task(tg.store(task_pointer(new task_type(tg, func))));
    }
    //------------------------------------------------------------------------//
#endif

protected:
    // Protected variables
    ThreadPool* m_pool;

private:
    static TaskManager*& fgInstance();
};

//======================================================================================//
