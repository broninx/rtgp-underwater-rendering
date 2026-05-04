
#pragma once


#define SAFE_DELETE(p) { if(p) { delete p; p = nullptr; } }
// dimensions of the window
#define SCR_WIDHT 1920
#define SCR_HEIGHT 1080
#define TERRAIN_SIZE 2049
#define ROUGHNESS_TERR 1.0f
#define MAX_HEIGHT_TERR 200.0f
#define MIN_HEIGHT_TERR 0.0f
#define TERRAIN_SCALE 1.0f
#define STARTING_X (TERRAIN_SIZE - 1 ) / 2
#define STARTING_Z (TERRAIN_SIZE - 1 ) / 2
#define STARTING_Y MAX_HEIGHT_TERR
#define FISH_NUM 600
#define STONE_NUM 30
#define BOAT_NUM 1
#define FOG_DENS 0.006f
#define KA 0.3f
#define KD 0.6f
#define KS 0.1f
#define SHININESS 25.0f
