// Std. Includes
#include <string>

// Loader for OpenGL extensions
// http://glad.dav1d.de/
// THIS IS OPTIONAL AND NOT REQUIRED, ONLY USE THIS IF YOU DON'T WANT GLAD TO INCLUDE windows.h
// GLAD will include windows.h for APIENTRY if it was not previously defined.
// Make sure you have the correct definition for APIENTRY for platforms which define _WIN32 but don't use __stdcall
#ifdef _WIN32
    #define APIENTRY __stdcall
#endif

#include <glad/glad.h>

// GLFW library to create window and to manage I/O
#include <glfw/glfw3.h>

// another check related to OpenGL loader
// confirm that GLAD didn't include windows.h
#ifdef _WINDOWS_
    #error windows.h was included!
#endif

#include <utils/shader.h>
#include <utils/model.h>
#include <utils/camera.h>
#include <utils/util_func.h>

// we load the GLM classes used in the application
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "terrain/geomip_grid.h"
#include "terrain/midpoint_disp.h"

#include <utils/util_def.h>

// boolean to activate/deactivate wireframe rendering
GLboolean wireframe = GL_FALSE;

// callback functions for keyboard and mouse events
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);


// enums to identify the models, shaders and textures used in the application
enum SceneObj{ CUBE, CATFISH, STONE, BOAT, PLANE, SPHERE, QUAD}; 
enum ShaderType{ TERRAIN, GENERAL, GENERALONE, SKYBOX, SURFACE, GODRAYS};
enum Textures {SANDTERRAIN,CATFISHTXT, STONETXT, BOATTXT, CAUSTICTXT, WATERNRM};

class Render
{

private:

    enum Models {FISHMOD, STONEMOD, BOATMOD};

    struct ModelTransform {
        std::vector<glm::vec3> worldPos;
        std::vector<glm::vec3> worldScale;
        std::vector<float> angleRotation;
        std::vector<float> velocities;
    };
    struct GodRay {
        glm::vec3 origin;
        float width;
        float length;
    };

    GLFWwindow* window = NULL;
    Camera* m_cam;
    GLboolean m_isWireframe = false;
    std::vector<Model> m_models;
    std::vector<Shader> m_shaders;
    std::vector<Texture> m_textures; 
    glm::mat4 m_view = glm::mat4(1.0f);
    glm::mat4 m_projection = glm::mat4(1.0f);
    GLint m_lastX, m_lastY;
    GLboolean m_firstMouse = true;
    GLfloat m_deltaTime = 0.0f; // time between current frame and last frame
    GLfloat m_lastFrame = 0.0f; // time of last frame
    GLfloat m_currentFrame = 0.0f; // time of current frame
    MidpointDispTerrain m_terrain;
    int keys[1024];
    int counter = 0;
    glm::vec3 m_sunDir;
    ModelTransform m_fish;
    ModelTransform m_stone;
    ModelTransform m_boat;
    GodRay m_godRays[NUM_SHAFTS];


    // method to initialize the window
    void CreateWindow()
    {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

        // we create the application's window
        window = glfwCreateWindow(SCR_WIDHT, SCR_HEIGHT, "RTGP_lecture05", nullptr, nullptr);
    }

    // method to initialize the callbacks
    void InitCallbacks()
    {
        // we put in relation the window and the callbacks
        glfwSetKeyCallback(window, key_callback);
        glfwSetCursorPosCallback(window, mouse_callback);

        // we disable the mouse cursor
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }

    // method to intialize the shaders
    void InitShaders()
    {
        m_shaders.push_back(Shader("shaders/terrain.vert", "shaders/terrain.frag"));
        m_shaders.push_back(Shader("shaders/general.vert", "shaders/general.frag"));
        m_shaders.push_back(Shader("shaders/generalOne.vert", "shaders/general.frag"));
        m_shaders.push_back(Shader("shaders/skybox.vert", "shaders/skybox.frag"));
        m_shaders.push_back(Shader("shaders/surface.vert", "shaders/surface.frag"));
        m_shaders.push_back(Shader("shaders/godRays.vert", "shaders/godRays.frag"));
    }

