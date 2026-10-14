#include "../../include/isp/ISPPipeline.hpp"

#include "../../include/isp/Denoise.hpp"
#include "../../include/isp/ExposureGain.hpp"
#include "../../include/isp/Sharpen.hpp"
#include "../../include/isp/WhiteBalance.hpp"

#include <stdexcept>

namespace isp {

ISPPipeline::ISPPipeline(const ISPConfig& config)
    : config_(config) {}

cv::Mat ISPPipeline::process(const cv::Mat& input) const {
    if (input.empty()) {
        throw std::invalid_argument("Input frame must not be empty");
    }

    if (input.depth() != CV_8U ||
        (input.channels() != 1 && input.channels() != 3)) {
        throw std::invalid_argument(
            "Input must be an 8-bit grayscale or BGR frame"
        );
    }

    // Stage 1: Exposure and gain
    cv::Mat current = ExposureGain::apply(
        input,
        config_.exposureStops,
        config_.gain
    );

    // Stage 2: White balance (requires a BGR image)
    if (config_.enableWhiteBalance) {
        current = WhiteBalance::apply(
            current,
            config_.blueGain,
            config_.greenGain,
            config_.redGain
        );
    }

    // Stage 3: Edge-preserving denoising
    if (config_.enableDenoise) {
        current = Denoise::apply(
            current,
            config_.denoiseDiameter,
            config_.denoiseSigmaColor,
            config_.denoiseSigmaSpace
        );
    }

    // Stage 4: Sharpening
    if (config_.enableSharpen) {
        current = Sharpen::apply(
            current,
            config_.sharpenAmount,
            config_.sharpenSigma
        );
    }

    return current;
}

}