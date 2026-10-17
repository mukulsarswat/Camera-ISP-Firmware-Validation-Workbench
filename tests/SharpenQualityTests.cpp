
#include "isp/Sharpen.hpp"

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

#include <iostream>

int main() {
    cv::setNumThreads(1);

    // Create an image with a clear vertical edge.
    cv::Mat clean(128, 128, CV_8UC1, cv::Scalar(40));
    clean.colRange(64, 128).setTo(180);

    // Blur the image to soften the edge.
    cv::Mat softened;
    cv::GaussianBlur(
        clean,
        softened,
        cv::Size(9, 9),
        2.0
    );

    // Sharpen the softened image.
    const cv::Mat sharpened =
        isp::Sharpen::apply(softened, 1.0, 1.0);

    // Compare edge strength using the Sobel gradient.
    auto meanGradient = [](const cv::Mat& image) {
        cv::Mat gx;
        cv::Mat gy;
        cv::Mat magnitude;

        cv::Sobel(image, gx, CV_32F, 1, 0);
        cv::Sobel(image, gy, CV_32F, 0, 1);
        cv::magnitude(gx, gy, magnitude);

        // Measure only a narrow region around the known edge.
        const cv::Mat edgeRegion =
            magnitude.colRange(58, 70);

        return cv::mean(edgeRegion)[0];
    };

    const double softenedEdge = meanGradient(softened);
    const double sharpenedEdge = meanGradient(sharpened);

    std::cout << "Softened edge strength: "
              << softenedEdge << '\n';
    std::cout << "Sharpened edge strength: "
              << sharpenedEdge << '\n';

    if (sharpenedEdge > softenedEdge &&
        sharpened.size() == softened.size() &&
        sharpened.type() == softened.type()) {
        std::cout << "[PASS] Sharpening enhanced edge strength\n";
        return 0;
    }

    std::cerr << "[FAIL] Sharpening quality check failed\n";
    return 1;
}
