#pragma once

#include <opencv2/core.hpp>

#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <deque>
#include <mutex>

namespace camera {

enum class DriverState {
    Uninitialized,
    Initialized,
    Streaming,
    Stopped,
    Error
};

class SimulatedCameraDriver {
public:
    explicit SimulatedCameraDriver(std::size_t bufferCapacity = 3);

    bool initialize();
    bool start();

    bool submitFrame(const cv::Mat& frame);

    cv::Mat acquireFrame(std::chrono::milliseconds timeout);

    bool stop();
    bool injectFault();
    bool recover();

    DriverState state() const;
    std::size_t droppedFrames() const;
    std::size_t queuedFrames() const;

private:
    bool isValidFrame(const cv::Mat& frame) const;

    const std::size_t bufferCapacity_;

    mutable std::mutex mutex_;
    std::condition_variable frameAvailable_;
    std::deque<cv::Mat> buffers_;

    DriverState state_ = DriverState::Uninitialized;
    std::size_t droppedFrames_ = 0;
};

}