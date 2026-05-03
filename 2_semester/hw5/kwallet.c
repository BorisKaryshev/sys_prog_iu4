/*
 *  kwallet_ioctl.c  –  kernel-space crypto-wallet using ioctl
 *
 *  Device: /dev/kwallet
 *
 *  ioctl commands are generated from the device's major number.
 *  Userspace must compute them in the same way.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>
#include <linux/string.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/ioctl.h>

#define WALLET_MAX_SLOTS   100
#define WALLET_LINE_MAX    256
#define WALLET_DEV_NAME    "kwallet"

/* ---- internal wallet structure (unchanged) ---- */
struct wallet_entry {
    char key_id[32];
    char priv_key[64];
    int  creation_id;          /* >0 = occupied */
};

static struct wallet_entry wallet_table[WALLET_MAX_SLOTS];
static DEFINE_MUTEX(wallet_lock);
static int wallet_next_cid = 1;

/* ---- ioctl request structures (unchanged) ---- */
struct wallet_req_create {
    char key_id[32];
    char priv_key[64];
    int creation_id;      /* out */
};

struct wallet_req_read {
    char key_id[32];
    char priv_key[64];    /* out */
};

struct wallet_req_change {
    char key_id[32];
    char old_priv_key[64];
    char new_priv_key[64];
    int creation_id;      /* out */
};

struct wallet_req_delete {
    char key_id[32];
    int status;           /* out: 0 = success, -errno */
};

struct wallet_req_deleteall {
    int status;           /* out: 0 */
};

struct wallet_req_readall {
    char __user *buffer;  /* userspace buffer for dump */
    size_t bufsize;       /* size of buffer */
    int out_len;          /* out: bytes written (excluding '\0') */
};

/* ---- runtime ioctl command numbers (calculated from major number) ---- */
static unsigned int wallet_ioc_create;
static unsigned int wallet_ioc_read;
static unsigned int wallet_ioc_change;
static unsigned int wallet_ioc_delete;
static unsigned int wallet_ioc_deleteall;
static unsigned int wallet_ioc_readall;

/* ---- helper functions (unchanged) ---- */
static int lookup_key(const char *name)
{
    int i;
    for (i = 0; i < WALLET_MAX_SLOTS; ++i)
        if (wallet_table[i].creation_id &&
            strcmp(wallet_table[i].key_id, name) == 0)
            return i;
    return -1;
}

static int find_free_slot(void)
{
    int i;
    for (i = 0; i < WALLET_MAX_SLOTS; ++i)
        if (wallet_table[i].creation_id == 0)
            return i;
    return -ENOSPC;
}

static bool is_wallet_empty(void)
{
    int i;
    for (i = 0; i < WALLET_MAX_SLOTS; ++i)
        if (wallet_table[i].creation_id)
            return false;
    return true;
}

static int dump_wallet(char *buf, size_t buf_size)
{
    char *p = buf;
    int i;
    *p = 0;
    for (i = 0; i < WALLET_MAX_SLOTS; ++i) {
        if (wallet_table[i].creation_id) {
            p += snprintf(p, buf_size - (p - buf),
                          "%d %s %s\n",
                          wallet_table[i].creation_id,
                          wallet_table[i].key_id,
                          wallet_table[i].priv_key);
            if (p >= buf + buf_size)
                break;
        }
    }
    if (p == buf)
        return snprintf(buf, buf_size, "(empty)\n");
    else
        return p - buf;
}

