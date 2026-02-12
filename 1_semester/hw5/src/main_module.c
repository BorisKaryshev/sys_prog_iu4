#include "main.h"

#include <stdio.h>
#include <unistd.h>

return_code_t_e run_create(const char* path) {
    if (access(path, F_OK) == 0) {
        return CREATE_FILE_EXISTS;
    }

    FILE* fd = fopen(path, "w");
    if (!fd) {
        return OTHER_ERROR;
    }

    fclose(fd);
    return OK;
}

return_code_t_e run_delete(const char* path) {
    if (access(path, F_OK) != 0) {
        return DELETE_NOT_FOUND;
    }

    if (remove(path)) {
        return OTHER_ERROR;
    }
    return OK;
}

return_code_t_e run(operation_t_e operation, const char* path) {
    switch (operation) {
        case CREATE_FILE:
            return run_create(path);
        case DELETE_FILE:
            return run_delete(path);
        case DEFAULT_OPERATION:
            return UNEXPECTED_OPERATION;
    }

    return OTHER_ERROR;
}
