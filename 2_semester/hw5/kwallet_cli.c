/*
 *  crypto_wallet.c  –  userspace tool to control /dev/kwallet via ioctl
 *
 *  Interactive mode with case‑insensitive commands.
 *  Dynamically builds ioctl commands from the device's major number.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <errno.h>
#include <ctype.h>

struct wallet_req_create {
    char key_id[32];
    char priv_key[64];
    int creation_id;
};

struct wallet_req_read {
    char key_id[32];
    char priv_key[64];
};

struct wallet_req_change {
    char key_id[32];
    char old_priv_key[64];
    char new_priv_key[64];
    int creation_id;
};

struct wallet_req_delete {
    char key_id[32];
    int status;
};

struct wallet_req_deleteall {
    int status;
};

struct wallet_req_readall {
    char *buffer;
    size_t bufsize;
    int out_len;
};

/* Runtime ioctl command numbers – filled after we know the device's major number */
static unsigned int cmd_create;
static unsigned int cmd_read;
static unsigned int cmd_change;
static unsigned int cmd_delete;
static unsigned int cmd_deleteall;
static unsigned int cmd_readall;

static const char *dev_path = "/dev/kwallet";

static void to_lower(char *str) {
    for (; *str; ++str)
        *str = tolower((unsigned char)*str);
}

static void print_help(const char *prog) {
    fprintf(stdout,
        "Wallet control tool – interactive mode (case‑insensitive commands)\n"
        "Commands:\n"
        "  CREATE <key_id> <priv_key>          – add a new key\n"
        "  READ   <key_id>                     – retrieve private key\n"
        "  CHANGE <key_id> <old> <new>         – update private key\n"
        "  DELETE <key_id>                     – remove a key\n"
        "  DELETEALL                           – remove all keys\n"
        "  READALL                             – show all key/value pairs\n"
        "  EXIT / QUIT                         – exit program\n"
        "\n"
        "Example: CREATE mykey 0xabcdef123456\n"
    );
    (void)prog;
}

