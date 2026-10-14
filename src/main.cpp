#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>

#include <iostream>
#include <string>

int main() {
    const int width = 640;
    const int height = 480;

    // Create a simulated BGR camera frame.
    cv::Mat frame(
        height,
        width,
        CV_8UC3,
        cv::Scalar(0, 0, 0)
    );

    // Draw a green rectangle.
    cv::rectangle(
        frame,
        cv::Point(100, 100),
        cv::Point(300, 250),
        cv::Scalar(0, 255, 0),
        -1
    );

    // Draw a blue circle.
    cv::circle(
        frame,
        cv::Point(420, 200),
        70,
        cv::Scalar(255, 0, 0),
        -1
    );

    const std::string outputPath = "build/sample_frame.png";

    if (!cv::imwrite(outputPath, frame)) {
        std::cerr << "Error: Failed to save the frame.\n";
        return 1;
    }

    std::cout << "Camera ISP Workbench\n";
    std::cout << "Frame width: " << frame.cols << '\n';
    std::cout << "Frame height: " << frame.rows << '\n';
    std::cout << "Channels: " << frame.channels() << '\n';
    std::cout << "Saved: " << outputPath << '\n';

    return 0;
}