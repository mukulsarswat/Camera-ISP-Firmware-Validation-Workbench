
#pragma once

#include <opencv2/core.hpp>

namespace isp {

class ExposureGain {
public:
    // exposure stops: 0 means unchanged, +1 doubles brightness.
    // gain: 1.0 means unchanged, 2.0 doubles brightness.
    static cv::Mat apply(
        const cv::Mat& input,
        double exposureStops,
        double gain
    );
};

} // namespace isp
