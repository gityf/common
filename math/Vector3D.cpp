/*
** Copyright (C) 2018 Wang Yaofu
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: The source file of class Vector3D.
*/

#include "Vector3D.h"
#include <iostream>
#include <math.h>

//默认构造函数,初始一个零向量
Vector3D::Vector3D() {
    x = 0.0f;
    y = 0.0f;
    z = 0.0f;
}

//复制构造函数
Vector3D::Vector3D(const Vector3D &a) {
    x = a.x;
    y = a.y;
    z = a.z;
}

//带参数的构造函数，用三个值完成初始化
Vector3D::Vector3D(float nx, float ny, float nz) {
    x = nx;
    y = ny;
    z = nz;
}

//析构函数
Vector3D::~Vector3D() {};

//标准对象操作
//重载赋值运算符，并返回引用，以实现左值。
Vector3D &Vector3D::operator=(const Vector3D &a) {
    x = a.x;
    y = a.y;
    z = a.z;
    return *this;
}

//重载"=="操作符
bool Vector3D::operator==(const Vector3D &a) const {
    return x == a.x && y == a.y && z == a.z;
}

//重载"!="操作符
bool Vector3D::operator!=(const Vector3D &a) const {
    return x != a.x || y != a.y || z != a.z;
}

//置为零向量
void Vector3D::zero() {
    x = y = z = 0.0f;
}

//重载一元"-"运算符
Vector3D Vector3D::operator-() const {
    return Vector3D(-x, -y, -z);
}

//重载二元"+"和"-"运算符
Vector3D Vector3D::operator+(const Vector3D &a) const {

    return Vector3D(x + a.x, y + a.y, z + a.z);
}

Vector3D Vector3D::operator-(const Vector3D &a) const {
    return Vector3D(x - a.x, y - a.y, z - a.z);
}

//标量的乘、除法
Vector3D Vector3D::operator*(float a) const {
    return Vector3D(x * a, y * a, z * a);

}

Vector3D Vector3D::operator/(float a) const {
    float oneOverA = 1.0f / a;//注意这里不对"除零"进行处理
    return Vector3D(x * oneOverA, y * oneOverA, z * oneOverA);
}

//重载自反运算符
Vector3D &Vector3D::operator+=(const Vector3D &a) {

    x += a.x;
    y += a.y;
    z += a.z;
    return *this;
}

Vector3D &Vector3D::operator-=(const Vector3D &a) {
    x -= a.x;
    y -= a.y;
    z -= a.z;
    return *this;
}

Vector3D &Vector3D::operator*=(float a) {
    x *= a;
    y *= a;
    z *= a;
    return *this;
}

Vector3D &Vector3D::operator/=(float a) {
    float oneOverA = 1.0f / a;
    x *= oneOverA;
    y *= oneOverA;
    z *= oneOverA;
    return *this;
}

//向量标准化
void Vector3D::normalize() {
    float magSq = x * x + y * y + z * z;
    if (magSq > 0.0f) {//检查除零
        float oneOverMag = 1.0f / sqrt(magSq);
        x *= oneOverMag;
        y *= oneOverMag;
        z *= oneOverMag;
    }
}

//向量点乘，重载标准的乘法运算符
float Vector3D::operator*(const Vector3D &a) const {
    return x * a.x + y * a.y + z * a.z;
}

void Vector3D::printVector3D() {
    std::cout << "(" << x << "," << y << "," << z << ")" << std::endl;

}

//求向量模
float Vector3D::vectorMag(const Vector3D &a) {
    return sqrt(a.x * a.x + a.y * a.y + a.z * a.z);
}

/*
Cross Product叉乘公式
aXb = | i   j   k  |
      | a.x a.y a.z|
      | b.x b.y b.z| = (a.y*b.z -a.z*b.y)i + (a.z*b.x - a.x*b.z)j + (a.x+b.y - a.y*b.x)k
*/

//计算两向量的叉乘
Vector3D Vector3D::crossProduct(const Vector3D &a, const Vector3D &b) {
    return Vector3D(
            a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x
    );
}

//计算两点间的距离
float Vector3D::distance(const Vector3D &a, const Vector3D &b) {

    float dx = a.x - b.x;
    float dy = a.y - b.y;
    float dz = a.z - b.z;
    return sqrt(dx * dx + dy * dy + dz * dz);
}