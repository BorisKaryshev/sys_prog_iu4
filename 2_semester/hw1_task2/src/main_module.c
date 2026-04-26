#include "main.h"
#include "named_pipe.h"
#include "unnamed_pipe.h"

#include <stdlib.h>

return_code_t_e run(options_t opts) {
    if (opts.input_pipe_path == NULL || opts.output_pipe_path == NULL) {
        return run_unnamed_pipes();
    }
    return run_named_pipe(opts);
}
