#include "../include/camera/FrameScheduler.hpp"

#include <opencv2/core.hpp>

#include <iostream>

bool testAcceptedFrameGetsSequenceNumber() {
    camera::FrameScheduler scheduler(3);

    cv::Mat frame(16, 16, CV_8UC3, cv::Scalar(10, 20, 30));

    if (!scheduler.submitFrame(frame, 0.0, 33.0, 5.0)) {
        return false;
    }

    camera::SchedulingResult result;

    return scheduler.processNext(result) &&
           result.sequence == 1 &&
           result.deadlineMet &&
           result.completionTimeMs == 5.0;
}

bool testLateFrameIsCounted() {
    camera::FrameScheduler scheduler(3);

    cv::Mat frame(16, 16, CV_8UC3, cv::Scalar(10, 20, 30));

    if (!scheduler.submitFrame(frame, 0.0, 10.0, 12.0)) {
        return false;
    }

    camera::SchedulingResult result;

    return scheduler.processNext(result) &&
           !result.deadlineMet &&
           scheduler.lateFrames() == 1;
}

bool testQueueOverflowDropsNewestFrame() {
    camera::FrameScheduler scheduler(1);

    cv::Mat first(8, 8, CV_8UC1, cv::Scalar(20));
    cv::Mat second(8, 8, CV_8UC1, cv::Scalar(40));

    if (!scheduler.submitFrame(first, 0.0, 10.0, 2.0)) {
        return false;
    }

    if (scheduler.submitFrame(second, 0.0, 10.0, 2.0)) {
        return false;
    }

    return scheduler.queuedFrames() == 1 &&
           scheduler.droppedFrames() == 1 &&
           scheduler.submittedFrames() == 1;
}

bool testFifoOrderAndQueueDelay() {
    camera::FrameScheduler scheduler(3);

    cv::Mat frame(8, 8, CV_8UC1, cv::Scalar(20));

    if (!scheduler.submitFrame(frame, 0.0, 20.0, 5.0) ||
        !scheduler.submitFrame(frame, 0.0, 20.0, 3.0)) {
        return false;
    }

    camera::SchedulingResult first;
    camera::SchedulingResult second;

    if (!scheduler.processNext(first) ||
        !scheduler.processNext(second)) {
        return false;
    }

    return first.sequence == 1 &&
           second.sequence == 2 &&
           first.completionTimeMs == 5.0 &&
           second.startTimeMs == 5.0 &&
           second.completionTimeMs == 8.0;
}

bool testInvalidTimingRejected() {
    camera::FrameScheduler scheduler(2);

    cv::Mat frame(8, 8, CV_8UC1, cv::Scalar(20));

    return !scheduler.submitFrame(frame, -1.0, 10.0, 2.0) &&
           !scheduler.submitFrame(frame, 5.0, 4.0, 2.0) &&
           !scheduler.submitFrame(frame, 0.0, 10.0, -2.0) &&
           scheduler.queuedFrames() == 0;
}

bool testEmptyQueue() {
    camera::FrameScheduler scheduler(2);
    camera::SchedulingResult result;

    return !scheduler.processNext(result);
}

int main() {
    int failures = 0;

    const auto check = [&](const char* name, bool passed) {
        if (passed) {
            std::cout << "PASS: " << name << '\n';
        } else {
            std::cerr << "FAIL: " << name << '\n';
            ++failures;
        }
    };

    check("sequence number and on-time frame",
          testAcceptedFrameGetsSequenceNumber());

    check("late-frame detection",
          testLateFrameIsCounted());

    check("queue overflow",
          testQueueOverflowDropsNewestFrame());

    check("FIFO order and queue delay",
          testFifoOrderAndQueueDelay());

    check("invalid timing rejected",
          testInvalidTimingRejected());

    check("empty queue",
          testEmptyQueue());

    if (failures == 0) {
        std::cout << "All frame scheduler tests passed\n";
        return 0;
    }

    return 1;
}