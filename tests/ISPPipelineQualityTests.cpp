#include <algorithm>
#include "../include/isp/ISPConfig.hpp"
#include "../include/isp/ISPPipeline.hpp"
#include "../include/validation/ImageQualityMetrics.hpp"

#include <opencv2/core.hpp>

#include <cmath>
#include <iostream>
#include <stdexcept>
#include <string>

namespace
{

    void require(bool condition, const std::string &message)
    {
        if (!condition)
        {
            throw std::runtime_error(message);
        }
    }

    cv::Mat createCleanImage()
    {
        constexpr int width = 128;
        constexpr int height = 128;

        cv::Mat image(height, width, CV_8UC3);

        for (int row = 0; row < height; ++row)
        {
            for (int col = 0; col < width; ++col)
            {
                const auto value = static_cast<unsigned char>(
                    60 + (col * 100 / (width - 1)));

                // BGR image with identical channels.
                image.at<cv::Vec3b>(row, col) =
                    cv::Vec3b(value, value, value);
            }
        }

        return image;
    }

    cv::Mat addGaussianNoise(const cv::Mat &cleanImage)
    {
        cv::Mat noise(cleanImage.size(), CV_16SC3);
        cv::Mat noisyImage(cleanImage.size(), CV_8UC3);

        cv::RNG rng(12345);
        rng.fill(noise, cv::RNG::NORMAL, 0.0, 12.0);

        for (int row = 0; row < cleanImage.rows; ++row)
        {
            for (int col = 0; col < cleanImage.cols; ++col)
            {
                const cv::Vec3b clean =
                    cleanImage.at<cv::Vec3b>(row, col);
                const cv::Vec3s noisePixel =
                    noise.at<cv::Vec3s>(row, col);

                cv::Vec3b noisy;

                for (int channel = 0; channel < 3; ++channel)
                {
                    const int value =
                        static_cast<int>(clean[channel]) +
                        static_cast<int>(noisePixel[channel]);

                    noisy[channel] = static_cast<unsigned char>(
                        std::clamp(value, 0, 255));
                }

                noisyImage.at<cv::Vec3b>(row, col) = noisy;
            }
        }

        return noisyImage;
    }

    void testEndToEndISPQuality()
    {
        const cv::Mat cleanImage = createCleanImage();
        const cv::Mat noisyImage = addGaussianNoise(cleanImage);

        isp::ISPConfig config;
        isp::ISPPipeline pipeline(config);

        const isp::ISPResult result =
            pipeline.processWithMetrics(noisyImage);

        require(result.outputValid,
                "ISP pipeline reported an invalid output");

        require(!result.image.empty(),
                "ISP output image is empty");

        require(result.image.size() == cleanImage.size(),
                "ISP output dimensions changed");

        require(result.image.type() == cleanImage.type(),
                "ISP output type changed");

        const double inputMSE =
            validation::ImageQualityMetrics::mse(
                noisyImage, cleanImage);

        const double outputMSE =
            validation::ImageQualityMetrics::mse(
                result.image, cleanImage);

        const double inputPSNR =
            validation::ImageQualityMetrics::psnr(
                noisyImage, cleanImage);

        const double outputPSNR =
            validation::ImageQualityMetrics::psnr(
                result.image, cleanImage);

        std::cout << "\nEnd-to-end ISP quality results\n";
        std::cout << "Input MSE:   " << inputMSE << '\n';
        std::cout << "Output MSE:  " << outputMSE << '\n';
        std::cout << "Input PSNR:  " << inputPSNR << " dB\n";
        std::cout << "Output PSNR: " << outputPSNR << " dB\n";

        require(result.stages.size() == 4,
                "Expected four ISP stage metrics");

        for (const auto &stage : result.stages)
        {
            require(!stage.name.empty(),
                    "ISP stage name must not be empty");

            require(std::isfinite(stage.durationMs) &&
                        stage.durationMs >= 0.0,
                    "ISP stage duration is invalid");

            std::cout << "Stage: " << stage.name
                      << " | " << stage.durationMs << " ms\n";
        }

        require(std::isfinite(result.totalDurationMs) &&
                    result.totalDurationMs > 0.0,
                "Total ISP processing duration is invalid");

        // Quality is measured against the known clean reference.
        // We will use the observed results to establish a reliable
        // improvement threshold for this test image.
        require(std::isfinite(inputMSE) &&
                    std::isfinite(outputMSE) &&
                    std::isfinite(inputPSNR) &&
                    std::isfinite(outputPSNR),
                "Image quality metrics must be finite");

        require(outputMSE < inputMSE,
                "ISP pipeline did not reduce MSE");

        require(outputPSNR > inputPSNR,
                "ISP pipeline did not improve PSNR");

        std::cout << "End-to-end ISP quality test passed.\n";
    }

} // namespace

int main()
{
    try
    {
        cv::setNumThreads(1);
        testEndToEndISPQuality();
        return 0;
    }
    catch (const std::exception &error)
    {
        std::cerr << "Test failed: " << error.what() << '\n';
        return 1;
    }
}