static void process_command(int fd, char *line) {
    // remove trailing newline
    line[strcspn(line, "\n")] = '\0';

    char cmd[32];
    char arg1[64], arg2[64], arg3[64];
    int n = sscanf(line, "%31s %63s %63s %63s", cmd, arg1, arg2, arg3);

    if (n == 0) return; // empty line

    // make command case‑insensitive
    to_lower(cmd);

    if (strcmp(cmd, "exit") == 0 || strcmp(cmd, "quit") == 0) {
        printf("Exiting.\n");
        exit(0);
    }
    else if (strcmp(cmd, "create") == 0) {
        if (n != 3) {
            fprintf(stderr, "Usage: CREATE <key_id> <priv_key>\n");
            return;
        }
        struct wallet_req_create req;
        memset(&req, 0, sizeof(req));
        strncpy(req.key_id, arg1, sizeof(req.key_id)-1);
        strncpy(req.priv_key, arg2, sizeof(req.priv_key)-1);
        int ret = ioctl(fd, cmd_create, &req);
        if (ret < 0) {
            perror("ioctl CREATE");
            return;
        }
        printf("Created: creation_id = %d\n", req.creation_id);
    }
    else if (strcmp(cmd, "read") == 0) {
        if (n != 2) {
            fprintf(stderr, "Usage: READ <key_id>\n");
            return;
        }
        struct wallet_req_read req;
        memset(&req, 0, sizeof(req));
        strncpy(req.key_id, arg1, sizeof(req.key_id)-1);
        int ret = ioctl(fd, cmd_read, &req);
        if (ret < 0) {
            perror("ioctl READ");
            return;
        }
        printf("%s %s\n", req.key_id, req.priv_key);
    }
    else if (strcmp(cmd, "change") == 0) {
        if (n != 4) {
            fprintf(stderr, "Usage: CHANGE <key_id> <old_priv_key> <new_priv_key>\n");
            return;
        }
        struct wallet_req_change req;
        memset(&req, 0, sizeof(req));
        strncpy(req.key_id, arg1, sizeof(req.key_id)-1);
        strncpy(req.old_priv_key, arg2, sizeof(req.old_priv_key)-1);
        strncpy(req.new_priv_key, arg3, sizeof(req.new_priv_key)-1);
        int ret = ioctl(fd, cmd_change, &req);
        if (ret < 0) {
            perror("ioctl CHANGE");
            return;
        }
        printf("Changed: creation_id = %d\n", req.creation_id);
    }
    else if (strcmp(cmd, "delete") == 0) {
        if (n != 2) {
            fprintf(stderr, "Usage: DELETE <key_id>\n");
            return;
        }
        struct wallet_req_delete req;
        memset(&req, 0, sizeof(req));
        strncpy(req.key_id, arg1, sizeof(req.key_id)-1);
        int ret = ioctl(fd, cmd_delete, &req);
        if (ret < 0) {
            perror("ioctl DELETE");
            return;
        }
        printf("Deleted (status=%d)\n", req.status);
    }
    else if (strcmp(cmd, "deleteall") == 0) {
        if (n != 1) {
            fprintf(stderr, "Usage: DELETEALL (no arguments)\n");
            return;
        }
        struct wallet_req_deleteall req;
        memset(&req, 0, sizeof(req));
        int ret = ioctl(fd, cmd_deleteall, &req);
        if (ret < 0) {
            perror("ioctl DELETEALL");
            return;
        }
        printf("All deleted (status=%d)\n", req.status);
    }
    else if (strcmp(cmd, "readall") == 0) {
        if (n != 1) {
            fprintf(stderr, "Usage: READALL (no arguments)\n");
            return;
        }
        char buf[4096];
        struct wallet_req_readall req;
        memset(&req, 0, sizeof(req));
        req.buffer = buf;
        req.bufsize = sizeof(buf);
        int ret = ioctl(fd, cmd_readall, &req);
        if (ret < 0) {
            perror("ioctl READALL");
            return;
        }
        if (req.out_len >= 0 && req.out_len < (int)sizeof(buf))
            buf[req.out_len] = '\0';
        else
            buf[sizeof(buf)-1] = '\0';
        printf("%s", buf);
    }
    else {
        fprintf(stderr, "Unknown command: %s\n", cmd);
        fprintf(stderr, "Type 'EXIT' to quit.\n");
    }
}

int main(int argc, char **argv) {
    print_help(argv[0]);

    int fd = open(dev_path, O_RDWR);
    if (fd < 0) {
        perror("open /dev/kwallet");
        return 1;
    }

    /* Obtain the device's major number to use as ioctl magic */
    struct stat st;
    if (fstat(fd, &st) < 0) {
        perror("fstat");
        close(fd);
        return 1;
    }
    unsigned int major_num = major(st.st_rdev);
    unsigned int magic = major_num & 0xFF;

    /* Build ioctl command numbers dynamically */
    cmd_create   = _IOC(_IOC_READ | _IOC_WRITE, magic, 1, sizeof(struct wallet_req_create));
    cmd_read     = _IOC(_IOC_READ,              magic, 2, sizeof(struct wallet_req_read));
    cmd_change   = _IOC(_IOC_READ | _IOC_WRITE, magic, 3, sizeof(struct wallet_req_change));
    cmd_delete   = _IOC(_IOC_WRITE,             magic, 4, sizeof(struct wallet_req_delete));
    cmd_deleteall= _IOC(_IOC_WRITE,             magic, 5, sizeof(struct wallet_req_deleteall));
    cmd_readall  = _IOC(_IOC_READ,              magic, 6, sizeof(struct wallet_req_readall));

    printf("Device major = %u, ioctl magic = 0x%02x\n", major_num, magic);

    char line[512];
    while (1) {
        printf("> ");
        fflush(stdout);
        if (!fgets(line, sizeof(line), stdin)) {
            printf("\nExiting.\n");
            break;
        }
        process_command(fd, line);
    }

    close(fd);
    return 0;
}
