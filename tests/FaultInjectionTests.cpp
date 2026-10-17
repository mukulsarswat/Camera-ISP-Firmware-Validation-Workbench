#include <thread>
#include "../src/camera/SimulatedCameraDriver.hpp"
#include "../include/isp/ExposureGain.hpp"

#include <opencv2/core.hpp>

#include <cmath>
#include <iostream>
#include <stdexcept>

using camera::DriverState;
using camera::SimulatedCameraDriver;
using isp::ExposureGain;

bool testZeroCapacityRejected()
{
    try
    {
        SimulatedCameraDriver driver(0);
    }
    catch (const std::invalid_argument &)
    {
        return true;
    }
    return false;
}
bool testRepeatedFaultRecovery()
{
    SimulatedCameraDriver driver(2);

    if (!driver.initialize())
    {
        return false;
    }

    cv::Mat frame(
        16, 16, CV_8UC3,
        cv::Scalar(10, 20, 30));

    for (int cycle = 0; cycle < 3; ++cycle)
    {
        if (!driver.start())
        {
            return false;
        }

        if (!driver.submitFrame(frame))
        {
            return false;
        }

        if (!driver.injectFault())
        {
            return false;
        }

        if (driver.state() != DriverState::Error ||
            driver.queuedFrames() != 0)
        {
            return false;
        }

        if (!driver.recover())
        {
            return false;
        }

        if (driver.state() != DriverState::Initialized)
        {
            return false;
        }
    }

    return driver.start() &&
           driver.submitFrame(frame);
}
bool testFaultWakesWaitingConsumer()
{
    SimulatedCameraDriver driver(2);

    if (!driver.initialize() || !driver.start())
    {
        return false;
    }

    bool receivedEmptyFrame = false;

    std::thread consumer([&driver, &receivedEmptyFrame]()
                         {
        cv::Mat frame = driver.acquireFrame(
            std::chrono::seconds(5)
        );

        receivedEmptyFrame = frame.empty(); });

    const bool faultInjected = driver.injectFault();

    consumer.join();

    return faultInjected &&
           receivedEmptyFrame &&
           driver.state() == DriverState::Error;
}

bool testInvalidFramesRejected()
{
    SimulatedCameraDriver driver(2);
    if (!driver.initialize() || !driver.start())
    {
        return false;
    }

    cv::Mat empty;
    cv::Mat unsupportedDepth(
        16, 16, CV_16UC1, cv::Scalar(100));
    cv::Mat unsupportedChannels(
        16, 16, CV_8UC4, cv::Scalar(10, 20, 30, 40));

    return !driver.submitFrame(empty) &&
           !driver.submitFrame(unsupportedDepth) &&
           !driver.submitFrame(unsupportedChannels) &&
           driver.queuedFrames() == 0;
}

bool testSubmitRejectedWhenNotStreaming()
{
    SimulatedCameraDriver driver(2);
    cv::Mat frame(16, 16, CV_8UC3, cv::Scalar(10, 20, 30));

    if (driver.submitFrame(frame))
    {
        return false;
    }

    if (!driver.initialize())
    {
        return false;
    }

    return !driver.submitFrame(frame);
}

bool testOverflowDropsNewestFrame()
{
    SimulatedCameraDriver driver(1);
    if (!driver.initialize() || !driver.start())
    {
        return false;
    }

    cv::Mat first(16, 16, CV_8UC1, cv::Scalar(40));
    cv::Mat second(16, 16, CV_8UC1, cv::Scalar(90));

    if (!driver.submitFrame(first))
    {
        return false;
    }

    if (driver.submitFrame(second))
    {
        return false;
    }

    if (driver.droppedFrames() != 1 ||
        driver.queuedFrames() != 1)
    {
        return false;
    }

    cv::Mat acquired = driver.acquireFrame(
        std::chrono::milliseconds(10));

    return !acquired.empty() &&
           acquired.at<unsigned char>(0, 0) == 40;
}

