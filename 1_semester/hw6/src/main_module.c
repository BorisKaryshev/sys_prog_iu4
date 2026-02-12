#include "main.h"

#include <stdio.h>
#include <unistd.h>

return_code_t_e run(pass_manager_options_t opts) {
    if (opts.container_to_use == LIST) {
        return OK;
    }

    return OTHER_ERROR;
}
