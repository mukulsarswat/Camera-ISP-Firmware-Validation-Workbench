#pragma once

#include "FrameScheduler.hpp"
#include "../isp/ISPPipeline.hpp"

#include <cstddef>

namespace camera {

struct IntegratedFrameResult {
    SchedulingResult scheduling;
    isp::ISPResult ispResult;

    double processingBudgetMs = 0.0;
    bool measuredProcessingWithinBudget = false;
};

class IntegratedFrameProcessor {
public:
    IntegratedFrameProcessor(
        const isp::ISPConfig& config,
        std::size_t queueCapacity = 3
    );

    bool submitFrame(
        const cv::Mat& frame,
        double arrivalTimeMs,
        double deadlineMs,
        double estimatedProcessingMs
    );

    bool processNext(IntegratedFrameResult& result);

    std::size_t droppedFrames() const;
    std::size_t lateFrames() const;

private:
    FrameScheduler scheduler_;
    isp::ISPPipeline pipeline_;
};

}