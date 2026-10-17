#include "../include/camera/IntegratedFrameProcessor.hpp"

#include <opencv2/core.hpp>

#include <iostream>

bool testFrameFlowsThroughSchedulerAndISP() {
    isp::ISPConfig config;

    camera::IntegratedFrameProcessor processor(config, 3);

    cv::Mat frame(
        64, 80, CV_8UC3,
        cv::Scalar(80, 100, 120)
    );

    if (!processor.submitFrame(frame, 0.0, 1000.0, 5.0)) {
        return false;
    }

    camera::IntegratedFrameResult result;

    if (!processor.processNext(result)) {
        return false;
    }

    return result.scheduling.sequence == 1 &&
           result.ispResult.outputValid &&
           !result.ispResult.image.empty() &&
           result.ispResult.stages.size() == 4 &&
           result.processingBudgetMs == 1000.0 &&
           result.measuredProcessingWithinBudget;
}

bool testSchedulerDeadlineMissIsPreserved() {
    isp::ISPConfig config;

    camera::IntegratedFrameProcessor processor(config, 2);

    cv::Mat frame(
        32, 32, CV_8UC3,
        cv::Scalar(30, 60, 90)
    );

    if (!processor.submitFrame(frame, 0.0, 2.0, 5.0)) {
        return false;
    }

    camera::IntegratedFrameResult result;

    if (!processor.processNext(result)) {
        return false;
    }

    return !result.scheduling.deadlineMet &&
           processor.lateFrames() == 1 &&
           result.ispResult.outputValid;
}

bool testQueueOverflowIsReported() {
    isp::ISPConfig config;

    camera::IntegratedFrameProcessor processor(config, 1);

    cv::Mat frame(
        16, 16, CV_8UC1,
        cv::Scalar(80)
    );

    const bool firstAccepted =
        processor.submitFrame(frame, 0.0, 50.0, 2.0);

    const bool secondAccepted =
        processor.submitFrame(frame, 0.0, 50.0, 2.0);

    return firstAccepted &&
           !secondAccepted &&
           processor.droppedFrames() == 1;
}

bool testEmptyQueueReturnsFalse() {
    isp::ISPConfig config;

    camera::IntegratedFrameProcessor processor(config, 2);
    camera::IntegratedFrameResult result;

    return !processor.processNext(result);
}

int main() {
    cv::setNumThreads(1);
    int failures = 0;


    const auto check = [&](const char* name, bool passed) {
        if (passed) {
            std::cout << "PASS: " << name << '\n';
        } else {
            std::cerr << "FAIL: " << name << '\n';
            ++failures;
        }
    };

    check(
        "scheduler-to-ISP processing",
        testFrameFlowsThroughSchedulerAndISP()
    );

    check(
        "simulated deadline miss",
        testSchedulerDeadlineMissIsPreserved()
    );

    check(
        "queue overflow reporting",
        testQueueOverflowIsReported()
    );

    check(
        "empty queue",
        testEmptyQueueReturnsFalse()
    );

    if (failures == 0) {
        std::cout << "All integrated frame processor tests passed\n";
        return 0;
    }

    return 1;
}