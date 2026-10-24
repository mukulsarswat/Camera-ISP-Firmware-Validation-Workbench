
#include "../include/isp/WhiteBalance.hpp"

#include <opencv2/core.hpp>

#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>

namespace {

bool testNeutralGains() {
    cv::Mat input(1, 1, CV_8UC3, cv::Scalar(100, 120, 140));

    const cv::Mat output =
        isp::WhiteBalance::apply(input, 1.0, 1.0, 1.0);

    return output.at<cv::Vec3b>(0, 0) ==
           input.at<cv::Vec3b>(0, 0);
}

bool testIndependentChannelGains() {
    cv::Mat input(1, 1, CV_8UC3, cv::Scalar(100, 100, 100));

    const cv::Mat output =
        isp::WhiteBalance::apply(input, 2.0, 1.0, 0.5);

    const cv::Vec3b pixel = output.at<cv::Vec3b>(0, 0);

    return pixel[0] == 200 &&
           pixel[1] == 100 &&
           pixel[2] == 50;
}

bool testClipping() {
    cv::Mat input(1, 1, CV_8UC3, cv::Scalar(200, 100, 50));

    const cv::Mat output =
        isp::WhiteBalance::apply(input, 2.0, 1.0, 1.0);

    return output.at<cv::Vec3b>(0, 0)[0] == 255;
}

bool testZeroGains() {
    cv::Mat input(1, 1, CV_8UC3, cv::Scalar(100, 120, 140));

    const cv::Mat output =
        isp::WhiteBalance::apply(input, 0.0, 0.0, 0.0);

    const cv::Vec3b pixel = output.at<cv::Vec3b>(0, 0);

    return pixel == cv::Vec3b(0, 0, 0);
}

bool testFractionalGains() {
    cv::Mat input(1, 1, CV_8UC3, cv::Scalar(101, 99, 51));

    const cv::Mat output =
        isp::WhiteBalance::apply(input, 0.5, 1.5, 2.0);

    const cv::Vec3b pixel = output.at<cv::Vec3b>(0, 0);

    // Expected results use OpenCV's 8-bit saturating conversion.
    return pixel[0] == cv::saturate_cast<unsigned char>(101 * 0.5) &&
           pixel[1] == cv::saturate_cast<unsigned char>(99 * 1.5) &&
           pixel[2] == cv::saturate_cast<unsigned char>(51 * 2.0);
}

bool testInvalidGains() {
    cv::Mat input(1, 1, CV_8UC3, cv::Scalar(100, 100, 100));

    const double invalidValues[] = {
        -1.0,
        8.1,
        std::numeric_limits<double>::infinity(),
        -std::numeric_limits<double>::infinity(),
        std::numeric_limits<double>::quiet_NaN()
    };

    for (double gain : invalidValues) {
        try {
            isp::WhiteBalance::apply(input, gain, 1.0, 1.0);
        } catch (const std::invalid_argument&) {
            continue;
        }

        return false;
    }

    return true;
}

bool testInvalidInput() {
    cv::Mat empty;

    try {
        isp::WhiteBalance::apply(empty, 1.0, 1.0, 1.0);
    } catch (const std::invalid_argument&) {
        return true;
    }

    return false;
}

} // namespace

int main() {
    int failures = 0;

    const auto runTest = [&failures](const char* name, bool passed) {
        std::cout << (passed ? "[PASS] " : "[FAIL] ")
                  << name << '\n';

        if (!passed) {
            ++failures;
        }
    };

    runTest("Neutral gains preserve pixels", testNeutralGains());
    runTest("Independent channel gains", testIndependentChannelGains());
    runTest("Channel clipping", testClipping());
    runTest("Zero gains produce black", testZeroGains());
    runTest("Fractional gains use saturating conversion", testFractionalGains());
    runTest("Invalid gains are rejected", testInvalidGains());
    runTest("Empty input is rejected", testInvalidInput());

    std::cout << "\nTests passed: " << (7 - failures) << "/7\n";

    return failures == 0 ? 0 : 1;
}
