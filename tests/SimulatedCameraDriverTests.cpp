#include "../src/camera/SimulatedCameraDriver.hpp"

#include <opencv2/core.hpp>

#include <chrono>
#include <iostream>
#include <stdexcept>

using camera::DriverState;
using camera::SimulatedCameraDriver;
using namespace std::chrono_literals;

bool testInitializationAndStart() {
    SimulatedCameraDriver driver(3);

    if (driver.state() != DriverState::Uninitialized) {
        return false;
    }

    if (!driver.initialize() || !driver.start()) {
        return false;
    }

    return driver.state() == DriverState::Streaming;
}

bool testSubmitAndAcquireFrame() {
    SimulatedCameraDriver driver(3);

    driver.initialize();
    driver.start();

    cv::Mat input(32, 32, CV_8UC3, cv::Scalar(10, 20, 30));

    if (!driver.submitFrame(input)) {
        return false;
    }

    // Change the source to verify that submission cloned the frame.
    input.setTo(cv::Scalar(200, 200, 200));

    cv::Mat acquired = driver.acquireFrame(10ms);

    if (acquired.empty()) {
        return false;
    }

    const cv::Vec3b pixel = acquired.at<cv::Vec3b>(0, 0);

    return pixel[0] == 10 &&
           pixel[1] == 20 &&
           pixel[2] == 30;
}

bool testFullQueueDropsNewestFrame() {
    SimulatedCameraDriver driver(1);

    driver.initialize();
    driver.start();

    cv::Mat first(16, 16, CV_8UC1, cv::Scalar(40));
    cv::Mat second(16, 16, CV_8UC1, cv::Scalar(90));

    if (!driver.submitFrame(first)) {
        return false;
    }

    if (driver.submitFrame(second)) {
        return false;
    }

    if (driver.droppedFrames() != 1 ||
        driver.queuedFrames() != 1) {
        return false;
    }

    cv::Mat acquired = driver.acquireFrame(10ms);

    return !acquired.empty() &&
           acquired.at<unsigned char>(0, 0) == 40;
}

bool testEmptyQueueTimesOut() {
    SimulatedCameraDriver driver(2);

    driver.initialize();
    driver.start();

    cv::Mat acquired = driver.acquireFrame(1ms);

    return acquired.empty();
}

bool testFaultAndRecovery() {
    SimulatedCameraDriver driver(2);

    driver.initialize();
    driver.start();

    cv::Mat frame(16, 16, CV_8UC3, cv::Scalar(10, 20, 30));
    driver.submitFrame(frame);

    if (!driver.injectFault()) {
        return false;
    }

    if (driver.state() != DriverState::Error ||
        driver.queuedFrames() != 0) {
        return false;
    }

    if (!driver.recover()) {
        return false;
    }

    if (driver.state() != DriverState::Initialized) {
        return false;
    }

    return driver.start() &&
           driver.state() == DriverState::Streaming;
}

bool testInvalidFrameRejected() {
    SimulatedCameraDriver driver(2);

    driver.initialize();
    driver.start();

    return !driver.submitFrame(cv::Mat{});
}

int main() {
    int failures = 0;

    const auto check = [&](const char* name, bool passed) {
        if (passed) {
            std::cout << "PASS: " << name << '\n';
        } else {
            std::cerr << "FAIL: " << name << '\n';
            ++failures;
        }
    };

    check("initialization and start", testInitializationAndStart());
    check("submit and acquire", testSubmitAndAcquireFrame());
    check("full queue drops newest", testFullQueueDropsNewestFrame());
    check("empty queue timeout", testEmptyQueueTimesOut());
    check("fault and recovery", testFaultAndRecovery());
    check("invalid frame rejection", testInvalidFrameRejected());

    if (failures == 0) {
        std::cout << "All simulated camera driver tests passed\n";
        return 0;
    }

    return 1;
}