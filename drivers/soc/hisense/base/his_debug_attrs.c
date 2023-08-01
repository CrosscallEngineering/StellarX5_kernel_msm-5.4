/*
 * Copyright (C) 2005-2016 Hisense, Inc.
 *
 * Author:
 *   wangyongqing <wangyongqing1@hisense.com>
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
#include <linux/his_debug_base.h>
#include <hiconfig/flag_parti_layout.h>

#define HISENSE_DEBUG_ATTR(attr_name, attr_offset) \
	static ssize_t attr_name##_store(struct kobject *kobj, \
			struct kobj_attribute *attr, const char *buf, size_t len) \
	{ \
		int ret; \
		u32 value; \
					\
		ret = kstrtou32(buf, 10, &value); \
		if (ret) { \
			pr_err("%s: kstrtol is wrong!\n", __func__); \
			return -EINVAL; \
		} \
		  \
		pr_buf_info("%s : enable %u\n", __func__, value); \
		hs_store_attr_value((loff_t)attr_offset, value); \
		\
		return len; \
	} \
	static ssize_t attr_name##_show(struct kobject *kobj, \
			struct kobj_attribute *attr, char *buf) \
	{ \
		u32 flag = 0; \
		char partition[128] = {0}; \
		\
		his_get_partition_path(partition, 128, SYS_FLAG_PARTITION); \
		his_read_file(partition, (loff_t)attr_offset, (void *)&flag, sizeof(flag)); \
		\
		return snprintf(buf, PAGE_SIZE, "%u\n", flag); \
	} \
	static struct kobj_attribute attr_name##_attr = __ATTR_RW(attr_name);

/* Debug cmdline early setup macro */
#define DEFINED_DEBUG_EARLY_PARAM(cmdline, setted_bit)  \
	static int __init early_param_##cmdline(char *p) { \
		boot_debug_flag |= setted_bit; \
		pr_err("%s: set %s\n", __func__, #setted_bit); \
		return 0; \
	} \
	early_param(#cmdline, early_param_##cmdline)

size_t his_read_file(const char *path, loff_t offset, void *buf, u32 size)
{
	size_t ret = 0;
	struct file *filp = NULL;

	filp = filp_open(path, O_RDONLY, 0644);
	if (IS_ERR(filp)) {
		pr_err("Failed open file=%s, ret=%p\n", path, filp);
		return PTR_ERR(filp);
	}

	ret = kernel_read(filp, buf, size, &offset);
	filp_close(filp, NULL);

	return ret;
}

size_t his_write_file(const char *path, loff_t offset, void *buf, u32 size)
{
	size_t ret = 0;
	struct file *filp = NULL;

	filp = filp_open(path, O_RDWR | O_CREAT, 0644);
	if (IS_ERR(filp)) {
		pr_err("Failed to open %s\n", path);
		return PTR_ERR(filp);
	}

	ret = kernel_write(filp, buf, size, &offset);
	filp_close(filp, NULL);

	return ret;
}

static uint32_t boot_debug_flag;
void set_debug_flag_bit(int set_bit)
{
	boot_debug_flag |= set_bit;
}

void clear_debug_flag_bit(int set_bit)
{
	boot_debug_flag &= ~set_bit;
}

bool get_debug_flag_bit(int get_bit)
{
	if (boot_debug_flag & get_bit)
		return true;
	else
		return false;
}
EXPORT_SYMBOL(get_debug_flag_bit);

DEFINED_DEBUG_EARLY_PARAM(enable_debug, DEBUG_ENABLE_BIT);
DEFINED_DEBUG_EARLY_PARAM(print_active_ws, PRINT_WAKELOCK_BIT);
DEFINED_DEBUG_EARLY_PARAM(serial_enable, SERIAL_ENABLE_BIT);
DEFINED_DEBUG_EARLY_PARAM(audio_debug, AUDIO_DEBUG_BIT);
DEFINED_DEBUG_EARLY_PARAM(debug_fs_switch, FS_DEBUG_BIT);

/* common function */
static void hs_store_attr_value(u32 offset, u32 value)
{
	u32 flag;
	u32 old_value;
	char partition[128] = {0};

	his_get_partition_path(partition, 128, SYS_FLAG_PARTITION);
	his_read_file(partition, offset, &flag, sizeof(flag));
	old_value = flag;
	if (old_value == !!value) {
		pr_info("%s: already set to %u\n", __func__, value);
		return;
	}

	if (value == 1)
		flag = 1;
	else
		flag = 0;

	his_write_file(partition, offset, (void *)&flag, sizeof(flag));
}

/* sysfs attributes for debug, In /sys/debug_control/ directory */
HISENSE_DEBUG_ATTR(serial_console_ctrl, FLAG_ENABLE_SERIAL_CONSOLE_OFFSET);
HISENSE_DEBUG_ATTR(print_sleep_gpio, FLAG_PRINT_SLEEP_GPIO_OFFSET);
HISENSE_DEBUG_ATTR(print_active_ws, FLAG_PRINT_ACTIVE_WS_OFFSET);
HISENSE_DEBUG_ATTR(hs_disable_avb, FLAG_HS_DISABLE_AVB_OFFSET);
HISENSE_DEBUG_ATTR(audio_debug, FLAG_HS_AUDIO_DEBUG_OFFSET);

static struct attribute *his_debug_ctrl_attrs[] = {
	&serial_console_ctrl_attr.attr,
	&print_sleep_gpio_attr.attr,
	&print_active_ws_attr.attr,
	&hs_disable_avb_attr.attr,
	&audio_debug_attr.attr,
	NULL,
};

static struct attribute_group his_debug_ctrl_attr_group = {
	.attrs = his_debug_ctrl_attrs,
};

static int __init hisense_debug_ctrl_init(void)
{
	int ret;

	ret = his_register_sysfs_attr_group(&his_debug_ctrl_attr_group);
	if (ret < 0) {
		pr_err("Error creating poweroff sysfs group, ret=%d\n", ret);
		return ret;
	}

	pr_info("%s: create sysfs ok\n", __func__);

	return 0;
}
late_initcall(hisense_debug_ctrl_init);

