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

#ifndef __OPENVQ_ALGORITHM_H
#define __OPENVQ_ALGORITHM_H

#include <memory>
#include <string>
#include <queue>
#include <mutex>
#include <condition_variable>

#include <boost/program_options/options_description.hpp>
#include <io/config.h> // Libav-related imports
#include <io/VideoSequence.h>


namespace opts = boost::program_options;

class Algorithm {
protected:
    int maxFrames = std::numeric_limits<int>::max();
    AVPixelFormat pixelFormat = AV_PIX_FMT_YUV444P;
    Logger logger;
    opts::options_description options;
    std::string csvDbURL;

    Algorithm(std::string algorithmName);

public:
    virtual ~Algorithm() {
    };

    virtual void init(int argc, const char **argv) = 0;

    virtual int run() = 0;

    virtual void writeCSV(const std::string &identifier, const std::vector<double> &values);
};


typedef std::function<void(std::shared_ptr<Frame> srcFrame,
                           std::shared_ptr<Frame> pvsFrame,
                           unsigned t)> IntraFrameFunction;

typedef std::function<void(std::shared_ptr<Frame> srcCurr,
                           std::shared_ptr<Frame> pvsCurr,
                           std::shared_ptr<Frame> srcPrev,
                           std::shared_ptr<Frame> pvsPrev,
                           unsigned tCurr)> InterFrameFunction;

class FullReferenceAlgorithm : public Algorithm {
protected:
    std::string srcURL;
    std::string pvsURL;
    VideoSequence src;
    VideoSequence pvs;
    unsigned int sequenceLength;

    FullReferenceAlgorithm(std::string algorithmName);

    virtual void validateInput(VideoInfo &srcInfo, VideoInfo &pvsInfo);

    virtual void logVideoInfo(VideoInfo &info, std::string sequenceIdentifier);

    virtual void makePass(IntraFrameFunction body);

    virtual void makePassWithPrev(InterFrameFunction body);

public:
    virtual void init(int argc, const char **argv) override;
};

class ParallelFullReferenceAlgorithm : public FullReferenceAlgorithm {
protected:
    struct Job {
        virtual bool run() = 0;
    };

    class IntraFrameJob : public Job {
        IntraFrameFunction body;
        std::shared_ptr<Frame> srcFrame, pvsFrame;
        unsigned t;

    public:
        IntraFrameJob(IntraFrameFunction body, std::shared_ptr<Frame> srcFrame, std::shared_ptr<Frame> pvsFrame,
                      unsigned t);

        bool run();
    };

    class InterFrameJob : public Job {
        InterFrameFunction body;
        std::shared_ptr<Frame> srcCurr, srcPrev, pvsCurr, pvsPrev;
        unsigned t;

    public:
        InterFrameJob(InterFrameFunction body, std::shared_ptr<Frame> srcCurr, std::shared_ptr<Frame> srcPrev,
                      std::shared_ptr<Frame> pvsCurr, std::shared_ptr<Frame> pvsPrev, unsigned t);

        bool run();
    };

    class JobQueue {
        unsigned long cap;
        bool closed;
        std::queue<std::shared_ptr<Job> > q;
        std::mutex qm, cm;
        std::condition_variable cv;

        struct StopJob : public Job {
            bool run() { return false; }
        };

    public:
        JobQueue(unsigned long capacity);

        bool push(std::shared_ptr<Job> job);

        std::shared_ptr<Job> pop();

        void close();
    };

    static void worker(std::shared_ptr<JobQueue> q);

    unsigned jFactor;

    ParallelFullReferenceAlgorithm(std::string algorithmName);

    void makePass(IntraFrameFunction body);

    void makePassWithPrev(InterFrameFunction body);

public:
    virtual void init(int argc, const char **argv) override;
};

#endif //__OPENVQ_ALGORITHM_H