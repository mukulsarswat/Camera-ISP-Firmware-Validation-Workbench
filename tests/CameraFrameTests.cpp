
#include "../src/camera/CameraFrame.hpp"

#include <opencv2/core.hpp>

#include <iostream>

bool testValidBgrFrame() {
    cv::Mat image(480, 640, CV_8UC3, cv::Scalar(0, 0, 0));
    camera::CameraFrame frame(image);

    return frame.isValid()
        && frame.width() == 640
        && frame.height() == 480
        && frame.channels() == 3;
}

bool testValidGrayscaleFrame() {
    cv::Mat image(480, 640, CV_8UC1, cv::Scalar(128));
    camera::CameraFrame frame(image);

    return frame.isValid()
        && frame.channels() == 1;
}

bool testEmptyFrame() {
    cv::Mat image;
    camera::CameraFrame frame(image);

    return !frame.isValid();
}

bool testUnsupportedPixelDepth() {
    cv::Mat image(100, 100, CV_16UC3, cv::Scalar(0, 0, 0));
    camera::CameraFrame frame(image);

    return !frame.isValid();
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

    runTest("Valid BGR frame", testValidBgrFrame());
    runTest("Valid grayscale frame", testValidGrayscaleFrame());
    runTest("Empty frame rejected", testEmptyFrame());
    runTest("Unsupported pixel depth rejected",
            testUnsupportedPixelDepth());

    std::cout << "\nTests passed: " << (4 - failures) << "/4\n";

    return failures == 0 ? 0 : 1;
}
