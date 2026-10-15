#pragma once

#include "SimulatedCameraDriver.hpp"
#include "../../include/isp/ISPPipeline.hpp"

#include <chrono>
#include <cstddef>

namespace camera {

struct CameraProcessingResult {
    bool frameAcquired = false;
    isp::ISPResult ispResult;
    std::size_t droppedFrames = 0;
};

class CameraProcessingSession {
public:
    CameraProcessingSession(
        const isp::ISPConfig& config,
        std::size_t bufferCapacity = 3
    );

    bool start();

    bool submitFrame(const cv::Mat& frame);

    CameraProcessingResult processNext(
        std::chrono::milliseconds timeout
    );

    bool stop();
    bool injectFault();
    bool recover();

    DriverState state() const;
    std::size_t droppedFrames() const;

private:
    SimulatedCameraDriver driver_;
    isp::ISPPipeline pipeline_;
};

}