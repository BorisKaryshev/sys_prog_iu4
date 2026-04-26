/*
 *  kwallet.c  –  kernel-space “crypto-wallet” demo
 *
 *  one char device  /dev/kwallet
 *  protocol (space separated, NL terminated, max 256 B):
 *      CREATE <key_id> <priv_key>
 *      READ   <key_id>                → prints "key_id priv_key"
 *      READALL                        → prints full wallet (one line per entry)
 *      CHANGE <key_id> <old_priv_key> <new_priv_key>
 *      DELETE <key_id>
 *      DELETEALL
 *
 *  reply: one line
 *      <creation_id>                   … success (CREATE/CHANGE)
 *      <key_id> <priv_key>             … success (READ)
 *      0                               … success (DELETE/DELETEALL)
 *      -1 <errno>                      … failure
 *
 *  Behaviour:
 *    - write() accepts a command and stores its response internally.
 *    - read() returns:
 *        * the response of the last command (once per open file descriptor),
 *        * then EOF (0) for subsequent reads on the same fd.
 *      No automatic wallet dump.
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

#define WALLET_MAX_SLOTS   100
#define WALLET_LINE_MAX    256
#define WALLET_DEV_NAME    "kwallet"

struct wallet_entry {
    char key_id[32];
    char priv_key[64];
    int  creation_id;          /* >0 = occupied */
};

static struct wallet_entry wallet_table[WALLET_MAX_SLOTS];
static DEFINE_MUTEX(wallet_lock);
static int wallet_next_cid = 1;

static dev_t kwallet_devno;
static struct cdev kwallet_cdev;
static struct class *kwallet_class;

/* Pending response from the last write() (global) */
static char pending_response[WALLET_LINE_MAX];
static bool response_ready;
static DEFINE_MUTEX(response_lock);

/* Per‑file state: each fd can read the pending response only once */
struct kwallet_file_state {
    bool response_consumed;   /* this fd already read the pending response */
};

/* ---- helpers -------------------------------------------------- */
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

/* Generate the full wallet dump into a caller-provided buffer.
 * Returns the number of bytes written (excluding final NUL).
 */
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

/* ---- WRITE ---------------------------------------------------- */
static ssize_t kwallet_write(struct file *filp, const char __user *ubuf,
                             size_t len, loff_t *offs)
{
    char line[WALLET_LINE_MAX];
    char cmd[32], arg1[64], arg2[64], arg3[64];
    int idx, ret;
    char reply[WALLET_LINE_MAX];
    int reply_len;
    size_t orig_len = len;
    int i;

    if (len >= sizeof(line))
        return -EINVAL;
    if (copy_from_user(line, ubuf, len))
        return -EFAULT;
    line[len] = '\0';
    if (len > 0 && line[len-1] == '\n')
        line[--len] = '\0';

    arg1[0] = arg2[0] = arg3[0] = '\0';
    sscanf(line, "%31s %63s %63s %63s", cmd, arg1, arg2, arg3);

    mutex_lock(&wallet_lock);

    if (strcasecmp(cmd, "CREATE") == 0) {
        if (!arg1[0] || !arg2[0]) {
            ret = -EINVAL;
            goto out_err;
        }
        if (lookup_key(arg1) >= 0) {
            ret = -EEXIST;
            goto out_err;
        }
        idx = find_free_slot();
        if (idx < 0) {
            ret = idx;
            goto out_err;
        }
        strscpy(wallet_table[idx].key_id,   arg1, sizeof(wallet_table[0].key_id));
        strscpy(wallet_table[idx].priv_key, arg2, sizeof(wallet_table[0].priv_key));
        wallet_table[idx].creation_id = wallet_next_cid++;
        reply_len = snprintf(reply, sizeof(reply), "%d\n", wallet_table[idx].creation_id);
    }
    else if (strcasecmp(cmd, "READ") == 0) {
        if (!arg1[0]) {
            ret = -EINVAL;
            goto out_err;
        }
        idx = lookup_key(arg1);
        if (idx < 0) {
            ret = -ENOENT;
            goto out_err;
        }
        /* Print only key_id and priv_key (no creation id) */
        reply_len = snprintf(reply, sizeof(reply), "%s %s\n",
                             wallet_table[idx].key_id,
                             wallet_table[idx].priv_key);
    }
    else if (strcasecmp(cmd, "READALL") == 0) {
        reply_len = dump_wallet(reply, sizeof(reply));
    }
    else if (strcasecmp(cmd, "CHANGE") == 0) {
        if (!arg1[0] || !arg2[0] || !arg3[0]) {
            ret = -EINVAL;
            goto out_err;
        }
        idx = lookup_key(arg1);
        if (idx < 0) {
            ret = -ENOENT;
            goto out_err;
        }
        if (strcmp(wallet_table[idx].priv_key, arg2) != 0) {
            ret = -EPERM;
            goto out_err;
        }
        strscpy(wallet_table[idx].priv_key, arg3, sizeof(wallet_table[0].priv_key));
        reply_len = snprintf(reply, sizeof(reply), "%d\n", wallet_table[idx].creation_id);
    }
    else if (strcasecmp(cmd, "DELETE") == 0) {
        if (!arg1[0]) {
            ret = -EINVAL;
            goto out_err;
        }
        idx = lookup_key(arg1);
        if (idx < 0) {
            ret = -ENOENT;
            goto out_err;
        }
        memset(&wallet_table[idx], 0, sizeof(wallet_table[idx]));
        /* Reset next_cid if wallet becomes empty */
        if (is_wallet_empty())
            wallet_next_cid = 1;
        reply_len = snprintf(reply, sizeof(reply), "0\n");
    }
    else if (strcasecmp(cmd, "DELETEALL") == 0) {
        for (i = 0; i < WALLET_MAX_SLOTS; ++i) {
            if (wallet_table[i].creation_id)
                memset(&wallet_table[i], 0, sizeof(wallet_table[i]));
        }
        wallet_next_cid = 1;   /* wallet is definitely empty now */
        reply_len = snprintf(reply, sizeof(reply), "0\n");
    }
    else {
        ret = -EINVAL;
        goto out_err;
    }

    mutex_unlock(&wallet_lock);

    /* Store the response globally */
    mutex_lock(&response_lock);
    strscpy(pending_response, reply, sizeof(pending_response));
    response_ready = true;
    mutex_unlock(&response_lock);

    return orig_len;

out_err:
    mutex_unlock(&wallet_lock);
    return ret;
}

