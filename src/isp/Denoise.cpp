#include "../../include/isp/Denoise.hpp"

#include <opencv2/imgproc.hpp>

#include <cmath>
#include <stdexcept>

namespace isp {

cv::Mat Denoise::apply(
    const cv::Mat& input,
    int diameter,
    double sigmaColor,
    double sigmaSpace
) {
    if (input.empty()) {
        throw std::invalid_argument("Input frame must not be empty");
    }

    if (input.depth() != CV_8U ||
        (input.channels() != 1 && input.channels() != 3)) {
        throw std::invalid_argument(
            "Denoising requires an 8-bit grayscale or BGR image"
        );
    }

    if (diameter <= 0) {
        throw std::invalid_argument(
            "Filter diameter must be positive"
        );
    }

    if (!std::isfinite(sigmaColor) ||
        !std::isfinite(sigmaSpace) ||
        sigmaColor <= 0.0 ||
        sigmaSpace <= 0.0) {
        throw std::invalid_argument(
            "Filter sigma values must be finite and positive"
        );
    }

    cv::Mat output;

    cv::bilateralFilter(
        input,
        output,
        diameter,
        sigmaColor,
        sigmaSpace
    );

    return output;
}

} // namespace isp