#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <time.h>
#include <math.h>

#include <CL/cl.h>
#include "util.h"


int main() {
  cl_int errCode;
  srand(time(NULL));

  // setup data
  int dataLength = 1024*2;
  float *data = (float*)malloc(sizeof(float) * dataLength);
  for(int i = 0;i < dataLength;i++) {
    data[i] = rand() / (float)RAND_MAX;
  }

  // get the platform and device
  cl_platform_id platform;
  cl_uint platformsCount;
  errCode = clGetPlatformIDs(1, &platform, &platformsCount);
  clError(platformsCount > 0, errCode, "Could not get platforms");
  cl_device_id device;
  cl_uint devicesCount;
  errCode = clGetDeviceIDs((cl_platform_id)platform, CL_DEVICE_TYPE_DEFAULT, 1, &device, &devicesCount);
  clError(devicesCount > 0, errCode, "Could not get device");

  // setup the kernel and queue
  cl_context_properties props[] = {
    CL_CONTEXT_PLATFORM, (cl_context_properties)platform,
    0
  };
  cl_context context = clCreateContext(props, 1, &device, NULL, NULL, NULL);
  cl_program program = buildProgramWithFile(context, 1, &device, "findSum.cl");
  cl_kernel kernel = clCreateKernel(program, "findSum", NULL);
  cl_command_queue cqueue = clCreateCommandQueue(context, device, 0, &errCode);

  // find the work group size
  size_t localSize = 0;
  clGetKernelWorkGroupInfo(kernel, device, (cl_kernel_work_group_info)CL_KERNEL_WORK_GROUP_SIZE, sizeof(size_t), &localSize, NULL);

  // create the buffers
  size_t globalSize = (dataLength/2);
  size_t buffSize = sizeof(float) * globalSize;
  cl_mem input1 = clCreateBuffer(context, CL_MEM_READ_ONLY, buffSize, NULL, NULL);
  cl_mem input2 = clCreateBuffer(context, CL_MEM_READ_ONLY, buffSize, NULL, NULL);
  cl_mem output = clCreateBuffer(context, CL_MEM_WRITE_ONLY, buffSize, NULL, NULL);

  // copy data into buffers
  clEnqueueWriteBuffer(cqueue, input1, CL_TRUE, 0, buffSize, data, 0, NULL, NULL);
  clEnqueueWriteBuffer(cqueue, input2, CL_TRUE, 0, buffSize, data+globalSize, 0, NULL, NULL);

  //set kernel arguments
  clSetKernelArg(kernel, 0, sizeof(cl_mem), &input1);
  clSetKernelArg(kernel, 1, sizeof(cl_mem), &input2);
  clSetKernelArg(kernel, 2, sizeof(cl_mem), &output);

  // enqueue tasks
  cl_event event;
  size_t stepGlobalSize = globalSize;
  size_t stepBuffSize = buffSize;
  // here we enqueue a bunch of kernel tasks followed by
  // buffer copies. each time we copy each half of the output buffer
  // to each of the input buffer and then queue another kernel
  // execution. This is repeated till we end up with 1 value
  for(int i = 0;i < (log2(globalSize)+1);i++) {
    // global size should be divisible by local size so
    // we flatline the step size once it reaches localSize
    // and ignore the junk additions at the end
    stepGlobalSize = max(stepGlobalSize/2, localSize);
    stepBuffSize /= 2;

    errCode = clEnqueueNDRangeKernel(cqueue, kernel, 1, NULL, &stepGlobalSize, &localSize, i==0?0:1, i == 0?NULL:&event, &event);
    clError(true, errCode, "Could not queue kernel");
    clEnqueueCopyBuffer(cqueue, output, input1, 0, 0, stepBuffSize, 1, &event, &event);
    clEnqueueCopyBuffer(cqueue, output, input2, stepBuffSize, 0, stepBuffSize, 1, &event, &event);
  }

  clFinish(cqueue);
  float result;
  clEnqueueReadBuffer(cqueue, output, CL_TRUE, 0, sizeof(float), &result, 0, NULL, NULL);

  //verify the result
  float sum;
  for(int i = 0;i < dataLength;i++) {
    sum += data[i];
  }
  if((int)(sum*100) != (int)(result*100)) { // compare upto two decimal places
    printf("Result mismatch  %f != %f\n", sum, result);
  } else {
    printf("OpenCL Result %f, Normal Result %f\n", result, sum);
  }

  free(data);
  clReleaseCommandQueue(cqueue);
  clReleaseKernel(kernel);
  clReleaseProgram(program);
  clReleaseContext(context);

  return 0;
}


















