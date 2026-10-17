
#include "../src/camera/SimulatedCameraDriver.hpp"
#include "../include/isp/ExposureGain.hpp"

#include <opencv2/core.hpp>

#include <cmath>
#include <iostream>
#include <stdexcept>

using camera::DriverState;
using camera::SimulatedCameraDriver;
using isp::ExposureGain;

bool testZeroCapacityRejected() {
    try {
        SimulatedCameraDriver driver(0);
    } catch (const std::invalid_argument&) {
        return true;
    }
    return false;
}

bool testInvalidFramesRejected() {
    SimulatedCameraDriver driver(2);
    if (!driver.initialize() || !driver.start()) {
        return false;
    }

    cv::Mat empty;
    cv::Mat unsupportedDepth(
        16, 16, CV_16UC1, cv::Scalar(100)
    );
    cv::Mat unsupportedChannels(
        16, 16, CV_8UC4, cv::Scalar(10, 20, 30, 40)
    );

    return !driver.submitFrame(empty) &&
           !driver.submitFrame(unsupportedDepth) &&
           !driver.submitFrame(unsupportedChannels) &&
           driver.queuedFrames() == 0;
}

bool testSubmitRejectedWhenNotStreaming() {
    SimulatedCameraDriver driver(2);
    cv::Mat frame(16, 16, CV_8UC3, cv::Scalar(10, 20, 30));

    if (driver.submitFrame(frame)) {
        return false;
    }

    if (!driver.initialize()) {
        return false;
    }

    return !driver.submitFrame(frame);
}

bool testOverflowDropsNewestFrame() {
    SimulatedCameraDriver driver(1);
    if (!driver.initialize() || !driver.start()) {
        return false;
    }

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

    cv::Mat acquired = driver.acquireFrame(
        std::chrono::milliseconds(10)
    );

    return !acquired.empty() &&
           acquired.at<unsigned char>(0, 0) == 40;
}

bool testFaultClearsQueueAndRecovers() {
    SimulatedCameraDriver driver(2);
    if (!driver.initialize() || !driver.start()) {
        return false;
    }

    cv::Mat frame(16, 16, CV_8UC3, cv::Scalar(10, 20, 30));
    if (!driver.submitFrame(frame)) {
        return false;
    }

    if (!driver.injectFault()) {
        return false;
    }

    if (driver.state() != DriverState::Error ||
        driver.queuedFrames() != 0 ||
        driver.submitFrame(frame)) {
        return false;
    }

    if (!driver.recover()) {
        return false;
    }

    if (driver.state() != DriverState::Initialized ||
        driver.submitFrame(frame)) {
        return false;
    }

    return driver.start() && driver.submitFrame(frame);
}

bool testInvalidExposureRejected() {
    cv::Mat frame(16, 16, CV_8UC3, cv::Scalar(100, 100, 100));

    try {
        ExposureGain::apply(frame, 11.0, 1.0);
    } catch (const std::invalid_argument&) {
        return true;
    }

    return false;
}

bool testInvalidGainRejected() {
    cv::Mat frame(16, 16, CV_8UC3, cv::Scalar(100, 100, 100));

    try {
        ExposureGain::apply(frame, 0.0, 17.0);
    } catch (const std::invalid_argument&) {
        return true;
    }

    return false;
}

bool testNonFiniteGainRejected() {
    cv::Mat frame(16, 16, CV_8UC3, cv::Scalar(100, 100, 100));

    try {
        ExposureGain::apply(
            frame, 0.0, std::nan("")
        );
    } catch (const std::invalid_argument&) {
        return true;
    }

    return false;
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

    check("zero buffer capacity rejected", testZeroCapacityRejected());
    check("invalid frames rejected", testInvalidFramesRejected());
    check("submit rejected when not streaming", testSubmitRejectedWhenNotStreaming());
    check("overflow drops newest frame", testOverflowDropsNewestFrame());
    check("fault clears queue and recovers", testFaultClearsQueueAndRecovers());
    check("invalid exposure rejected", testInvalidExposureRejected());
    check("invalid gain rejected", testInvalidGainRejected());
    check("non-finite gain rejected", testNonFiniteGainRejected());

    if (failures == 0) {
        std::cout << "All fault-injection tests passed\n";
        return 0;
    }

    std::cerr << failures << " fault-injection test(s) failed\n";
    return 1;
}
