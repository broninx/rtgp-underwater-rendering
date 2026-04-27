#pragma once

#include "terrain.h"
#include "lod_manager.h"

class MidpointDispTerrain : public Terrain {

 public:
    MidpointDispTerrain() {}

    void CreateMidpointDisplacement(int Size, int PatchSize, float Roughness, float MinHeight, float MaxHeight);

 private:
    void CreateMidpointDisplacementF32(float Roughness);
    void DiamondStep(int RectSize, float CurHeight);
    void SquareStep(int RectSize, float CurHeight);
};