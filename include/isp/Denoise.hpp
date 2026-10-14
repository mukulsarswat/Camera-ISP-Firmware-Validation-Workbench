#pragma once

#include <opencv2/core.hpp>

namespace isp {

class Denoise {
public:
    static cv::Mat apply(
        const cv::Mat& input,
        int diameter = 5,
        double sigmaColor = 25.0,
        double sigmaSpace = 25.0
    );
};

} // namespace isp