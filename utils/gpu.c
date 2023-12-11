#include "gpu.h"
#include <CL/cl.h>

void gpu_run_kernel(gpu_context_t * context, size_t work_dim, const size_t * global_item_size, const size_t * local_item_size) {
    clEnqueueNDRangeKernel(context->command_queue, context->kernel, work_dim, NULL,
                           global_item_size, local_item_size, 0, NULL, NULL);
}

gpu_context_t gpu_load_kernel(char const * kernel_path, char const * kernel_name) {
    // Get platform and device information
    cl_platform_id platform_id = NULL;
    cl_device_id device_id = NULL;
    cl_uint ret_num_devices;
    cl_uint ret_num_platforms;
    cl_int ret = clGetPlatformIDs(1, &platform_id, &ret_num_platforms);
    ret = clGetDeviceIDs( platform_id, CL_DEVICE_TYPE_DEFAULT, 1,
            &device_id, &ret_num_devices);

    // Load the kernel source code into the array source_str
    FILE *fp;
    char *source_str;
    size_t source_size;

    fp = fopen(kernel_path, "r");
    if (!fp) {
        fprintf(stderr, "Failed to load kernel.\n");
        exit(1);
    }
    source_str = (char*)malloc(MAX_SOURCE_SIZE);
    source_size = fread( source_str, 1, MAX_SOURCE_SIZE, fp);
    fclose( fp );


    gpu_context_t gpu_context;
    gpu_context.context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &ret);
    gpu_context.command_queue = clCreateCommandQueueWithProperties(gpu_context.context, device_id, NULL, &ret);
    gpu_context.program = clCreateProgramWithSource(gpu_context.context, 1, (const char **)&source_str, (const size_t *)&source_size, &ret);
    gpu_context.kernel = clCreateKernel(gpu_context.program, kernel_name, &ret);
    gpu_context.num_buffers = 0;

     // Build the program
    ret = clBuildProgram(gpu_context.program, 1, &device_id, NULL, NULL, NULL);

    free(source_str);

    return gpu_context;
}

unsigned int gpu_allocate_args(gpu_context_t * context, size_t size, cl_mem_flags mem_type) {
    cl_int ret;
    unsigned int const num_buf = context->num_buffers;
    context->buffers[num_buf] = clCreateBuffer(context->context, CL_MEM_READ_ONLY, size, NULL, &ret);
    ret = clSetKernelArg(context->kernel, num_buf, sizeof(cl_mem), (void *)context->buffers + num_buf);
    return context->num_buffers++;
}

void gpu_set_args(gpu_context_t * context, unsigned int buffer_idx, size_t size, void * ptr) {
    cl_int ret = clEnqueueWriteBuffer(context->command_queue, context->buffers[buffer_idx],
                                      CL_TRUE, 0, size, ptr, 0, NULL, NULL);
}

void gpu_get_args(gpu_context_t * context, unsigned int buffer_idx, size_t size, void * ptr) {
    cl_int ret = clEnqueueReadBuffer(context->command_queue, context->buffers[buffer_idx],
                                      CL_TRUE, 0, size, ptr, 0, NULL, NULL);
}

void gpu_flush(gpu_context_t * context) {
    cl_int ret;
    ret = clFlush(context->command_queue);
    ret = clFinish(context->command_queue);
}

void gpu_destroy_context(gpu_context_t * context) {
    cl_int ret;
    ret = clReleaseKernel(context->kernel);
    ret = clReleaseProgram(context->program);
    for (unsigned int i = 0; i < context->num_buffers; i++) {
        ret = clReleaseMemObject(context->buffers[i]);
    }
    ret = clReleaseCommandQueue(context->command_queue);
    ret = clReleaseContext(context->context);
}
