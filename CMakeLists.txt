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
)

target_link_libraries(camera_isp
    PRIVATE ${OpenCV_LIBS}
)

target_include_directories(camera_isp
    PRIVATE ${OpenCV_INCLUDE_DIRS}
)