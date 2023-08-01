/*
 * Copyright (C) 2008-2014 Hisense, Inc.
 *
 * Author:
 *   kongzhiqiang <kongzhiqiang@hisense.com>
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

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/of.h>
#include <linux/string.h>
#include <linux/input.h>
#include <linux/pm.h>
#include <asm/setup.h>
#include <linux/his_debug_base.h>
#include <linux/awaken_sys_event.h>

#define DEV_MAX_KEY_NUM     8
#define SYS_BOOT_INFO       "kbootinfo"

struct device_keymap {
	int key_num;

	struct keymap {
		int code;
		const char *name;
	} keymap[DEV_MAX_KEY_NUM];
};

#define BOOTINFO_ATTR(_name) \
static struct kobj_attribute _name##_attr = { \
	.attr   = {	                              \
		.name = __stringify(_name),	          \
		.mode = 0444,                         \
	},                                        \
	.show   = _name##_show,	                  \
	.store  = NULL,	                          \
}

char poweron_reason[POWER_REASON_LEN] = {0};
char poweroff_reason[POWER_REASON_LEN] = {0};

struct device_bootinfo dev_bi;
static struct device_keymap dev_kmap;

static void of_get_device_keymap(struct device_node *np)
{
	int i = 0;
	int size = 0;

	size = of_property_count_strings(np, "dev,keymap-names");
	if ((size <= 0) || (size > DEV_MAX_KEY_NUM)) {
		pr_buf_err("device keymap size is error\n");
		return;
	}

	pr_err("found Key num: %d\n", size);
	dev_kmap.key_num = size;
	for (i = 0; i < dev_kmap.key_num; i++) {
		of_property_read_string_index(np, "dev,keymap-names",
				i, &dev_kmap.keymap[i].name);

		of_property_read_u32_index(np, "dev,keymap-values",
				i, &dev_kmap.keymap[i].code);
	}
}

static void of_parse_device_info(void)
{
	struct device_node *np = NULL;

	np = of_find_node_by_path("/soc/his_devinfo");
	if (!np) {
		np = of_find_node_by_path("/his_devinfo");
		if (!np) {
			pr_buf_err("Can not find his_devinfo node\n");
			return;
		}
	}

	/* get the partition path */
	dev_bi.parti_path = of_get_property(np, "dev,parti-path", NULL);

	of_get_device_keymap(np);

	his_of_get_u32_array(np, "dev,prot-gpios", dev_bi.prot_gpios,
			&dev_bi.prot_num, PROTECTED_GPIO_NUM);
}

char *his_get_partition_path(char *path, int size, char *pname)
{
	int len = 0;

	len = strlcpy(path, dev_bi.parti_path, size);
	strlcat(path, pname, size - len);

	return path;
}

void input_print_keyevent_log(u32 type, u32 code, int value)
{
	int i = 0;

	if (type != EV_KEY)
		return;

	if (dev_kmap.key_num == 0) {
		pr_buf_info("%d key %s\n", code,
				value ? "pressed" : "released");
	} else {
		for (i = 0; i < dev_kmap.key_num; i++) {
			if (dev_kmap.keymap[i].code == code) {
				pr_buf_info("%s key %s\n", dev_kmap.keymap[i].name,
						value ? "pressed" : "released");
				break;
			}
		}
	}

}

inline void his_set_curr_backlight_state(u32 bl_level)
{
	static int last_level;

	/* print lcd backlight control log when turn on or off */
	if ((last_level == 0) || (bl_level == 0)) {
		pr_buf_info("Backlight: set lcd backlight from %d to %d\n",
				last_level, bl_level);
		last_level = bl_level;

		if (bl_level == 0) {
			dev_bi.backlight_on = 0;
		} else {
			dev_bi.backlight_on = 1;
#if IS_ENABLED(CONFIG_SAVE_AWAKEN_EVENT)
			update_backlight_on_time();
#endif /* CONFIG_SAVE_AWAKEN_EVENT */
		}
	}
}
EXPORT_SYMBOL(his_set_curr_backlight_state);

int __init alarm_normal_pon_setup(char *s)
{
	if (!strcmp(s, "1"))
		dev_bi.alarm_mode = 1;

	pr_buf_err("%s: alarm_ahead_enable = %d\n", __func__, dev_bi.alarm_mode);

	return 1;
}
__setup("alarm_ahead.enable=", alarm_normal_pon_setup);

int __init boot_charger_mode_init(char *s)
{
	if (!strcmp(s, "charger"))
		dev_bi.bootmode = BOOT_CHARGER_MODE;
	else if (!strcmp(s, "factory2"))
		dev_bi.bootmode = BOOT_FACTORY_MODE;
	else if (!strcmp(s, "recovery"))
		dev_bi.bootmode = BOOT_RECOVERY_MODE;
	else if (!strcmp(s, "cali"))
		dev_bi.bootmode = BOOT_CALI_MODE;
	else if (!strcmp(s, "silence"))
		dev_bi.bootmode = BOOT_SILENCE_MODE;
	else
		dev_bi.bootmode = BOOT_NORMAL_MODE;

	pr_err("%s: buffer= %s bootmode= %d\n", __func__, s, dev_bi.bootmode);

	return 0;
}
__setup("androidboot.mode=", boot_charger_mode_init);

