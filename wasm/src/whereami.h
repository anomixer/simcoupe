#ifndef WHEREAMI_H
#define WHEREAMI_H

#ifdef __cplusplus
extern "C" {
#endif

int wai_getExecutablePath(char* out, int capacity, int* dirname_length);
int wai_getModulePath(char* out, int capacity, int* dirname_length);

#ifdef __cplusplus
}
#endif

#endif
