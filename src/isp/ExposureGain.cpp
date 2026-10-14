
#include "../../include/isp/ExposureGain.hpp"

#include <cmath>
#include <stdexcept>

namespace isp {

cv::Mat ExposureGain::apply(
    const cv::Mat& input,
    double exposureStops,
    double gain
) {
    if (input.empty()) {
        throw std::invalid_argument("Input frame must not be empty");
    }

    if (input.depth() != CV_8U) {
        throw std::invalid_argument("Input must use 8-bit pixels");
    }

    if (!std::isfinite(exposureStops) ||
        exposureStops < -10.0 ||
        exposureStops > 10.0) {
        throw std::invalid_argument("Exposure stops must be between -10 and 10");
    }

    if (!std::isfinite(gain) || gain < 0.0 || gain > 16.0) {
        throw std::invalid_argument("Gain must be between 0 and 16");
    }

    const double exposureMultiplier = std::pow(2.0, exposureStops);
    const double totalMultiplier = exposureMultiplier * gain;

    cv::Mat output;
    input.convertTo(output, input.type(), totalMultiplier);

    return output;
}

} // namespace isp