static int __init powerup_reason_setup(char *p)
{
	strlcpy(poweron_reason, p, POWER_REASON_LEN);
	pr_err("%s: poweron_reason= %s\n", __func__, poweron_reason);

	return 0;
}
early_param("powerup_reason", powerup_reason_setup);

static ssize_t powerup_reason_show(struct kobject *kobj,
		struct kobj_attribute *attr, char *buf)
{
	int ret;

	ret = snprintf(buf, POWER_REASON_LEN, "%s\n", poweron_reason);

	return ret;
}
BOOTINFO_ATTR(powerup_reason);

static int __init poweroff_reason_setup(char *p)
{
	strlcpy(poweroff_reason, p, POWER_REASON_LEN);
	pr_err("%s: poweroff_reason= %s\n", __func__, poweroff_reason);

	return 0;
}
early_param("poweroff_reason", poweroff_reason_setup);

static ssize_t poweroff_reason_show(struct kobject *kobj,
		struct kobj_attribute *attr, char *buf)
{
	int ret;

	ret = snprintf(buf, POWER_REASON_LEN, "%s\n", poweroff_reason);

	return ret;
}
BOOTINFO_ATTR(poweroff_reason);

static int __init board_uid_setup(char *p)
{
	if ((*p >= '0') && (*p <= '9'))
		dev_bi.board_id = *p - '0';

	pr_err("%s: board_uid= %d\n", __func__, dev_bi.board_id);

	return 0;
}
early_param("androidboot.boardid", board_uid_setup);

static ssize_t board_id_show(struct kobject *kobj,
		struct kobj_attribute *attr, char *buf)
{
	char *s = buf;

	s += snprintf(s, 4, "%d", dev_bi.board_id);

	return s-buf;
}
BOOTINFO_ATTR(board_id);

static int __init encrypt_status_setup(char *p)
{
	if (!strcmp(p, "0"))
		dev_bi.phone_is_encrypt = 0;
	else
		dev_bi.phone_is_encrypt = 1;

	pr_err("%s: encrypt_status= %d\n", __func__,
			dev_bi.phone_is_encrypt);

	return 0;
}
early_param("androidboot.encrypt", encrypt_status_setup);

static ssize_t encrypt_status_show(struct kobject *kobj,
		struct kobj_attribute *attr, char *buf)
{
	int ret;

	ret = snprintf(buf, 16, "%d\n", dev_bi.phone_is_encrypt);

	return ret;
}
BOOTINFO_ATTR(encrypt_status);

static struct attribute *bootattr[] = {
	&poweroff_reason_attr.attr,
	&powerup_reason_attr.attr,
	&board_id_attr.attr,
	&encrypt_status_attr.attr,
	NULL,
};

static struct attribute_group bootattr_group = {
	.attrs = bootattr,
};

/* Battery Voltage in bootloader stage beyond high threshold(3800mV)
Add for TF update in safty battery status, because-of ADSP charger
hard limiting in recovery */
static int __init battvoltage_safety_setup(char *p)
{
	if (!strcmp(p, "0"))
		dev_bi.battvoltage_safety = 0;
	else
		dev_bi.battvoltage_safety = 1;

	pr_err("%s: battery_voltage_safety= %d\n", __func__,
			dev_bi.battvoltage_safety);

	return 0;
}
early_param("battvoltage_safety", battvoltage_safety_setup);

static ssize_t battvoltage_safety_show(struct kobject *kobj,
		struct kobj_attribute *attr, char *buf)
{
	int ret;

	ret = snprintf(buf, 20, "%d\n", dev_bi.battvoltage_safety);

	return ret;
}
static struct kobj_attribute battvoltage_safety_attr = __ATTR_RO(battvoltage_safety);

static struct attribute *boot_dbcontrol_attrs[] = {
	&battvoltage_safety_attr.attr,
	NULL,
};

static struct attribute_group boot_dbcontrol_group = {
	.attrs = boot_dbcontrol_attrs,
};

static void print_device_bootinfo(void)
{
	pr_err("===========================\n");
	pr_err("Boot mode:   %d\n", dev_bi.bootmode);
	pr_err("Alarm mode:  %d\n", dev_bi.alarm_mode);
	pr_err("DDR size:    %lld\n", dev_bi.ddr_size);
	pr_err("sector num:  %lld\n", dev_bi.sectors_num);
	pr_err("Sector size: %d\n", dev_bi.sector_size);
	pr_err("===========================\n");
}

static int __init bootinfo_init(void)
{
	int ret = -ENOMEM;
	struct kobject *bootinfo_kobj = NULL;

	of_parse_device_info();
	bootinfo_kobj = kobject_create_and_add(SYS_BOOT_INFO, NULL);
	if (bootinfo_kobj == NULL) {
		pr_err("bootinfo_init: bootinfo kobject create failed\n");
		return ret;
	}

	ret = sysfs_create_group(bootinfo_kobj, &bootattr_group);
	if (ret) {
		pr_err("bootinfo_init: bootattr group create failed\n");
		goto sys_fail;
	}

	ret = his_register_sysfs_attr_group(&boot_dbcontrol_group);
	if (ret)
	    pr_err("bootinfo_init: boot dbcontrol group creat failed\n");

	print_device_bootinfo();
	return ret;

sys_fail:
	kobject_del(bootinfo_kobj);
	return ret;
}
late_initcall(bootinfo_init);
