cmake_minimum_required(VERSION 3.20)

project(CameraISPFirmwareWorkbench
    VERSION 0.2.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(OpenCV REQUIRED COMPONENTS
    core
    imgproc
    imgcodecs
)

add_executable(camera_isp
    src/main.cpp
    src/camera/CameraFrame.cpp
)

target_link_libraries(camera_isp
    PRIVATE ${OpenCV_LIBS}
)

target_include_directories(camera_isp
    PRIVATE ${OpenCV_INCLUDE_DIRS}
)



enable_testing()

add_executable(camera_frame_tests
    tests/CameraFrameTests.cpp
    src/camera/CameraFrame.cpp
)

target_include_directories(camera_frame_tests
    PRIVATE
        include
        ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(camera_frame_tests
    PRIVATE ${OpenCV_LIBS}
)

add_test(
    NAME CameraFrameTests
    COMMAND camera_frame_tests
)

target_sources(camera_isp PRIVATE
    src/isp/ExposureGain.cpp
)

add_executable(exposure_gain_tests
    tests/ExposureGainTests.cpp
    src/isp/ExposureGain.cpp
)

target_include_directories(exposure_gain_tests
    PRIVATE
        include
        ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(exposure_gain_tests
    PRIVATE ${OpenCV_LIBS}
)

add_test(
    NAME ExposureGainTests
    COMMAND exposure_gain_tests
)
target_sources(camera_isp PRIVATE
    src/isp/WhiteBalance.cpp
)

add_executable(white_balance_tests
    tests/WhiteBalanceTests.cpp
    src/isp/WhiteBalance.cpp
)

target_include_directories(white_balance_tests
    PRIVATE
        include
        ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(white_balance_tests
    PRIVATE ${OpenCV_LIBS}
)

add_test(
    NAME WhiteBalanceTests
    COMMAND white_balance_tests
)

target_sources(camera_isp PRIVATE
    src/isp/Denoise.cpp
)
add_executable(denoise_tests
    tests/DenoiseTests.cpp
    src/isp/Denoise.cpp
)

target_include_directories(denoise_tests
    PRIVATE
        include
        ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(denoise_tests
    PRIVATE ${OpenCV_LIBS}
)

add_test(
    NAME DenoiseTests
    COMMAND denoise_tests
)

target_sources(camera_isp
    PRIVATE
        src/isp/Sharpen.cpp
)

add_executable(sharpen_tests
    tests/SharpenTests.cpp
    src/isp/Sharpen.cpp
)

target_include_directories(sharpen_tests
    PRIVATE
        include
        ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(sharpen_tests
    PRIVATE
        ${OpenCV_LIBS}
)

add_test(
    NAME SharpenTests
    COMMAND sharpen_tests
)

target_sources(camera_isp
    PRIVATE
        src/isp/ISPPipeline.cpp
)

add_executable(isp_pipeline_tests
    tests/ISPPipelineTests.cpp
    src/isp/ISPPipeline.cpp
    src/isp/ExposureGain.cpp
    src/isp/WhiteBalance.cpp
    src/isp/Denoise.cpp
    src/isp/Sharpen.cpp
)

target_include_directories(isp_pipeline_tests
    PRIVATE
        include
        ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(isp_pipeline_tests
    PRIVATE
        ${OpenCV_LIBS}
)

add_test(
    NAME ISPPipelineTests
    COMMAND isp_pipeline_tests
)

add_executable(frame_diagnostics_tests
    tests/FrameDiagnosticsTests.cpp
    src/diagnostics/FrameDiagnostics.cpp
    src/isp/ISPPipeline.cpp
    src/isp/ExposureGain.cpp
    src/isp/WhiteBalance.cpp
    src/isp/Denoise.cpp
    src/isp/Sharpen.cpp
)

target_include_directories(frame_diagnostics_tests
    PRIVATE
        include
        ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(frame_diagnostics_tests
    PRIVATE
        ${OpenCV_LIBS}
)

add_test(
    NAME FrameDiagnosticsTests
    COMMAND frame_diagnostics_tests
)

add_executable(camera_driver_tests
    tests/SimulatedCameraDriverTests.cpp
    src/camera/SimulatedCameraDriver.cpp
)

target_include_directories(camera_driver_tests
    PRIVATE
        include
        ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(camera_driver_tests
    PRIVATE
        ${OpenCV_LIBS}
)

add_test(
    NAME SimulatedCameraDriverTests
    COMMAND camera_driver_tests
)
add_executable(camera_session_tests
    tests/CameraProcessingSessionTests.cpp
    src/camera/CameraProcessingSession.cpp
    src/camera/SimulatedCameraDriver.cpp
    src/isp/ISPPipeline.cpp
    src/isp/ExposureGain.cpp
    src/isp/WhiteBalance.cpp
    src/isp/Denoise.cpp
    src/isp/Sharpen.cpp
)

target_include_directories(camera_session_tests
    PRIVATE
        include
        ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(camera_session_tests
    PRIVATE
        ${OpenCV_LIBS}
)

add_test(
    NAME CameraProcessingSessionTests
    COMMAND camera_session_tests
)

add_executable(frame_scheduler_tests
    tests/FrameSchedulerTests.cpp
    src/camera/FrameScheduler.cpp
)

target_include_directories(frame_scheduler_tests
    PRIVATE
        include
        ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(frame_scheduler_tests
    PRIVATE
        ${OpenCV_LIBS}
)

add_test(
    NAME FrameSchedulerTests
    COMMAND frame_scheduler_tests
)

add_executable(integrated_frame_processor_tests
    tests/IntegratedFrameProcessorTests.cpp
    src/camera/IntegratedFrameProcessor.cpp
    src/camera/FrameScheduler.cpp
    src/isp/ISPPipeline.cpp
    src/isp/ExposureGain.cpp
    src/isp/WhiteBalance.cpp
    src/isp/Denoise.cpp
    src/isp/Sharpen.cpp
)

target_include_directories(integrated_frame_processor_tests
    PRIVATE
        include
        ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(integrated_frame_processor_tests
    PRIVATE
        ${OpenCV_LIBS}
)

add_test(
    NAME IntegratedFrameProcessorTests
    COMMAND integrated_frame_processor_tests
)