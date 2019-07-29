/*
** Copyright (C) 2019 Wang Yaofu
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: The implement file of class SnowFlakeId.
*/

#pragma once

#include <stdint.h>
#include <mutex>
#include <sys/time.h>

/**
 * @brief 分布式id生成类
 * https://segmentfault.com/a/1190000011282426
 * https://github.com/twitter/snowflake/blob/snowflake-2010/src/main/scala/com/twitter/service/snowflake/IdWorker.scala
 *
 * 64bit id: 0000  0000  0000  0000  0000  0000  0000  0000  0000  0000  0000  0000  0000  0000  0000  0000
 *           ||                                                           ||     ||     |  |              |
 *           |└---------------------------时间戳---------------------------┘└中心-┘└机器--┘  └----序列号-----┘
 *           |
 *         不用
 */
class SnowFlakeId {
    // start timestamp ms [2019-01-01 00:00:00]
    const static uint64_t kFromEpoch    = 1546272000000;
    // bits of workder id
    const static uint32_t kWorkerIdBits = 5;
    // bits of data center id
    const static uint32_t kDataCenterIdBits = 5;
    // bits of sequence
    const static uint32_t kSequenceBits = 12;
    // left shift bits of worker id
    const static uint32_t kWorkerIdLeftShift = kSequenceBits;
    // left shift bits of data center id
    const static uint32_t kDataCenterIdLeftShift = kWorkerIdLeftShift + kWorkerIdBits;
    // left shift bits of timestamp
    const static uint32_t kTimestampLeftShift = kDataCenterIdLeftShift + kDataCenterIdBits;
    // max value of data center id is 31
    const static uint32_t kMaxDataCenterId = -1 ^ (-1 << kDataCenterIdBits);
    // max value of sequence is 4095
    const static uint32_t kSequenceMask = -1 ^ (-1 << kSequenceBits);
public:
    SnowFlakeId() {
        workerId_ = 0;
        datacenterId_ = 0;
        sequence_ = 0;
        lastTimestamp_ = 0;
    }
    ~SnowFlakeId() {}

public:
    // get next id
    bool nextId(uint64_t &to) {
        // std::unique_lock<std::mutex> lk(mutex_);;
        nowMs_ = NowMs();
        if (nowMs_ < lastTimestamp_) {
            return false;
        } else if (nowMs_ == lastTimestamp_) {
            // same timestamp, generate sequence in one ms.
            sequence_ = (sequence_ + 1) & kSequenceMask;
            if (0 == sequence_) {
                nowMs_ = tilNextMs(lastTimestamp_);
            }
        } else {
            sequence_ = 0;
        }
        lastTimestamp_ = nowMs_;
        to = ((nowMs_ - kFromEpoch) << kTimestampLeftShift)
                | (datacenterId_ << kDataCenterIdLeftShift)
                | (workerId_ << kWorkerIdLeftShift)
                | sequence_;
        return true;
    }

    // set worker id
    void setWorkerId(uint32_t workerId) {
        workerId_ = workerId;
    }

    // set data center id
    void setDatacenterId(uint32_t datacenterId) {
        datacenterId_ = datacenterId;
    }

protected:
    uint64_t tilNextMs(uint64_t lastTimestamp) {
        uint64_t timestamp = NowMs();
        while (timestamp <= lastTimestamp) {
            timestamp = NowMs();
        }
        return timestamp;
    }

    uint64_t NowMs() {
        struct timeval timeNow;
        gettimeofday(&timeNow, NULL);
        return (timeNow.tv_sec) * 1000 + timeNow.tv_usec / 1000;
    }
private:
    // worker id [0-31]
    uint32_t workerId_;
    // data center id [0-31]
    uint32_t datacenterId_;
    // sequence 0-4095
    uint32_t sequence_;
    uint64_t nowMs_;
    // last timestamp for generate id
    uint64_t lastTimestamp_;
    std::mutex mutex_;
};

