
#include "../include/validation/ImageQualityMetrics.hpp"

#include <opencv2/core.hpp>

#include <cmath>
#include <iostream>
#include <stdexcept>

bool testIdenticalImages() {
    cv::Mat image(10, 10, CV_8UC3, cv::Scalar(50, 100, 150));

    return validation::ImageQualityMetrics::mse(image, image) == 0.0
        && std::isinf(
            validation::ImageQualityMetrics::psnr(image, image)
        );
}

bool testKnownMSE() {
    cv::Mat reference(2, 2, CV_8UC1, cv::Scalar(100));
    cv::Mat changed(2, 2, CV_8UC1, cv::Scalar(110));

    // Every pixel differs by 10, so MSE must be 10 * 10 = 100.
    return std::abs(
        validation::ImageQualityMetrics::mse(reference, changed) - 100.0
    ) < 1e-9;
}

bool testPSNRDecreasesWithLargerError() {
    cv::Mat reference(4, 4, CV_8UC1, cv::Scalar(100));
    cv::Mat smallError(4, 4, CV_8UC1, cv::Scalar(105));
    cv::Mat largeError(4, 4, CV_8UC1, cv::Scalar(120));

    const double smallPSNR =
        validation::ImageQualityMetrics::psnr(reference, smallError);

    const double largePSNR =
        validation::ImageQualityMetrics::psnr(reference, largeError);

    return smallPSNR > largePSNR;
}

bool testMismatchedImagesRejected() {
    cv::Mat first(10, 10, CV_8UC1, cv::Scalar(100));
    cv::Mat second(8, 8, CV_8UC1, cv::Scalar(100));

    try {
        validation::ImageQualityMetrics::mse(first, second);
    } catch (const std::invalid_argument&) {
        return true;
    }

    return false;
}

int main() {
    const bool results[] = {
        testIdenticalImages(),
        testKnownMSE(),
        testPSNRDecreasesWithLargerError(),
        testMismatchedImagesRejected()
    };

    const char* names[] = {
        "Identical images",
        "Known MSE value",
        "PSNR reflects larger error",
        "Mismatched images rejected"
    };

    int failures = 0;

    for (int i = 0; i < 4; ++i) {
        std::cout << (results[i] ? "[PASS] " : "[FAIL] ")
                  << names[i] << '\n';

        if (!results[i]) {
            ++failures;
        }
    }

    return failures == 0 ? 0 : 1;
}
