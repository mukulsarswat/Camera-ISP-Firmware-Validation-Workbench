#include "SimulatedCameraDriver.hpp"

#include <stdexcept>
#include <utility>

namespace camera {

SimulatedCameraDriver::SimulatedCameraDriver(
    std::size_t bufferCapacity
)
    : bufferCapacity_(bufferCapacity) {
    if (bufferCapacity_ == 0) {
        throw std::invalid_argument(
            "Buffer capacity must be greater than zero"
        );
    }
}

bool SimulatedCameraDriver::isValidFrame(
    const cv::Mat& frame
) const {
    return !frame.empty() &&
           frame.depth() == CV_8U &&
           (frame.channels() == 1 || frame.channels() == 3);
}

bool SimulatedCameraDriver::initialize() {
    std::lock_guard<std::mutex> lock(mutex_);

    if (state_ != DriverState::Uninitialized &&
        state_ != DriverState::Stopped) {
        return false;
    }

    buffers_.clear();
    state_ = DriverState::Initialized;

    return true;
}

bool SimulatedCameraDriver::start() {
    std::lock_guard<std::mutex> lock(mutex_);

    if (state_ != DriverState::Initialized) {
        return false;
    }

    state_ = DriverState::Streaming;
    return true;
}

bool SimulatedCameraDriver::submitFrame(
    const cv::Mat& frame
) {
    if (!isValidFrame(frame)) {
        return false;
    }

    {
        std::lock_guard<std::mutex> lock(mutex_);

        if (state_ != DriverState::Streaming) {
            return false;
        }

        if (buffers_.size() >= bufferCapacity_) {
            ++droppedFrames_;
            return false;
        }

        // Clone the image so the producer can safely reuse its input.
        buffers_.push_back(frame.clone());
    }

    frameAvailable_.notify_one();
    return true;
}

cv::Mat SimulatedCameraDriver::acquireFrame(
    std::chrono::milliseconds timeout
) {
    if (timeout.count() < 0) {
        throw std::invalid_argument(
            "Timeout must not be negative"
        );
    }

    std::unique_lock<std::mutex> lock(mutex_);

    const bool available = frameAvailable_.wait_for(
        lock,
        timeout,
        [this] {
            return !buffers_.empty() ||
                   state_ != DriverState::Streaming;
        }
    );

    if (!available || buffers_.empty()) {
        return {};
    }

    cv::Mat frame = std::move(buffers_.front());
    buffers_.pop_front();

    return frame;
}

bool SimulatedCameraDriver::stop() {
    {
        std::lock_guard<std::mutex> lock(mutex_);

        if (state_ != DriverState::Streaming) {
            return false;
        }

        state_ = DriverState::Stopped;
    }

    frameAvailable_.notify_all();
    return true;
}

bool SimulatedCameraDriver::injectFault() {
    {
        std::lock_guard<std::mutex> lock(mutex_);

        if (state_ != DriverState::Streaming) {
            return false;
        }

        state_ = DriverState::Error;
        buffers_.clear();
    }

    frameAvailable_.notify_all();
    return true;
}

bool SimulatedCameraDriver::recover() {
    std::lock_guard<std::mutex> lock(mutex_);

    if (state_ != DriverState::Error) {
        return false;
    }

    buffers_.clear();
    state_ = DriverState::Initialized;

    return true;
}

DriverState SimulatedCameraDriver::state() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return state_;
}

std::size_t SimulatedCameraDriver::droppedFrames() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return droppedFrames_;
}

std::size_t SimulatedCameraDriver::queuedFrames() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return buffers_.size();
}

}