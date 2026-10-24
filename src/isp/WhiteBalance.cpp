
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

    // Convert to floating point to perform channel-wise gain adjustment.
    cv::Mat output;
    input.convertTo(output, CV_32FC3);

    // OpenCV images use BGR channel order.
    const cv::Scalar gainScale(
        blueGain,
        greenGain,
        redGain
    );

    // Apply the corresponding gain to each channel.
    cv::multiply(output, gainScale, output);

    // Convert back to 8-bit, saturating values to the valid range.
    output.convertTo(output, CV_8UC3);

    return output;
}

} // namespace isp
