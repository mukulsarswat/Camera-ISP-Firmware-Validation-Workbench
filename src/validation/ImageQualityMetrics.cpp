
#include "../../include/validation/ImageQualityMetrics.hpp"

#include <cmath>
#include <stdexcept>

namespace validation {

namespace {

void validateImages(
    const cv::Mat& reference,
    const cv::Mat& image
) {
    if (reference.empty() || image.empty()) {
        throw std::invalid_argument("Images must not be empty");
    }

    if (reference.size() != image.size() ||
        reference.type() != image.type()) {
        throw std::invalid_argument(
            "Images must have identical dimensions and types"
        );
    }

    if (reference.depth() != CV_8U) {
        throw std::invalid_argument(
            "Images must use 8-bit pixels"
        );
    }
}

} // namespace

double ImageQualityMetrics::mse(
    const cv::Mat& reference,
    const cv::Mat& image
) {
    validateImages(reference, image);

    cv::Mat difference;
    cv::absdiff(reference, image, difference);
    difference.convertTo(difference, CV_64F);

    difference = difference.mul(difference);

    const cv::Scalar channelSums = cv::sum(difference);

    double total = 0.0;

    for (int channel = 0; channel < reference.channels(); ++channel) {
        total += channelSums[channel];
    }

    const double pixelCount =
        static_cast<double>(reference.total()) *
        reference.channels();

    return total / pixelCount;
}

double ImageQualityMetrics::psnr(
    const cv::Mat& reference,
    const cv::Mat& image
) {
    const double error = mse(reference, image);

    if (error == 0.0) {
        return INFINITY;
    }

    constexpr double maxPixelValue = 255.0;

    return 10.0 * std::log10(
        (maxPixelValue * maxPixelValue) / error
    );
}

} // namespace validation
