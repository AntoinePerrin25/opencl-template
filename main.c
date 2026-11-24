#define CL_TARGET_OPENCL_VERSION 120
#include <CL/cl.h>
#include <stdio.h>
#include <stddef.h>
#define SIZE 3

int main() {
    cl_device_id gpu;
    cl_context context;
    cl_int err;
    cl_command_queue queue;
    cl_program program;
    cl_kernel kernel;
    FILE *f;
    char kernel_code[1024] = {0};
    size_t count=SIZE;
    float in[SIZE] = {9, 25, 64};
    float out[SIZE] = {0, 0, 0};
    cl_mem buffer_in, buffer_out;
    cl_platform_id platform;
    err = clGetPlatformIDs(1, &platform, NULL);
    err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &gpu, NULL);
    context = clCreateContext(NULL, 1, &gpu, NULL, NULL, &err);
    queue = clCreateCommandQueue(context, gpu, 0, &err);
    fopen_s(&f, "kernel.cl", "r");
    if (f == NULL) {
        printf("Failed to open kernel.cl\n");
        return 1;
    }
    size_t kernel_size = fread(kernel_code, 1, sizeof(kernel_code) - 1, f);
    fclose(f);
    kernel_code[kernel_size] = '\0';
    const char *source = kernel_code;
    program = clCreateProgramWithSource(context, 1, &source, &kernel_size, &err);
    err = clBuildProgram(program, 1, &gpu, NULL, NULL, NULL);
    if (err != CL_SUCCESS) {
        char log[4096] = {0};
        clGetProgramBuildInfo(program, gpu, CL_PROGRAM_BUILD_LOG, sizeof(log), log, NULL);
        printf("Echec de compilation du kernel :\n%s\n", log);
        return 1;
    }
    buffer_in = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(float) * SIZE, NULL, &err);
    buffer_out = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(float) * SIZE, NULL, &err);
    clEnqueueWriteBuffer(queue, buffer_in, CL_TRUE, 0, sizeof(float) * SIZE, in, 0, NULL, NULL);
    kernel = clCreateKernel(program, "my_sqrt", &err);

    clSetKernelArg(kernel, 0, sizeof(cl_mem), &buffer_in);
    clSetKernelArg(kernel, 1, sizeof(cl_mem), &buffer_out);
    clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &count, NULL, 0, NULL, NULL);
    clFinish(queue);



    clEnqueueReadBuffer(queue, buffer_out, CL_TRUE, 0, 
                        sizeof(float) * SIZE, out, 0, NULL, NULL);
    for (int i = 0; i < SIZE; i++) {
        printf("%f ", out[i]);
    }

    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);
    return 0;
}