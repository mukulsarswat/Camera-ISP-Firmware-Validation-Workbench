#pragma once

#include "../isp/ISPPipeline.hpp"

#include <string>

namespace diagnostics {

class FrameDiagnostics {
public:
    static bool writeCsv(
        const std::string& path,
        int frameId,
        const cv::Mat& input,
        const isp::ISPResult& result
    );
};

}