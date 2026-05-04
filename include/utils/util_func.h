#pragma once


#include <random>
#include <iostream>
#include <glm/glm.hpp>
#include "util_def.h"
#define PI 3.14159265359f

static std::random_device rd;
static std::mt19937 gen(rd());

inline float randomFloatRange(float min, float max)
{
    // static ensures the generator is seeded only once
    
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

inline void spreadXYZ(std::vector<glm::vec3>& pos, int startingPoint, int endingPoint, glm::vec3 center, float spreadRad)
{
    for (int i = startingPoint; i < endingPoint; i++)
    {
        float theta = PI * randomFloatRange(0.0f, 1.0f);
        float phi = acos(2.0f * randomFloatRange(0.0f, 1.0f) - 1.0f); // azimuth [0, 2pi]

        //uniform volume radius
        float r = spreadRad * cbrt(randomFloatRange(0.0f, 1.0f));

        pos.push_back(center + glm::vec3(r * sin(phi) * cos(theta), r * sin(phi) * sin(theta), r * cos(phi)));
    }
}


inline bool tooClose(std::vector<glm::vec3> vecPos, int numElem, glm::vec3 newPos, float minValue)
{
    for (int i = 0; i < numElem; i++)
    {
        if(Distance(vecPos[i], newPos) <= minValue)
        {
            return false;
        }
    }
    return true;
    
}
//spreadXYZ n times
inline void spreadXYZnt(std::vector<glm::vec3>& pos, float spreadRad, int numInstance, int numDiv)
{
    if (numInstance % numDiv != 0 )
    {
        printf("fun spreadXYZ: %i mod %i must be 0\n", numInstance, numDiv);
        exit(0);
    }
    float randx, randy,randz;
    const float terrSizef = (float) TERRAIN_SIZE; 
    const int batch = numInstance / numDiv;
    for(int i = 0; i < numDiv; i++)
    {
        randx = randomFloatRange(terrSizef / 4.0f - 30.0f, (terrSizef - (terrSizef / 4.0f) + 30.0f));
        randz = randomFloatRange(terrSizef / 4.0f - 30.0f, (terrSizef - (terrSizef / 4.0f) + 30.0f));
        randy = randomFloatRange((float)MAX_HEIGHT_TERR , terrSizef / 2.0f);
        spreadXYZ(pos, batch * i, batch * (i+1), glm::vec3(randx, randy, randz), spreadRad);
    }
}

