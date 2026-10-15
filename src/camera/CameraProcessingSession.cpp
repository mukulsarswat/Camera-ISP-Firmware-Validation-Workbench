#include "CameraProcessingSession.hpp"

#include <stdexcept>
#include <utility>

namespace camera {

CameraProcessingSession::CameraProcessingSession(
    const isp::ISPConfig& config,
    std::size_t bufferCapacity
)
    : driver_(bufferCapacity),
      pipeline_(config) {}

bool CameraProcessingSession::start() {
    if (driver_.state() == DriverState::Initialized) {
        return driver_.start();
    }

    if (driver_.state() != DriverState::Uninitialized &&
        driver_.state() != DriverState::Stopped) {
        return false;
    }

    if (!driver_.initialize()) {
        return false;
    }

    return driver_.start();
}

bool CameraProcessingSession::submitFrame(
    const cv::Mat& frame
) {
    return driver_.submitFrame(frame);
}

CameraProcessingResult CameraProcessingSession::processNext(
    std::chrono::milliseconds timeout
) {
    CameraProcessingResult result;

    cv::Mat frame = driver_.acquireFrame(timeout);

    result.droppedFrames = driver_.droppedFrames();

    if (frame.empty()) {
        return result;
    }

    result.frameAcquired = true;
    result.ispResult = pipeline_.processWithMetrics(frame);
    result.droppedFrames = driver_.droppedFrames();

    return result;
}

bool CameraProcessingSession::stop() {
    return driver_.stop();
}

bool CameraProcessingSession::injectFault() {
    return driver_.injectFault();
}

bool CameraProcessingSession::recover() {
    return driver_.recover();
}

DriverState CameraProcessingSession::state() const {
    return driver_.state();
}

std::size_t CameraProcessingSession::droppedFrames() const {
    return driver_.droppedFrames();
}

}