#include "../../include/isp/Sharpen.hpp"

#include <opencv2/imgproc.hpp>

#include <cmath>
#include <stdexcept>

namespace isp {

cv::Mat Sharpen::apply(
    const cv::Mat& input,
    double amount,
    double sigma
) {
    if (input.empty()) {
        throw std::invalid_argument("Input image must not be empty");
    }

    if (input.depth() != CV_8U ||
        (input.channels() != 1 && input.channels() != 3)) {
        throw std::invalid_argument(
            "Input must be an 8-bit grayscale or BGR image"
        );
    }

    if (!std::isfinite(amount) || amount < 0.0 || amount > 5.0) {
        throw std::invalid_argument("Amount must be between 0 and 5");
    }

    if (!std::isfinite(sigma) || sigma <= 0.0 || sigma > 10.0) {
        throw std::invalid_argument("Sigma must be greater than 0 and at most 10");
    }

    if (amount == 0.0) {
        return input.clone();
    }

    cv::Mat blurred;
    cv::GaussianBlur(
        input,
        blurred,
        cv::Size(0, 0),
        sigma
    );

    cv::Mat output;

    // Unsharp masking: original + amount * (original - blurred).
    cv::addWeighted(
        input,
        1.0 + amount,
        blurred,
        -amount,
        0.0,
        output
    );

    return output;
}

}