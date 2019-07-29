//
// Created by wyf on 2019/7/29.
//

#ifndef __COMMON_IDGENERATOR_H__
#define __COMMON_IDGENERATOR_H__

#include <cstdint>
#include <sys/time.h>

/*
 *          Version
 *           |
 * 64bit id: 0000  0000  0000  0000  0000  0000  0000  0000  0000  0000  0000  0000  0000  0000  0000  0000
 *            |||  |                                          ||            |  |                          |
 *            |└┘  └---------------------Timestamp sec--------┘└---Machine--┘  └----------sequence--------┘
 *            | M
 *           Type
 *  Version: 1 bits
 *      0: default, 1: for update
 *  Type: 1 bits
 *      0: max peak, 1: min granularity
 *  M: generate method
 *      00: Embedded mode, 01: center server mode, 10: REST/RPC mode, 11: Reserve
 *  Timestamp: 30 bits
 *      2^30  = 34 Years
 *  Sequence: 20 bits
 *      2^20 = 1048675 = 100w
 *  Machine: 10 bits
 *      2^10 = 1024 machine
 ***********************************************************************************************************
 *         Version
 *           |
 * 64bit id: 0000  0000  0000  0000  0000  0000  0000  0000  0000  0000  0000  0000  0000  0000  0000  0000
 *            |||  |                                                        |  |            ||            |
 *            |└┘  └---------------------Timestamp msec---------------------┘  └---Machine--┘└--sequence--┘
 *            | M
 *           Type
 *  Version: 1 bits
 *      0: default, 1: for update
 *  Type: 1 bits
 *      0: max peak, 1: min granularity
 *  M: generate method
 *      00: Embedded mode, 01: center server mode, 10: REST/RPC mode, 11: Reserve
 *  Timestamp: 40 bits
 *      2^40  = 34 Years
 *  Sequence: 10 bits
 *      2^10 = 1024  1k/1ms
 *  Machine: 10 bits
 *      2^10 = 1024 machine
 */

enum EIdType {
    kIdTypeMaxPeak,
    kIdTypeMinGranularity
};

enum EIdMethod {
    kIdMethodEmbedded,
    kIdMethodCenterServer,
    kIdMethodRESTorRPC,
    kIdMethodReserve
};

enum EIdVersion {
    kIdVersionRunning,
    kIdVersionUpdate
};

struct IdMeta {
    uint8_t versionBits;
    uint8_t typeBits;
    uint8_t methodBits;
    uint8_t timestampBits;
    uint8_t machineBits;
    uint8_t sequenceBits;
    uint32_t sequenceMask;
    uint32_t versionLeftShift;
    uint32_t typeLeftShift;
    uint32_t methodLeftShift;
    uint32_t timestampLeftShift;
    uint32_t machineLeftShift;
    EIdVersion idVersion;
    EIdType idType;
    EIdMethod  idMethod;
};
struct IdBitValue {
    uint64_t version;
    uint64_t type;
    uint64_t method;
    uint32_t machine;
};

class IdGenerator {
    // start timestamp ms [2019-01-01 00:00:00]
    const static uint64_t kFromEpoch    = 1546272000000;
public:
    explicit IdGenerator() {
        sequence_ = 0;
        lastTimestamp_ = 0;
        nowTimeStamp_ = 0;
        idMeta_.idType = kIdTypeMaxPeak;
        idMeta_.idVersion = kIdVersionRunning;
        idMeta_.idMethod = kIdMethodEmbedded;
        idBitValue_.machine = 0;
        idBitValue_.version = 0;
        idBitValue_.type = 0;
        idBitValue_.method = 0;
        setIdType(idMeta_.idType);
    }
    ~IdGenerator() {}

    void setIdType(EIdType idType) {
        idMeta_.idType = idType;
    }
    void setIdMethod(EIdMethod idMethod) {
        idMeta_.idMethod = idMethod;
    }
    void setIdVersion(EIdVersion idVersion) {
        idMeta_.idVersion = idVersion;
    }

