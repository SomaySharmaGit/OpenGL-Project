#define CL_TARGET_OPENCL_VERSION 300

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


    err = clGetPlatformIDs(1, &platform_id, NULL);
    err = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_GPU, 1, &device_id, NULL);
    if(err != CL_SUCCESS)
    {
        printf("Error: Failed to create a device group!\n");
        return 1;
    }   
    
    size_t device_message_size;
    err = clGetDeviceInfo(device_id, CL_DEVICE_NAME, 0, NULL, &device_message_size);

    char *message = new char[device_message_size];
    err = clGetDeviceInfo(device_id, CL_DEVICE_NAME, device_message_size, message, NULL);

    printf(message);

    delete[] message;

    return 0;
}
