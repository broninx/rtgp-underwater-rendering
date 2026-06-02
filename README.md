# rtgp-underwater-rendering

This project implements a real-time underwater scene using OpenGL and C++, relying on graphical approximations rather than physically based simulation. The terrain is generated procedurally at runtime via a midpoint-displacement height-map algorithm, and additional static models are placed to improve scene plausibility. A dynamic diurnal cycle drives the environment: a diffuse directional light follows a solar path, triggering a day phase when the sun is above the horizon and a night phase when it falls below. During the day, caustics are projected onto the terrain using an animated texture, and crepuscular rays are rendered originating from the water surface along the sun direction. The water surface is a textured quad animated through a normal map to suggest wave motion. Depth-dependent underwater fog is applied using an exponential attenuation coefficient relative to camera distance, tinting distant geometry blue. The skybox blends two colors based on the vertical viewing angle, producing a smooth gradient from zenith to nadir. At night, the skybox, fog, water surface, and global illumination parameters are uniformly reduced, resulting in a dark environment. Small schools of fish traverse the scene as autonomous agents to add motion. Together, these techniques construct a time-varying underwater setting with consistent visual cues.

## Table of Contents
- [System Overview](#System-Overview)
- [Terrain](#Terrain)
- [Underwater Visual Effects Implementation](#Underwater-Visual-Effects-Implementation)
- [Scene Composition](#Scene-Composition)
- [Shader Program](#Shader-Program)
- [Performance](#Performance)
- [User Interaction & Camera](#User-Interaction-&-Camera)
- [References](#References)

## System Overview

### Tool and Libraries
The implementation relies on OpenGL for rendering, with GLFW managing the application window and input, GLAD handling extension loading, GLM providing vector and matrix mathematics, Assimp for importing external 3D models, and stb_image for loading texture assets.

### Application Architecture
The code is structured around a central Render object whose public interface follows a three-stage lifecycle. The Init method performs all one-time setup—creating the window, compiling shaders, and loading models and textures—and returns a non‑zero integer if any initialisation step fails. The Run method encapsulates the main loop: it clears the framebuffer, updates the scene state based on elapsed time and user input, executes all draw calls, and swaps buffers, repeating this cycle until the user presses the Escape key. When the loop exits, the Render object is destroyed, releasing all allocated graphics resources, and the program terminates cleanly.

## Terrain

The terrain is generated entirely at runtime using a midpoint-displacement algorithm, stored as a heightmap, and then rendered via a geometry clipmap (geomipmapping) approach with dynamic level-of-detail (LOD) determined by camera distance.

### Heightmap Generation
The class [`MidpointDispTerrain`](https://github.com/broninx/rtgp-underwater-rendering/blob/main/src/terrain/midpoint_disp.h) derives from [`Terrain`](https://github.com/broninx/rtgp-underwater-rendering/blob/main/src/terrain/terrain.h) and overrides the initialisation to produce a random fractal landscape. The method [`CreateMidpointDisplacement`](https://github.com/broninx/rtgp-underwater-rendering/blob/main/src/terrain/midpoint_disp.cpp#L4-L23) takes a terrain size (which must be a power of two plus one, e.g. 257, 513), a patch size for the subsequent geomip grid, a roughness factor, and min/max height bounds. 

Internally, the algorithm uses a classic square-diamond subdivision. The heightmap is first allocated as a 2D array of floats via `Array2D<float>`. The recursive subdivision works on a rectangle size that starts as the next power of two above the terrain size. In each iteration, [`DiamondStep`](https://github.com/broninx/rtgp-underwater-rendering/blob/main/src/terrain/midpoint_disp.cpp#L44-L75) computes the centre point of each square by averaging the four corner heights and adding a random offset proportional to the current amplitude (`CurHeight`). [`SquareStep`](https://github.com/broninx/rtgp-underwater-rendering/blob/main/src/terrain/midpoint_disp.cpp#L78-L133) then fills the edge midpoints by averaging the four surrounding diamond points (the centre of the current square and the three neighbours) plus random displacement. 

After each full cycle, the rectangle size is halved and the random amplitude is scaled by $`2^{-Roughness}`$, causing finer details to have smaller perturbations. Once subdivision completes, the heightmap values are normalised linearly into the user-specified min/max range. The base class `Terrain` stores the resulting heightmap and provides bilinear interpolation for smooth height queries at arbitrary world coordinates.

### Geomipmapping and LOD
The terrain geometry is managed by [`GeomipGrid`](https://github.com/broninx/rtgp-underwater-rendering/blob/main/src/terrain/geomip_grid.h), which implements a discrete LOD scheme based on precomputed triangle strips organised into patches. During initialisation ([`CreateGeomipGrid`](https://github.com/broninx/rtgp-underwater-rendering/blob/main/src/terrain/geomip_grid.cpp#L36-L71)), the grid is divided into a regular array of patches of size $PatchSize × PatchSize$ vertices. The number of patches along each axis is $(Width−1)/(PatchSize−1)$, which must be an integer. The maximum possible LOD level is derived from $log2(PatchSize−1)−1log2(PatchSize−1)−1$, and the `LodManager` stores that maximum.

Rather than recomputing index buffers at runtime, the entire set of possible index configurations is built once. For each LOD level, the method InitIndicesLODSingle generates triangle fans that step across the patch with a vertex stride equal to $2^{LOD+1}$. At the boundaries of a patch, the fan adapts to the neighbour's LOD: the four sides (left, right, top, bottom) can be independently flagged as either using the core LOD or a coarser one (stored as 0 or 1 in the `LodInfo` structure). Every permutation of these four flags is precomputed, and the start index and count of each permutation's index buffer segment are stored in the 4D array `m_lodInfo[lod].info[L][R][T][B]`.
The index buffer is uploaded to the GPU as `GL_STATIC_DRAW`, so no dynamic index manipulation occurs at runtime. Vertex positions are computed from the heightmap once, and normals are then calculated by averaging face normals of the finest LOD triangles.

<img width="1308" height="767" alt="lod-diagram" src="https://github.com/user-attachments/assets/2b5c23ae-5aa8-437b-b4dd-4fb72c1eb756" />

### LOD Management
The [`LodManager`](https://github.com/broninx/rtgp-underwater-rendering/blob/main/src/terrain/lod_manager.h) determines, every frame, which LOD each patch should use and how its edges must transition to match adjacent patches. The camera position is passed to `Update`. 

First, [`UpdateLodMapPass1`](https://github.com/broninx/rtgp-underwater-rendering/blob/main/src/terrain/lod_manager.cpp#L46-L65) calculates the Euclidean distance from the camera to the centre of each patch, scaled by the world scale. The distance is mapped to a core LOD via a set of linear distance regions computed in [`CalcLodRegions`](https://github.com/broninx/rtgp-underwater-rendering/blob/main/src/terrain/lod_manager.cpp#L156-L176). The farthest visible distance `Z_FAR` is divided into increasing intervals (the first region is the smallest, each subsequent one larger), so that nearer patches receive higher detail. The resulting core LOD for every patch is stored in a 2D array `m_map`. 

In [`UpdateLodMapPass2`](https://github.com/broninx/rtgp-underwater-rendering/blob/main/src/terrain/lod_manager.cpp#L67-L121), edge transition flags are set by comparing each patch’s core LOD with that of its four neighbours. If a neighbour has a higher LOD (i.e. coarser geometry), the shared side of the current patch is flagged to use the neighbour’s coarser level, avoiding T-junctions. This two‑pass scheme ensures seamless transitions without cracking. 

### Rendering
At render time, [`GeomipGrid::Render`](https://github.com/broninx/rtgp-underwater-rendering/blob/main/src/terrain/geomip_grid.cpp#L334-L359) queries the `LodManager` for the LOD parameters of each patch. For a given patch at grid position (PatchX, PatchZ), the LOD flags `(Core, Left, Right, Top, Bottom)` are used to index into the precomputed `LodInfo` structure. The appropriate index buffer sub‑range is bound, and the draw call uses `glDrawElementsBaseVertex` with the patch’s base vertex offset, so that the same index buffer can serve all patches without re‑uploading data. All patches are drawn in a single vertex array object binding, resulting in an efficient, single‑pass terrain render.

Together, the procedural heightmap generation and the LOD‑aware geomipmapping provide a flexible, runtime‑configurable landscape that adapts detail based on viewer distance, avoiding both geometric aliasing and unnecessary vertex processing.

## Underwater Visual Effects Implementation

## Scene Composition

## Shader Program 

## Performance 

## User Interaction & Camera

## References