    void init() {
        if (kIdVersionUpdate == idMeta_.idVersion) {
            idBitValue_.version = 1;
        } else {
            idBitValue_.version = 0;
        }

        if (kIdMethodEmbedded == idMeta_.idMethod) {
            idBitValue_.method = 0;
        } else if (kIdMethodCenterServer == idMeta_.idMethod) {
            idBitValue_.method = 1;
        } else if (kIdMethodRESTorRPC == idMeta_.idMethod) {
            idBitValue_.method = 2;
        } else if (kIdMethodReserve == idMeta_.idMethod) {
            idBitValue_.method = 3;
        } else {
            idBitValue_.method = 0;
        }

        if (kIdTypeMaxPeak == idMeta_.idType) {
            idBitValue_.type = 0;
            useMaxPeakType();
        } else if (kIdTypeMinGranularity == idMeta_.idType) {
            idBitValue_.type = 1;
            useMinGranularityType();
        } else {
            idBitValue_.type = 0;
            useMaxPeakType();
        }
    }

    void setMachineId(uint32_t machineId) {
        idBitValue_.machine = machineId;
    }

    // get next id
    bool nextId(uint64_t &to) {
        // std::unique_lock<std::mutex> lk(mutex_);;
        nowTimeStamp_ = getNowTimestamp();
        if (nowTimeStamp_ < lastTimestamp_) {
            return false;
        } else if (nowTimeStamp_ == lastTimestamp_) {
            // same timestamp, generate sequence in one ms.
            sequence_ = (sequence_ + 1) & idMeta_.sequenceMask;
            if (0 == sequence_) {
                nowTimeStamp_ = tilNextTimesteamp(lastTimestamp_);
            }
        } else {
            sequence_ = 0;
        }
        lastTimestamp_ = nowTimeStamp_;


        to = idBitValue_.version | idBitValue_.type | idBitValue_.method
             | ((nowTimeStamp_ - kFromEpoch) << idMeta_.timestampLeftShift)
             | (idBitValue_.machine << idMeta_.methodLeftShift)
             | sequence_;
        return true;
    }

protected:

    void useMaxPeakType() {
        idMeta_.versionBits = 1;
        idMeta_.typeBits = 1;
        idMeta_.methodBits = 2;
        idMeta_.timestampBits = 30;
        idMeta_.machineBits = 10;
        idMeta_.sequenceBits = 20;
        idMeta_.idType = kIdTypeMaxPeak;
        genIdMetaShift();
    }

    void useMinGranularityType() {
        idMeta_.versionBits = 1;
        idMeta_.typeBits = 1;
        idMeta_.methodBits = 2;
        idMeta_.timestampBits = 40;
        idMeta_.machineBits = 10;
        idMeta_.sequenceBits = 10;
        idMeta_.idType = kIdTypeMinGranularity;
        genIdMetaShift();
    }

    uint64_t getNowTimestamp() {
        if (kIdTypeMaxPeak == idMeta_.idType) {
            return NowSec();
        } else if (kIdTypeMinGranularity == idMeta_.idType) {
            return NowMs();
        }
        return NowSec();
    }
    uint64_t tilNextTimesteamp(uint64_t lastTimestamp) {
        uint64_t timestamp = getNowTimestamp();
        while (timestamp <= lastTimestamp) {
            timestamp = getNowTimestamp();
        }
        return timestamp;
    }

    uint64_t NowMs() {
        struct timeval timeNow;
        gettimeofday(&timeNow, NULL);
        return (timeNow.tv_sec) * 1000 + timeNow.tv_usec / 1000;
    }

    uint64_t NowSec() {
        struct timeval timeNow;
        gettimeofday(&timeNow, NULL);
        return (timeNow.tv_sec);
    }

    void genIdMetaShift() {
        // max value of sequence
        idMeta_.sequenceMask = -1 ^ (-1 << idMeta_.sequenceBits);
        idMeta_.machineLeftShift = idMeta_.sequenceBits;
        idMeta_.timestampLeftShift = idMeta_.machineLeftShift + idMeta_.machineBits;
        idMeta_.methodLeftShift = idMeta_.timestampLeftShift + idMeta_.timestampBits;
        idMeta_.typeLeftShift = idMeta_.methodLeftShift + idMeta_.methodBits;
        idMeta_.versionLeftShift = idMeta_.typeLeftShift + idMeta_.typeBits;
        // to generate value
        idBitValue_.version = idBitValue_.version << idMeta_.versionLeftShift;
        idBitValue_.type = idBitValue_.type << idMeta_.typeLeftShift;
        idBitValue_.method = idBitValue_.method << idMeta_.methodLeftShift;
    }

private:
    IdMeta idMeta_;
    IdBitValue idBitValue_;

    // sequence 0-4095
    uint32_t sequence_;
    uint64_t nowTimeStamp_;
    // last timestamp for generate id
    uint64_t lastTimestamp_;
};
#endif //__COMMON_IDGENERATOR_H__
