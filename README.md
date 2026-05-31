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


## Underwater Visual Effects Implementation

## Scene Composition

## Shader Program 

## Performance 

## User Interaction & Camera

## References

