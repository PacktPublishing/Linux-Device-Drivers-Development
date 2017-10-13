#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/kobject.h>

struct d_attr {
    struct attribute attr;
    int value; /* This is our data */
};

static struct d_attr notify = {
    .attr.name="notify",
    .attr.mode = 0644,
    .value = 0,
};

static struct d_attr trigger = {
    .attr.name="trigger",
    .attr.mode = 0644,
    .value = 0,
};

static struct attribute * d_attrs[] = {
    &notify.attr,
    &trigger.attr,
    NULL
};

static ssize_t show(struct kobject *kobj, struct attribute *attr, char *buf)
{
    struct d_attr *da = container_of(attr, struct d_attr, attr);
    pr_info( "hello: show called (%s)\n", da->attr.name );
    return scnprintf(buf, PAGE_SIZE, "%s: %d\n", da->attr.name, da->value);
}
static struct kobject *mykobj;

static ssize_t store(struct kobject *kobj, struct attribute *attr, const char *buf, size_t len)
{
    struct d_attr *da = container_of(attr, struct d_attr, attr);

    sscanf(buf, "%d", &da->value);
    pr_info("sysfs_notify store %s = %d\n", da->attr.name, da->value);

    if (strcmp(da->attr.name, "notify") == 0){
        notify.value = da->value;
        sysfs_notify(mykobj, NULL, "notify");
    }
    else if(strcmp(da->attr.name, "trigger") == 0){
        trigger.value = da->value;
        sysfs_notify(mykobj, NULL, "trigger");
    }
    return sizeof(int);
}

static struct sysfs_ops s_ops = {
    .show = show,
    .store = store,
};

static struct kobj_type k_type = {
    .sysfs_ops = &s_ops,
    .default_attrs = d_attrs,
};

static struct kobject *mykobj;
static int __init hello_module_init(void)
{
    int err = -1;
    pr_info("Pollable sysfs hello: init\n");
    mykobj = kzalloc(sizeof(*mykobj), GFP_KERNEL);
    /* mykobj = kobject_create() is not exported */
    if (mykobj) {
        kobject_init(mykobj, &k_type);
        if (kobject_add(mykobj, NULL, "%s", "hello")) {
             err = -1;
             pr_info("Hello: kobject_add() failed\n");
             kobject_put(mykobj);
             mykobj = NULL;
        }
        err = 0;
    }
    return err;
}

static void __exit hello_module_exit(void)
{
    if (mykobj) {
        kobject_put(mykobj);
        kfree(mykobj);
    }
    pr_info("Pollable sysfs hello: exit\n");
}

module_init(hello_module_init);
module_exit(hello_module_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("John Madieu <john.madieu@gmail.com>");