    //method to initialize the models
    void InitModels()
    {
        m_models.push_back(Model("models/cube.obj")); // used for the enviroment map
        m_models.push_back(Model("models/catfish_obj/catfishRawModel.obj"));
        m_models.push_back(Model("models/stone.OBJ"));
        m_models.push_back(Model("models/boat.obj"));
        m_models.push_back(Model("models/plane.obj"));
        m_models.push_back(Model("models/sphere.obj"));
        m_models.push_back(Model("models/quad.obj"));
    }

    // method to initialize the textures
    void InitTextures()
    {
        // we add textures to the vector of textures and we load them
        m_textures.push_back(Texture(GL_TEXTURE_2D, "textures/sand_white.png"));
        m_textures.push_back(Texture(GL_TEXTURE_2D, "textures/catfish.png"));
        m_textures.push_back(Texture(GL_TEXTURE_2D, "textures/stone_tex/stone_diffuse.png"));
        m_textures.push_back(Texture(GL_TEXTURE_2D, "textures/Wood_Cherry_Original.jpg"));
        m_textures.push_back(Texture(GL_TEXTURE_2D, "textures/Caustic_Free.jpg"));
        m_textures.push_back(Texture(GL_TEXTURE_2D, "textures/water_normal.jpg"));

        m_textures[SANDTERRAIN].Load();
        m_textures[CATFISHTXT].Load();
        m_textures[STONETXT].Load();
        m_textures[BOATTXT].Load();
        m_textures[CAUSTICTXT].Load();
        m_textures[WATERNRM].Load();
    }

    void InitGodRays()
    {
        float yPos = m_terrain.GetMaxHeight() + m_terrain.GetSize() /2.0f;
        float range = (float)TERRAIN_SIZE / 2.0f;

        for(int i = 0; i < NUM_SHAFTS; i++)
        {
            float x = randomFloatRange(STARTING_X - range, STARTING_X + range);
            float z = randomFloatRange(STARTING_Z - range, STARTING_Z + range);
            float width = randomFloatRange(MIN_SHAFT_WIDTH, MAX_SHAFT_WIDTH);
            float length = randomFloatRange(MIN_SHAFT_LENGTH, MAX_SHAFT_LENGTH);
            m_godRays[i].origin = glm::vec3(x, yPos, z);
            m_godRays[i].width = width;
            m_godRays[i].length = length;
        }

    }

    // method to initialize the terrain
    void InitTerrain()
    {
        // we initialize the terrain
        m_terrain.Init(TERRAIN_SCALE);

        int patchSize = 33;
        m_terrain.CreateMidpointDisplacement(TERRAIN_SIZE, patchSize, ROUGHNESS_TERR, MIN_HEIGHT_TERR, MAX_HEIGHT_TERR);

    }

    void InitObjWorldPos()
    {
        // initialize positions of the fishes 

        //spread a batch of fishes in a spherical space
        const float spreadRad = 30.0f;
        const int numDiv = NUM_DIV_FISH;
        spreadXYZnt(m_fish.worldPos, spreadRad, FISH_NUM, numDiv);

        // initialize scale and velocity of the fishes
        for(int i = 0; i < FISH_NUM; i ++)
        {
            m_fish.worldScale.push_back(glm::vec3(randomFloatRange(0.03f, 0.09f)));
        }

        for(int i = 0; i < NUM_DIV_FISH; i++)
        {
            m_fish.velocities.push_back(randomFloatRange(0.02, 0.06));
        }

        // initialize positions of the stones
        const float terrSizef = (float) TERRAIN_SIZE; 
        float randx, randy, randz;
        for(int i = 0; i < STONE_NUM; i ++)
        {
            /*  we spread the stones in a wide area around the center of the terrain, 
                to avoid having them all clustered in the center (where the fishes are) 
                and to have some of them also in the external part of the terrain, where the cam can go. We also add a small margin of 30.0f 
                to avoid having stones too close to the borders of the terrain,
                where the height is very low and we could have some visual artifacts */
            randx = randomFloatRange(terrSizef / 4.0f - 30.0f, (terrSizef - (terrSizef / 4.0f) + 30.0f));
            randz = randomFloatRange(terrSizef / 4.0f - 30.0f, (terrSizef - (terrSizef / 4.0f) + 30.0f));
            randy = m_terrain.GetHeight(randx, randz);

            // then we add a random position in the sphere of radius spreadRad
            m_stone.worldPos.push_back(glm::vec3(randx, randy, randz));
            m_stone.worldScale.push_back(glm::vec3(randomFloatRange(5.0f, 15.0f)));
        }

        // initialization boats
        float x, y, z;
        x = STARTING_X - 82.7f; 
        z = STARTING_Z + 132.4f; 
        y = m_terrain.GetHeight(x, z) + 10.0f;
        m_boat.worldPos.push_back(glm::vec3(x, y, z));
        m_boat.worldScale.push_back(glm::vec3(10.0f));
        m_boat.angleRotation.push_back(180.0f);
    }

