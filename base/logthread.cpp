
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <stdarg.h>
#include <iostream>
#include "logthread.h"

#define  RET_OK 0
#define  RET_ERROR -1
//Note:
// we don't synchronize in this function
// it's done by it's caller.
ostringstream *LogThread::switchStream() {
    //get string stream array index
    int index = mSwitchCount % 2;

    ostringstream *oldstream = mCurInfoStream;

    //modify current string stream
    mCurInfoStream = &mInfoStream[index];

    //reset the recrods counter
    mCurRecords = 0;

    //post-handling
    mSwitchCount++;

    return oldstream;
}

//
// init the log file
//
int LogThread::initLogFile(const string& logfile, 
                           ELogLevel level,
                           unsigned int refreshlines) 
{

    //save information
    mLogFile = logfile;
    mLevel = level;  
    mLPParam = refreshlines;

    //init the last write time
    memset(&mLastWriteTime, 0, sizeof(mLastWriteTime));
    ThreadTimer::instance()->getTimeOfDay(&mLastWriteTime);
    //gettimeofday(&mLastWriteTime, NULL);

    return RET_OK;
}

int LogThread::initLogFile() {
    return initLogFile(mLogFile, mLevel, mLPParam);
}

//
// when extend new write policy, we need
// modify this function.
//
bool LogThread::checkWritable() {
    //check kLogPolicyByRecords policy
    if(mPolicy == kLogPolicyByRecords) {
        ScopeLock lock(mMutex);
        if(mCurRecords >= mLPParam) {
            //we need write
            return true;
        }
    }

    //now we can't support log policy by file size.
    if(mPolicy == kLogPolicyByFileSize) {
        //reserve for future
        return true;
    }

    //check last write time
    struct timeval nowTval;
    memset(&nowTval, 0, sizeof(nowTval));
    ThreadTimer::instance()->getTimeOfDay(&nowTval);
    long udiff = ThreadTimer::instance()->getDiffTvInUSec(nowTval, mLastWriteTime);
    if(udiff >= MAX_WRITEINTERVAL) {
        //max write interval reached
        return true;
    }

    //invalid polily, it always return true
    return false;
}

string LogThread::createFileNameWithDayStamp(const string& filename) {
    string dsname = filename;

    //parameter check
    if(dsname.empty()) {
        return dsname;
    }

    struct timeval tmv;
    struct tm tme;
    char buf[128];

    memset(buf, 0, sizeof(buf));
    memset(&tmv, 0, sizeof(tmv));
    memset(&tme, 0, sizeof(tme));
    ThreadTimer::instance()->getTimeOfDay(&tmv);
    localtime_r(&tmv.tv_sec, &tme);
    snprintf(buf, sizeof(buf)-1, "%04d%02d%02d", 
        tme.tm_year+1900, tme.tm_mon+1, tme.tm_mday);

    dsname += ".";
    dsname += string(buf); 

    return dsname;
}
void LogThread::write2file(const string& loginfo) {
    string logfile = createFileNameWithDayStamp(mLogFile);
    if(logfile.empty()) {
        coutLog(__FILE__, __LINE__, __FUNCTION__, LL_ERROR,
            "log file with day stamp is NULL.");
        return;
    }

    int fd = open(logfile.c_str(), O_RDWR|O_APPEND|O_CREAT|O_LARGEFILE, 0644);
    if(0 > fd) {
        //open failed
        coutLog(__FILE__, __LINE__,  __FUNCTION__, LL_ERROR,
            "open file [%s] failed. errno=%d", logfile.c_str(), errno);
        return;
    }

    //write to log file
    int ret = write(fd, loginfo.c_str(), loginfo.length());
    if(0 > ret) {
        //write error
        coutLog(__FILE__, __LINE__, __FUNCTION__, LL_ERROR, 
            "write to log file [%s] failed. errno=%d", logfile.c_str(), errno);
    }

    close(fd);
}

void LogThread::run() {
    while (!mStopRequested) {
        //check whether writable 
        if(checkWritable()) {
            //get the lock
            mMutex.lock();
            //get string stream
            ostringstream *wstream = switchStream();     

            string loginfo = wstream->str();
            wstream->str("");
            mMutex.unlock();

            if(!loginfo.empty()) {
                //save the last write time
                memset(&mLastWriteTime, 0, sizeof(mLastWriteTime));
                ThreadTimer::instance()->getTimeOfDay(&mLastWriteTime);
                //gettimeofday(&mLastWriteTime, NULL);

                write2file(loginfo);
            }
        } 

        usleep(100000);
    }
    //flush buffered logs and close log file
    clean();
}

// stop the local thread.
// ---------------------------------------------------------------------------
//
void LogThread::on_stop() {
    mStopRequested = true;
}

int LogThread::genLogHeaderInfo(const string& filename, int line,
    const string& funcName, ELogLevel level, char* aOutBuf) {
    int off = 0, len = 0;
    if (mLogOption & kLogPrefixTime) {
        struct timeval tmv;
        struct tm tme;
        memset(&tmv, 0, sizeof(tmv));
        memset(&tme, 0, sizeof(tme));
        ThreadTimer::instance()->getTimeOfDay(&tmv);
        localtime_r(&tmv.tv_sec, &tme);
        off = strftime(aOutBuf, 50, "%D %H:%M:%S.", &tme);
        len += off;
        off = sprintf(aOutBuf+len, "%03d <%-8s>",
            (int)tmv.tv_usec/1000, getLevelString(level).c_str());
        len += off;
    }
    if (mLogOption & kLogPrefixPidTid) {
        off = sprintf(aOutBuf+len, "[%d,%lu] ", (int)getpid(), (int)pthread_self());
        len += off;
    }
    if (mLogOption & kLogPrefixFileLineFunc) {
        off = sprintf(aOutBuf+len, "(%s, %06d, %s)-",
            filename.c_str(), line, funcName.c_str());
        len += off;
    }
    return len;
}

