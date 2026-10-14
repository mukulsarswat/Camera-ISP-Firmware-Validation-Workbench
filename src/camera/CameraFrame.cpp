
#include "CameraFrame.hpp"

namespace camera {

CameraFrame::CameraFrame(const cv::Mat& image)
    : image_(image.clone()) {
}

bool CameraFrame::isValid() const {
    if (image_.empty()) {
        return false;
    }

    if (image_.depth() != CV_8U) {
        return false;
    }

    const int channelCount = image_.channels();

    return channelCount == 1 || channelCount == 3;
}

int CameraFrame::width() const {
    return image_.cols;
}

int CameraFrame::height() const {
    return image_.rows;
}

int CameraFrame::channels() const {
    return image_.channels();
}

const cv::Mat& CameraFrame::image() const {
    return image_;
}

} // namespace camera
