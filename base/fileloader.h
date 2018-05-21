/*
** Copyright (C) 2015 Wang Yaofu
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: The header file of class FileLoader.
*/
#pragma once
#include <string>

using std::string;

class FileLoader {
public:
    FileLoader();
    virtual ~FileLoader();
    virtual bool handleLine(const string& line) = 0;
    // load file.
    bool load(const string& file);
private:
};