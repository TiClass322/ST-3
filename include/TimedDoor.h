// Copyright 2021 GHA Test Team

#ifndef INCLUDE_TIMEDDOOR_H_
#define INCLUDE_TIMEDDOOR_H_

#include <mutex>
#include <thread>

class DoorTimerAdapter;
class Timer;
class Door;
class TimedDoor;

class TimerClient {
 public:
  virtual ~TimerClient() = default;
  virtual void Timeout() = 0;
};

class Door {
 public:
  virtual ~Door() = default;
  virtual void lock() = 0;
  virtual void unlock() = 0;
  virtual bool isDoorOpened() = 0;
};

class DoorTimerAdapter : public TimerClient {
 private:
  TimedDoor& door;
 public:
  explicit DoorTimerAdapter(TimedDoor&);
  void Timeout();
};

class TimedDoor : public Door {
 private:
  DoorTimerAdapter * adapter;
  int iTimeout;
  bool isOpened;
  std::mutex doorMutex;
  std::thread timerThread;
 public:
  explicit TimedDoor(int);
  virtual ~TimedDoor();
  virtual bool isDoorOpened();
  void unlock();
  void lock();
  int  getTimeOut() const;
  virtual void throwState();
};

class Timer {
 public:
  std::thread tregister(int, TimerClient*);
 private:
  void sleep(int);
};

#endif  // INCLUDE_TIMEDDOOR_H_