/* ---- ioctl handler with runtime command numbers ---- */
static long kwallet_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    void __user *argp = (void __user *)arg;
    int ret = 0;
    int idx;

    mutex_lock(&wallet_lock);

    if (cmd == wallet_ioc_create) {
        struct wallet_req_create req;
        if (copy_from_user(&req, argp, sizeof(req))) {
            ret = -EFAULT;
            goto out;
        }
        if (!req.key_id[0] || !req.priv_key[0]) {
            ret = -EINVAL;
            goto out;
        }
        if (lookup_key(req.key_id) >= 0) {
            ret = -EEXIST;
            goto out;
        }
        idx = find_free_slot();
        if (idx < 0) {
            ret = idx;
            goto out;
        }
        strscpy(wallet_table[idx].key_id, req.key_id, sizeof(wallet_table[0].key_id));
        strscpy(wallet_table[idx].priv_key, req.priv_key, sizeof(wallet_table[0].priv_key));
        wallet_table[idx].creation_id = wallet_next_cid++;
        req.creation_id = wallet_table[idx].creation_id;
        if (copy_to_user(argp, &req, sizeof(req)))
            ret = -EFAULT;
    } else if (cmd == wallet_ioc_read) {
        struct wallet_req_read req;
        if (copy_from_user(&req, argp, sizeof(req))) {
            ret = -EFAULT;
            goto out;
        }
        if (!req.key_id[0]) {
            ret = -EINVAL;
            goto out;
        }
        idx = lookup_key(req.key_id);
        if (idx < 0) {
            ret = -ENOENT;
            goto out;
        }
        strscpy(req.priv_key, wallet_table[idx].priv_key, sizeof(req.priv_key));
        if (copy_to_user(argp, &req, sizeof(req)))
            ret = -EFAULT;
    } else if (cmd == wallet_ioc_change) {
        struct wallet_req_change req;
        if (copy_from_user(&req, argp, sizeof(req))) {
            ret = -EFAULT;
            goto out;
        }
        if (!req.key_id[0] || !req.old_priv_key[0] || !req.new_priv_key[0]) {
            ret = -EINVAL;
            goto out;
        }
        idx = lookup_key(req.key_id);
        if (idx < 0) {
            ret = -ENOENT;
            goto out;
        }
        if (strcmp(wallet_table[idx].priv_key, req.old_priv_key) != 0) {
            ret = -EPERM;
            goto out;
        }
        strscpy(wallet_table[idx].priv_key, req.new_priv_key, sizeof(wallet_table[0].priv_key));
        req.creation_id = wallet_table[idx].creation_id;
        if (copy_to_user(argp, &req, sizeof(req)))
            ret = -EFAULT;
    } else if (cmd == wallet_ioc_delete) {
        struct wallet_req_delete req;
        if (copy_from_user(&req, argp, sizeof(req))) {
            ret = -EFAULT;
            goto out;
        }
        if (!req.key_id[0]) {
            ret = -EINVAL;
            goto out;
        }
        idx = lookup_key(req.key_id);
        if (idx < 0) {
            ret = -ENOENT;
            goto out;
        }
        memset(&wallet_table[idx], 0, sizeof(wallet_table[idx]));
        if (is_wallet_empty())
            wallet_next_cid = 1;
        req.status = 0;
        if (copy_to_user(argp, &req, sizeof(req)))
            ret = -EFAULT;
    } else if (cmd == wallet_ioc_deleteall) {
        struct wallet_req_deleteall req;
        int i;
        for (i = 0; i < WALLET_MAX_SLOTS; ++i) {
            if (wallet_table[i].creation_id)
                memset(&wallet_table[i], 0, sizeof(wallet_table[i]));
        }
        wallet_next_cid = 1;
        req.status = 0;
        if (copy_to_user(argp, &req, sizeof(req)))
            ret = -EFAULT;
    } else if (cmd == wallet_ioc_readall) {
        struct wallet_req_readall req;
        char *kernel_buf;

        if (copy_from_user(&req, argp, sizeof(req))) {
            ret = -EFAULT;
            goto out;
        }
        if (req.bufsize == 0 || !req.buffer) {
            ret = -EINVAL;
            goto out;
        }
        kernel_buf = kmalloc(req.bufsize, GFP_KERNEL);
        if (!kernel_buf) {
            ret = -ENOMEM;
            goto out;
        }
        req.out_len = dump_wallet(kernel_buf, req.bufsize);
        if (copy_to_user(req.buffer, kernel_buf, req.out_len + 1)) {
            kfree(kernel_buf);
            ret = -EFAULT;
            goto out;
        }
        if (copy_to_user(argp, &req, sizeof(req))) {
            kfree(kernel_buf);
            ret = -EFAULT;
            goto out;
        }
        kfree(kernel_buf);
    } else {
        ret = -ENOTTY;
    }

