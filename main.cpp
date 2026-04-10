#define CL_TARGET_OPENCL_VERSION 300

#include"fileReader.h"
#include <stdio.h>
#include <iostream>
#include <CL/cl.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <math.h>
#include "SHADER.h"

const int WIDTH = 800;
const int HEIGHT = 600;

cl_platform_id platform_id;
cl_device_id device_id;

cl_context context;
cl_command_queue commands;
cl_program program;

GLFWwindow* window;

unsigned int VBO;

const char *vertexShaderText;
unsigned int vertexShader;
const char *fragmentShaderText;
unsigned int fragmentShader;

unsigned int shaderProgram;


int err;

int success;
char infoLog[512];


void framebuffer_size_callback(GLFWwindow* window, int width, int height);

int main(){        
    //platform binding
    err = clGetPlatformIDs(1, &platform_id, NULL);
    err = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_GPU, 1, &device_id, NULL);
    if(err != CL_SUCCESS)
    {
        printf("Error: Failed to create a device group!\n");
        return 1;
    }
    //compute context
    context = clCreateContext(0,1, &device_id, NULL, NULL, &err);
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


    //Initialize OpenGL
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    window = glfwCreateWindow(WIDTH, HEIGHT, "Fluid Dynamics Simulation", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "ERROR:Failed to create GLFW window!\n" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "ERROR: Failed to initialize GLAD\n" << std::endl;   
    }

    glViewport(0,0,800,600);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    float vertices[] = {
        -0.5f, -0.5f, 0.0f,
        0.5f, -0.5f, 0.0f,
        0.0f, 0.5f, 0.0f
    };

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    vertexShaderText = readFile("C:/Users/somay/Desktop/OpenGL Project/vertexShader.txt").c_str();
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderText, NULL);
    glCompileShader(vertexShader);

  
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR: Failed to compile vertex shader!\n" << infoLog << std::endl;
    }

    fragmentShaderText = readFile("C:/Users/somay/Desktop/OpenGL Project/fragmentShader.txt").c_str();
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderText, NULL);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR: Failed to compile vertex shader!\n" << infoLog << std::endl;
    }

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glUseProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*) 0);
    glEnableVertexAttribArray(0);


    while(!glfwWindowShouldClose(window))
    {
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
    
    
        glfwSwapBuffers(window);
        glfwPollEvents();
    }






    



    return 0;
}


void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0,0, width, height);
}


