#define CL_TARGET_OPENCL_VERSION 120
#include <CL/cl.h>
#include <stdio.h>
#include <stdlib.h>

#define SIZE 3

int main(void) {
    cl_platform_id platform = NULL;
    cl_device_id device = NULL;
    cl_context context = NULL;
    cl_command_queue queue = NULL;
    cl_program program = NULL;
    cl_kernel kernel = NULL;
    cl_mem buffer_in = NULL;
    cl_mem buffer_out = NULL;
    cl_int err = CL_SUCCESS;
    float in[SIZE] = {9, 25, 64};
    float out[SIZE] = {0, 0, 0};
    size_t count = SIZE;

    err = clGetPlatformIDs(1, &platform, NULL);
    if (err != CL_SUCCESS) {
        fprintf(stderr, "OpenCL platform not found\n");
        return 1;
    }

    err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);
    if (err != CL_SUCCESS) {
        err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 1, &device, NULL);
    }
    if (err != CL_SUCCESS) {
        fprintf(stderr, "OpenCL device not found\n");
        return 1;
    }

    context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
    if (err != CL_SUCCESS) {
        fprintf(stderr, "Failed to create OpenCL context\n");
        return 1;
    }

    queue = clCreateCommandQueue(context, device, 0, &err);
    if (err != CL_SUCCESS) {
        fprintf(stderr, "Failed to create OpenCL command queue\n");
        clReleaseContext(context);
        return 1;
    }

    FILE *f = fopen("kernel.cl", "rb");
    if (f == NULL) {
        fprintf(stderr, "Failed to open kernel.cl\n");
        clReleaseCommandQueue(queue);
        clReleaseContext(context);
        return 1;
    }

    fseek(f, 0, SEEK_END);
    long length = ftell(f);
    rewind(f);
    if (length <= 0) {
        fprintf(stderr, "Failed to read kernel.cl\n");
        fclose(f);
        clReleaseCommandQueue(queue);
        clReleaseContext(context);
        return 1;
    }

    char *source = (char *)malloc((size_t)length + 1);
    if (source == NULL) {
        fclose(f);
        clReleaseCommandQueue(queue);
        clReleaseContext(context);
        return 1;
    }
    size_t source_size = fread(source, 1, (size_t)length, f);
    fclose(f);
    source[source_size] = '\0';

    program = clCreateProgramWithSource(context, 1, (const char **)&source,
                                        &source_size, &err);
    free(source);
    if (err != CL_SUCCESS) {
        fprintf(stderr, "Failed to create OpenCL program\n");
        clReleaseCommandQueue(queue);
        clReleaseContext(context);
        return 1;
    }

    err = clBuildProgram(program, 1, &device, NULL, NULL, NULL);
    if (err != CL_SUCCESS) {
        char log[4096] = {0};
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG,
                              sizeof(log), log, NULL);
        fprintf(stderr, "Kernel compilation failed:\n%s\n", log);
        clReleaseProgram(program);
        clReleaseCommandQueue(queue);
        clReleaseContext(context);
        return 1;
    }

    buffer_in = clCreateBuffer(context, CL_MEM_READ_ONLY,
                               sizeof(float) * SIZE, NULL, &err);
    buffer_out = clCreateBuffer(context, CL_MEM_WRITE_ONLY,
                                sizeof(float) * SIZE, NULL, &err);
    if (err != CL_SUCCESS) {
        fprintf(stderr, "Failed to create OpenCL buffers\n");
        clReleaseProgram(program);
        clReleaseCommandQueue(queue);
        clReleaseContext(context);
        return 1;
    }

    err = clEnqueueWriteBuffer(queue, buffer_in, CL_TRUE, 0,
                               sizeof(float) * SIZE, in, 0, NULL, NULL);
    kernel = clCreateKernel(program, "my_sqrt", &err);
    if (err != CL_SUCCESS) {
        fprintf(stderr, "Failed to create OpenCL kernel\n");
        clReleaseMemObject(buffer_in);
        clReleaseMemObject(buffer_out);
        clReleaseProgram(program);
        clReleaseCommandQueue(queue);
        clReleaseContext(context);
        return 1;
    }

    clSetKernelArg(kernel, 0, sizeof(cl_mem), &buffer_in);
    clSetKernelArg(kernel, 1, sizeof(cl_mem), &buffer_out);
    err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &count,
                                 NULL, 0, NULL, NULL);
    if (err == CL_SUCCESS) {
        clFinish(queue);
        err = clEnqueueReadBuffer(queue, buffer_out, CL_TRUE, 0,
                                  sizeof(float) * SIZE, out, 0, NULL, NULL);
    }
    if (err != CL_SUCCESS) {
        fprintf(stderr, "OpenCL execution failed\n");
    } else {
        for (int i = 0; i < SIZE; i++) {
            printf("%f ", out[i]);
        }
        printf("\n");
    }

    clReleaseMemObject(buffer_in);
    clReleaseMemObject(buffer_out);
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);
    return err == CL_SUCCESS ? 0 : 1;
}