bool testFaultClearsQueueAndRecovers()
{
    SimulatedCameraDriver driver(2);
    if (!driver.initialize() || !driver.start())
    {
        return false;
    }

    cv::Mat frame(16, 16, CV_8UC3, cv::Scalar(10, 20, 30));
    if (!driver.submitFrame(frame))
    {
        return false;
    }

    if (!driver.injectFault())
    {
        return false;
    }

    if (driver.state() != DriverState::Error ||
        driver.queuedFrames() != 0 ||
        driver.submitFrame(frame))
    {
        return false;
    }

    if (!driver.recover())
    {
        return false;
    }

    if (driver.state() != DriverState::Initialized ||
        driver.submitFrame(frame))
    {
        return false;
    }

    return driver.start() && driver.submitFrame(frame);
}
bool testInvalidLifecycleTransitions() {
    SimulatedCameraDriver driver(2);

    cv::Mat frame(
        16, 16, CV_8UC3,
        cv::Scalar(10, 20, 30)
    );

    // Cannot start before initialization.
    if (driver.start()) {
        return false;
    }

    // Cannot stop before streaming.
    if (driver.stop()) {
        return false;
    }

    // Initialize and start streaming.
    if (!driver.initialize() || !driver.start()) {
        return false;
    }

    // Cannot initialize or start an already streaming driver.
    if (driver.initialize() || driver.start()) {
        return false;
    }

    // Stop streaming successfully.
    if (!driver.stop()) {
        return false;
    }

    // Cannot stop twice or submit after stopping.
    if (driver.stop() || driver.submitFrame(frame)) {
        return false;
    }

    return true;
}
bool testReinitializeAfterStop() {
    SimulatedCameraDriver driver(2);

    cv::Mat frame(
        16, 16, CV_8UC3,
        cv::Scalar(10, 20, 30)
    );

    if (!driver.initialize() ||
        !driver.start() ||
        !driver.submitFrame(frame) ||
        !driver.stop()) {
        return false;
    }

    // Initialization should clear old queued frames.
    if (!driver.initialize() ||
        driver.queuedFrames() != 0) {
        return false;
    }

    if (!driver.start() || !driver.submitFrame(frame)) {
        return false;
    }

    cv::Mat acquired = driver.acquireFrame(
        std::chrono::milliseconds(100)
    );

    return !acquired.empty() &&
           acquired.rows == 16 &&
           acquired.cols == 16;
}
bool testConcurrentFrameProducerConsumer() {
    constexpr int totalFrames = 1000;
    constexpr std::size_t capacity = 3;

    SimulatedCameraDriver driver(capacity);

    if (!driver.initialize() || !driver.start()) {
        return false;
    }

    int acquiredCount = 0;

    std::thread consumer([&]() {
        while (true) {
            cv::Mat frame = driver.acquireFrame(
                std::chrono::milliseconds(20)
            );

            if (!frame.empty()) {
                ++acquiredCount;

                if (frame.rows != 16 || frame.cols != 16) {
                    return;
                }

                continue;
            }

            if (driver.state() != DriverState::Streaming &&
                driver.queuedFrames() == 0) {
                break;
            }
        }
    });

    int rejectedCount = 0;

    for (int i = 0; i < totalFrames; ++i) {
        cv::Mat frame(
            16, 16, CV_8UC3,
            cv::Scalar(i % 256, 20, 30)
        );

        if (!driver.submitFrame(frame)) {
            ++rejectedCount;
        }

        if (driver.queuedFrames() > capacity) {
            driver.stop();
            consumer.join();
            return false;
        }
    }

    if (!driver.stop()) {
        consumer.join();
        return false;
    }

    consumer.join();

    return driver.queuedFrames() == 0 &&
           acquiredCount + rejectedCount == totalFrames &&
           static_cast<std::size_t>(rejectedCount) ==
               driver.droppedFrames();
}

bool testInvalidExposureRejected()
{
    cv::Mat frame(16, 16, CV_8UC3, cv::Scalar(100, 100, 100));

    try
    {
        ExposureGain::apply(frame, 11.0, 1.0);
    }
    catch (const std::invalid_argument &)
    {
        return true;
    }

    return false;
}

bool testInvalidGainRejected()
{
    cv::Mat frame(16, 16, CV_8UC3, cv::Scalar(100, 100, 100));

    try
    {
        ExposureGain::apply(frame, 0.0, 17.0);
    }
    catch (const std::invalid_argument &)
    {
        return true;
    }

    return false;
}

bool testNonFiniteGainRejected()
{
    cv::Mat frame(16, 16, CV_8UC3, cv::Scalar(100, 100, 100));

    try
    {
        ExposureGain::apply(
            frame, 0.0, std::nan(""));
    }
    catch (const std::invalid_argument &)
    {
        return true;
    }

    return false;
}

int main()
{
    int failures = 0;

    const auto check = [&](const char *name, bool passed)
    {
        if (passed)
        {
            std::cout << "PASS: " << name << '\n';
        }
        else
        {
            std::cerr << "FAIL: " << name << '\n';
            ++failures;
        }
    };

    check("zero buffer capacity rejected", testZeroCapacityRejected());
    check("invalid frames rejected", testInvalidFramesRejected());
    check("submit rejected when not streaming", testSubmitRejectedWhenNotStreaming());
    check("overflow drops newest frame", testOverflowDropsNewestFrame());
    check("fault clears queue and recovers", testFaultClearsQueueAndRecovers());
    check("repeated fault recovery",testRepeatedFaultRecovery());
    check("fault wakes waiting consumer",testFaultWakesWaitingConsumer());
    check("invalid exposure rejected", testInvalidExposureRejected());
    check("invalid gain rejected", testInvalidGainRejected());
    check("non-finite gain rejected", testNonFiniteGainRejected());
    check("invalid lifecycle transitions rejected",testInvalidLifecycleTransitions());
    check("reinitialize after stop",testReinitializeAfterStop());
    check("concurrent frame producer-consumer",testConcurrentFrameProducerConsumer());

    if (failures == 0)
    {
        std::cout << "All fault-injection tests passed\n";
        return 0;
    }

    std::cerr << failures << " fault-injection test(s) failed\n";
    return 1;
}
