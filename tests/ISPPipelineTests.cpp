#include "../include/isp/ISPPipeline.hpp"

#include <opencv2/core.hpp>

#include <iostream>
#include <stdexcept>

bool testNeutralConfigurationPreservesUniformImage() {
    cv::Mat input(64, 64, CV_8UC3, cv::Scalar(100, 120, 140));

    isp::ISPConfig config;
    config.enableDenoise = false;
    config.enableSharpen = false;

    isp::ISPPipeline pipeline(config);
    cv::Mat output = pipeline.process(input);

    return cv::norm(input, output, cv::NORM_INF) == 0.0;
}

bool testPipelinePreservesFrameProperties() {
    cv::Mat input(120, 160, CV_8UC3, cv::Scalar(80, 100, 120));

    isp::ISPConfig config;
    isp::ISPPipeline pipeline(config);

    cv::Mat output = pipeline.process(input);

    return output.rows == input.rows &&
           output.cols == input.cols &&
           output.type() == input.type();
}

bool testGrayscaleWithoutWhiteBalance() {
    cv::Mat input(48, 48, CV_8UC1, cv::Scalar(100));

    isp::ISPConfig config;
    config.enableWhiteBalance = false;

    isp::ISPPipeline pipeline(config);
    cv::Mat output = pipeline.process(input);

    return output.channels() == 1 &&
           output.rows == input.rows &&
           output.cols == input.cols;
}

bool testEmptyInputRejected() {
    isp::ISPConfig config;
    isp::ISPPipeline pipeline(config);

    try {
        pipeline.process(cv::Mat{});
    } catch (const std::invalid_argument&) {
        return true;
    }

    return false;
}

int main() {
    cv::setNumThreads(1);
    int failures = 0;

    if (!testNeutralConfigurationPreservesUniformImage()) {
        std::cerr << "FAIL: neutral configuration changed uniform pixels\n";
        ++failures;
    }

    if (!testPipelinePreservesFrameProperties()) {
        std::cerr << "FAIL: frame properties changed\n";
        ++failures;
    }

    if (!testGrayscaleWithoutWhiteBalance()) {
        std::cerr << "FAIL: grayscale processing failed\n";
        ++failures;
    }

    if (!testEmptyInputRejected()) {
        std::cerr << "FAIL: empty input was not rejected\n";
        ++failures;
    }

    if (failures == 0) {
        std::cout << "All ISP pipeline tests passed\n";
        return 0;
    }

    return 1;
}