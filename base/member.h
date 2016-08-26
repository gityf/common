/*
** Copyright (C) 2014 Wang Yaofu
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: The header file of class member define.
*/
#ifndef _COMMON_CLASS_MEMBER_H_
#define _COMMON_CLASS_MEMBER_H_
#define MEMBER(type, name) \
    private: type m##name; \
    public: type get_##name() const {return this->m##name;} \
    public: void set_##name(type _arg){this->m##name=_arg;} \
    private:
#endif  // _COMMON_CLASS_MEMBER_H_
