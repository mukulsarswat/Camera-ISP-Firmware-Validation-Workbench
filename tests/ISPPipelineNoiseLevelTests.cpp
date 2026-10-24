
#include "../include/isp/ISPConfig.hpp"
#include "../include/isp/ISPPipeline.hpp"
#include "../include/validation/ImageQualityMetrics.hpp"

#include <opencv2/core.hpp>

#include <algorithm>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace
{
    struct NoiseScenario
    {
        std::string name;
        double standardDeviation;
        unsigned int seed;
    };

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

                image.at<cv::Vec3b>(row, col) =
                    cv::Vec3b(value, value, value);
            }
        }

        return image;
    }

    cv::Mat addGaussianNoise(
        const cv::Mat &cleanImage,
        double standardDeviation,
        unsigned int seed)
    {
        cv::Mat cleanFloat;
        cleanImage.convertTo(cleanFloat, CV_32FC3);

        cv::Mat noise(cleanImage.size(), CV_32FC3);
        cv::RNG rng(seed);

        rng.fill(
            noise,
            cv::RNG::NORMAL,
            cv::Scalar::all(0.0),
            cv::Scalar::all(standardDeviation));

        cv::Mat noisyFloat = cleanFloat + noise;
        cv::Mat noisyImage;

        // Saturate only when converting back to 8-bit pixels.
        noisyFloat.convertTo(noisyImage, CV_8UC3);

        return noisyImage;
    }

    double testNoiseScenario(
        const cv::Mat &cleanImage,
        const NoiseScenario &scenario)
    {
        const cv::Mat noisyImage = addGaussianNoise(
            cleanImage,
            scenario.standardDeviation,
            scenario.seed);

        const double actualNoiseMSE =
            validation::ImageQualityMetrics::mse(
                noisyImage, cleanImage);

        std::cout << "\nRequested noise sigma: "
                  << scenario.standardDeviation
                  << "\nActual input noise MSE: "
                  << actualNoiseMSE << '\n';

        isp::ISPConfig config;

        // Disable sharpening to focus this test on denoising.
        config.enableSharpen = false;

        isp::ISPPipeline pipeline(config);

        const isp::ISPResult result =
            pipeline.processWithMetrics(noisyImage);

        require(
            result.outputValid,
            scenario.name + ": pipeline output is invalid");

        require(
            !result.image.empty(),
            scenario.name + ": output image is empty");

        require(
            result.image.size() == cleanImage.size(),
            scenario.name + ": output dimensions changed");

        require(
            result.image.type() == cleanImage.type(),
            scenario.name + ": output image type changed");
        

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

        require(
            std::isfinite(inputMSE) &&
                std::isfinite(outputMSE) &&
                std::isfinite(inputPSNR) &&
                std::isfinite(outputPSNR),
            scenario.name + ": quality metrics are not finite");

        std::cout << "\nScenario: " << scenario.name
                  << " (noise sigma = "
                  << scenario.standardDeviation << ")\n";

        std::cout << "Input MSE:   " << inputMSE << '\n';
        std::cout << "Output MSE:  " << outputMSE << '\n';
        std::cout << "Input PSNR:  " << inputPSNR << " dB\n";
        std::cout << "Output PSNR: " << outputPSNR << " dB\n";

        std::cout << "MSE improvement: "
                  << (inputMSE - outputMSE) / inputMSE * 100.0
                  << "%\n";

        std::cout << "PSNR change: "
                  << outputPSNR - inputPSNR << " dB\n";

        require(
            result.stages.size() == 3,
            scenario.name + ": expected three active ISP stages");

        require(
            std::isfinite(result.totalDurationMs) &&
                result.totalDurationMs > 0.0,
            scenario.name + ": invalid total processing time");
        require(
            !result.stages.empty(),
            scenario.name + ": ISP stage name must not be empty");

        for (const auto &stage : result.stages)
        {
            require(
                std::isfinite(stage.durationMs) &&
                    stage.durationMs >= 0.0,
                scenario.name + ": invalid stage timing");
        }

        require(
            outputMSE < inputMSE,
            scenario.name + ": ISP failed to reduce MSE");

        require(
            outputPSNR > inputPSNR,
            scenario.name + ": ISP failed to improve PSNR");

        std::cout << "Quality result: improved\n";

        return actualNoiseMSE;
    }
} // namespace

int main()
{
    try
    {
        cv::setNumThreads(1);

        const cv::Mat cleanImage = createCleanImage();

        const std::vector<NoiseScenario> scenarios = {
            {"Low noise", 5.0, 12345},
            {"Medium noise", 12.0, 23456},
            {"High noise", 25.0, 34567}};

        double previousNoiseMSE = -1.0;

        for (const auto &scenario : scenarios)
        {
            const double currentNoiseMSE =
                testNoiseScenario(cleanImage, scenario);

            if (previousNoiseMSE >= 0.0)
            {
                require(
                    currentNoiseMSE > previousNoiseMSE,
                    "Input noise MSE must increase with each noise scenario");
            }

            previousNoiseMSE = currentNoiseMSE;
        }

        std::cout
            << "\nAll noise-level ISP validation checks passed.\n";

        return 0;
    }
    catch (const std::exception &error)
    {
        std::cerr << "Test failed: " << error.what() << '\n';
        return 1;
    }
}
