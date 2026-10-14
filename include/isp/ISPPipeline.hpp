#pragma once

#include "ISPConfig.hpp"

#include <opencv2/core.hpp>

namespace isp {

class ISPPipeline {
public:
    explicit ISPPipeline(const ISPConfig& config);

    cv::Mat process(const cv::Mat& input) const;

private:
    ISPConfig config_;
};

}