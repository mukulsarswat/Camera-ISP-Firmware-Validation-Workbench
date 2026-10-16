#include "../../include/camera/IntegratedFrameProcessor.hpp"

namespace camera {

IntegratedFrameProcessor::IntegratedFrameProcessor(
    const isp::ISPConfig& config,
    std::size_t queueCapacity
)
    : scheduler_(queueCapacity),
      pipeline_(config) {}

bool IntegratedFrameProcessor::submitFrame(
    const cv::Mat& frame,
    double arrivalTimeMs,
    double deadlineMs,
    double estimatedProcessingMs
) {
    return scheduler_.submitFrame(
        frame,
        arrivalTimeMs,
        deadlineMs,
        estimatedProcessingMs
    );
}

bool IntegratedFrameProcessor::processNext(
    IntegratedFrameResult& result
) {
    SchedulingResult scheduled;

    if (!scheduler_.processNext(scheduled)) {
        return false;
    }

    result.scheduling = scheduled;
    result.ispResult = pipeline_.processWithMetrics(scheduled.image);

    // The budget is the time available from frame arrival to its deadline.
    result.processingBudgetMs =
        scheduled.deadlineMs - scheduled.arrivalTimeMs;

    // This compares measured ISP execution time with the available budget.
    // It does not include simulated queue waiting time.
    result.measuredProcessingWithinBudget =
        result.ispResult.outputValid &&
        result.ispResult.totalDurationMs <= result.processingBudgetMs;

    return true;
}

std::size_t IntegratedFrameProcessor::droppedFrames() const {
    return scheduler_.droppedFrames();
}

std::size_t IntegratedFrameProcessor::lateFrames() const {
    return scheduler_.lateFrames();
}

}