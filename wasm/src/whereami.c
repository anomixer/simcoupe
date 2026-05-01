#include <stdlib.h>
#include <string.h>

int wai_getExecutablePath(char* out, int capacity, int* dirname_length) {
    const char* dummy = "/simcoupe";
    int len = strlen(dummy);
    if (out) {
        strncpy(out, dummy, capacity);
    }
    if (dirname_length) {
        *dirname_length = 0;
    }
    return len;
}

int wai_getModulePath(char* out, int capacity, int* dirname_length) {
    return wai_getExecutablePath(out, capacity, dirname_length);
}
