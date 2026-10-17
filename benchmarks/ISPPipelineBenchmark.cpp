
#include "../include/isp/ISPPipeline.hpp"

#include <opencv2/core.hpp>

#include <algorithm>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <stdexcept>
#include <string>
#include <vector>

using Clock = std::chrono::steady_clock;

struct BenchmarkStats {
    double meanMs = 0.0;
    double medianMs = 0.0;
    double p95Ms = 0.0;
    double maxMs = 0.0;
    double fps = 0.0;
};

BenchmarkStats summarize(std::vector<double> samples) {
    if (samples.empty()) {
        throw std::invalid_argument("Benchmark samples must not be empty");
    }

    std::sort(samples.begin(), samples.end());

    BenchmarkStats stats;

    stats.meanMs =
        std::accumulate(samples.begin(), samples.end(), 0.0)
        / static_cast<double>(samples.size());

    const std::size_t middle = samples.size() / 2;

    if (samples.size() % 2 == 0) {
        stats.medianMs =
            (samples[middle - 1] + samples[middle]) / 2.0;
    } else {
        stats.medianMs = samples[middle];
    }

    const std::size_t p95Index =
        (95 * samples.size() + 99) / 100 - 1;

    stats.p95Ms = samples[p95Index];
    stats.maxMs = samples.back();

    if (stats.meanMs > 0.0) {
        stats.fps = 1000.0 / stats.meanMs;
    }

    return stats;
}

BenchmarkStats benchmarkPipeline(
    int width,
    int height,
    int warmupIterations,
    int measuredIterations
) {
    cv::Mat input(
        height,
        width,
        CV_8UC3,
        cv::Scalar(80, 120, 160)
    );

    isp::ISPConfig config;
    isp::ISPPipeline pipeline(config);

    // Warm-up runs reduce the influence of first-use overhead.
    for (int i = 0; i < warmupIterations; ++i) {
        const cv::Mat output = pipeline.process(input);

        if (output.empty()) {
            throw std::runtime_error("ISP produced an empty output");
        }
    }

    std::vector<double> samples;
    samples.reserve(measuredIterations);

    for (int i = 0; i < measuredIterations; ++i) {
        const auto start = Clock::now();
        const cv::Mat output = pipeline.process(input);
        const auto end = Clock::now();

        if (output.empty() ||
            output.rows != height ||
            output.cols != width ||
            output.type() != input.type()) {
            throw std::runtime_error("ISP output validation failed");
        }

        const double elapsedMs =
            std::chrono::duration<double, std::milli>(
                end - start
            ).count();

        samples.push_back(elapsedMs);
    }

    return summarize(samples);
}

int main() {
    constexpr int warmupIterations = 3;
    constexpr int measuredIterations = 30;

    std::cout << std::fixed << std::setprecision(3);
    std::cout << "Camera ISP Pipeline Benchmark\n";
    std::cout << "Warm-up iterations: " << warmupIterations << '\n';
    std::cout << "Measured iterations: " << measuredIterations << "\n\n";

    std::cout
        << std::left << std::setw(14) << "Resolution"
        << std::right << std::setw(12) << "Mean (ms)"
        << std::setw(12) << "Median"
        << std::setw(12) << "P95"
        << std::setw(12) << "Max"
        << std::setw(12) << "FPS"
        << '\n';

    try {
        const int resolutions[][2] = {
            {640, 480},
            {1280, 720}
        };

        for (const auto& resolution : resolutions) {
            const int width = resolution[0];
            const int height = resolution[1];

            const BenchmarkStats stats = benchmarkPipeline(
                width,
                height,
                warmupIterations,
                measuredIterations
            );

            const std::string label =
                std::to_string(width) + "x" + std::to_string(height);

            std::cout
                << std::left << std::setw(14) << label
                << std::right << std::setw(12) << stats.meanMs
                << std::setw(12) << stats.medianMs
                << std::setw(12) << stats.p95Ms
                << std::setw(12) << stats.maxMs
                << std::setw(12) << stats.fps
                << '\n';
        }
    } catch (const std::exception& error) {
        std::cerr << "Benchmark failed: " << error.what() << '\n';
        return 1;
    }

    return 0;
}