    void SetUniforms(GLuint prog, glm::vec3 lightVDir, glm::vec3& topColor, glm::vec3& botColor, float dayPhase)
    {
        // light direction
        glUniform3fv(glGetUniformLocation(prog, "gLightDir"), 1, glm::value_ptr(lightVDir));

        // density of the fog, top color and bottom color
        const float fogD = FOG_DENS;
        glUniform1f(glGetUniformLocation(prog, "densityFog"), fogD);
        glUniform3fv(glGetUniformLocation(prog, "topColor"), 1, glm::value_ptr(topColor));
        glUniform3fv(glGetUniformLocation(prog, "botColor"), 1, glm::value_ptr(botColor));

        // min height and max height to calculate the color of the fog
        glUniform1f(glGetUniformLocation(prog, "gMinHeight"), m_terrain.GetMinHeight());

        const float worldSize = TERRAIN_SIZE / 2;
        glUniform1f(glGetUniformLocation(prog, "gMaxHeight"), worldSize);

        // position of the cam to calculate the fog factor
        glUniform3fv(glGetUniformLocation(prog, "camPos"), 1, glm::value_ptr(m_cam->getCamPos()));

        glUniformMatrix4fv(glGetUniformLocation(prog, "viewMatrix"), 1, GL_FALSE, glm::value_ptr(m_view));
        glUniformMatrix4fv(glGetUniformLocation(prog, "projectionMatrix"), 1, GL_FALSE, glm::value_ptr(m_projection));

        glUniform3fv(glGetUniformLocation(prog, "specularColor"), 1, glm::value_ptr(glm::vec3(1.0f)));

        float ka = KA;
        glUniform1f(glGetUniformLocation(prog, "Ka"), ka);
        float kd = KD;
        glUniform1f(glGetUniformLocation(prog, "Kd"), kd);
        float ks = KS;
        glUniform1f(glGetUniformLocation(prog, "Ks"), ks);
        float shininess = SHININESS;
        glUniform1f(glGetUniformLocation(prog, "shininess"), shininess);
    }

public:

    Render(){}

    virtual ~Render()
    {
        SAFE_DELETE(m_cam);
        glfwTerminate();
    }

