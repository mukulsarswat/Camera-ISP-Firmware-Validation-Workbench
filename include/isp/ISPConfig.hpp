#pragma once

namespace isp {

struct ISPConfig {
    // Exposure and brightness
    double exposureStops = 0.0;
    double gain = 1.0;

    // White balance gains for BGR channels
    bool enableWhiteBalance = true;
    double blueGain = 1.0;
    double greenGain = 1.0;
    double redGain = 1.0;

    // Denoising
    bool enableDenoise = true;
    int denoiseDiameter = 5;
    double denoiseSigmaColor = 25.0;
    double denoiseSigmaSpace = 25.0;

    // Sharpening
    bool enableSharpen = true;
    double sharpenAmount = 1.0;
    double sharpenSigma = 1.0;
};

}