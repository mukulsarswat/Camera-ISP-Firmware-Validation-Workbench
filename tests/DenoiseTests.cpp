#include "../include/isp/Denoise.hpp"

#include <opencv2/core.hpp>

#include <iostream>
#include <stdexcept>

bool testOutputProperties() {
    cv::Mat input(100, 100, CV_8UC3, cv::Scalar(50, 100, 150));

    const cv::Mat output = isp::Denoise::apply(input);

    return !output.empty()
        && output.size() == input.size()
        && output.type() == input.type();
}

bool testUniformImagePreserved() {
    cv::Mat input(40, 40, CV_8UC1, cv::Scalar(128));

    const cv::Mat output = isp::Denoise::apply(input);

    return cv::countNonZero(output != input) == 0;
}

bool testGrayscaleAccepted() {
    cv::Mat input(50, 50, CV_8UC1, cv::Scalar(100));

    const cv::Mat output = isp::Denoise::apply(input);

    return output.type() == CV_8UC1;
}

bool testInvalidDiameterRejected() {
    cv::Mat input(20, 20, CV_8UC3, cv::Scalar(100, 100, 100));

    try {
        isp::Denoise::apply(input, 0);
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

    runTest("Output properties preserved", testOutputProperties());
    runTest("Uniform image preserved", testUniformImagePreserved());
    runTest("Grayscale input accepted", testGrayscaleAccepted());
    runTest("Invalid diameter rejected", testInvalidDiameterRejected());

    std::cout << "\nTests passed: " << (4 - failures) << "/4\n";

    return failures == 0 ? 0 : 1;
}