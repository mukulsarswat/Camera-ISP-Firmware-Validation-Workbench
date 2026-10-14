#include "../../include/isp/WhiteBalance.hpp"

#include <cmath>
#include <stdexcept>

namespace isp {

cv::Mat WhiteBalance::apply(
    const cv::Mat& input,
    double blueGain,
    double greenGain,
    double redGain
) {
    if (input.empty()) {
        throw std::invalid_argument("Input frame must not be empty");
    }

    if (input.depth() != CV_8U || input.channels() != 3) {
        throw std::invalid_argument(
            "White balance requires an 8-bit, three-channel BGR image"
        );
    }

    const double gains[] = {blueGain, greenGain, redGain};

    for (double gain : gains) {
        if (!std::isfinite(gain) || gain < 0.0 || gain > 8.0) {
            throw std::invalid_argument(
                "White balance gains must be between 0 and 8"
            );
        }
    }

    cv::Mat output = input.clone();

    for (int row = 0; row < output.rows; ++row) {
        cv::Vec3b* pixels = output.ptr<cv::Vec3b>(row);

        for (int col = 0; col < output.cols; ++col) {
            pixels[col][0] = cv::saturate_cast<unsigned char>(
                pixels[col][0] * blueGain
            );

            pixels[col][1] = cv::saturate_cast<unsigned char>(
                pixels[col][1] * greenGain
            );

            pixels[col][2] = cv::saturate_cast<unsigned char>(
                pixels[col][2] * redGain
            );
        }
    }

    return output;
}

} // namespace isp
