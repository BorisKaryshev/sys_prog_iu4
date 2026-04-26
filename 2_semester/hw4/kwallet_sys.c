/*
 *  kwallet.c  –  kernel-space “crypto-wallet” demo (sysfs version)
 *
 *  Appears as /sys/kwallet/kwallet
 *
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
 *  Behaviour (adapted for sysfs):
 *    - write() accepts a command, stores its response in a global buffer.
 *    - read() returns that response (multiple reads return the same data
 *      until the next write).
 *    - Negative errno values are never returned from write() – they are
 *      encoded into the response string.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/mutex.h>
#include <linux/string.h>
#include <linux/errno.h>
#include <linux/slab.h>

#define WALLET_MAX_SLOTS   100
#define WALLET_LINE_MAX    256

struct wallet_entry {
    char key_id[32];
    char priv_key[64];
    int  creation_id;          /* >0 = occupied */
};

static struct wallet_entry wallet_table[WALLET_MAX_SLOTS];
static DEFINE_MUTEX(wallet_lock);
static int wallet_next_cid = 1;

/* Global response (shared by all open file descriptors) */
static char global_response[WALLET_LINE_MAX];
static bool response_ready;
static DEFINE_MUTEX(response_lock);

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
static ssize_t kwallet_write(struct file *filp, struct kobject *kobj,
                             const struct bin_attribute *attr,
                             char *buf, loff_t off, size_t count)
{
    char line[WALLET_LINE_MAX];
    char cmd[32], arg1[64], arg2[64], arg3[64];
    int idx, ret;
    char reply[WALLET_LINE_MAX];
    int reply_len;
    size_t orig_len = count;
    int i;

    if (count >= sizeof(line))
        return -EINVAL;
    memcpy(line, buf, count);
    line[count] = '\0';
    if (count > 0 && line[count-1] == '\n')
        line[--count] = '\0';

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
        if (is_wallet_empty())
            wallet_next_cid = 1;
        reply_len = snprintf(reply, sizeof(reply), "0\n");
    }
    else if (strcasecmp(cmd, "DELETEALL") == 0) {
        for (i = 0; i < WALLET_MAX_SLOTS; ++i) {
            if (wallet_table[i].creation_id)
                memset(&wallet_table[i], 0, sizeof(wallet_table[i]));
        }
        wallet_next_cid = 1;
        reply_len = snprintf(reply, sizeof(reply), "0\n");
    }
    else {
        ret = -EINVAL;
        goto out_err;
    }

    mutex_unlock(&wallet_lock);

    /* Store the global response */
    mutex_lock(&response_lock);
    strscpy(global_response, reply, sizeof(global_response));
    response_ready = true;
    mutex_unlock(&response_lock);

    return orig_len;

out_err:
    mutex_unlock(&wallet_lock);
    /* On error, store a "-1 <errno>\n" response */
    mutex_lock(&response_lock);
    snprintf(global_response, sizeof(global_response), "-1 %d\n", -ret);
    response_ready = true;
    mutex_unlock(&response_lock);
    return orig_len;   /* always return success to the write() syscall */
}

/* ---- READ ---------------------------------------------------- */
static ssize_t kwallet_read(struct file *filp, struct kobject *kobj,
                            const struct bin_attribute *attr,
                            char *buf, loff_t off, size_t count)
{
    size_t total_len;
    ssize_t ret = 0;

    mutex_lock(&response_lock);

    if (!response_ready) {
        mutex_unlock(&response_lock);
        return 0;   /* EOF */
    }

    total_len = strlen(global_response);
    if (off >= total_len) {
        mutex_unlock(&response_lock);
        return 0;
    }

    if (off + count > total_len)
        count = total_len - off;

    memcpy(buf, global_response + off, count);
    mutex_unlock(&response_lock);

    return count;
}

/* ---- sysfs infrastructure -------------------------------------- */
static struct bin_attribute kwallet_bin_attr = {
    .attr = {
        .name = "kwallet",
        .mode = 0666,
    },
    .read = kwallet_read,
    .write = kwallet_write,
    .size = 0,   /* variable size */
};

static struct kobject *kwallet_kobj;

/* ---- init / exit --------------------------------------------- */
static int __init kwallet_init(void)
{
    int ret;

    memset(wallet_table, 0, sizeof(wallet_table));
    wallet_next_cid = 1;
    response_ready = false;

    /* Create /sys/kwallet directory */
    kwallet_kobj = kobject_create_and_add("kwallet", NULL);
    if (!kwallet_kobj) {
        pr_err("kwallet: failed to create /sys/kwallet\n");
        return -ENOMEM;
    }

    /* Create the binary attribute file /sys/kwallet/kwallet */
    ret = sysfs_create_bin_file(kwallet_kobj, &kwallet_bin_attr);
    if (ret) {
        pr_err("kwallet: failed to create sysfs file\n");
        kobject_put(kwallet_kobj);
        return ret;
    }

    pr_info("kwallet: loaded, file /sys/kwallet/kwallet ready\n");
    return 0;
}

static void __exit kwallet_exit(void)
{
    sysfs_remove_bin_file(kwallet_kobj, &kwallet_bin_attr);
    kobject_put(kwallet_kobj);
    pr_info("kwallet: unloaded\n");
}

module_init(kwallet_init);
module_exit(kwallet_exit);

MODULE_DESCRIPTION("simple kernel-space wallet demo (sysfs version)");
MODULE_AUTHOR("Boris Karyshev");
MODULE_LICENSE("GPL");
