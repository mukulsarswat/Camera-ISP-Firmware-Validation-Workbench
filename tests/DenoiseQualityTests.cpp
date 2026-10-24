
#include "../include/isp/Denoise.hpp"
#include "../include/validation/ImageQualityMetrics.hpp"

#include <opencv2/core.hpp>

#include <iostream>
#include <stdexcept>

namespace {

cv::Mat createCleanImage() {
    cv::Mat clean(128, 128, CV_8UC1);

    for (int row = 0; row < clean.rows; ++row) {
        for (int col = 0; col < clean.cols; ++col) {
            clean.at<unsigned char>(row, col) =
                static_cast<unsigned char>(80 + col / 4);
        }
    }

    return clean;
}

cv::Mat addGaussianNoise(
    const cv::Mat& clean,
    double standardDeviation,
    unsigned int seed
) {
    cv::Mat noise(clean.size(), CV_16SC1);
    cv::RNG rng(seed);
    rng.fill(
        noise,
        cv::RNG::NORMAL,
        0.0,
        standardDeviation
    );

    cv::Mat clean16;
    clean.convertTo(clean16, CV_16SC1);
    clean16 += noise;

    cv::Mat noisy;
    clean16.convertTo(noisy, CV_8UC1);

    return noisy;
}

bool testNoiseLevel(
    const cv::Mat& clean,
    double noiseStdDev,
    unsigned int seed
) {
    const cv::Mat noisy =
        addGaussianNoise(clean, noiseStdDev, seed);

    const cv::Mat denoised =
        isp::Denoise::apply(noisy, 5, 25.0, 5.0);

    const double noisyMSE =
        validation::ImageQualityMetrics::mse(clean, noisy);
    const double denoisedMSE =
        validation::ImageQualityMetrics::mse(clean, denoised);

    const double noisyPSNR =
        validation::ImageQualityMetrics::psnr(clean, noisy);
    const double denoisedPSNR =
        validation::ImageQualityMetrics::psnr(clean, denoised);

    std::cout << "Noise stddev: " << noiseStdDev << '\n'
              << "  Noisy MSE:     " << noisyMSE << '\n'
              << "  Denoised MSE:  " << denoisedMSE << '\n'
              << "  Noisy PSNR:    " << noisyPSNR << " dB\n"
              << "  Denoised PSNR: " << denoisedPSNR << " dB\n";

    if (denoised.size() != clean.size() ||
        denoised.type() != clean.type()) {
        std::cerr << "[FAIL] Output dimensions or type changed\n";
        return false;
    }

    if (!(denoisedMSE < noisyMSE &&
          denoisedPSNR > noisyPSNR)) {
        std::cerr << "[FAIL] Quality did not improve at noise stddev "
                  << noiseStdDev << '\n';
        return false;
    }

    std::cout << "[PASS] Noise stddev " << noiseStdDev
              << ": image quality improved\n";
    return true;
}

} // namespace

int main() {
    cv::setNumThreads(1);

    const cv::Mat clean = createCleanImage();

    const bool lowNoise = testNoiseLevel(clean, 5.0, 12345);
    const bool mediumNoise = testNoiseLevel(clean, 15.0, 23456);
    const bool highNoise = testNoiseLevel(clean, 25.0, 34567);

    if (lowNoise && mediumNoise && highNoise) {
        std::cout << "[PASS] All denoising quality scenarios passed\n";
        return 0;
    }

    std::cerr << "[FAIL] One or more denoising quality scenarios failed\n";
    return 1;
}
