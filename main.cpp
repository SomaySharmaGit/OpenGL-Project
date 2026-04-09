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

int err;


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

    while(!glfwWindowShouldClose(window))
    {
        glfwSwapBuffers(window);
        glfwPollEvents();
    }



    



    return 0;
}


void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0,0, width, height);
}