out:
    mutex_unlock(&wallet_lock);
    return ret;
}

/* ---- file operations ---- */
static int kwallet_open(struct inode *inode, struct file *filp)
{
    return 0;
}

static int kwallet_release(struct inode *inode, struct file *filp)
{
    return 0;
}

static const struct file_operations kwallet_fops = {
    .owner          = THIS_MODULE,
    .open           = kwallet_open,
    .release        = kwallet_release,
    .unlocked_ioctl = kwallet_ioctl,
};

/* ---- module init/exit ---- */
static dev_t kwallet_devno;
static struct cdev kwallet_cdev;
static struct class *kwallet_class;

/* Helper: build ioctl command number from major number and direction */
static inline unsigned int build_ioctl_cmd(int major, unsigned int nr,
                                           unsigned int dir, unsigned int size)
{
    unsigned int magic = (major & 0xFF);
    return _IOC(dir, magic, nr, size);
}

static int __init kwallet_init(void)
{
    int rc;
    unsigned int major;

    memset(wallet_table, 0, sizeof(wallet_table));
    wallet_next_cid = 1;

    rc = alloc_chrdev_region(&kwallet_devno, 0, 1, WALLET_DEV_NAME);
    if (rc < 0)
        return rc;

    major = MAJOR(kwallet_devno);

    /* Build ioctl command numbers based on the actual major number */
    wallet_ioc_create   = build_ioctl_cmd(major, 1, _IOC_WRITE | _IOC_READ, sizeof(struct wallet_req_create));
    wallet_ioc_read     = build_ioctl_cmd(major, 2, _IOC_READ,            sizeof(struct wallet_req_read));
    wallet_ioc_change   = build_ioctl_cmd(major, 3, _IOC_WRITE | _IOC_READ, sizeof(struct wallet_req_change));
    wallet_ioc_delete   = build_ioctl_cmd(major, 4, _IOC_WRITE,           sizeof(struct wallet_req_delete));
    wallet_ioc_deleteall = build_ioctl_cmd(major, 5, _IOC_WRITE,           sizeof(struct wallet_req_deleteall));
    wallet_ioc_readall  = build_ioctl_cmd(major, 6, _IOC_READ,            sizeof(struct wallet_req_readall));

    cdev_init(&kwallet_cdev, &kwallet_fops);
    rc = cdev_add(&kwallet_cdev, kwallet_devno, 1);
    if (rc)
        goto err_reg;

    kwallet_class = class_create(WALLET_DEV_NAME);
    if (IS_ERR(kwallet_class)) {
        rc = PTR_ERR(kwallet_class);
        goto err_cdev;
    }

    device_create(kwallet_class, NULL, kwallet_devno, NULL, WALLET_DEV_NAME);
    pr_info("kwallet: loaded, major=%d, ioctl commands built from major\n", major);
    return 0;

err_cdev:
    cdev_del(&kwallet_cdev);
err_reg:
    unregister_chrdev_region(kwallet_devno, 1);
    return rc;
}

static void __exit kwallet_exit(void)
{
    device_destroy(kwallet_class, kwallet_devno);
    class_destroy(kwallet_class);
    cdev_del(&kwallet_cdev);
    unregister_chrdev_region(kwallet_devno, 1);
    pr_info("kwallet: unloaded\n");
}

module_init(kwallet_init);
module_exit(kwallet_exit);

MODULE_DESCRIPTION("kernel-space crypto-wallet using ioctl (dynamic magic from major)");
MODULE_AUTHOR("Boris Karyshev");
MODULE_LICENSE("GPL");
