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


const int WIDTH = 600;
const int HEIGHT = 600;

cl_platform_id platform_id;
cl_device_id device_id;

cl_context context;
cl_command_queue commands;
cl_program program;
cl_kernel kernel;

cl_mem sharedMemory;
cl_mem density;
cl_mem forces; 
cl_mem velocities;

GLFWwindow* window;

const int limit = 8;
const int stride = 4 * limit;
const int lift = 2 * limit;
const int wallLength = 40;
const int floorLength = 4 * wallLength;
const int thickness = 12;
const int particles = stride * lift;
const int totalParticles = particles + (2 * wallLength * thickness) + (floorLength * thickness) + 8;


unsigned int VBO;
unsigned int VAO;

const char *vertexShaderText;
unsigned int vertexShader;
const char *fragmentShaderText;
unsigned int fragmentShader;
const char *kernelText;

unsigned int shaderProgram;


int err;

int success;
char infoLog[512];

std::string vSource;
std::string fSource;
std::string kernelSource;


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
int initOpenGL();
int initOpenCL();
int compileShaders();
void cleanUp();
int kernel1();
int kernel2();
int kernel3();
static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);

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

    cl_float4 vertices[totalParticles] = {};

    float sparsityX = 1.3f;
    float sparsityY = 0.4f;
    float xInterval = sparsityX/(float)(stride);
    float yInterval = sparsityY/(float)(lift);


    for(int i = 0; i < stride ; i++)
    {
        for(int j = 0; j < lift; j++)
        {
            int index = (i * lift) + j;
            float x = i * xInterval - (0.4 * sparsityX);
            float y = j * yInterval;
            vertices[index] = {x,y,1.0f,1.0f};
            
        }
    }


    
    for(int i = 0; i < thickness; i++)
    {
        for(int j = 0; j < wallLength; j++)
        {
            int offset = particles + (i * wallLength * 2);
            vertices[j + offset] = {-0.93f - (0.01f * (float)i) , (float)j/wallLength - 0.7f, 0.0f, 0.0f};
            vertices[j + offset + wallLength] = {0.93f + (0.03f * (float)i), (float)j/wallLength - 0.7f, 0.0f, 0.0f};
        }
    }


    for(int i = 0; i < thickness; i++)
    {
        for(int j = 0; j < floorLength; j++)
        {
            int offset = particles + (thickness * wallLength * 2) + (i * floorLength);
            vertices[j + offset] = {2.0f * (float)j/floorLength - 0.98f, -0.75f - ((0.01f) * (float)i), 0.0f, 0.0f};
        }
    }
    




    //Create Buffer for Vertex Data


    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*) 0);
    glEnableVertexAttribArray(0);
    glEnable(GL_PROGRAM_POINT_SIZE);


    glFinish(); 
    

    sharedMemory = clCreateFromGLBuffer(context, CL_MEM_READ_WRITE, VBO, &err);
    if(err != CL_SUCCESS)
    {
        std::cout << "ERROR: Failed to create shared memory\n!" << std::endl;
    }

    density = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(vertices)/3, NULL, NULL);
    if(!density)
    {
        std::cout << "ERROR: Failed to create density buffer!\n" << std::endl;
    }

    forces = clCreateBuffer(context, CL_MEM_READ_WRITE, totalParticles * sizeof(cl_float2), NULL, NULL);
    if(!forces)
    {
        std::cout << "ERROR: Failed to create forces buffer!\n" << std::endl;
    }
    
    velocities = clCreateBuffer(context, CL_MEM_READ_WRITE, totalParticles * sizeof(cl_float2), NULL, NULL);




    while(!glfwWindowShouldClose(window))
    {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        if(kernel1() != 0)
        {
            std::cout << "ERROR: Failed to execute kernel1!\n" << std::endl;
            return 1;
        }

        if(kernel2() != 0)
        {
            std::cout << "ERROR: Failed to execute kernel2!\n" << std::endl;
            return 1;
        }
        
        if(kernel3() != 0)
        {
            std::cout << "ERROR: Failed to execute kernel3!\n" << std::endl;
            return 1;
        }

        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);

       printf("X: %f Y: %f\n", xpos, ypos);

        float xConv = (float)xpos/300 - 1.0f;
        float yConv = (float)-ypos/300 + 1.0f;
        float position[4 * 8] = {};

        for(int k = 0; k < 8; k++)
        {
            position[k * 4] = xConv;
            position[k * 4 + 1] = yConv;
            position[k * 4 + 2] = 0.0f;
            position[k * 4 + 3] = 0.0f;
        }

        size_t fl = sizeof(cl_float4);



        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);
        glBufferSubData(GL_ARRAY_BUFFER, (totalParticles - 8) * fl, fl * 8, &position);
        glFinish();
        glDrawArrays(GL_POINTS, 0, totalParticles);


    
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    cl_float results[totalParticles];

    err = clEnqueueReadBuffer(commands, density, CL_TRUE, 0, sizeof(cl_float) * totalParticles, results, 0, NULL, NULL);
    if(err != CL_SUCCESS)
    {
        std::cout << "ERROR: Failed to read density!\n";
        return 1;
    }

    err = clEnqueueAcquireGLObjects(commands, 1, &sharedMemory, 0, NULL, NULL);
    if(err != CL_SUCCESS)
    {
        printf("ERROR: Failed to aquire GLObjects!\n");
        return 1;
    }    

    cl_float4 results2[totalParticles];

    err = clEnqueueReadBuffer(commands, sharedMemory, CL_TRUE, 0, sizeof(cl_float4) * totalParticles, results2, 0, NULL, NULL);

        if(err != CL_SUCCESS)
    {
        std::cout << "ERROR: Failed to read position!\n";
        return 1;
    }

    err = clEnqueueReleaseGLObjects(commands, 1, &sharedMemory, 0, NULL, NULL);
    if(err != CL_SUCCESS)
    {
        printf("ERROR: Failed to relase gl objects!\n");
        return 1;
    }

    clFinish(commands);

    for(int i = 0; i < totalParticles; i++)
    {
       //printf("Position: x: %f, y: %f, Density: %f\n", results2[i].s[0], results2[i].s[1], results[i]);

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

    glViewport(0,0,600,600);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);


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

    kernelSource = readFile("C:/Users/somay/Desktop/OpenGL Project/kernel.txt");
    kernelText = kernelSource.c_str();

    program = clCreateProgramWithSource(context, 1, (const char **) &kernelText, NULL, &err);
    if(!program)
    {
        printf("Error: Failed to create a program!\n");
    }

    err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
    if(err != CL_SUCCESS)
    {
        size_t len;
        char buffer[2048];

        printf("Error: Failed to builld program!\n");
        clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &len );
        printf("%s\n", buffer);
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
        std::cout << "ERROR: Failed to compile fragment shader!\n" << infoLog << std::endl;
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

