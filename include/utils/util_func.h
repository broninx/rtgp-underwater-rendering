#pragma once


#include <random>
#include <iostream>
#include <glm/glm.hpp>

inline float randomFloatRange(float min, float max)
{
    // static ensures the generator is seeded only once
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(min, max);
    return dis(gen);
}

template <typename T>
inline T nextPow2(T n)
{
    if (n <= 1) return 1;
    T power = 1;
    while (power < n)
        power <<= 1;
    return power;
}

inline int pow2i(int num) {
    int power = 1;
    return power <<= num;
}

inline float DistanceSquared(const glm::vec3& v1, const glm::vec3& v2)
{
    float delta_x = v1.x - v2.x;
    float delta_y = v1.y - v2.y;
    float delta_z = v1.z - v2.z;

    float DistanceSquared = delta_x * delta_x + delta_y * delta_y + delta_z * delta_z;

    return DistanceSquared;
}

inline float Distance(const glm::vec3& v1,const glm::vec3& v2)
{
    float DistSquared = DistanceSquared(v1, v2);
    float distance = sqrtf(DistSquared);
    return distance;
}

template<typename T>
inline T clamp(const T& value, const T& min, const T& max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}