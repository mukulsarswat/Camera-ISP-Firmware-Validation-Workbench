#include "../include/diagnostics/FrameDiagnostics.hpp"
#include "../include/isp/ISPPipeline.hpp"

#include <opencv2/core.hpp>

#include <cstdio>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>

int main() {
    cv::setNumThreads(1);
    cv::Mat input(64, 80, CV_8UC3, cv::Scalar(100, 120, 140));

    isp::ISPConfig config;
    isp::ISPPipeline pipeline(config);

    const isp::ISPResult result = pipeline.processWithMetrics(input);

    if (result.image.empty() || !result.outputValid) {
        std::cerr << "FAIL: output validation\n";
        return 1;
    }

    if (result.totalDurationMs < 0.0 ||
        result.stages.size() != 4) {
        std::cerr << "FAIL: stage metrics\n";
        return 1;
    }

    for (const auto& stage : result.stages) {
        if (stage.durationMs < 0.0) {
            std::cerr << "FAIL: negative stage duration\n";
            return 1;
        }
    }

    const std::string path = "validation_report.csv";

    if (!diagnostics::FrameDiagnostics::writeCsv(
            path, 1, input, result)) {
        std::cerr << "FAIL: could not write CSV report\n";
        return 1;
    }

    std::ifstream file(path);

    if (!file.is_open()) {
        std::cerr << "FAIL: could not read CSV report\n";
        return 1;
    }

    const std::string csv(
        (std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>()
    );

    if (csv.find("frame_id") == std::string::npos ||
        csv.find("output_valid") == std::string::npos ||
        csv.find("PASS") == std::string::npos) {
        std::cerr << "FAIL: CSV content is incomplete\n";
        return 1;
    }

    std::cout << "All frame diagnostics tests passed\n";
    return 0;
}