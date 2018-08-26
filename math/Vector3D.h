/*
** Copyright (C) 2018 Wang Yaofu
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: The header file of class Vector3D.
*/

#ifndef _COMMON_MATH_VECTOR3D_H_
#define _COMMON_MATH_VECTOR3D_H_

class Vector3D {
public:
    float x, y, z;

    //构造函数
    //默认构造函数，初始一个零向量
    Vector3D();

    //复制构造函数
    Vector3D(const Vector3D &a);

    //带参数的构造函数，用三个值完成初始化
    Vector3D(float nx, float ny, float nz);

    //析构函数
    ~Vector3D();

    //标准对象操作
    //重载赋值运算符，并返回引用，以实现左值。
    Vector3D &operator=(const Vector3D &a);

    //重载"=="操作符
    bool operator==(const Vector3D &a) const;

    //重载"!="操作符
    bool operator!=(const Vector3D &a) const;

    //向量运算

    //置为零向量
    void zero();

    //重载一元"-"运算符
    Vector3D operator-() const;

    //重载二元"+"和"-"运算符
    Vector3D operator+(const Vector3D &a) const;

    Vector3D operator-(const Vector3D &a) const;

    //标量的乘、除法
    Vector3D operator*(float a) const;

    Vector3D operator/(float a) const;

    //重载自反运算符
    Vector3D &operator+=(const Vector3D &a);

    Vector3D &operator-=(const Vector3D &a);

    Vector3D &operator*=(float a);

    Vector3D &operator/=(float a);

    //向量标准化
    void normalize();

    //向量点乘，重载标准的乘法运算符
    float operator*(const Vector3D &a) const;

    //求向量模
    float vectorMag(const Vector3D &a);

    //计算两向量的叉乘
    Vector3D crossProduct(const Vector3D &a, const Vector3D &b);

    //计算两点间的距离
    float distance(const Vector3D &a, const Vector3D &b);

    //打印向量
    void printVector3D();

};

#endif