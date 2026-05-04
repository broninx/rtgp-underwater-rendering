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

// static int g_seed;

enum SceneObj{ CUBE, CATFISH, STONE, BOAT}; 
enum ShaderType{ TERRAIN, GENERAL, GENERALONE, SKYBOX };
enum Textures {SANDTERRAIN,CATFISHTXT, STONETXT, BOATTXT, CAUSTICTXT};
// callback functions for keyboard and mouse events
class Render
{
private:
    enum Models {FISHMOD, STONEMOD, BOATMOD};
    struct ModelTransform {
        std::vector<glm::vec3> worldPos;
        std::vector<glm::vec3> worldScale;
        std::vector<float> angleRotation;
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

    GLuint m_fishVBO;

    

    void CreateWindow(){
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        // we set if the window is resizable
        glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

        // we create the application's window
        window = glfwCreateWindow(SCR_WIDHT, SCR_HEIGHT, "RTGP_lecture05", nullptr, nullptr);
        
    }

    void InitCallbacks(){
        // we put in relation the window and the callbacks
        glfwSetKeyCallback(window, key_callback);
        glfwSetCursorPosCallback(window, mouse_callback);

        // we disable the mouse cursor
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    }
    void InitShaders(){
        m_shaders.push_back(Shader("shaders/terrain.vert", "shaders/terrain.frag"));
        m_shaders.push_back(Shader("shaders/general.vert", "shaders/general.frag"));
        m_shaders.push_back(Shader("shaders/generalOne.vert", "shaders/general.frag"));
        m_shaders.push_back(Shader("shaders/skybox.vert", "shaders/skybox.frag"));
    }

    void InitModels(){

        // we load the model(s) (code of Model class is in include/utils/model.h)
        m_models.push_back(Model("models/cube.obj")); // used for the enviroment map
        m_models.push_back(Model("models/catfish_obj/catfishRawModel.obj"));
        m_models.push_back(Model("models/stone.OBJ"));
        m_models.push_back(Model("models/boat.obj"));
       
    }

    void InitTextures(){
        m_textures.push_back(Texture(GL_TEXTURE_2D, "textures/sand_white.png"));
        m_textures[SANDTERRAIN].Load();
        m_textures.push_back(Texture(GL_TEXTURE_2D, "textures/catfish.png"));
        m_textures[CATFISHTXT].Load();
        m_textures.push_back(Texture(GL_TEXTURE_2D, "textures/stone_tex/stone_diffuse.png"));
        m_textures[STONETXT].Load();
        m_textures.push_back(Texture(GL_TEXTURE_2D, "textures/Wood_Cherry_Original.jpg"));
        m_textures[BOATTXT].Load();
        m_textures.push_back(Texture(GL_TEXTURE_2D, "textures/Caustic_Free.jpg"));
        m_textures[CAUSTICTXT].Load();
    }

    void InitTerrain(){
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
        const int numDiv = 60;
        spreadXYZnt(m_fish.worldPos, spreadRad, FISH_NUM, numDiv);
        for(int i = 0; i < FISH_NUM; i ++)
        {
            m_fish.worldScale.push_back(glm::vec3(randomFloatRange(0.03f, 0.09f)));
        }

        // initialize positions of the stones

        const float terrSizef = (float) TERRAIN_SIZE; 
        float randx, randy, randz;
        for(int i = 0; i < STONE_NUM; i ++)
        {
            randx = randomFloatRange(terrSizef / 4.0f - 30.0f, (terrSizef - (terrSizef / 4.0f) + 30.0f));
            randz = randomFloatRange(terrSizef / 4.0f - 30.0f, (terrSizef - (terrSizef / 4.0f) + 30.0f));
            randy = m_terrain.GetHeight(randx, randz);
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
    Render(){
    }

    virtual ~Render(){
       SAFE_DELETE(m_cam);
       glfwTerminate();
    }

    int Init(){

        // we create a camera. We pass the initial position as a parameter to the constructor. 
        //The last boolean tells that we want a camera "anchored" to the ground
        m_cam = new Camera(glm::vec3(STARTING_X, STARTING_Y, STARTING_Z));
        
        glfwInit();

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
       

        m_models[CATFISH].SetInstanced();
        m_models[STONE].SetInstanced();
        m_models[BOAT].SetInstanced();


        InitTerrain();

        InitObjWorldPos();

        InitTextures();

        glFrontFace(GL_CW);
        glEnable(GL_DEPTH_TEST); //TODO: move this in the main funcion
        glClearColor(0.26f, 0.46f, 0.98f, 1.0f); //TODO: move this in the main funcion
        return 0;
    }

    void Run(){
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

    void RenderScene(){


        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        /////////// SUNLIGHT DIRECTION /////////////////////////

        static float horizontalAngle = 0.0f;
        static float verticalAngle = 0.0f;

        horizontalAngle >= 360.0f ? horizontalAngle = 0.002f : horizontalAngle += 0.002f;
        verticalAngle  >= 360.0f ? verticalAngle  = 0.002f : verticalAngle += 0.002f;

        float azimuth = glm::radians(horizontalAngle);
        float elevation = glm::radians(verticalAngle);

        m_sunDir.x = cos(elevation) * sin(azimuth);
        m_sunDir.y = sin(elevation);
        m_sunDir.z = cos(elevation) * cos(azimuth);

        GLuint prog;

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

        // m_shaders[GENERALONE].Use();
        // prog = m_shaders[GENERALONE].Program;

        // CATFISHES //

        m_shaders[GENERAL].Use();
        prog = m_shaders[GENERAL].Program;

        //texture
        m_textures[CATFISHTXT].Bind(GL_TEXTURE0);
        glUniform1i(glGetUniformLocation(prog, "gTexture"), 0);

        SetUniforms(prog, lightVDir, topColor, botColor, dayPhase);

        std::vector<glm::mat4> fishModels(FISH_NUM);
        for (int i = 0; i < FISH_NUM; i++)
        {
            glm::mat4 mat = glm::mat4(1.0f);
            mat = glm::translate(mat, m_fish.worldPos[i]); // spread out along X
            mat = glm::scale(mat, m_fish.worldScale[i]); // spread out along X

            fishModels[i] = mat;
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
