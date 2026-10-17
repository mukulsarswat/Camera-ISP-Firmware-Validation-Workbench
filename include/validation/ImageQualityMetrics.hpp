
#pragma once

#include <opencv2/core.hpp>

namespace validation {

class ImageQualityMetrics {
public:
    // Mean squared error between an image and its reference.
    static double mse(
        const cv::Mat& reference,
        const cv::Mat& image
    );

    // Peak signal-to-noise ratio, expressed in decibels.
    static double psnr(
        const cv::Mat& reference,
        const cv::Mat& image
    );
};

} // namespace validation