    int Init()
    {
        // we create a camera. We pass the initial position as a parameter to the constructor. 
        m_cam = new Camera(glm::vec3(STARTING_X, STARTING_Y, STARTING_Z));
        
        glfwInit();

        // we add a stencil buffer to the framebuffer, in order to be able to use the stencil test for the water surface rendering
        glfwWindowHint(GLFW_STENCIL_BITS, 8);
        CreateWindow();
        if (!window)
        {
            std::cout << "Failed to create GLFW window" << std::endl;
            return -1;
        }

        glfwMakeContextCurrent(window);

        // Disable V-Sync
        glfwSwapInterval(0);

        // GLAD tries to load the context set by GLFW
        if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress))
        {
            std::cout << "Failed to initialize OpenGL context" << std::endl;
            return -1;
        }

        // we define the viewport dimensions
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);
        m_projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDHT / (float)SCR_HEIGHT, 0.1f, 1200.0f);

        InitCallbacks();

        InitShaders();

        InitModels();
       
        // we set the models to be instanced, so that we can render multiple instances of the same model with a single draw call
        m_models[CATFISH].SetInstanced();
        m_models[STONE].SetInstanced();
        m_models[BOAT].SetInstanced();
        m_models[QUAD].SetInstanced();

        InitTerrain();

        InitObjWorldPos();

        InitTextures();

        InitGodRays();

        glFrontFace(GL_CW);
        glEnable(GL_DEPTH_TEST); //TODO: move this in the main funcion
        glClearColor(0.26f, 0.46f, 0.98f, 1.0f); //TODO: move this in the main funcion

        return 0;
    }

    void Run()
    {
        while (!glfwWindowShouldClose(window))
        {
            // we calculate the time difference between the current frame and the last frame
            m_currentFrame = glfwGetTime();
            m_deltaTime = m_currentFrame - m_lastFrame;

            counter++;

            if(m_deltaTime >= 1.0f / 30.0f)
            {
                std::string FPS = std::to_string((1.0f / m_deltaTime) * counter);
                std::string ms = std::to_string((m_deltaTime / counter) * 100.0f);
                std::string title = "rtgp-underwater-rendering - " + FPS + " FPS, " + ms + " ms";
                glfwSetWindowTitle(window, title.c_str());
                counter = 0;
                m_lastFrame = m_currentFrame;
            }

            // Check is an I/O event is happening
            glfwPollEvents();

            // we apply the camera movements following the keys pressed
            apply_camera_movements();

            // View matrix (=camera): position, view direction, camera "up" vector
            m_view = m_cam->GetViewMatrix();

            // we set the rendering mode
            // Draw in wireframe
            glPolygonMode(GL_FRONT_AND_BACK, (m_isWireframe ? GL_LINE : GL_FILL));

            // we render the scene
            RenderScene();

            // we swap the buffers of the current window
            glfwSwapBuffers(window);
        }
    }

    void RenderScene()
    {
        GLuint prog;
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        /////////// SUNLIGHT DIRECTION /////////////////////////

        static float horizontalAngle = 270.0f;
        static float verticalAngle = 270.0f;

        horizontalAngle >= 360.0f ? horizontalAngle = 0.002f : horizontalAngle += 0.002f;
        verticalAngle  >= 360.0f ? verticalAngle  = 0.002f : verticalAngle += 0.002f;

        float azimuth = glm::radians(horizontalAngle);
        float elevation = glm::radians(verticalAngle);

        m_sunDir.x = cos(elevation) * sin(azimuth);
        m_sunDir.y = sin(elevation);
        m_sunDir.z = cos(elevation) * cos(azimuth);

        glm::vec3 revLightDir = m_sunDir * -1.0f;

        //////////////////// COLORS SETUP /////////////////////////////////////////

        float dayPhase = glm::smoothstep(-1.0f, 1.0f, revLightDir.y);
        
        static const glm::vec3 dayTopColor = glm::vec3(0.25f, 0.65f, 0.95f);
        static const glm::vec3 nightTopColor = glm::vec3(0.02f, 0.04f, 0.12f);
        static const glm::vec3 dayBotColor = glm::vec3(0.08f, 0.12f, 0.36f); 
        static const glm::vec3 nightBotColor = glm::vec3(0.0f, 0.005f, 0.02f);

        glm::vec3 topColor = glm::mix(nightTopColor, dayTopColor, dayPhase);
        glm::vec3 botColor = glm::mix(nightBotColor, dayBotColor, dayPhase);

        /////////////////// OBJECTS ////////////////////////////////////////////////

        glm::vec3 lightVDir = glm::normalize(glm::mat3(m_view) * revLightDir); 

        // CATFISHES //

        m_shaders[GENERAL].Use();
        prog = m_shaders[GENERAL].Program;

        //texture
        m_textures[CATFISHTXT].Bind(GL_TEXTURE0);
        glUniform1i(glGetUniformLocation(prog, "gTexture"), 0);

        SetUniforms(prog, lightVDir, topColor, botColor, dayPhase);

        int numFishPerBatch = FISH_NUM / NUM_DIV_FISH;
        std::vector<glm::mat4> fishModels(FISH_NUM);
        for(int i = 0; i < NUM_DIV_FISH; i++)
        {
            for (int j = 0; j < numFishPerBatch; j++)
            {
                // the general idx is determined by the batch idx and the idx inside the batch
                int idx = i * numFishPerBatch + j;

                // we update the position of the fish along the Z axis, using its velocity
                m_fish.worldPos[idx].z += m_fish.velocities[i];

                // if the fish goes beyond the terrain, we reset its position to the beginning of the terrain
                if (m_fish.worldPos[idx].z >= TERRAIN_SIZE) {m_fish.worldPos[idx].z = 0;}
                glm::mat4 mat = glm::mat4(1.0f);
                mat = glm::translate(mat, m_fish.worldPos[idx]); // spread out along X
                mat = glm::scale(mat, m_fish.worldScale[idx]); // spread out along X

                fishModels[idx] = mat;
            }
        }

        m_models[CATFISH].SetVBOI(fishModels, FISH_NUM);

        m_models[CATFISH].Draw(FISH_NUM);


        // STONE //

        m_textures[STONETXT].Bind(GL_TEXTURE0);
        glUniform1i(glGetUniformLocation(prog, "gTexture"), 0);

        std::vector<glm::mat4> stoneModels(STONE_NUM);
        for (int i = 0; i < STONE_NUM; i++)
        {
            glm::mat4 mat = glm::mat4(1.0f);
            mat = glm::translate(mat, m_stone.worldPos[i]); // spread out along X
            mat = glm::scale(mat, m_stone.worldScale[i]); // spread out along X

            stoneModels[i] = mat;
        }

        m_models[STONE].SetVBOI(stoneModels, STONE_NUM);

        m_models[STONE].Draw(STONE_NUM);


        // BOAT //

        m_textures[BOATTXT].Bind(GL_TEXTURE0);
        glUniform1i(glGetUniformLocation(prog, "gTexture"), 0);

        std::vector<glm::mat4> boatModels(BOAT_NUM);
        glm::mat4 mat = glm::mat4(1.0f);

        mat = glm::translate(mat, m_boat.worldPos[0]); // spread out along X
        mat = mat * glm::rotate(glm::mat4(1.0f), glm::radians(m_boat.angleRotation[0]), glm::vec3(1,0,0));
        mat = glm::scale(mat, m_boat.worldScale[0]); // spread out along X
        boatModels[0] = mat;

        m_models[BOAT].SetVBOI(boatModels, BOAT_NUM);

        m_models[BOAT].Draw(BOAT_NUM);

        // TERRAIN //
        m_shaders[TERRAIN].Use();
        prog = m_shaders[TERRAIN].Program;

        SetUniforms(prog, lightVDir, topColor, botColor, dayPhase);

        glUniform1f(glGetUniformLocation(prog, "dayPhase"), dayPhase);
        glUniform1f(glGetUniformLocation(prog, "uTime"), (float)m_currentFrame);

        m_textures[SANDTERRAIN].Bind(GL_TEXTURE0);
        glUniform1i(glGetUniformLocation(prog, "gTerrainTexture"), 0);

        m_textures[CAUSTICTXT].Bind(GL_TEXTURE1);
        glUniform1i(glGetUniformLocation(prog, "gTexCaustic"), 1);

        m_terrain.Render(m_cam->getCamPos());


        /////////////////// SKYBOX ////////////////////////////////////////////////
        m_shaders[SKYBOX].Use();
        prog = m_shaders[SKYBOX].Program;

        GLint OldCullFaceMode;
        glGetIntegerv(GL_CULL_FACE_MODE, &OldCullFaceMode);
        GLint OldDepthFuncMode;
        glGetIntegerv(GL_DEPTH_FUNC, &OldDepthFuncMode);

        glCullFace(GL_FRONT);

        glDepthFunc(GL_LEQUAL);

        glm::mat4 view = glm::mat4(glm::mat3(m_view));
        glm::mat4 VP = m_projection * view;
        glUniformMatrix4fv(glGetUniformLocation(prog, "gVP"), 1, GL_FALSE, glm::value_ptr(VP));
        glUniform3fv(glGetUniformLocation(prog, "topColor"), 1, glm::value_ptr(topColor));
        glUniform3fv(glGetUniformLocation(prog, "botColor"), 1, glm::value_ptr(botColor));

        m_models[CUBE].Draw();

        glCullFace(OldCullFaceMode);
        glDepthFunc(OldDepthFuncMode);


        /////////////////// WATER SURFACE ////////////////////////////////////////

        glDepthFunc(GL_LESS);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_FALSE);
        m_shaders[SURFACE].Use();
        prog = m_shaders[SURFACE].Program;

        m_textures[WATERNRM].Bind(GL_TEXTURE0);

        SetUniforms(prog, lightVDir, topColor, botColor, dayPhase);

        const float d = DISTORTION_STR;
        const float f = FRESNEL_POW;
        glUniform1f(glGetUniformLocation(prog, "uTime"), (float)m_currentFrame);
        glUniform1f(glGetUniformLocation(prog, "distortStr"), d);
        glUniform1f(glGetUniformLocation(prog, "fresnelPow"), f);

        glm::mat4 matPlane = glm::mat4(1.0f);
        float planeY =  m_terrain.GetMaxHeight() + m_terrain.GetSize() / 2;  
        matPlane = glm::translate(matPlane, glm::vec3(STARTING_X, planeY, STARTING_Z));
        matPlane = glm::scale(matPlane, glm::vec3(2000.0f, 1.0f, 2000.0f));
        glUniformMatrix4fv(glGetUniformLocation(prog, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(matPlane));

        m_models[PLANE].Draw();

        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND); 

        //////////////////// GOD RAYS ////////////////////////////////////////////

        glm::vec3 rayDir = glm::normalize(revLightDir);

        // Camera forward (billboard helper)
        glm::vec3 camForward = glm::vec3(-m_view[0][2], -m_view[1][2], -m_view[2][2]);
        glm::vec3 right = glm::normalize(glm::cross(camForward, rayDir));

        // Build an instance matrix for every shaft
        std::vector<glm::mat4> rayMats(NUM_SHAFTS);
        for (int i = 0; i < NUM_SHAFTS; ++i)
        {
            glm::vec3 origin = m_godRays[i].origin;
            // Matrix that maps quad local space:
            //   local Y (0->1)  ->  rayDir direction, length = shaftLength
            //   local X (0->1)  ->  right direction, width = shaftWidth
            glm::mat4 model = glm::mat4(1.0f);
            model[0] = glm::vec4(right * m_godRays[i].width, 0.0f);      // X axis -> width
            model[1] = glm::vec4(rayDir * m_godRays[i].length, 0.0f);    // Y axis -> length
            model[2] = glm::vec4(glm::cross(right, rayDir), 0.0f); // Z axis (perpendicular)
            model[3] = glm::vec4(origin, 1.0f);
            rayMats[i] = model;
        }

        // Update the instance buffer for the quad model
        m_models[QUAD].SetVBOI(rayMats, NUM_SHAFTS);

        // ---- Draw with additive blending ----
        m_shaders[GODRAYS].Use();
        prog = m_shaders[GODRAYS].Program;
        glUniformMatrix4fv(glGetUniformLocation(prog, "viewMatrix"), 1, GL_FALSE, glm::value_ptr(m_view));
        glUniformMatrix4fv(glGetUniformLocation(prog, "projectionMatrix"), 1, GL_FALSE, glm::value_ptr(m_projection));
        // glUniform1f(glGetUniformLocation(prog, "intensity"), 1.2f);
        glUniform1f(glGetUniformLocation(prog, "dayPhase"), dayPhase);
        const float fogD = FOG_DENS;
        glUniform1f(glGetUniformLocation(prog, "densityFog"), fogD);
        float worldHeight = m_terrain.GetWorldHeight(m_cam->getCamPos().x, m_cam->getCamPos().z);
        float worldSize = m_terrain.GetWorldSize();
        float dist = (worldHeight + worldSize / 2) - m_cam->getCamPos().y ; // distance from the water surface, used to attenuate the god rays
        glUniform1f(glGetUniformLocation(prog, "distFromWater"), dist);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);   // additive
        // glDepthMask(GL_FALSE);
        // glDisable(GL_CULL_FACE);

        m_models[QUAD].Draw(NUM_SHAFTS);   // instanced draw

        glDisable(GL_BLEND);
        // glDepthMask(GL_TRUE);
        // glEnable(GL_CULL_FACE);
    }

    void apply_camera_movements()
    {
        // if a single WASD key is pressed, then we will apply the full value of velocity v in the corresponding direction.
        // However, if two keys are pressed together in order to move diagonally (W+D, W+A, S+D, S+A), 
        // then the camera will apply a compensation factor to the velocities applied in the single directions, 
        // in order to have the full v applied in the diagonal direction  
        // the XOR on A and D is to avoid the application of a wrong attenuation in the case W+A+D or S+A+D are pressed together.  
        GLboolean diagonal_movement = (keys[GLFW_KEY_W] ^ keys[GLFW_KEY_S]) && (keys[GLFW_KEY_A] ^ keys[GLFW_KEY_D]); 
        m_cam->SetMovementCompensation(diagonal_movement);
        float worldHeight = m_terrain.GetWorldHeight(m_cam->getCamPos().x, m_cam->getCamPos().z);
        float worldSize = m_terrain.GetWorldSize();
        if(keys[GLFW_KEY_W])
            m_cam->ProcessKeyboard(FORWARD, m_deltaTime , worldSize, worldHeight);
        if(keys[GLFW_KEY_S])
            m_cam->ProcessKeyboard(BACKWARD, m_deltaTime , worldSize, worldHeight);
        if(keys[GLFW_KEY_A])
            m_cam->ProcessKeyboard(LEFT, m_deltaTime , worldSize, worldHeight);
        if(keys[GLFW_KEY_D])
            m_cam->ProcessKeyboard(RIGHT, m_deltaTime , worldSize, worldHeight);
        if(keys[GLFW_KEY_SPACE])
            m_cam->ProcessKeyboard(UP, m_deltaTime , worldSize, worldHeight);
        if(keys[GLFW_KEY_LEFT_CONTROL])
            m_cam->ProcessKeyboard(DOWN, m_deltaTime , worldSize, worldHeight);
    }

    void KeyboardCB(uint key, int state){

        if (key == GLFW_KEY_ESCAPE && state == GLFW_PRESS)
        {
            glfwDestroyWindow(window);
            glfwTerminate();
            exit(0);
        }
        
        if (key == GLFW_KEY_F && state == GLFW_PRESS)            
            m_isWireframe = !m_isWireframe;

        // we keep trace of the pressed keys
        // with this method, we can manage 2 keys pressed at the same time:
        // many I/O managers often consider only 1 key pressed at the time (the first pressed, until it is released)
        // using a boolean array, we can then check and manage all the keys pressed at the same time
        if(state == GLFW_PRESS)
            keys[key] = true;
        else if(state == GLFW_RELEASE)
            keys[key] = false;

    }

    void MouseCB(int button, int action, int xpos, int ypos){
        // we move the camera view following the mouse cursor
        // we calculate the offset of the mouse cursor from the position in the last frame
        // when rendering the first frame, we do not have a "previous state" for the mouse, so we set the previous state equal to the initial values (thus, the offset will be = 0)
        if(m_firstMouse) //TODO: think abt a better way to manage this situation, maybe with a method in the Camera class
        {
            m_lastX = xpos;
            m_lastY = ypos;
            m_firstMouse = false;
        }

        // offset of mouse cursor position
        GLfloat xoffset = xpos - m_lastX;
        GLfloat yoffset = m_lastY - ypos; // we invert the y offset because we want that a movement of the mouse upwards corresponds to a positive y offset, and vice versa

        // the new position will be the previous one for the next frame
        m_lastX = xpos;
        m_lastY = ypos;

        // we pass the offset to the Camera class instance in order to update the rendering
        m_cam->ProcessMouseMovement(xoffset, yoffset);

    }


};


Render* render = new Render();

//////////////////////////////////////////
// callback for keyboard events
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    render->KeyboardCB(key, action);
}

//////////////////////////////////////////
// callback for mouse events
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    render->MouseCB(0, 0, xpos, ypos);
}


int main()
{
    if(render->Init() != 0)
        return -1;

    render->Run();

    delete render;
    return 0;
}
