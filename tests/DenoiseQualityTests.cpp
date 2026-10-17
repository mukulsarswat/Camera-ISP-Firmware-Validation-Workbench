
#include "../include/isp/Denoise.hpp"
#include "../include/validation/ImageQualityMetrics.hpp"

#include <opencv2/core.hpp>

#include <iostream>

int main() {
    cv::setNumThreads(1);

    // Create a clean image with a smooth gradient.
    cv::Mat clean(128, 128, CV_8UC1);

    for (int row = 0; row < clean.rows; ++row) {
        for (int col = 0; col < clean.cols; ++col) {
            clean.at<unsigned char>(row, col) =
                static_cast<unsigned char>(80 + col / 4);
        }
    }

    // Add reproducible Gaussian noise.
    cv::Mat noise(clean.size(), CV_16SC1);
    cv::RNG rng(12345);
    rng.fill(noise, cv::RNG::NORMAL, 0.0, 15.0);

    cv::Mat noisy16;
    clean.convertTo(noisy16, CV_16SC1);
    noisy16 += noise;

    cv::Mat noisy;
    noisy16.convertTo(noisy, CV_8UC1);

    // Apply the existing denoising implementation.
    const cv::Mat denoised = isp::Denoise::apply(
        noisy, 5, 25.0, 5.0
    );

    const double noisyMSE =
        validation::ImageQualityMetrics::mse(clean, noisy);

    const double denoisedMSE =
        validation::ImageQualityMetrics::mse(clean, denoised);

    const double noisyPSNR =
        validation::ImageQualityMetrics::psnr(clean, noisy);

    const double denoisedPSNR =
        validation::ImageQualityMetrics::psnr(clean, denoised);

    std::cout << "Noisy MSE:      " << noisyMSE << '\n';
    std::cout << "Denoised MSE:   " << denoisedMSE << '\n';
    std::cout << "Noisy PSNR:     " << noisyPSNR << " dB\n";
    std::cout << "Denoised PSNR:  " << denoisedPSNR << " dB\n";

    if (denoisedMSE < noisyMSE && denoisedPSNR > noisyPSNR) {
        std::cout << "[PASS] Denoising improved reference-image quality\n";
        return 0;
    }

    std::cerr << "[FAIL] Denoising did not improve reference-image quality\n";
    return 1;
}
