
#include "camera/CameraFrame.hpp"

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>

#include <iostream>
#include <string>

int main() {
    const int width = 640;
    const int height = 480;

    cv::Mat rawFrame(
        height,
        width,
        CV_8UC3,
        cv::Scalar(0, 0, 0)
    );

    cv::rectangle(
        rawFrame,
        cv::Point(100, 100),
        cv::Point(300, 250),
        cv::Scalar(0, 255, 0),
        -1
    );

    cv::circle(
        rawFrame,
        cv::Point(420, 200),
        70,
        cv::Scalar(255, 0, 0),
        -1
    );

    camera::CameraFrame frame(rawFrame);

    if (!frame.isValid()) {
        std::cerr << "Error: Invalid camera frame.\n";
        return 1;
    }

    const std::string outputPath = "build/sample_frame.png";

    if (!cv::imwrite(outputPath, frame.image())) {
        std::cerr << "Error: Failed to save frame.\n";
        return 1;
    }

    std::cout << "Camera ISP Workbench\n";
    std::cout << "Frame valid: " << std::boolalpha
              << frame.isValid() << '\n';
    std::cout << "Width: " << frame.width() << '\n';
    std::cout << "Height: " << frame.height() << '\n';
    std::cout << "Channels: " << frame.channels() << '\n';
    std::cout << "Saved: " << outputPath << '\n';

    return 0;
}
