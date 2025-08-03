#pragma once
#include <Windows.h>
#include <unordered_set>
#include <stdint.h>
#include <stdio.h>
#include <locale.h>
#include <thread>
#include <chrono>
#include <cstdio>
#include <cmath> 
#include <d3d8types.h>
#include <algorithm>

float calc3d_dist(const D3DVECTOR& p1, const D3DVECTOR& p2);


struct Vector3 {
    float x, y, z;
    Vector3() : x(0), y(0), z(0) {}
    Vector3(float x, float y, float z) : x(x), y(y), z(z) {}
    Vector3 operator+(const Vector3& other) const {
        return Vector3(x + other.x, y + other.y, z + other.z);
    }
    Vector3 operator-(const Vector3& other) const {
        return Vector3(x - other.x, y - other.y, z - other.z);
    }
    Vector3 operator*(float scalar) const {
        return Vector3(x * scalar, y * scalar, z * scalar);
    }
    float length() const {
        return std::sqrt(x * x + y * y + z * z);
    }
    Vector3 normalized() const {
        float len = length();
        if (len == 0) return Vector3(0, 0, 0);
        return *this * (1.0f / len);
    }

};
struct vec3 {
    float x, y, z;

    vec3() : x(0), y(0), z(0) {}
    vec3(float x_val, float y_val, float z_val) : x(x_val), y(y_val), z(z_val) {}

    void print() const {
        printf("x: %.2f, y: %.2f, z: %.2f\n", x, y, z);
    }

    vec3 operator+(const vec3& other) const {
        return vec3(x + other.x, y + other.y, z + other.z);
    }

    vec3 operator-(const vec3& other) const {
        return vec3(x - other.x, y - other.y, z - other.z);
    }

    vec3 operator*(double scalar) const {
        return vec3(x * scalar, y * scalar, z * scalar);
    }

    bool empty() const {
        return x == 0 && y == 0 && z == 0;
    }
};
float distance(const Vector3& a, const Vector3& b);



