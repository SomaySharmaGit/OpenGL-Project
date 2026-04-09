#define CL_TARGET_OPENCL_VERSION 300

#include"fileReader.h"
#include <stdio.h>
#include <iostream>
#include <CL/cl.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <math.h>
#include "SHADER.h"

int main(){

    int err;

    cl_platform_id platform_id;
    cl_device_id device_id;

    cl_context context;
    cl_command_queue commands;
    cl_program program;
        
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
    commands = clCreateCommandQueue(context, device_id, 0, &err);
    if(!commands)
    {
        printf("Error: Failed to create command queue!\n");
        return 1;
    }

    
    



    return 0;
}
