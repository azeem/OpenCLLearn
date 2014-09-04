#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <time.h>

#include <CL/cl.h>

int min(int a, int b) {
  if(a < b) {
    return a;
  } else {
    return b;
  }
}

int max(int a, int b) {
  if(a > b) {
    return a;
  } else {
    return b;
  }
}

void error(const char *message) {
  fprintf(stderr, "ERROR: %s\n", message);
  exit(0);
}

void clError(int assertion, cl_int errCode, const char *msg) {
    char message[200];
    const char *errDesc;
    switch (errCode) {
        case CL_SUCCESS:                            errDesc = "Success!"; break;
        case CL_DEVICE_NOT_FOUND:                   errDesc = "Device not found."; break;
        case CL_DEVICE_NOT_AVAILABLE:               errDesc = "Device not available"; break;
        case CL_COMPILER_NOT_AVAILABLE:             errDesc = "Compiler not available"; break;
        case CL_MEM_OBJECT_ALLOCATION_FAILURE:      errDesc = "Memory object allocation failure"; break;
        case CL_OUT_OF_RESOURCES:                   errDesc = "Out of resources"; break;
        case CL_OUT_OF_HOST_MEMORY:                 errDesc = "Out of host memory"; break;
        case CL_PROFILING_INFO_NOT_AVAILABLE:       errDesc = "Profiling information not available"; break;
        case CL_MEM_COPY_OVERLAP:                   errDesc = "Memory copy overlap"; break;
        case CL_IMAGE_FORMAT_MISMATCH:              errDesc = "Image format mismatch"; break;
        case CL_IMAGE_FORMAT_NOT_SUPPORTED:         errDesc = "Image format not supported"; break;
        case CL_BUILD_PROGRAM_FAILURE:              errDesc = "Program build failure"; break;
        case CL_MAP_FAILURE:                        errDesc = "Map failure"; break;
        case CL_INVALID_VALUE:                      errDesc = "Invalid value"; break;
        case CL_INVALID_DEVICE_TYPE:                errDesc = "Invalid device type"; break;
        case CL_INVALID_PLATFORM:                   errDesc = "Invalid platform"; break;
        case CL_INVALID_DEVICE:                     errDesc = "Invalid device"; break;
        case CL_INVALID_CONTEXT:                    errDesc = "Invalid context"; break;
        case CL_INVALID_QUEUE_PROPERTIES:           errDesc = "Invalid queue properties"; break;
        case CL_INVALID_COMMAND_QUEUE:              errDesc = "Invalid command queue"; break;
        case CL_INVALID_HOST_PTR:                   errDesc = "Invalid host pointer"; break;
        case CL_INVALID_MEM_OBJECT:                 errDesc = "Invalid memory object"; break;
        case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR:    errDesc = "Invalid image format descriptor"; break;
        case CL_INVALID_IMAGE_SIZE:                 errDesc = "Invalid image size"; break;
        case CL_INVALID_SAMPLER:                    errDesc = "Invalid sampler"; break;
        case CL_INVALID_BINARY:                     errDesc = "Invalid binary"; break;
        case CL_INVALID_BUILD_OPTIONS:              errDesc = "Invalid build options"; break;
        case CL_INVALID_PROGRAM:                    errDesc = "Invalid program"; break;
        case CL_INVALID_PROGRAM_EXECUTABLE:         errDesc = "Invalid program executable"; break;
        case CL_INVALID_KERNEL_NAME:                errDesc = "Invalid kernel name"; break;
        case CL_INVALID_KERNEL_DEFINITION:          errDesc = "Invalid kernel definition"; break;
        case CL_INVALID_KERNEL:                     errDesc = "Invalid kernel"; break;
        case CL_INVALID_ARG_INDEX:                  errDesc = "Invalid argument index"; break;
        case CL_INVALID_ARG_VALUE:                  errDesc = "Invalid argument value"; break;
        case CL_INVALID_ARG_SIZE:                   errDesc = "Invalid argument size"; break;
        case CL_INVALID_KERNEL_ARGS:                errDesc = "Invalid kernel arguments"; break;
        case CL_INVALID_WORK_DIMENSION:             errDesc = "Invalid work dimension"; break;
        case CL_INVALID_WORK_GROUP_SIZE:            errDesc = "Invalid work group size"; break;
        case CL_INVALID_WORK_ITEM_SIZE:             errDesc = "Invalid work item size"; break;
        case CL_INVALID_GLOBAL_OFFSET:              errDesc = "Invalid global offset"; break;
        case CL_INVALID_EVENT_WAIT_LIST:            errDesc = "Invalid event wait list"; break;
        case CL_INVALID_EVENT:                      errDesc = "Invalid event"; break;
        case CL_INVALID_OPERATION:                  errDesc = "Invalid operation"; break;
        case CL_INVALID_GL_OBJECT:                  errDesc = "Invalid OpenGL object"; break;
        case CL_INVALID_BUFFER_SIZE:                errDesc = "Invalid buffer size"; break;
        case CL_INVALID_MIP_LEVEL:                  errDesc = "Invalid mip-map level"; break;
        default: errDesc = "Unknown"; break;
    }

    if(errCode != CL_SUCCESS || !assertion) {
      sprintf(message, "%s: Code: %d (%s)", msg, errCode, errDesc);
      error(message);
    }
}

cl_program buildProgramWithFile(cl_context context,
                                cl_uint numDevices,
                                cl_device_id *deviceList,
                                const char *fileName) {
  cl_int errCode;

  // read file source
  struct stat st;
  stat(fileName, &st);
  size_t fileSize = st.st_size;
  if(fileSize == 0) {
    error("Empty source file");
  }
  char *buf = (char *)malloc(fileSize);
  FILE *file = fopen(fileName, "r");
  fread(buf, fileSize, 1, file);
  fclose(file);

  const char *srcStrings[] = {buf};
  cl_program program = clCreateProgramWithSource(context, 1, srcStrings, &fileSize, &errCode);
  clError(true, errCode, "Unable to create program");

  errCode = clBuildProgram(program, numDevices, deviceList, "", NULL, NULL);

  if(errCode != CL_SUCCESS) {
    char buffer[1024];
    size_t bufferSize;
    fprintf(stderr, "OpenCL Build Error\n------------------\n");
    for(cl_uint i = 0;i < numDevices;i++) {
      clGetProgramBuildInfo(program, deviceList[i], (cl_program_build_info)CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &bufferSize);
      buffer[bufferSize] = '\0';
      fprintf(stderr, "%s\n", buffer);
    }
    exit(0);
  }

  free(buf);
  return program;
}
