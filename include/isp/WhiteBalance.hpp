
#pragma once

#include <opencv2/core.hpp>

namespace isp {

class WhiteBalance {
public:
    // Gains are specified in BGR order to match OpenCV images.
    static cv::Mat apply(
        const cv::Mat& input,
        double blueGain,
        double greenGain,
        double redGain
    );
};

} // namespace isp
