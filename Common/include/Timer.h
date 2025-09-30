#pragma once
#include <iostream>
#include <functional>
#include <chrono>
#include <list>
#include <thread>
#include <mutex>
#include "ThreadPool.h"

constexpr long long TickMs = 100;
constexpr long long WheelSize = 100;
constexpr long long WorkThread = 4;

struct TimerTask
{
    using TimerCallback = std::function<void()>;

    uint32_t m_id = 0;
    bool m_isEvery = false;
    long long m_intervalMs = 0;
    std::chrono::steady_clock::time_point m_expireTime;
    std::shared_ptr<std::atomic<bool>> m_canceled;
    TimerCallback m_cb;
};

class TimerHandle
{
public:
    TimerHandle(uint64_t id, std::shared_ptr<std::atomic<bool>> canceled)
        : m_id(id), m_canceled(canceled) {}

    inline void Cancel()
    {
        if (m_canceled)
        {
            *m_canceled = true;
        }
    }

    inline uint64_t GetId() const { return m_id; }

private:
    uint64_t m_id;
    std::shared_ptr<std::atomic<bool>> m_canceled;
};

struct TimerWheel
{
    long long m_tickMs;
    long long m_wheelSize;
    std::atomic<long long> m_currentIndex;
    std::vector<std::list<TimerTask>> m_slots;
    std::vector<std::mutex> m_slotMutexes;
    std::shared_ptr<TimerWheel> p_next;

    TimerWheel(long long tickMs, long long wheelSize, std::shared_ptr<TimerWheel> next = nullptr)
        : m_tickMs(tickMs), m_wheelSize(wheelSize), m_currentIndex(0), m_slots(wheelSize), m_slotMutexes(wheelSize), p_next(std::move(next)) {}
};

class TimeHolder
{
public:
    TimeHolder();

    void Hold(std::chrono::milliseconds msec);

    void Reset();

private:
    std::chrono::steady_clock::time_point m_t;
};

class Timer
{
public:
    Timer(long long tickMs = TickMs, long long wheelSize = WheelSize, long long workThread = WorkThread);

    ~Timer();

    void Start();

    TimerHandle RunAt(std::chrono::steady_clock::time_point time, TimerTask::TimerCallback cb);

    TimerHandle RunAfter(long long delayMs, TimerTask::TimerCallback cb);

    TimerHandle RunEvery(long long intervalMs, TimerTask::TimerCallback cb);

    void Shutdown();

private:
    void AddTask(std::shared_ptr<TimerWheel> wheel, const TimerTask &task);

    void Cascade(std::shared_ptr<TimerWheel> wheel);

    void AddTask(const TimerTask &task);

    void Tick();

    std::shared_ptr<TimerWheel> p_headWheel;
    std::atomic<bool> m_isRunning;
    std::thread m_thread;
    ThreadPool m_workPool;
};