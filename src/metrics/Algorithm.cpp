/*
 * This file is part of the video quality assessment toolkit OpenVQ
 *
 * Copyright (C) 2015 Henrik Bjørlo, Kristian Skarseth, Carsten Griwodz
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License version 3
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/parsers.hpp>
#include <chrono>
#include <thread>
#include <sstream>
#include <fstream>
#include <iomanip>

#include "Algorithm.h"


Algorithm::Algorithm(std::string algorithmName)
        : logger(algorithmName),
          options("Allowed options for algorithm " + algorithmName) {
    options.add_options()
            ("max-frames,t", opts::value<int>(&maxFrames), "Set frame limit")
            ("help,h", "Print help message")
            ("csv", opts::value<std::string>(&csvDbURL),
             "Path to csv file to which the indicators will be added as a new line");
}

void Algorithm::writeCSV(const std::string &identifier, const std::vector<double> &values) {
    if (csvDbURL.empty())
        return;

    std::ofstream db(csvDbURL, std::ofstream::app);
    if (!db) {
        logger(ERROR) << "Could not write to database file: " << csvDbURL;
    }
    db << std::setprecision(std::numeric_limits<double>::digits10 + 1);
    db << identifier;
    for (unsigned i = 0; i < values.size(); i++) {
        db << "," << values[i];
    }
    db << std::endl;
    db.close();
    logger(DEBUG) << "Wrote values to database " << csvDbURL;
}

FullReferenceAlgorithm::FullReferenceAlgorithm(std::string algorithmName)
        : Algorithm(algorithmName) {
    options.add_options()
            ("src,s", opts::value<std::string>(&srcURL)->required(), "Path to source video sequence (required)")
            ("pvs,p", opts::value<std::string>(&pvsURL)->required(), "Path to processed video sequence (required)");
}

void FullReferenceAlgorithm::init(int argc, const char **argv) {
    opts::variables_map vm;
    opts::parsed_options parsed = opts::command_line_parser(argc, argv).
            options(options).
            allow_unregistered().
            run();
    opts::store(parsed, vm);

    if (vm.count("help")) {
        std::stringstream what;
        what << options;
        throw std::runtime_error(what.str().c_str());
    }

    try {
        opts::notify(vm);
    } catch (std::exception &e) {
        std::stringstream what;
        what << e.what() << std::endl << options;
        throw std::runtime_error(what.str().c_str());
    }

    VideoInfo srcInfo = src.init(srcURL, maxFrames, pixelFormat);
    VideoInfo pvsInfo = pvs.init(pvsURL, maxFrames, pixelFormat);
    validateInput(srcInfo, pvsInfo);

    sequenceLength = srcInfo.frame_count;
}

void FullReferenceAlgorithm::validateInput(VideoInfo &srcInfo, VideoInfo &pvsInfo) {
    logVideoInfo(srcInfo, "SRC");
    logVideoInfo(pvsInfo, "PVS");

    srcInfo.frame_count = std::min(srcInfo.frame_count, maxFrames);
    pvsInfo.frame_count = std::min(pvsInfo.frame_count, maxFrames);

    if (srcInfo.width != pvsInfo.width || srcInfo.height != pvsInfo.height
        || srcInfo.avg_framerate != pvsInfo.avg_framerate
        || srcInfo.frame_count != pvsInfo.frame_count) {
        throw std::runtime_error("SRC and PVS dimensions don't match.");
    }
}

void FullReferenceAlgorithm::logVideoInfo(VideoInfo &info, std::string sequenceIdentifier) {
    logger(INFO) << sequenceIdentifier << " is " << info.filename;
    logger(DEBUG) << " - Resolution: " << info.width << "x" << info.height << "@" << info.avg_framerate;
    logger(DEBUG) << " - Duration:   " << info.duration << " seconds (" << info.frame_count << " frames)";
}

void FullReferenceAlgorithm::makePass(IntraFrameFunction body) {
    src.rewind();
    pvs.rewind();

    std::shared_ptr<Frame> srcCurr, pvsCurr;

    unsigned int t = 0;
    Logger::initProgress();
    while (t <= sequenceLength) {
        srcCurr = src.nextFrame();
        pvsCurr = pvs.nextFrame();
        if (!srcCurr)
            break;
        assert (pvsCurr);

        body(srcCurr, pvsCurr, t);

        t++;
        Logger::logProgress(t, sequenceLength);
    }
    Logger::resetProgress();
}

void FullReferenceAlgorithm::makePassWithPrev(InterFrameFunction body) {
    src.rewind();
    pvs.rewind();

    std::shared_ptr<Frame> srcCurr, srcPrev, pvsCurr, pvsPrev;

    unsigned int t = 0;
    Logger::initProgress();
    while (t <= sequenceLength) {
        srcCurr = src.nextFrame();
        pvsCurr = pvs.nextFrame();
        if (!srcCurr)
            break;
        assert (pvsCurr);

        body(srcCurr, pvsCurr, srcPrev, pvsPrev, t);

        srcPrev = srcCurr;
        pvsPrev = pvsCurr;
        t++;
        Logger::logProgress(t, sequenceLength);
    }
    Logger::resetProgress();
}

ParallelFullReferenceAlgorithm::ParallelFullReferenceAlgorithm(std::string algorithmName)
        : FullReferenceAlgorithm(algorithmName) {
    options.add_options()("num_threads,j", opts::value<unsigned>(&jFactor), "Number of threads wanted. "
            "If no value is given, the algorithm tries to determine the number of threads supported by the hardware.");
}

void ParallelFullReferenceAlgorithm::init(int argc, const char **argv) {
    FullReferenceAlgorithm::init(argc, argv);

    opts::variables_map vm;
    opts::parsed_options parsed = opts::command_line_parser(argc, argv).
            options(options).
            allow_unregistered().
            run();
    opts::store(parsed, vm);

    if (!vm.count("num_threads")) {
        jFactor = std::thread::hardware_concurrency();
        if (jFactor <= 0) {
            jFactor = 1;
            logger(DEBUG) << "Could not determine hardware concurrency capabilities. "
            << "Defaulting to single thread operation.";
        }
    }
}

void ParallelFullReferenceAlgorithm::makePass(IntraFrameFunction body) {
    if (jFactor == 1) {
        FullReferenceAlgorithm::makePass(body);
        return;
    }

    src.rewind();
    pvs.rewind();

    auto jobQueue = std::make_shared<JobQueue>(jFactor);

    std::vector<std::thread> workers;
    for (unsigned i = 0; i < jFactor; ++i)
        workers.push_back(std::thread(worker, jobQueue));

    std::shared_ptr<Frame> srcCurr, pvsCurr;
    unsigned int t = 0;
    Logger::initProgress();
    while (t <= sequenceLength) {
        srcCurr = src.nextFrame();
        pvsCurr = pvs.nextFrame();
        if (!srcCurr)
            break;
        assert(pvsCurr);

        jobQueue->push(std::make_shared<IntraFrameJob>(body, srcCurr, pvsCurr, t));

        t++;
        Logger::logProgress(t, sequenceLength);
    }
    jobQueue->close();
    std::for_each(workers.begin(), workers.end(), [](std::thread &worker) {
        worker.join();
    });
    Logger::resetProgress();
}


void ParallelFullReferenceAlgorithm::makePassWithPrev(InterFrameFunction body) {
    if (jFactor == 1) {
        FullReferenceAlgorithm::makePassWithPrev(body);
        return;
    }

    src.rewind();
    pvs.rewind();

    auto jobQueue = std::make_shared<JobQueue>(jFactor);

    std::vector<std::thread> workers;
    for (unsigned i = 0; i < jFactor; ++i)
        workers.push_back(std::thread(worker, jobQueue));

    std::shared_ptr<Frame> srcCurr, srcPrev, pvsCurr, pvsPrev;
    unsigned int t = 0;
    Logger::initProgress();
    while (t <= sequenceLength) {
        srcCurr = src.nextFrame();
        pvsCurr = pvs.nextFrame();
        if (!srcCurr)
            break;
        assert(pvsCurr);

        jobQueue->push(std::make_shared<InterFrameJob>(body, srcCurr, pvsCurr, srcPrev, pvsPrev, t));

        srcPrev = srcCurr;
        pvsPrev = pvsCurr;
        t++;
        Logger::logProgress(t, sequenceLength);
    }
    jobQueue->close();
    std::for_each(workers.begin(), workers.end(), [](std::thread &worker) {
        worker.join();
    });
    Logger::resetProgress();
}

#define SCOPED_GUARDED_RETURN_IF(__mutex__, __bool_expr__, __retval__) { std::lock_guard<std::mutex> __g__(__mutex__); if (__bool_expr__) return __retval__; }

ParallelFullReferenceAlgorithm::JobQueue::JobQueue(unsigned long capacity) : cap(capacity), closed(false) { }

bool ParallelFullReferenceAlgorithm::JobQueue::push(std::shared_ptr<Job> job) {
    SCOPED_GUARDED_RETURN_IF(cm, closed, false);

    std::unique_lock<std::mutex> l(qm);
    cv.wait(l, [&] { return q.size() < cap; });
    q.push(job);
    l.unlock();
    cv.notify_all();
    return true;
}

std::shared_ptr<ParallelFullReferenceAlgorithm::Job> ParallelFullReferenceAlgorithm::JobQueue::pop() {
    std::unique_lock<std::mutex> l(qm);
    while (q.empty()) {
        SCOPED_GUARDED_RETURN_IF(cm, closed, std::make_shared<StopJob>());
        cv.wait(l);
    }
    std::shared_ptr<Job> j = q.front();
    q.pop();
    l.unlock();
    cv.notify_all();
    return j;
}

void ParallelFullReferenceAlgorithm::JobQueue::close() {
    {
        std::lock_guard<std::mutex> g(cm);
        closed = true;
    }
    cv.notify_all();
}

ParallelFullReferenceAlgorithm::IntraFrameJob::IntraFrameJob(
        IntraFrameFunction body, std::shared_ptr<Frame> srcFrame, std::shared_ptr<Frame> pvsFrame, unsigned t
) : body(body), srcFrame(srcFrame), pvsFrame(pvsFrame), t(t) {
}

bool ParallelFullReferenceAlgorithm::IntraFrameJob::run() {
    body(srcFrame, pvsFrame, t);
    return true;
}

ParallelFullReferenceAlgorithm::InterFrameJob::InterFrameJob(
        InterFrameFunction body, std::shared_ptr<Frame> srcCurr, std::shared_ptr<Frame> srcPrev,
        std::shared_ptr<Frame> pvsCurr, std::shared_ptr<Frame> pvsPrev, unsigned t
) : body(body), srcCurr(srcCurr), srcPrev(srcPrev), pvsCurr(pvsCurr), pvsPrev(pvsPrev), t(t) {
}

bool ParallelFullReferenceAlgorithm::InterFrameJob::run() {
    body(srcCurr, srcPrev, pvsCurr, pvsPrev, t);
    return true;
}

void ParallelFullReferenceAlgorithm::worker(std::shared_ptr<JobQueue> q) {
    while (q->pop()->run()) { }
}