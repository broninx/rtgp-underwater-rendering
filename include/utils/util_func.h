#pragma once
#include <random>
#include <iostream>


float randomFloatRange(float min, float max)
{
    // static ensures the generator is seeded only once
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(min, max);
    return dis(gen);
}


template <typename T>
T nextPow2(T n)
{
    if (n <= 1) return 1;
    T power = 1;
    while (power < n)
        power <<= 1;
    return power;
}