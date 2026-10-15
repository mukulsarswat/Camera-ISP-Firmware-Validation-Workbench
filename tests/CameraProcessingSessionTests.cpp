#include "../src/camera/CameraProcessingSession.hpp"

#include <opencv2/core.hpp>

#include <chrono>
#include <iostream>
#include <stdexcept>

using namespace std::chrono_literals;

bool testEndToEndFrameProcessing() {
    isp::ISPConfig config;

    camera::CameraProcessingSession session(config, 3);

    if (!session.start()) {
        return false;
    }

    cv::Mat input(
        64, 80, CV_8UC3,
        cv::Scalar(80, 100, 120)
    );

    if (!session.submitFrame(input)) {
        return false;
    }

    camera::CameraProcessingResult result =
        session.processNext(50ms);

    return result.frameAcquired &&
           result.ispResult.outputValid &&
           !result.ispResult.image.empty() &&
           result.ispResult.image.size() == input.size() &&
           result.ispResult.stages.size() == 4;
}

bool testTimeoutWithoutFrame() {
    isp::ISPConfig config;

    camera::CameraProcessingSession session(config, 2);

    if (!session.start()) {
        return false;
    }

    camera::CameraProcessingResult result =
        session.processNext(1ms);

    return !result.frameAcquired &&
           result.ispResult.image.empty();
}

bool testDroppedFrameCounter() {
    isp::ISPConfig config;

    camera::CameraProcessingSession session(config, 1);

    if (!session.start()) {
        return false;
    }

    cv::Mat first(
        32, 32, CV_8UC3,
        cv::Scalar(20, 30, 40)
    );

    cv::Mat second(
        32, 32, CV_8UC3,
        cv::Scalar(100, 110, 120)
    );

    if (!session.submitFrame(first)) {
        return false;
    }

    if (session.submitFrame(second)) {
        return false;
    }

    if (session.droppedFrames() != 1) {
        return false;
    }

    camera::CameraProcessingResult result =
        session.processNext(10ms);

    return result.frameAcquired &&
           result.droppedFrames == 1 &&
           result.ispResult.outputValid;
}

bool testFaultRecovery() {
    isp::ISPConfig config;

    camera::CameraProcessingSession session(config, 2);

    if (!session.start()) {
        return false;
    }

    if (!session.injectFault()) {
        return false;
    }

    if (session.state() != camera::DriverState::Error) {
        return false;
    }

    if (!session.recover()) {
        return false;
    }

    return session.start() &&
           session.state() == camera::DriverState::Streaming;
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

    check(
        "end-to-end frame processing",
        testEndToEndFrameProcessing()
    );

    check(
        "timeout without frame",
        testTimeoutWithoutFrame()
    );

    check(
        "dropped-frame counter",
        testDroppedFrameCounter()
    );

    check(
        "fault recovery",
        testFaultRecovery()
    );

    if (failures == 0) {
        std::cout << "All camera processing session tests passed\n";
        return 0;
    }

    return 1;
}