bool LogThread::coutLog(const string& filename, int line, const string& funcName,
                        ELogLevel level, const char *fmt, ...) {
    //check the level
    if(level > mLevel) {
        //level lower
        return true;
    }
    va_list mark;
    char buf[2049] = {0};
    int off = genLogHeaderInfo(filename, line, funcName, level, buf);
    va_start(mark, fmt);
    vsnprintf(buf+off, 2048-off, fmt, mark);
    va_end(mark);
    ScopeLock lock(mCoutMutex);
    std::cout << buf << std::endl;

    return true;
}

bool LogThread::writeLog(const string& filename, int line,
    const string& funcName, ELogLevel level, const char *fmt, ...) {
    //check the level
    if(level > mLevel) {
        //level lower
        return true;
    }
    va_list mark;
    char buf[2049] = {0};
    int off = genLogHeaderInfo(filename, line, funcName, level, buf);
    va_start(mark, fmt);
    vsnprintf(buf+off, 2048-off, fmt, mark);
    va_end(mark);

    //dump to cache
    ScopeLock lock(mMutex);
    if (mLogOption & kLogNoThreadTag) {
        strcat(buf, "\n");
        write2file(buf);
    } else {
        (*mCurInfoStream) << buf << "\n";
        mCurRecords++;
    }
    return true;
}

void LogThread::logHexAsc(const string& filename, int line,
    const string& funcName, ELogLevel level, const void *aBuf, int aBufLen) {
    if (level > mLevel) {
        return;
    }

    // Check args
    if (aBuf == NULL) {
        // Null buffer address
        return;
    }
    char logHeaderbuf[1024] = {0};
    genLogHeaderInfo(filename, line, funcName, level, logHeaderbuf);

    char out[100];         // Formatted output string
    // Dump the contents of the data buffer, one 16-byte line at a time
    const unsigned char* dat = (const unsigned char *) aBuf;
    string hexAscStr = logHeaderbuf; 
    hexAscStr.append("\nOFFS  ----------------- HEXADECIMAL------------------"
                     "*------ASCII-----*\n");
    for (int off = 0x0000;  off < aBufLen;  off += 16) {
        int j;
        int o = 0;

        // Format and print the contents of this line

        // Format the address offset
        sprintf(out+o, "%04X:", off & 0xFFFF);
        o += 5;

        // Format the data as hex
        for (j = 0;  j < 16  &&  off+j < aBufLen;  j++) {
            sprintf(out+o, " %02X", dat[off+j]);
            o += 3;
        }

        for (;  j < 16;  j++) {
            sprintf(out+o, "   ");
            o += 3;
        }

        // Format the data as printable characters
        out[o++] = ' ';
        out[o++] = ' ';
        out[o++] = '(';

        for (j = 0;  j < 16  &&  off+j < aBufLen;  j++) {
            int ch = dat[off+j];
            out[o++] = isprint(ch) ? ch : '.';
        }

        out[o++] = ')';
        out[o++] = '\n';
        out[o++] = '\0';

        // Print the formatted output line
        hexAscStr.append(out);
    }
    ScopeLock lock(mMutex);
    if (mLogOption & kLogNoThreadTag) {
        write2file(hexAscStr);
    } else {
        (*mCurInfoStream) << hexAscStr;
        mCurRecords++;
    }
}

void LogThread::logRaw(ELogLevel level, const char* aRawBuf) {
    ScopeLock lock(mMutex);
    if (mLogOption & kLogNoThreadTag) {
        write2file(aRawBuf);
    } else {
        (*mCurInfoStream) << aRawBuf;
        mCurRecords++;
    }
}

string LogThread::getLevelString(ELogLevel level) {
    string levelstr = "UNKNOWN";

    switch(level) {
    case LL_ERROR:
        levelstr = "ERROR";
        break;
    case LL_WARN:
        levelstr = "WARNING";
        break;
    case LL_NOTICE:
        levelstr = "NOTICE";
        break;
    case LL_INFO:
        levelstr = "INFO";
        break;
    case LL_DBG:
        levelstr = "DEBUG";
        break;
    case LL_VARS:
        levelstr = "VARS";
        break;
    case LL_ALL:
        levelstr = "ALL";
        break;
    };

    return levelstr;
}

void LogThread::clean() {
    //get the lock
    mMutex.lock();

    //write the buffered log
    string loginfo;
    if(mCurInfoStream) {
        loginfo = mCurInfoStream->str();
        mCurInfoStream->str() = "";
    }

    switchStream();
    if(mCurInfoStream) {
        loginfo += mCurInfoStream->str();
        mCurInfoStream->str() = "";
    }

    //free the lock
    mMutex.unlock();

    if(!loginfo.empty()) {
        write2file(loginfo);
    }
}

bool startLogThread() {
    //init log thread
    if(RET_OK != LOGTHD()->initLogFile()) {
        LOG_COUT(LL_ERROR, "init log file failed.");
        return false;
    }
    LOGTHD()->clearLogOption(kLogNoThreadTag);

    //run log thread
    try {
        //ThreadTimer::instance()->start();
        LOGTHD()->start();
    } catch (const string& e) {
        LOG_COUT(LL_ERROR, "start log thread failed.");
        return false;
    }

    LOG_COUT(LL_INFO, "start log thread ok.");
    return true;
}

bool stopLogThread() {
    //stop log thread
    LOGTHD()->stop();

    return true;
}