/* ---- READ ---------------------------------------------------- */
static ssize_t kwallet_read(struct file *filp, char __user *buf,
                            size_t len, loff_t *offs)
{
    struct kwallet_file_state *state = filp->private_data;
    char full_reply[WALLET_LINE_MAX];
    size_t total_len;

    mutex_lock(&response_lock);

    /* Only return the pending response once per file descriptor */
    if (response_ready && !state->response_consumed) {
        strscpy(full_reply, pending_response, sizeof(full_reply));
        total_len = strlen(full_reply);
        state->response_consumed = true;
        mutex_unlock(&response_lock);

        /* Handle partial reads using file offset */
        if (*offs >= total_len)
            return 0;
        if (*offs + len > total_len)
            len = total_len - *offs;
        if (copy_to_user(buf, full_reply + *offs, len))
            return -EFAULT;
        *offs += len;
        return len;
    }

    /* No pending response (or already consumed) → EOF */
    mutex_unlock(&response_lock);
    return 0;
}

/* ---- OPEN / RELEASE ------------------------------------------- */
static int kwallet_open(struct inode *inode, struct file *filp)
{
    struct kwallet_file_state *state;

    state = kmalloc(sizeof(*state), GFP_KERNEL);
    if (!state)
        return -ENOMEM;

    state->response_consumed = false;
    filp->private_data = state;
    return 0;
}

static int kwallet_release(struct inode *inode, struct file *filp)
{
    kfree(filp->private_data);
    return 0;
}

/* ---- file ops ------------------------------------------------- */
static const struct file_operations kwallet_fops = {
    .owner   = THIS_MODULE,
    .open    = kwallet_open,
    .release = kwallet_release,
    .write   = kwallet_write,
    .read    = kwallet_read,
    .llseek  = default_llseek,
};

/* ---- init / exit --------------------------------------------- */
static int __init kwallet_init(void)
{
    int rc;

    memset(wallet_table, 0, sizeof(wallet_table));
    wallet_next_cid = 1;
    response_ready = false;

    rc = alloc_chrdev_region(&kwallet_devno, 0, 1, WALLET_DEV_NAME);
    if (rc < 0)
        return rc;

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
    pr_info("kwallet: loaded, device /dev/%s ready\n", WALLET_DEV_NAME);
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

MODULE_DESCRIPTION("simple kernel-space wallet demo");
MODULE_AUTHOR("Boris Karyshev");
MODULE_LICENSE("GPL");
