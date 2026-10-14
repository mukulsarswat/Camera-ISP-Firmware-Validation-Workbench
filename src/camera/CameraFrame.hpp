
#pragma once

#include <opencv2/core.hpp>

namespace camera {

class CameraFrame {
public:
    explicit CameraFrame(const cv::Mat& image);

    bool isValid() const;

    int width() const;
    int height() const;
    int channels() const;

    const cv::Mat& image() const;

private:
    cv::Mat image_;
};

} // namespace camera
