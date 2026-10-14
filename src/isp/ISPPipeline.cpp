#include "../../include/isp/ISPPipeline.hpp"

#include "../../include/isp/Denoise.hpp"
#include "../../include/isp/ExposureGain.hpp"
#include "../../include/isp/Sharpen.hpp"
#include "../../include/isp/WhiteBalance.hpp"

#include <chrono>
#include <stdexcept>

namespace isp {

ISPPipeline::ISPPipeline(const ISPConfig& config)
    : config_(config) {}

cv::Mat ISPPipeline::process(const cv::Mat& input) const {
    return processWithMetrics(input).image;
}

ISPResult ISPPipeline::processWithMetrics(
    const cv::Mat& input
) const {
    if (input.empty()) {
        throw std::invalid_argument("Input frame must not be empty");
    }

    if (input.depth() != CV_8U ||
        (input.channels() != 1 && input.channels() != 3)) {
        throw std::invalid_argument(
            "Input must be an 8-bit grayscale or BGR frame"
        );
    }

    using Clock = std::chrono::steady_clock;

    const auto pipelineStart = Clock::now();

    ISPResult result;
    cv::Mat current = input.clone();

    auto runStage = [&](const char* name, auto operation) {
        const auto start = Clock::now();

        current = operation(current);

        const auto end = Clock::now();

        const double durationMs =
            std::chrono::duration<double, std::milli>(
                end - start
            ).count();

        result.stages.push_back({name, durationMs});
    };

    runStage("exposure_gain", [&](const cv::Mat& frame) {
        return ExposureGain::apply(
            frame,
            config_.exposureStops,
            config_.gain
        );
    });

    if (config_.enableWhiteBalance) {
        runStage("white_balance", [&](const cv::Mat& frame) {
            return WhiteBalance::apply(
                frame,
                config_.blueGain,
                config_.greenGain,
                config_.redGain
            );
        });
    }

    if (config_.enableDenoise) {
        runStage("denoise", [&](const cv::Mat& frame) {
            return Denoise::apply(
                frame,
                config_.denoiseDiameter,
                config_.denoiseSigmaColor,
                config_.denoiseSigmaSpace
            );
        });
    }

    if (config_.enableSharpen) {
        runStage("sharpen", [&](const cv::Mat& frame) {
            return Sharpen::apply(
                frame,
                config_.sharpenAmount,
                config_.sharpenSigma
            );
        });
    }

    result.image = current;

    result.outputValid =
        !result.image.empty() &&
        result.image.size() == input.size() &&
        result.image.type() == input.type();

    result.totalDurationMs =
        std::chrono::duration<double, std::milli>(
            Clock::now() - pipelineStart
        ).count();

    return result;
}

}