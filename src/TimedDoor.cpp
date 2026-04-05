// Copyright 2021 GHA Test Team
#include "TimedDoor.h"
#include <stdexcept>
#include <thread>
#include <chrono>

// DoorTimerAdapter implementation
DoorTimerAdapter::DoorTimerAdapter(TimedDoor& d) : door(d) {}

void DoorTimerAdapter::Timeout() {
    if (door.isDoorOpened()) {
        door.throwState();
    }
}

// TimedDoor implementation
TimedDoor::TimedDoor(int timeout) : iTimeout(timeout), isOpened(false) {
    adapter = new DoorTimerAdapter(*this);
}

TimedDoor::~TimedDoor() {
    if (timerThread.joinable()) {
        timerThread.join();
    }
    delete adapter;
}

bool TimedDoor::isDoorOpened() {
    std::lock_guard<std::mutex> lock(doorMutex);
    return isOpened;
}

void TimedDoor::unlock() {
    std::lock_guard<std::mutex> lock(doorMutex);
    if (!isOpened) {
        if (timerThread.joinable()) {
            timerThread.join();
        }
        isOpened = true;
        Timer timer;
        timerThread = timer.tregister(iTimeout, adapter);
    }
}

void TimedDoor::lock() {
    std::lock_guard<std::mutex> lock(doorMutex);
    isOpened = false;
}

int TimedDoor::getTimeOut() const {
    return iTimeout;
}

void TimedDoor::throwState() {
    throw std::runtime_error("Door is still open!");
}

// Timer implementation
std::thread Timer::tregister(int seconds, TimerClient* c) {
    return std::thread([seconds, c]() {
        std::this_thread::sleep_for(std::chrono::seconds(seconds));
        if (c) {
            c->Timeout();
        }
    });
}

void Timer::sleep(int seconds) {
    std::this_thread::sleep_for(std::chrono::seconds(seconds));
}
