
#include "isp/ExposureGain.hpp"

#include <opencv2/core.hpp>

#include <cmath>
#include <iostream>
#include <stdexcept>

bool testNoChange() {
    cv::Mat input(1, 1, CV_8UC1, cv::Scalar(100));

    const cv::Mat output = isp::ExposureGain::apply(input, 0.0, 1.0);

    return output.at<unsigned char>(0, 0) == 100;
}

bool testDoubleBrightness() {
    cv::Mat input(1, 1, CV_8UC1, cv::Scalar(100));

    const cv::Mat output = isp::ExposureGain::apply(input, 1.0, 1.0);

    return output.at<unsigned char>(0, 0) == 200;
}

bool testPixelClipping() {
    cv::Mat input(1, 1, CV_8UC1, cv::Scalar(200));

    const cv::Mat output = isp::ExposureGain::apply(input, 1.0, 1.0);

    return output.at<unsigned char>(0, 0) == 255;
}

bool testInvalidGain() {
    cv::Mat input(1, 1, CV_8UC1, cv::Scalar(100));

    try {
        isp::ExposureGain::apply(input, 0.0, -1.0);
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

    runTest("No exposure or gain change", testNoChange());
    runTest("Positive one-stop exposure", testDoubleBrightness());
    runTest("8-bit pixel clipping", testPixelClipping());
    runTest("Invalid gain rejected", testInvalidGain());

    std::cout << "\nTests passed: " << (4 - failures) << "/4\n";

    return failures == 0 ? 0 : 1;
}
