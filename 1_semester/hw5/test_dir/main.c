#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>

static char *canonicalize_path(const char *path)
{
    if (!path)
        return NULL;
    size_t len = strlen(path);
    char *out = malloc(len + 1);
    if (!out)
        return NULL;
    size_t write = 0;
    const char *p = path;
    if (*p == '/')
        out[write++] = '/';
    while (*p) {
        while (*p == '/')
            p++;
        if (*p == '\0')
            break;
        const char *start = p;
        while (*p && *p != '/')
            p++;
        size_t comp_len = p - start;
        if (comp_len == 0)
            continue;
        if (comp_len == 1 && start[0] == '.')
            continue;
        else if (comp_len == 2 && start[0] == '.' && start[1] == '.') {
            if (write > 1) {
                size_t i = write - 1;
                while (i > 0 && out[i] != '/')
                    i--;
                write = i + 1;
            }
            continue;
        }
        memcpy(out + write, start, comp_len);
        write += comp_len;
        if (*p == '\0') {
        } else {
            out[write++] = '/';
        }
    }
    if (write == 0) {
        out[0] = '.';
        out[1] = '\0';
    } else {
        out[write] = '\0';
    }
    return out;
}

char *resolve(const char *path)
{
    if (!path)
        return NULL;
    if (path[0] == '/') {
        return canonicalize_path(path);
    }
    char cwd[PATH_MAX];
    if (!getcwd(cwd, sizeof(cwd))) {
        return NULL;
    }
    size_t cwd_len = strlen(cwd);
    size_t path_len = strlen(path);
    size_t total_len = cwd_len + 1 + path_len + 1;
    char *combined = malloc(total_len);
    if (!combined)
        return NULL;
    if (cwd_len == 1 && cwd[0] == '/') {
        snprintf(combined, total_len, "%s", path);
    } else {
        snprintf(combined, total_len, "%s/%s", cwd, path);
    }
    char *resolved = canonicalize_path(combined);
    free(combined);
    return resolved;
}

#ifdef TEST_RESOLVE
int main(void)
{
    const char *tests[] = {
        "./path/to/file",
        "../relative/../path",
        "/absolute/path/./to/../file",
        "////multiple////slashes////",
        NULL
    };
    for (int i = 0; tests[i] != NULL; ++i) {
        char *abs = resolve(tests[i]);
        if (abs) {
            printf("  \"%s\"  ->  \"%s\"\n", tests[i], abs);
            free(abs);
        } else {
            perror("resolve");
        }
    }
    return 0;
}
#endif
