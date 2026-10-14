#include "../../include/diagnostics/FrameDiagnostics.hpp"

#include <fstream>
#include <iomanip>

namespace diagnostics {

bool FrameDiagnostics::writeCsv(
    const std::string& path,
    int frameId,
    const cv::Mat& input,
    const isp::ISPResult& result
) {
    std::ofstream file(path);

    if (!file.is_open()) {
        return false;
    }

    auto getDuration = [&](const std::string& stageName) {
        for (const auto& stage : result.stages) {
            if (stage.name == stageName) {
                return stage.durationMs;
            }
        }

        return 0.0;
    };

    file << "frame_id,input_width,input_height,input_channels,"
            "output_width,output_height,output_channels,"
            "exposure_gain_ms,white_balance_ms,denoise_ms,"
            "sharpen_ms,total_ms,output_valid\n";

    file << std::fixed << std::setprecision(4);

    file << frameId << ','
         << input.cols << ','
         << input.rows << ','
         << input.channels() << ','
         << result.image.cols << ','
         << result.image.rows << ','
         << result.image.channels() << ','
         << getDuration("exposure_gain") << ','
         << getDuration("white_balance") << ','
         << getDuration("denoise") << ','
         << getDuration("sharpen") << ','
         << result.totalDurationMs << ','
         << (result.outputValid ? "PASS" : "FAIL")
         << '\n';

    return file.good();
}

}