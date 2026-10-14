#pragma once

#include <opencv2/core.hpp>

namespace isp {

class Sharpen {
public:
    static cv::Mat apply(
        const cv::Mat& input,
        double amount = 1.0,
        double sigma = 1.0
    );
};

}