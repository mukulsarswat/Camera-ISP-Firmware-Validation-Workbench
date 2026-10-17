<div align="center">

# Camera ISP & Firmware Validation Workbench

**A C++17 software simulation of a camera image-processing pipeline and
firmware-style validation workflow**

![C++17](https://img.shields.io/badge/C%2B%2B-17-blue)
![CMake](https://img.shields.io/badge/CMake-3.20%2B-064F8C)
![OpenCV](https://img.shields.io/badge/OpenCV-4.x%2F5.x-green)
![Tests](https://img.shields.io/badge/tests-12%20registered-success)
![CI](https://img.shields.io/badge/CI-GitHub%20Actions-2088FF)

Designed as a portfolio project exploring camera ISP concepts, C++
modular design, simulated driver behavior, frame scheduling,
diagnostics, and regression testing.
:::

------------------------------------------------------------------------

## Overview

Camera ISP & Firmware Validation Workbench is a **host-side software
simulation** built with C++17, CMake, and OpenCV. It brings together an
ISP-inspired image-processing pipeline and firmware-style camera
validation components in one testable project.

The project is intended to demonstrate software engineering concepts
relevant to camera firmware and multimedia systems, including:

-   ISP-inspired stages: exposure/gain, white balance, denoising, and
    sharpening.
-   A simulated camera-driver lifecycle with bounded buffering,
    timeouts, fault injection, and recovery.
-   A frame scheduler that models arrivals, processing budgets,
    deadlines, and dropped or late frames.
-   Integration of scheduled frames with the ISP pipeline.
-   Per-stage timing and frame diagnostics, including CSV output.
-   Unit/regression tests and a GitHub Actions CI workflow.
-   A repeatable benchmark for comparing pipeline latency at two image
    resolutions.

> **Scope note:** This is a software simulation running on a
> general-purpose computer. It does not implement a Qualcomm hardware
> ISP, a real camera kernel driver, sensor control over I²C, DMA
> hardware buffers, or a production RTOS. Scheduler timing is simulated;
> measured ISP latency is observed on the host machine.

## Architecture

The diagram below shows the major data path and validation components.
The driver/session path and scheduler/integrated-processor path are
separate entry points that share the ISP pipeline concept.

``` mermaid
flowchart TD
    A([Synthetic or supplied frame]) --> B[Frame validation]
    B --> C{Valid frame?}
    C -- No --> X[Reject input / record failure]
    C -- Yes --> D

    subgraph DRIVER["Simulated camera-driver path"]
      D[SimulatedCameraDriver]
      D --> E[Bounded frame queue]
      E --> F[CameraProcessingSession]
    end

    subgraph SCHEDULER["Scheduling and deadline path"]
      G[FrameScheduler]
      G --> H[Sequence numbers and timing model]
      H --> I[Queue / deadline / drop accounting]
      I --> J[IntegratedFrameProcessor]
    end

    F --> K
    J --> K[ISP Pipeline]
    subgraph ISP["ISP-inspired processing stages"]
      K --> L[Exposure and gain]
      L --> M[White balance]
      M --> N[Denoising]
      N --> O[Sharpening]
    end

    O --> P[Processed frame]
    P --> Q[Output validation]
    Q --> R[Stage latency metrics]
    R --> S[CSV diagnostics / benchmark output]
    I --> T[Scheduling metrics]
    T --> U[Deadline, late-frame and drop results]

    V[Fault injection and recovery tests] -. validates .-> D
    V -. validates .-> G
    V -. validates .-> K

    classDef input fill:#E8F2FF,stroke:#2563EB,color:#172554,stroke-width:1.5px;
    classDef process fill:#E8F7EF,stroke:#15803D,color:#052E16,stroke-width:1.5px;
    classDef monitor fill:#FFF4DF,stroke:#D97706,color:#451A03,stroke-width:1.5px;
    classDef failure fill:#FDECEC,stroke:#DC2626,color:#450A0A,stroke-width:1.5px;
    class A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P input;
    class Q,R,S,T,U monitor;
    class X,V failure;
```

### Main modules

  -----------------------------------------------------------------------------------
  Module                                          Responsibility
  ----------------------------------------------- -----------------------------------
  `src/isp/`                                      ISP-inspired image-processing
                                                  stages and pipeline orchestration

  `src/camera/SimulatedCameraDriver.*`            Simulated driver state machine,
                                                  bounded queue, frame
                                                  submission/acquisition, fault and
                                                  recovery

  `src/camera/CameraProcessingSession.*`          Connects the simulated driver to
                                                  the ISP pipeline

  `include/camera/FrameScheduler.hpp` and         Queue and timing model for frame
  `src/camera/FrameScheduler.cpp`                 arrivals, processing, deadlines,
                                                  drops, and late frames

  `include/camera/IntegratedFrameProcessor.hpp`   Connects scheduled frames to ISP
  and `src/camera/IntegratedFrameProcessor.cpp`   processing and compares measured
                                                  processing time against a budget

  `src/diagnostics/FrameDiagnostics.cpp`          Frame/pipeline diagnostics and CSV
                                                  reporting

  `benchmarks/`                                   Repeatable pipeline benchmark and
                                                  saved sample results

  `tests/`                                        Regression tests for modules,
                                                  integration, and fault scenarios

  `.github/workflows/ci.yml`                      GitHub Actions build-and-test
                                                  workflow
  -----------------------------------------------------------------------------------

Some camera-related headers and implementation files live under
`src/camera/`, while scheduler/integrated public headers live under
`include/camera/`; the table reflects the current repository layout.

## ISP pipeline

The pipeline currently contains these stages:

1.  **Exposure and gain** --- applies exposure-stop and gain multipliers
    to 8-bit input.
2.  **White balance** --- applies configurable per-channel gains when
    enabled.
3.  **Denoising** --- reduces image noise using the configured
    OpenCV-based operation.
4.  **Sharpening** --- applies configurable sharpening.
5.  **Validation and metrics** --- records stage timing, total duration,
    and whether the output is valid.

The pipeline supports configurable stages through `isp::ISPConfig`.
Stage enablement and parameter ranges are defined by the implementation;
invalid exposure/gain values are covered by tests.

## Simulated driver and scheduling

### Simulated driver

The driver model has these states:

-   `Uninitialized`
-   `Initialized`
-   `Streaming`
-   `Stopped`
-   `Error`

It includes bounded buffering, input-frame validation, frame cloning on
submission, timed acquisition, queue-overflow accounting, fault
injection, and recovery. When the queue is full, the current
implementation rejects the newest frame and increments the drop counter.

### Frame scheduler

The scheduler models frame sequence numbers, arrival time, deadline, and
simulated processing duration. It tracks queued and dropped frames, late
frames, and simulation time.

### Integrated processing

`IntegratedFrameProcessor` combines the scheduling model with ISP
processing. It exposes scheduling outcomes and measured host processing
duration relative to the configured processing budget. These two time
values have different meanings: the scheduler is a simulation, while ISP
duration is measured on the host.

## Requirements

-   C++17 compiler
-   CMake 3.20 or newer
-   OpenCV development libraries with `core`, `imgproc`, and `imgcodecs`
-   Git (for cloning and version control)

The project has been developed and benchmarked on an Apple Silicon
MacBook Air. CI builds on Ubuntu; exact OpenCV versions and performance
vary by environment.

## Build and run

### macOS (Homebrew)

If CMake and OpenCV are not installed:

``` bash
brew install cmake opencv
```

Clone and build:

``` bash
git clone https://github.com/mukulsarswat/Camera-ISP-Firmware-Validation-Workbench.git
cd Camera-ISP-Firmware-Validation-Workbench

cmake -S . -B build
cmake --build build -j2
```

Run the application:

``` bash
./build/camera_isp
```

### Ubuntu

Install dependencies:

``` bash
sudo apt-get update
sudo apt-get install -y build-essential cmake libopencv-dev
```

Then configure and build:

``` bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel 2
```

Run the application:

``` bash
./build/camera_isp
```

## Run the test suite

Run all registered tests:

``` bash
ctest --test-dir build --output-on-failure
```

The current CMake configuration registers 12 test executables covering:

-   Camera frame behavior
-   Exposure/gain
-   White balance
-   Denoising
-   Sharpening
-   ISP pipeline behavior
-   Frame diagnostics
-   Simulated camera driver
-   Camera processing session
-   Frame scheduling
-   Integrated frame processing
-   Fault injection and invalid parameters

A passing test suite checks the behaviors encoded in these tests; it
does not establish hardware-level or hard-real-time correctness.

## Performance benchmark

Build the benchmark as part of the normal CMake build and run it with:

``` bash
./build/isp_pipeline_benchmark
```

The benchmark performs 3 warm-up iterations and 30 measured iterations
for each resolution. It reports mean, median, P95, maximum latency, and
estimated single-threaded throughput calculated as
`1000 / mean latency`.

### Sample result

These results were recorded on an Apple Silicon MacBook Air in one run
and are included as a reference, not as a performance guarantee.

  -----------------------------------------------------------------------------
  Resolution      Mean (ms)  Median (ms)     P95 (ms)     Max (ms)    Estimated
                                                                            FPS
  ------------ ------------ ------------ ------------ ------------ ------------
  640 × 480           8.376        8.240        9.065       10.314      119.384

  1280 × 720         24.601       23.588       27.714       36.044       40.649
  -----------------------------------------------------------------------------

Run and save a fresh sample:

``` bash
./build/isp_pipeline_benchmark | tee benchmarks/latest_results.txt
```

Benchmark numbers depend on CPU load, compiler flags, OpenCV build
options, and system configuration. Estimated FPS is the reciprocal of
mean processing latency; it is not a guarantee of end-to-end camera
frame rate.

## Diagnostics

The diagnostics subsystem records pipeline timing and output validity.
CSV output can be used to compare runs and inspect regressions. The
exact fields are defined by the diagnostics writer; a sample row from
the project includes frame ID, input/output dimensions and channels,
stage durations, total duration, and output-valid status.

## Continuous integration

GitHub Actions is configured in `.github/workflows/ci.yml`. On pushes to
`main`, pull requests targeting `main`, and manual workflow dispatch,
the workflow:

1.  Checks out the repository.
2.  Installs CMake, a C++ compiler, Make, and OpenCV development
    packages.
3.  Configures the project in Release mode.
4.  Builds all CMake targets.
5.  Runs `ctest`.
6.  Executes the ISP benchmark.

The workflow verifies that the project builds and tests run in its
Ubuntu environment. It does not compare benchmark values against a fixed
threshold.

## Repository layout

``` text
.
├── .github/
│   └── workflows/
│       └── ci.yml
├── benchmarks/
│   ├── ISPPipelineBenchmark.cpp
│   └── latest_results.txt
├── include/
│   ├── camera/
│   │   ├── FrameScheduler.hpp
│   │   └── IntegratedFrameProcessor.hpp
│   ├── diagnostics/
│   │   └── FrameDiagnostics.hpp
│   └── isp/
│       ├── Denoise.hpp
│       ├── ExposureGain.hpp
│       ├── ISPConfig.hpp
│       ├── ISPPipeline.hpp
│       ├── Sharpen.hpp
│       └── WhiteBalance.hpp
├── src/
│   ├── camera/
│   ├── diagnostics/
│   ├── isp/
│   └── main.cpp
├── tests/
└── CMakeLists.txt
```

## Engineering concepts demonstrated

-   Modular C++17 design and CMake target organization
-   OpenCV image processing
-   State-machine modeling
-   Bounded queues and frame-drop policies
-   Timeouts and error recovery
-   Simulated scheduling and deadline accounting
-   Unit and integration testing
-   Fault injection and input validation
-   Latency measurement and percentile reporting
-   GitHub Actions CI

## Current limitations and future extensions

This project is a learning and validation workbench, not production
camera firmware. Potential next steps include:

-   Add configurable test scenarios and machine-readable benchmark
    summaries.
-   Add sanitizer builds (AddressSanitizer/UndefinedBehaviorSanitizer)
    to CI.
-   Add reproducible fault-injection scenarios for queue pressure and
    timeouts.
-   Separate platform interfaces from simulation implementations.
-   Explore ARM NEON optimization concepts and compare scalar versus
    optimized code.
-   Add sample image input/output and a demonstration recording.
-   For actual device validation, integrate with supported camera
    hardware and vendor-specific SDKs/drivers in an appropriate
    development environment.

## License

No license has been specified in this repository yet. Add a `LICENSE`
file before presenting the project as open source for reuse.
