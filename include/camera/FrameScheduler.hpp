#pragma once

#include <opencv2/core.hpp>

#include <cstddef>
#include <cstdint>
#include <deque>

namespace camera {

struct ScheduledFrame {
    std::uint64_t sequence = 0;
    cv::Mat image;

    double arrivalTimeMs = 0.0;
    double deadlineMs = 0.0;
    double simulatedProcessingMs = 0.0;
};

struct SchedulingResult {
    std::uint64_t sequence = 0;

    double arrivalTimeMs = 0.0;
    double startTimeMs = 0.0;
    double completionTimeMs = 0.0;
    double deadlineMs = 0.0;

    bool deadlineMet = false;
    cv::Mat image;
};

class FrameScheduler {
public:
    explicit FrameScheduler(std::size_t queueCapacity = 3);

    bool submitFrame(
        const cv::Mat& frame,
        double arrivalTimeMs,
        double deadlineMs,
        double simulatedProcessingMs
    );

    bool processNext(SchedulingResult& result);

    std::size_t queuedFrames() const;
    std::size_t droppedFrames() const;
    std::size_t lateFrames() const;
    std::uint64_t submittedFrames() const;
    double simulationTimeMs() const;

private:
    const std::size_t queueCapacity_;

    std::deque<ScheduledFrame> queue_;

    std::uint64_t nextSequence_ = 1;
    std::uint64_t submittedFrames_ = 0;

    std::size_t droppedFrames_ = 0;
    std::size_t lateFrames_ = 0;

    double simulationTimeMs_ = 0.0;
};

}