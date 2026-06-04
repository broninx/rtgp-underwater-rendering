# rtgp-underwater-rendering

## Usage

### Requirements
- A C++ compiler with C++11 support (GCC, Clang, or MSVC)
- GNU Make (or compatible build tool)

All other libraries (GLFW, GLAD, GLM, Assimp, Bullet, stb_image) are included in the repository and built automatically.

### Build
```bash
make
```

### Run
```bash
./main.out
```
### Controls
| Key | Action | 
|------|-------|
| Esc | Quit |
| w-a-s-d | Move around | 
| space-cntr | Move up-down | 
| mouse | Look around |
| f | Toggle wireframe |

## Table of Contents
- [Abstract](#Abstract)
- [System Overview](#System-Overview)
- [Terrain](#Terrain)
- [Scene Composition](#Scene-Composition)
- [Underwater Visual Effects Implementation](#Underwater-Visual-Effects-Implementation)
- [Shader Program](#Shader-Program)
- [Performance](#Performance)
- [User Interaction & Camera](#User-Interaction-&-Camera)
- [References](#References)

## Abstract

This project implements a real-time underwater scene using OpenGL and C++, relying on graphical approximations rather than physically based simulation. The terrain is generated procedurally at runtime via a midpoint-displacement height-map algorithm, and additional static models are placed to improve scene plausibility. A dynamic diurnal cycle drives the environment: a diffuse directional light follows a solar path, triggering a day phase when the sun is above the horizon and a night phase when it falls below. During the day, caustics are projected onto the terrain using an animated texture, and crepuscular rays are rendered originating from the water surface along the sun direction. The water surface is a textured quad animated through a normal map to suggest wave motion. Depth-dependent underwater fog is applied using an exponential attenuation coefficient relative to camera distance, tinting distant geometry blue. The skybox blends two colors based on the vertical viewing angle, producing a smooth gradient from zenith to nadir. At night, the skybox, fog, water surface, and global illumination parameters are uniformly reduced, resulting in a dark environment. Small schools of fish traverse the scene as autonomous agents to add motion. Together, these techniques construct a time-varying underwater setting with consistent visual cues.

## System Overview

### Tool and Libraries
The implementation relies on OpenGL for rendering, with GLFW v3.4 managing the application window and input, GLAD v0.1.36 handling extension loading, GLM 1.0.0 providing vector and matrix mathematics, Assimp v3.1.1 for importing external 3D models, and stb_image v2.30 for loading texture assets.

### Application Architecture
The code is structured around a central Render object whose public interface follows a three-stage lifecycle. The Init method performs all one-time setup—creating the window, compiling shaders, and loading models and textures—and returns a non‑zero integer if any initialisation step fails. The Run method encapsulates the main loop: it clears the framebuffer, updates the scene state based on elapsed time and user input, executes all draw calls, and swaps buffers, repeating this cycle until the user presses the Escape key. When the loop exits, the Render object is destroyed, releasing all allocated graphics resources, and the program terminates cleanly.

## Terrain

The terrain is generated entirely at runtime using a midpoint-displacement algorithm, stored as a heightmap, and then rendered via a geometry clipmap (geomipmapping) approach with dynamic level-of-detail (LOD) determined by camera distance.

### Heightmap Generation
The class [`MidpointDispTerrain`](https://github.com/broninx/rtgp-underwater-rendering/blob/main/src/terrain/midpoint_disp.h) derives from [`Terrain`](https://github.com/broninx/rtgp-underwater-rendering/blob/main/src/terrain/terrain.h) and overrides the initialisation to produce a random fractal landscape. The method [`CreateMidpointDisplacement`](https://github.com/broninx/rtgp-underwater-rendering/blob/main/src/terrain/midpoint_disp.cpp#L4-L23) takes a terrain size (which must be a power of two plus one, e.g. 257, 513), a patch size for the subsequent geomip grid, a roughness factor, and min/max height bounds. 

Internally, the algorithm uses a classic square-diamond subdivision. The heightmap is first allocated as a 2D array of floats via `Array2D<float>`. The recursive subdivision works on a rectangle size that starts as the next power of two above the terrain size. In each iteration, [`DiamondStep`](https://github.com/broninx/rtgp-underwater-rendering/blob/main/src/terrain/midpoint_disp.cpp#L44-L75) computes the centre point of each square by averaging the four corner heights and adding a random offset proportional to the current amplitude (`CurHeight`). [`SquareStep`](https://github.com/broninx/rtgp-underwater-rendering/blob/main/src/terrain/midpoint_disp.cpp#L78-L133) then fills the edge midpoints by averaging the four surrounding diamond points (the centre of the current square and the three neighbours) plus random displacement. 

After each full cycle, the rectangle size is halved and the random amplitude is scaled by $`2^{-Roughness}`$, causing finer details to have smaller perturbations. Once subdivision completes, the heightmap values are normalised linearly into the user-specified min/max range. The base class `Terrain` stores the resulting heightmap and provides bilinear interpolation for smooth height queries at arbitrary world coordinates.

### Geomipmapping and LOD
The terrain geometry is managed by [`GeomipGrid`](https://github.com/broninx/rtgp-underwater-rendering/blob/main/src/terrain/geomip_grid.h), which implements a discrete LOD scheme based on precomputed triangle strips organised into patches. During initialisation ([`CreateGeomipGrid`](https://github.com/broninx/rtgp-underwater-rendering/blob/main/src/terrain/geomip_grid.cpp#L36-L71)), the grid is divided into a regular array of patches of size $PatchSize * PatchSize$ vertices. The number of patches along each axis is $(Width−1)/(PatchSize−1)$, which must be an integer. The maximum possible LOD level is derived from $\log_2(PatchSize−1)−1$, and the `LodManager` stores that maximum.

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

## Scene Composition
The underwater environment is assembled from a procedurally generated terrain, a set of static and dynamic 3D models, a sky‑dome, a textured water surface, and volumetric light shafts. The terrain, produced by the midpoint‑displacement algorithm described in [Terrain section](#Terrain), acts as the seabed. It spans a square region of `TERRAIN_SIZE * TERRAIN_SIZE` vertices (with a resolution determined by the power‑of‑two height‑map size) and is scaled into world space by `TERRAIN_SCALE`. The resulting surface exhibits peaks and valleys within a height range bounded by `MIN_HEIGHT_TERR` and `MAX_HEIGHT_TERR`. The terrain is centred at world coordinates `(STARTING_X, 0, STARTING_Z)`, with the camera initially placed at that horizontal location and a given elevation `STARTING_Y`. Static detail is provided by a set of `STONE_NUM` stone models scattered across the seafloor. Their horizontal positions are randomly chosen within an annular region of the terrain (specifically, between one quarter of the terrain size minus 30 units and three quarters plus 30 units) to avoid clustering near the centre or edges. The vertical coordinate of each stone is obtained by sampling the terrain height at its random horizontal location, ensuring they rest on the seabed. Each stone is assigned a random uniform scale between 5.0 and 15.0, introducing variation in size. A single boat model is placed at a fixed location, its elevation set to the terrain height plus 10 units, and rotated by 180 degrees around the world X‑axis that have a fixed scale setted at the value 10.0. 

Dynamic elements consist of `FISH_NUM` fish models. Their initial positions are spread within a spherical volume of radius 30.0 centred at the terrain's centre, using a stratified distribution with `NUM_DIV_FISH` batches. Each fish is assigned a random scale between 0.03 and 0.09 and a per‑batch forward velocity chosen from the range `[0.02, 0.06]`. At each frame, fish translate along the positive world Z‑axis; when a fish's Z coordinate exceeds the terrain size, it wraps to zero, creating a continuous stream of moving animals across the scene.

Above the terrain, a large quad representing the water surface is placed at a height equal to the maximum terrain height plus half the terrain's world‑size, ensuring it sits above all underwater geometry. The quad is scaled uniformly by 2000.0 in X and Z to cover the entire visible area. The sky is rendered as a cube centred at the camera, with front face culling and a depth trick to guarantee it appears behind all other objects. The sky colour is a vertical gradient derived from the dynamic top and bottom water colours.

Finally, `NUM_SHAFTS` god ray quads are generated at the same elevation as the water surface. Their horizontal positions are randomly distributed within the terrain bounds. Each ray possesses a random width between `MIN_SHAFT_WIDTH` and `MAX_SHAFT_WIDTH`, and a random length between `MIN_SHAFT_LENGTH` and `MAX_SHAFT_LENGTH`. They are oriented along the sun direction and rendered as semi transparent, additive quads to create the impression of light shafts penetrating the water column.

All objects are drawn with the appropriate shaders, and their visual integration is governed by the unified fog, lighting, and day‑phase parameters better described in the next section.

## Underwater Visual Effects Implementation

### Dynamic Diurnal Illumination
The entire environment is driven by a continuous day‑night cycle. A directional light vector is computed by slowly incrementing azimuth and elevation angles each frame; the light direction used for shading is the reversed vector (`revLightDir`). A scalar dayPhase is derived as `smoothstep(-1.0, 1.0, revLightDir.y)`, producing a value near 1.0 when the sun is above the horizon (day) and near 0.0 when it is below (night). This phase is passed to all shaders and controls the interpolation between day and night configurations for sky colours, fog parameters, caustic intensity, god‑ray opacity, and water surface appearance. The diffuse light direction is transformed into view space for per‑fragment Blinn‑Phong calculations.

### Underwater Light Attenuation and Fog
Distance‑dependent fog simulates the scattering and absorption of light in water. In the fragment shaders ([general.frag](https://github.com/broninx/rtgp-underwater-rendering/blob/main/shaders/general.frag), [surface.frag](https://github.com/broninx/rtgp-underwater-rendering/blob/main/shaders/surface.frag), [terrain.frag](https://github.com/broninx/rtgp-underwater-rendering/blob/main/shaders/terrain.frag)), the fog factor is computed using an exponential model: $f=1−e^{−ρ⋅d}$, where $p$ is the uniform `densityFog` and $d$ is the Euclidean distance from the camera to the fragment in view space. The fog colour itself varies with depth. A depth parameter is calculated as $depthFog=y_{world}+(y_{world}−y_{cam})$, which exaggerates the vertical separation. This value is mapped through smoothstep between `gMinHeight` (terrain minimum) and `gMaxHeight` (half the terrain world size) to obtain a blend factor `t`.  

The final fog colour is a mix of the dynamic bottom and top water colours: `fogColor = mix(botColor, topColor, t)`. The lit fragment colour is then linearly interpolated with `fogColor` using the fog factor, making distant objects appear increasingly blue and eventually indistinguishable from the water column. 

### Skiy-Dome Gradient 
The sky is rendered as a full‑screen cube with front‑face culling and depth function set to `GL_LEQUAL`. In the skybox vertex shader, the homogeneous position is forced to `pos.xyww` to guarantee maximum depth. The fragment shader receives the world‑space vertex position, normalises it, and uses its vertical component to interpolate between the bottom colour (horizon) and top colour (zenith) via `smoothstep(-0.2, 0.5, dir.y)`. The two colours are the day phase driven `botColor` and `topColor`, ensuring that the sky naturally darkens at night and brightens by day.

### Water Surface Rendering
 A large horizontal quad, scaled to cover the entire visible area, is placed at a fixed height above the highest terrain point. It is drawn with alpha blending, depth writes disabled, and depth testing enabled, so that underwater objects remain visible through the surface. The fragment shader simulates both reflection and transparency.

 A normal map is scrolled over time using world‑space coordinates scaled by 0.002, producing a perturbed surface normal `N`. The view direction `V` (from the fragment to the camera) is reflected about `N` to obtain `R`. The vertical component of `R` is used to look up the sky colour via the same interpolation logic as the skybox, creating a dynamic reflection. A Fresnel term, computed as `1.0 - abs(dot(V, N))` raised to `fresnelPow`, determines how much water base colour (a deep blue) versus reflected sky is shown; at glancing angles the water becomes more reflective. Fog is applied to the final colour using the same exponential formula, and the `alpha` channel is set proportionally to `(1.0 - fogFactor)`, making the surface fade out with distance and allowing the seabed to show through. 

### Caustics Projection
Caustic light patterns are simulated on the terrain during daytime. The terrain fragment shader receives `dayPhase` and computes `causticStr = smoothstep(0.0, 1.0, dayPhase)`, so the effect disappears at night. A tileable caustic texture is sampled with coordinates equal to the terrain texture coordinates scaled by `CAUSTIC_SCALE` and offset by `uTime * CAUSTIC_SPEED`, producing animation. The caustic contribution is modulated by the diffuse lighting term `max(dot(N, gLightDir), 0.0)`, ensuring that caustics appear only where the sun would illuminate the surface. The resulting colour is added directly to the Blinn‑Phong lit colour before fog is applied, giving the appearance of focused light dancing on the seafloor.

### God Rays (Crepuscular Rays)
Light shafts that appear to originate from the water surface and travel in the sun's direction are implemented as instanced quads. At initialisation, a set of rays is generated with random horizontal positions within the terrain bounds, a random width and length, and placed at the water surface height. Each quad's model matrix is constructed so that its local Y axis aligns with the reversed light direction and its local X axis is the cross product of the camera's forward vector and the ray direction, achieving rough billboarding. The quads are rendered with additive blending (`GL_ONE`) to accumulate light. 

The god ray fragment shader receives a varying `vUV` where `vUV.y` goes from 0 at the water surface to 1 at the ray's lower end. The intensity is computed as `pow(1.0 - vUV.y, 2.0)`, giving a stronger appearance near the surface. This is multiplied by `rayStr = smoothstep(0.6, 1.0, dayPhase)` to disable the rays at night. A constant ray colour (light blue) is emitted, and the alpha is further attenuated by `(1.0 - fogFactor)`, where `fogFactor` is calculated from `distFromWater`, the vertical distance of the camera below the water surface, so that rays become fainter with depth.

### Animated Fish Schools
To add motion, schools of fish are rendered as instanced models. Their positions are spread in a spherical volume; each fish translates along the world Z‑axis at a constant velocity and wraps around when reaching the terrain boundary. This simple behaviour provides continuous, believable movement without complex simulation. The fish are drawn using the same general shader and lighting as static objects, benefiting from the fog and dynamic illumination.

## Performance 
All tests were performed on a desktop system with the following configuration: 

- V-Sink: disabled
- GPU: AMD Radeon RX 6600 at 1920 * 1080 
- CPU : AMD Ryzen 5 7600X 6-Core Processor
- Operating System: Pop!_OS 24.04 LTS
- Display resolution: 1920×1080 

| Descriptor | Value |
| -------- | -------- |
| Average FPS | 1700 |
| Average Frame Time | 0.06 ms |
| Max visible triangles | 8388608 |
| Typical triangles per frame (with LOD) | 1050000 - 1960000 |
| Draw calls per frame (without instancing) | 834 |
| Draw calls per frame (with instancing) | 7 |

## References
The OpenGL related code is based on the OpenGL tutorial of [Learn Opengl](https://learnopengl.com/), on the lab-lessons of the "Real-Time Graphics Programming Course" and on the YouTube playlist of OGLDEV, [Terrain Rendering](https://www.youtube.com/watch?v=4Rbk6xRzs6g&list=PLA0dXqQjCx0S9qG5dWLsheiCJV-_eLUM0).
