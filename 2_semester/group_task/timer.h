#ifndef TIMER_H
#define TIMER_H

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/kthread.h>
#include <linux/sysfs.h>
#include <linux/kobject.h>
#include <linux/string.h>
#include <linux/uaccess.h>
#include <linux/atomic.h>
#include <linux/sched.h>

#include "hidepid.h"

static struct task_struct *worker_task = NULL;
static atomic64_t timer_cnt;
static pid_t worker_pid = 0;
static struct kobject *hidden_timer_kobj;

static int worker_thread_fn(void *data)
{
    worker_pid = task_pid_nr(current);
    set_pid_to_ban(worker_pid);
    pr_info("Worker thread started (PID = %d)\n", worker_pid);

    while (!kthread_should_stop()) {
        atomic64_inc(&timer_cnt);
    }

    pr_info("Worker thread exiting\n");
    return 0;
}

static ssize_t timer_show(struct kobject *kobj, struct kobj_attribute *attr,
                          char *buf)
{
    uint64_t val = atomic64_read(&timer_cnt);
    return snprintf(buf, PAGE_SIZE, "%llu\n", (unsigned long long)val);
}

static ssize_t timer_store(struct kobject *kobj, struct kobj_attribute *attr,
                           const char *buf, size_t count)
{
    unsigned long long new_val;
    int ret;

    ret = kstrtoull(buf, 0, &new_val);
    if (ret)
        return ret;

    atomic64_set(&timer_cnt, new_val);
    return count;
}

static ssize_t pid_show(struct kobject *kobj, struct kobj_attribute *attr,
                        char *buf)
{
    if (worker_pid == 0)
        return snprintf(buf, PAGE_SIZE, "Worker not running\n");
    return snprintf(buf, PAGE_SIZE, "%d\n", worker_pid);
}

static struct kobj_attribute timer_attr = __ATTR(timer, 0644,
                                                  timer_show, timer_store);
static struct kobj_attribute pid_attr = __ATTR(pid, 0444,
                                                pid_show, NULL);

static struct attribute *attrs[] = {
    &timer_attr.attr,
    &pid_attr.attr,
    NULL,
};
static struct attribute_group attr_group = {
    .attrs = attrs,
};

static int hidden_timer_init(void)
{
    int ret;

    atomic64_set(&timer_cnt, 0);

    hidden_timer_kobj = kobject_create_and_add("hidden_timer", NULL);
    if (!hidden_timer_kobj)
        return -ENOMEM;

    ret = sysfs_create_group(hidden_timer_kobj, &attr_group);
    if (ret) {
        pr_err("Failed to create sysfs group\n");
        kobject_put(hidden_timer_kobj);
        return ret;
    }

    worker_task = kthread_run(worker_thread_fn, NULL, "hidden_timer_thread");
    if (IS_ERR(worker_task)) {
        pr_err("Failed to create worker thread\n");
        sysfs_remove_group(hidden_timer_kobj, &attr_group);
        kobject_put(hidden_timer_kobj);
        return PTR_ERR(worker_task);
    }

    pr_info("hidden_timer module loaded\n");
    return 0;
}

/* Module cleanup */
static void hidden_timer_exit(void)
{
    if (worker_task) {
        kthread_stop(worker_task);
        worker_task = NULL;
    }
    worker_pid = 0;

    sysfs_remove_group(hidden_timer_kobj, &attr_group);
    kobject_put(hidden_timer_kobj);

    pr_info("hidden_timer module unloaded\n");
}

#endif
