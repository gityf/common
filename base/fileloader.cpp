/*
** Copyright (C) 2015 Wang Yaofu
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: The source file of class FileLoader.
*/
#include <fstream>
#include "fileloader.h"
#include "exitcaller.h"
//#include "log.h"

FileLoader::FileLoader() {
}

FileLoader::~FileLoader() {
}

bool FileLoader::handleLine(const string& line) {
    return true;
}

bool FileLoader::load(const string& file) {
    std::ifstream ifs(file.c_str());
    if (!ifs.good()) {
        return false;
    }
    ExitCaller ec1([&]{ ifs.close(); });
    string line;
    size_t lineCount = 0, errLines = 0;
    while (std::getline(ifs, line)) {
        ++lineCount;
        if (!handleLine(line)) {
            //log_fatal("handle line error, line:" << lineCount);
            ++errLines;
        }
    }
    // log handle line error.
    //log_warn("load file:" << file << " end, all lines:"
    //    << lineCount << ", error lines:" << errLines);
    return lineCount != errLines;
}