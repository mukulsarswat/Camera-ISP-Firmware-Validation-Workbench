
#include "../include/isp/WhiteBalance.hpp"

#include <opencv2/core.hpp>

#include <iostream>
#include <stdexcept>

bool testNeutralGains() {
    cv::Mat input(1, 1, CV_8UC3, cv::Scalar(100, 120, 140));

    const cv::Mat output = isp::WhiteBalance::apply(
        input, 1.0, 1.0, 1.0
    );

    return output.at<cv::Vec3b>(0, 0)
        == input.at<cv::Vec3b>(0, 0);
}

bool testIndependentChannelGains() {
    cv::Mat input(1, 1, CV_8UC3, cv::Scalar(100, 100, 100));

    const cv::Mat output = isp::WhiteBalance::apply(
        input, 2.0, 1.0, 0.5
    );

    const cv::Vec3b pixel = output.at<cv::Vec3b>(0, 0);

    return pixel[0] == 200
        && pixel[1] == 100
        && pixel[2] == 50;
}

bool testClipping() {
    cv::Mat input(1, 1, CV_8UC3, cv::Scalar(200, 100, 50));

    const cv::Mat output = isp::WhiteBalance::apply(
        input, 2.0, 1.0, 1.0
    );

    return output.at<cv::Vec3b>(0, 0)[0] == 255;
}

bool testInvalidGain() {
    cv::Mat input(1, 1, CV_8UC3, cv::Scalar(100, 100, 100));

    try {
        isp::WhiteBalance::apply(input, -1.0, 1.0, 1.0);
    } catch (const std::invalid_argument&) {
        return true;
    }

    return false;
}

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
    runTest("Invalid gain rejected", testInvalidGain());

    std::cout << "\nTests passed: " << (4 - failures) << "/4\n";

    return failures == 0 ? 0 : 1;
}
