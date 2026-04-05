// Copyright 2021 GHA Test Team

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <cstdint>
#include "TimedDoor.h"
#include <thread>
#include <chrono>

using ::testing::Return;
using ::testing::Test;

// Test fixture for TimedDoor
struct TimedDoorTest : public Test {
    TimedDoor* door;
    const int timeout = 1; // 1 second timeout for tests

    void SetUp() override {
        door = new TimedDoor(timeout);
    }

    void TearDown() override {
        delete door;
    }
};

// 1. Test initial state
TEST_F(TimedDoorTest, IsInitiallyClosed) {
    ASSERT_FALSE(door->isDoorOpened());
}

// 2. Test unlock()
TEST_F(TimedDoorTest, UnlockOpensDoor) {
    door->unlock();
    ASSERT_TRUE(door->isDoorOpened());
    // Lock the door to prevent the timer from throwing in the destructor join
    door->lock();
}

// 3. Test lock()
TEST_F(TimedDoorTest, LockClosesDoor) {
    door->unlock();
    door->lock();
    ASSERT_FALSE(door->isDoorOpened());
}

// 4. Test getTimeOut()
TEST_F(TimedDoorTest, GetTimeoutReturnsCorrectValue) {
    ASSERT_EQ(door->getTimeOut(), timeout);
}

// 5. Test throwState() throws exception
TEST_F(TimedDoorTest, ThrowStateThrowsRuntimeError) {
    ASSERT_THROW(door->throwState(), std::runtime_error);
}

// 6. Test throwState() exception message
TEST_F(TimedDoorTest, ThrowStateThrowsExceptionWithCorrectMessage) {
    try {
        door->throwState();
        FAIL() << "Expected std::runtime_error";
    } catch(const std::runtime_error& err) {
        EXPECT_STREQ("Door is still open!", err.what());
    } catch(...) {
        FAIL() << "Expected std::runtime_error";
    }
}

// 7. Test unlocking an already unlocked door and then locking it
TEST_F(TimedDoorTest, UnlockTwiceThenLock) {
    door->unlock(); // Starts one timer
    door->unlock(); // Should do nothing because the door is already open
    door->lock();   // Door is now locked
    // Wait for the timer to fire. Because the door is locked, the adapter
    // should not throw an exception. The test passes if it doesn't crash.
    std::this_thread::sleep_for(std::chrono::seconds(timeout + 1));
    ASSERT_FALSE(door->isDoorOpened()); // Assert final state
}

// --- Mock-based tests ---

// Mock for TimerClient
class MockTimerClient : public TimerClient {
 public:
    MOCK_METHOD(void, Timeout, (), (override));
};

// 8. Test Timer calls Timeout
TEST(TimerTest, TimerCallsTimeout) {
    MockTimerClient mockClient;
    Timer timer;

    EXPECT_CALL(mockClient, Timeout()).Times(1);

    std::thread t = timer.tregister(1, &mockClient);
    std::this_thread::sleep_for(std::chrono::seconds(2));
    t.join();
}

// Mock for TimedDoor to test the adapter
class MockTimedDoor : public TimedDoor {
 public:
    explicit MockTimedDoor(int timeout) : TimedDoor(timeout) {}
    MOCK_METHOD(bool, isDoorOpened, (), (override));
    MOCK_METHOD(void, throwState, (), (override));
};

// 9. Test Adapter calls throwState when door is open
TEST(DoorTimerAdapterTest, TimeoutCallsThrowStateWhenOpen) {
    MockTimedDoor mockDoor(1);
    DoorTimerAdapter adapter(mockDoor);

    EXPECT_CALL(mockDoor, isDoorOpened()).WillOnce(Return(true));
    EXPECT_CALL(mockDoor, throwState()).Times(1);

    adapter.Timeout();
}

// 10. Test Adapter does not call throwState when door is closed
TEST(DoorTimerAdapterTest, TimeoutDoesNothingWhenClosed) {
    MockTimedDoor mockDoor(1);
    DoorTimerAdapter adapter(mockDoor);

    EXPECT_CALL(mockDoor, isDoorOpened()).WillOnce(Return(false));
    EXPECT_CALL(mockDoor, throwState()).Times(0);

    adapter.Timeout();
}

// 11. Test that creating a door does not open it
TEST(TimedDoorCreationTest, NewDoorIsClosed) {
    TimedDoor door(5);
    EXPECT_FALSE(door.isDoorOpened());
}
