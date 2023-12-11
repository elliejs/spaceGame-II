#include <stdio.h>
#include <stdlib.h>

#define CL_TARGET_OPENCL_VERSION 300
#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

#define MAX_SHADER_BUFFERS 32
#define MAX_SOURCE_SIZE (0x100000)

typedef
struct gpu_context_s {
    cl_context context;
    cl_command_queue command_queue;
    cl_program program;
    cl_kernel kernel;
    cl_mem buffers[MAX_SHADER_BUFFERS];
    unsigned int num_buffers;
}
gpu_context_t;

gpu_context_t gpu_load_kernel(char const * kernel_path, char const * kernel_name);
void gpu_run_kernel(gpu_context_t * context, size_t work_dim, const size_t * global_item_size, const size_t * local_item_size);
#define gpu_allocate_args_RO(context, size) gpu_allocate_args((context), (size), CL_MEM_READ_ONLY)
#define gpu_allocate_args_WO(context, size) gpu_allocate_args((context), (size), CL_MEM_WRITE_ONLY)
unsigned int gpu_allocate_args(gpu_context_t * context, size_t size, cl_mem_flags mem_type);
void gpu_set_args(gpu_context_t * context, unsigned int buffer_idx, size_t size, void * ptr);
void gpu_get_args(gpu_context_t * context, unsigned int buffer_idx, size_t size, void * ptr);
void gpu_flush(gpu_context_t * context);
void gpu_destroy_context(gpu_context_t * context);
