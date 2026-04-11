#define CL_TARGET_OPENCL_VERSION 300

#include"fileReader.h"
#include <stdio.h>
#include <iostream>
#include <CL/cl.h>
#include <CL/cl_gl.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <math.h>
#include "SHADER.h"
#include <windows.h>


const int WIDTH = 800;
const int HEIGHT = 600;

cl_platform_id platform_id;
cl_device_id device_id;

cl_context context;
cl_command_queue commands;
cl_program program;

cl_mem sharedMemory;

GLFWwindow* window;

unsigned int VBO;
unsigned int VAO;

const char *vertexShaderText;
unsigned int vertexShader;
const char *fragmentShaderText;
unsigned int fragmentShader;

unsigned int shaderProgram;


int err;

int success;
char infoLog[512];

std::string vSource;
std::string fSource;


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
int initOpenGL();
int initOpenCL();
int compileShaders();
void cleanUp();

int main(){        

    if(initOpenGL() != 0){
        std::cout << "ERROR: Failed to initialize OpenGL" << std::endl;
        return 1;
    }

    if(compileShaders() != 0) {
        std::cout << "ERROR: Failed to compile shaders" << std::endl;
        return 1;
    }

    if(initOpenCL()){
        std::cout << "ERROR: Failed to initialize OpenCL" << std::endl;
        return 1;
    }


    //Create Buffer for Vertex Data
    float vertices[] = {
        -0.5f, -0.5f, 0.0f,
        0.5f, -0.5f, 0.0f,
        0.0f, 0.5f, 0.0f
    };

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*) 0);
    glEnableVertexAttribArray(0);

    glFinish(); 
    

    sharedMemory = clCreateFromGLBuffer(context, CL_MEM_READ_WRITE, VBO, &err);
    if(err != CL_SUCCESS)
    {
        std::cout << "ERROR: Failed to create shared memory!" << std::endl;
    }

    while(!glfwWindowShouldClose(window))
    {
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
    
        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);


    
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    cleanUp();

    return 0;


    //clReleaseKernel
    //clReleaseProgram
 


    

}


void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0,0, width, height);
}

int initOpenGL()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(WIDTH, HEIGHT, "Fluid Dynamics Simulation", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "ERROR:Failed to create GLFW window!\n" << std::endl;
        glfwTerminate();
        return 1;
    }

     glfwMakeContextCurrent(window);

    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "ERROR: Failed to initialize GLAD\n" << std::endl;
        return 1;   
    }

    glViewport(0,0,800,600);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);


    return 0;
}

int initOpenCL()
{
     //platform binding
    err = clGetPlatformIDs(1, &platform_id, NULL);
    err = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_GPU, 1, &device_id, NULL);
    if(err != CL_SUCCESS)
    {
        printf("Error: Failed to create a device group!\n");
        return 1;
    }

    //compute context

    cl_context_properties properties[] = {
        CL_CONTEXT_PLATFORM, (cl_context_properties)platform_id,
        CL_GL_CONTEXT_KHR, (cl_context_properties)wglGetCurrentContext(), 
        CL_WGL_HDC_KHR, (cl_context_properties)wglGetCurrentDC(),
        0
    } ;

    context = clCreateContext(properties,1, &device_id, NULL, NULL, &err);
    if(!context)
    {
        printf("Error: Failed to create a context");
        return 1;
    }
    //command list
    commands = clCreateCommandQueueWithProperties(context, device_id, 0, &err);
    if(!commands)
    {
        printf("Error: Failed to create command queue!\n");
        return 1;
    }


    return 0;
}

int compileShaders()
{
    vSource = readFile("C:/Users/somay/Desktop/OpenGL Project/vertexShader.txt");
    vertexShaderText = vSource.c_str();
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderText, NULL);
    glCompileShader(vertexShader);

  
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR: Failed to compile vertex shader!\n" << infoLog << std::endl;
        return 1;
    }

    fSource = readFile("C:/Users/somay/Desktop/OpenGL Project/fragmentShader.txt");
    fragmentShaderText = fSource.c_str();
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderText, NULL);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR: Failed to compile vertex shader!\n" << infoLog << std::endl;
        return 1;
    }

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if(!success)
    {
        glGetShaderInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR: Failed to link program!\n";
        return 1;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);


    return 0;
}

void cleanUp()
{
    clReleaseMemObject(sharedMemory);
    clReleaseCommandQueue(commands);
    clReleaseContext(context);

    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);
    glDeleteProgram(shaderProgram);
    glfwTerminate();
}
