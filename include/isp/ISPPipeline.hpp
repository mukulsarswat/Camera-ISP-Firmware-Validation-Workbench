#pragma once

#include "ISPConfig.hpp"

#include <opencv2/core.hpp>

#include <string>
#include <vector>

namespace isp {

struct StageMetric {
    std::string name;
    double durationMs = 0.0;
};

struct ISPResult {
    cv::Mat image;
    std::vector<StageMetric> stages;
    double totalDurationMs = 0.0;
    bool outputValid = false;
};

class ISPPipeline {
public:
    explicit ISPPipeline(const ISPConfig& config);

    cv::Mat process(const cv::Mat& input) const;

    ISPResult processWithMetrics(const cv::Mat& input) const;

private:
    ISPConfig config_;
};

}