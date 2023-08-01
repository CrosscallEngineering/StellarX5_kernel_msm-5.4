/*
 * Copyright (C) 2016-2017 Hisense, Inc.
 *
 * Author:
 *   wangxufeng <wangxufeng@hisense.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/platform_device.h>
#include <linux/fs.h>
#include <linux/buffer_head.h>
#include <linux/slab.h>
#include <linux/debugfs.h>
#include <linux/his_debug_base.h>
#include <linux/proc_fs.h>
#include <linux/version.h>
#include <linux/of.h>
#include <linux/timex.h>
#include <linux/rtc.h>

#define DEBUG_NODE_NAME      "debug_control"

static struct kobject *hs_sysfs_root;
static struct dentry *hs_debugfs_root;
static struct proc_dir_entry *hs_procfs_root;

static struct kobject *his_create_sysfs_root(void)
{
	static int create_ok = 0;

	if (create_ok == 1)
		goto out;

	hs_sysfs_root = kobject_create_and_add(DEBUG_NODE_NAME, NULL);
	if (!hs_sysfs_root) {
		pr_err("%s: kobject create Failed!\n", __func__);
		return NULL;
	}

	create_ok = 1;
out:
	return hs_sysfs_root;
}

int his_register_sysfs_attr(struct attribute *attr)
{
	int ret = 0;
	struct kobject *kobj_root = NULL;

	if (!attr) {
		pr_err("%s attr is NULL\n", __func__);
		return -EINVAL;
	}

	kobj_root = his_create_sysfs_root();
	if (!kobj_root) {
		pr_err("%s kobject create failed\n", __func__);
		return -ENOMEM;
	}

	ret = sysfs_create_file(kobj_root, attr);
	if (ret < 0) {
		pr_err("%s Error creating sysfs attr %d\n", __func__, ret);
		return ret;
	}

	pr_info("%s register %s ok\n", __func__, attr->name);

	return 0;
}
EXPORT_SYMBOL(his_register_sysfs_attr);

int his_register_sysfs_attr_group(struct attribute_group *attr_group)
{
	int ret = 0;
	struct kobject *kobj_root = NULL;

	if (!attr_group) {
		pr_err("%s attr_group is NULL\n", __func__);
		return -EINVAL;
	}

	kobj_root = his_create_sysfs_root();
	if (!kobj_root) {
		pr_err("%s kobject create failed\n", __func__);
		return -ENOMEM;
	}

	ret = sysfs_create_group(kobj_root, attr_group);
	if (ret < 0) {
		pr_err("%s Error creating sysfs group %d\n", __func__, ret);
		return ret;
	}

	pr_info("%s register group ok\n", __func__);

	return 0;
}
EXPORT_SYMBOL(his_register_sysfs_attr_group);

struct kobject *his_register_sysfs_dir(const char *name)
{
	struct kobject *kobj_root;
	struct kobject *creat_kobj;

	kobj_root = his_create_sysfs_root();
	if (!kobj_root) {
		pr_err("%s kobject create failed\n", __func__);
		return NULL;
	}

	creat_kobj = kobject_create_and_add(name, kobj_root);
	if (!creat_kobj) {
		pr_err("%s kobject create and add failed\n", __func__);
		return NULL;
	}

	return creat_kobj;
}
EXPORT_SYMBOL(his_register_sysfs_dir);

static struct dentry *his_create_debugfs_root(void)
{
	static int create_ok = 0;

	if (create_ok == 1)
		goto out;

	hs_debugfs_root = debugfs_create_dir(DEBUG_NODE_NAME, NULL);
	if (IS_ERR(hs_debugfs_root))
		return NULL;

	create_ok = 1;
out:
	return hs_debugfs_root;
}

int his_register_debugfs_file(const char *name, umode_t mode, void *data,
		const struct file_operations *fops)
{
	struct dentry *entry = NULL;
	struct dentry *debugfs_root;

	debugfs_root = his_create_debugfs_root();
	if (debugfs_root == NULL)
		return -ENOMEM;

	entry = debugfs_create_file(name, mode, debugfs_root, data, fops);
	if (!entry)
		return -ENOMEM;

	return 0;
}
EXPORT_SYMBOL(his_register_debugfs_file);

struct dentry *his_register_debugfs_dir(const char *name)
{
	struct dentry *debugfs_root;

	debugfs_root = his_create_debugfs_root();
	if (debugfs_root == NULL)
		return NULL;

	return debugfs_create_dir(name, debugfs_root);
}
EXPORT_SYMBOL(his_register_debugfs_dir);

static struct proc_dir_entry *his_create_procfs_root(void)
{
	static int create_ok = 0;

	if (create_ok == 1)
		goto out;

	hs_procfs_root = proc_mkdir(DEBUG_NODE_NAME, NULL);
	if (IS_ERR(hs_procfs_root))
		return NULL;

	create_ok = 1;
out:
	return hs_procfs_root;
}

int his_create_procfs_file(const char *name, umode_t mode,
		const struct file_operations *fops)
{
	struct proc_dir_entry *entry = NULL;
	struct proc_dir_entry *procfs_root;

	procfs_root = his_create_procfs_root();
	if (procfs_root == NULL)
		return -ENOMEM;

	entry = proc_create(name, mode, procfs_root, fops);
	if (!entry)
		return -ENOMEM;

	return 0;
}
EXPORT_SYMBOL(his_create_procfs_file);

struct proc_dir_entry *his_create_procfs_dir(const char *name)
{
	struct proc_dir_entry *procfs_root;

	procfs_root = his_create_procfs_root();
	if (procfs_root == NULL)
		return NULL;

	return proc_mkdir(name, procfs_root);
}
EXPORT_SYMBOL(his_create_procfs_dir);

int his_of_get_u32_array(struct device_node *np, const char *prop_name,
		u32 *out, int *len, u32 max_size)
{
	int ret = 0;
	size_t sz;

	if (!of_get_property(np, prop_name, len)) {
		ret = -EINVAL;
		goto err_out;
	}

	sz = *len = *len / sizeof(u32);
	if (sz <= 0 || (max_size > 0 && (sz > max_size))) {
		pr_err("%s invalid size\n", prop_name);
		ret = -EINVAL;
		goto err_out;
	}

	pr_err("%s array size is %ld\n", prop_name, sz);
	ret = of_property_read_u32_array(np, prop_name, out, sz);
	if (ret < 0) {
		pr_err("%s failed read array %d\n", prop_name, ret);
		goto err_out;
	}

	return 0;
err_out:
	*len = 0;
	return ret;
}

void his_get_current_time(char *ptime, int size)
{
	struct timespec64 ts;
	struct rtc_time tm;

	ktime_get_real_ts64(&ts);
	rtc_time_to_tm(ts.tv_sec, &tm);
	snprintf(ptime, size, "%d-%02d-%02d_%02d:%02d:%02d",
			tm.tm_year + 1900,
			tm.tm_mon+1, tm.tm_mday,
			tm.tm_hour, tm.tm_min, tm.tm_sec);
}

