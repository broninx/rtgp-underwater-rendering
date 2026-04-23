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

// classes developed during lab lectures to manage shaders and to load models, and for FPS camera
#include <utils/shader.h>
#include <utils/model.h>
#include <utils/camera.h>


// we load the GLM classes used in the application
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "terrain.h"
#include "triangleList.h"

// include the library for the images loading
#include "tex_funs.h"
// number of lights in the scene
#define NR_LIGHTS 3
#define SAFE_DELETE(p) { if(p) { delete p; p = nullptr; } }
// dimensions of the window
#define SCR_WIDHT 1200
#define SCR_HEIGHT 900

// boolean to activate/deactivate wireframe rendering
GLboolean wireframe = GL_FALSE;

// callback functions for keyboard and mouse events
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);



// static int g_seed;

enum SceneObj{ CUBE, BUNNY, PLANE }; 
enum ShaderType{ NORMAL2COLOR, TERRAIN, SKYBOX, ILLUMINATION };

// callback functions for keyboard and mouse events
class Render
{
private:
    GLFWwindow* window = NULL;
    Camera* m_cam;
    GLboolean m_isWireframe = false;
    std::vector<Model> m_models;
    std::vector<Shader> m_shaders;
    glm::mat4 m_view = glm::mat4(1.0f);
    glm::mat4 m_projection = glm::mat4(1.0f);
    GLint m_lastX, m_lastY;
    GLboolean m_firstMouse = true;
    GLfloat m_deltaTime = 0.0f; // time between current frame and last frame
    GLfloat m_lastFrame = 0.0f; // time of last frame
    GLfloat m_currentFrame = 0.0f; // time of current frame
    Terrain m_terrain;
    int keys[1024];
    // TODO: std::vector<Texture> m_textures; is rlly there or in Model class?
   

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
        m_shaders.push_back(Shader("shaders/normal2color.vert", "shaders/normal2color.frag"));
        m_shaders.push_back(Shader("shaders/terrain.vert", "shaders/terrain.frag"));
        // we create a shader program used for the enviroment mapping
        // m_shaders.push_back(Shader("shaders/skybox.vert", "shaders/skybox.frag"));
        // we create the Shader Program used for objects (which presents different subroutines we can switch)
        // m_shaders.push_back(Shader("shaders/illumination.vert", "shaders/illumination.frag"));
    }

    void InitModels(){
        // we load the model(s) (code of Model class is in include/utils/model.h)
        m_models.push_back(Model("models/cube.obj")); // used for the enviroment map
        m_models.push_back(Model("models/bunny_lp.obj"));
        m_models.push_back(Model("models/plane.obj"));
    }

    void InitTerrain(){
        // we initialize the terrain
        float terrainScale = 0.2f;
        m_terrain.Init(terrainScale);
        #ifdef WIN32
            m_terrain.LoadFromFile("data\\heightmap.save");
        #else
            m_terrain.LoadFromFile("data/heightmap.save");
        #endif
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
        m_cam = new Camera(glm::vec3(0.0f, 0.0f, 7.0f), GL_FALSE);
        
        glfwInit();

        CreateWindow();
        if (!window)
        {
            std::cout << "Failed to create GLFW window" << std::endl;
            return -1;
        }
        glfwMakeContextCurrent(window);

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
        m_projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDHT / (float)SCR_HEIGHT, 0.1f, 100.0f);

        InitCallbacks();

        InitShaders();

        InitModels();

        InitTerrain();
        // TODO: InitTextures();

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
            m_lastFrame = m_currentFrame;

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
        /////////////////// OBJECTS ////////////////////////////////////////////////
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        //TERRAIN//
        m_shaders[TERRAIN].Use();
        glm::mat4 viewProj = m_projection * m_view;
        glUniformMatrix4fv(glGetUniformLocation(m_shaders[TERRAIN].Program, "gVP"), 1, GL_FALSE, glm::value_ptr(viewProj));
        m_terrain.Render();

        //BUNNY//
        // m_shaders[NORMAL2COLOR].Use();
        // glUniformMatrix4fv(glGetUniformLocation(m_shaders[NORMAL2COLOR].Program, "viewMatrix"), 1, GL_FALSE, glm::value_ptr(m_view));
        // glUniformMatrix4fv(glGetUniformLocation(m_shaders[NORMAL2COLOR].Program, "projectionMatrix"), 1, GL_FALSE, glm::value_ptr(m_projection));
        // //BUNNY
        // m_models[BUNNY].SetModelMatrix(glm::translate(glm::mat4(1.0f), glm::vec3(3.0f, 0.0f, 0.0f)));
        // m_models[BUNNY].SetModelMatrix(glm::scale(m_models[BUNNY].GetModelMatrix(), glm::vec3(0.5f, 0.5f, 0.5f)));
        // m_models[BUNNY].SetNormalMatrix(glm::inverseTranspose(glm::mat3(m_view*m_models[BUNNY].GetModelMatrix())));
        // glUniformMatrix4fv(glGetUniformLocation(m_shaders[NORMAL2COLOR].Program, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(m_models[BUNNY].GetModelMatrix()));
        // glUniformMatrix3fv(glGetUniformLocation(m_shaders[NORMAL2COLOR].Program, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(m_models[BUNNY].GetNormalMatrix()));
        // // we render the bunny
        // m_models[BUNNY].Draw();

        /////////////////// SKYBOX ////////////////////////////////////////////////

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
        
        if(keys[GLFW_KEY_W])
            m_cam->ProcessKeyboard(FORWARD, m_deltaTime);
        if(keys[GLFW_KEY_S])
            m_cam->ProcessKeyboard(BACKWARD, m_deltaTime);
        if(keys[GLFW_KEY_A])
            m_cam->ProcessKeyboard(LEFT, m_deltaTime);
        if(keys[GLFW_KEY_D])
            m_cam->ProcessKeyboard(RIGHT, m_deltaTime);
        if(keys[GLFW_KEY_SPACE])
            m_cam->ProcessKeyboard(UP, m_deltaTime);
        if(keys[GLFW_KEY_LEFT_CONTROL])
            m_cam->ProcessKeyboard(DOWN, m_deltaTime);
    }

    void KeyboardCB(uint key, int state){

        if (key == GLFW_KEY_ESCAPE && state == GLFW_PRESS)
            glfwSetWindowShouldClose(window, GL_TRUE);
        
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
