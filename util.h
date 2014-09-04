#ifndef  UTIL_H_INCLUDE
#define UTIL_H_INCLUDE

int max(int, int);
int min(int, int);
void error(const char *message);
void clError(int assertion, cl_int errCode, const char *msg);
cl_program buildProgramWithFile(cl_context context,
                                cl_uint numDevices,
                                cl_device_id *deviceList,
                                const char *fileName);

#endif