int kernel1()
{

    kernel = clCreateKernel(program, "density", &err);
    if(!kernel || err != CL_SUCCESS)
    {
        printf("Error: Failed to create kernel!\n");
        return 1;
    }

    err = clEnqueueAcquireGLObjects(commands, 1, &sharedMemory, 0, NULL, NULL);
    if(err != CL_SUCCESS)
    {
        printf("ERROR: Failed to aquire GLObjects!\n");
        return 1;
    }


    err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &sharedMemory);
    err = clSetKernelArg(kernel, 1, sizeof(cl_mem), &density);
    err = clSetKernelArg(kernel, 2, sizeof(cl_int), &totalParticles);

    if(err != CL_SUCCESS)
    {
        printf("ERROR: Failed to set kernel arg!\n");
         return 1;
    }

    const size_t global = totalParticles;
    const size_t local = limit;


    err = clEnqueueNDRangeKernel(commands, kernel, 1, NULL, &global, &local, 0, NULL, NULL);
    if(err != CL_SUCCESS)
    {
        printf("ERROR: Failed to enqueue nd-range kernel!\n");
        return 1;
    }
    clFinish(commands);

    err = clEnqueueReleaseGLObjects(commands, 1, &sharedMemory, 0, NULL, NULL);
    if(err != CL_SUCCESS)
    {
        printf("ERROR: Failed to relase gl objects!\n");
        return 1;
    }

    return 0;
    
}

int kernel2()
{
    kernel = clCreateKernel(program, "pressure", &err);
    if(!kernel || err != CL_SUCCESS)
    {
        printf("Error: Failed to create kernel!\n");
        return 1;
    }

    err = clEnqueueAcquireGLObjects(commands, 1, &sharedMemory, 0, NULL, NULL);
    if(err != CL_SUCCESS)
    {
        printf("ERROR: Failed to aquire GLObjects!\n");
        return 1;
    }

    err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &sharedMemory);
    err = clSetKernelArg(kernel, 1, sizeof(cl_mem), &density);
    err = clSetKernelArg(kernel, 2, sizeof(cl_mem), &forces);
    err = clSetKernelArg(kernel, 3, sizeof(cl_mem), &velocities);
    err = clSetKernelArg(kernel, 4, sizeof(cl_int), &totalParticles);


    if(err != CL_SUCCESS)
    {
        printf("ERROR: Failed to set kernel arg!\n");
        return 1;
    }

    const size_t global = particles;
    const size_t local = limit;

    err = clEnqueueNDRangeKernel(commands, kernel, 1, NULL, &global, &local, 0, NULL, NULL);
    if(err != CL_SUCCESS)
    {
        printf("ERROR: Failed to enqueue nd-range kernel!\n");
        return 1;
    }
    clFinish(commands);

    err = clEnqueueReleaseGLObjects(commands, 1, &sharedMemory, 0, NULL, NULL);
    if(err != CL_SUCCESS)
    {
        printf("ERROR: Failed to relase gl objects!\n");
        return 1;
    }

    return 0;

}

int kernel3()
{
    kernel = clCreateKernel(program, "updatePositions", &err);
    if(!kernel || err != CL_SUCCESS)
    {
        printf("Error: Failed to create kernel!\n");
        return 1;
    }

    err = clEnqueueAcquireGLObjects(commands, 1, &sharedMemory, 0, NULL, NULL);
    if(err != CL_SUCCESS)
    {
        printf("ERROR: Failed to aquire GLObjects!\n");
        return 1;
    }

    err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &sharedMemory);
    err = clSetKernelArg(kernel, 1, sizeof(cl_mem), &forces);
    err = clSetKernelArg(kernel, 2, sizeof(cl_mem), &velocities);

    if(err != CL_SUCCESS)
    {
        printf("ERROR: Failed to set kernel arg!\n");
        return 1;
    }

    const size_t global = particles;
    const size_t local = limit;

    err = clEnqueueNDRangeKernel(commands, kernel, 1, NULL, &global, &local, 0, NULL, NULL);
    if(err != CL_SUCCESS)
    {
        printf("ERROR: Failed to enqueue nd-range kernel!\n");
        return 1;
    }
    clFinish(commands);

    err = clEnqueueReleaseGLObjects(commands, 1, &sharedMemory, 0, NULL, NULL);
    if(err != CL_SUCCESS)
    {
        printf("ERROR: Failed to relase gl objects!\n");
        return 1;
    }

    return 0;


}

static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
}