#include "../../include/camera/FrameScheduler.hpp"

#include <cmath>
#include <stdexcept>

namespace camera {

FrameScheduler::FrameScheduler(std::size_t queueCapacity)
    : queueCapacity_(queueCapacity) {
    if (queueCapacity_ == 0) {
        throw std::invalid_argument(
            "Queue capacity must be greater than zero"
        );
    }
}

bool FrameScheduler::submitFrame(
    const cv::Mat& frame,
    double arrivalTimeMs,
    double deadlineMs,
    double simulatedProcessingMs
) {
    if (frame.empty() ||
        frame.depth() != CV_8U ||
        (frame.channels() != 1 && frame.channels() != 3)) {
        return false;
    }

    if (!std::isfinite(arrivalTimeMs) ||
        !std::isfinite(deadlineMs) ||
        !std::isfinite(simulatedProcessingMs) ||
        arrivalTimeMs < 0.0 ||
        deadlineMs < arrivalTimeMs ||
        simulatedProcessingMs < 0.0) {
        return false;
    }

    if (arrivalTimeMs < simulationTimeMs_) {
        return false;
    }

    if (queue_.size() >= queueCapacity_) {
        ++droppedFrames_;
        return false;
    }

    ScheduledFrame job;
    job.sequence = nextSequence_++;
    job.image = frame.clone();
    job.arrivalTimeMs = arrivalTimeMs;
    job.deadlineMs = deadlineMs;
    job.simulatedProcessingMs = simulatedProcessingMs;

    queue_.push_back(job);
    ++submittedFrames_;

    return true;
}

bool FrameScheduler::processNext(SchedulingResult& result) {
    if (queue_.empty()) {
        return false;
    }

    ScheduledFrame job = queue_.front();
    queue_.pop_front();

    const double startTime =
        simulationTimeMs_ > job.arrivalTimeMs
            ? simulationTimeMs_
            : job.arrivalTimeMs;

    const double completionTime =
        startTime + job.simulatedProcessingMs;

    result.sequence = job.sequence;
    result.arrivalTimeMs = job.arrivalTimeMs;
    result.startTimeMs = startTime;
    result.completionTimeMs = completionTime;
    result.deadlineMs = job.deadlineMs;
    result.deadlineMet = completionTime <= job.deadlineMs;
    result.image = job.image;

    simulationTimeMs_ = completionTime;

    if (!result.deadlineMet) {
        ++lateFrames_;
    }

    return true;
}

std::size_t FrameScheduler::queuedFrames() const {
    return queue_.size();
}

std::size_t FrameScheduler::droppedFrames() const {
    return droppedFrames_;
}

std::size_t FrameScheduler::lateFrames() const {
    return lateFrames_;
}

std::uint64_t FrameScheduler::submittedFrames() const {
    return submittedFrames_;
}

double FrameScheduler::simulationTimeMs() const {
    return simulationTimeMs_;
}

}