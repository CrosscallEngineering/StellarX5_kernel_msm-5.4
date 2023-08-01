/*
 * Copyright (C) 2013-2014 Hisense, Inc.
 *
 * Author:
 *   zhaoyufeng <zhaoyufeng@hisense.com>
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

#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/utsname.h>
#include <linux/export.h>
#include <linux/productinfo.h>
#include <linux/module.h>
#include <linux/his_debug_base.h>

#define PRODUCTINFO_BUFF_LEN  200

const char *deviceclassname[PRODUCTINFO_ID_NUM] = {
	"Vendor",
	"Board and product",
	"Hardware version",
	"LCD",
	"HWTCONFW",
	"HWTCONBL",
	"EPDVCOM",
	"CTP",
	"SUB CTP",
	"HDMI",
	"Main camera",
	"Sub camera",
	"Macro camera",
	"Wide-angle camera",
	"Front camera",
	"Scan camera",
	"DDR",
	"EMMC",
	"EMMC more",
	"UFS",
	"UFS more",
	"NAND",
	"Accelerometer sensor",
	"Compass sensor",
	"Alps sensor",
	"BT",
	"WIFI",
	"Codec",
	"Modem",
	"LED",
	"Main anx camera",
	"Gyro sensor",
	"Hall sensor",
	"CPU_ID",
	"Fused",
	"FingerPrint",
	"NFC",
	"ALS_1",
	"ALS_2",
	"EPD",
	"Barometer",
	"Temperature_Humidity",
	"SAR sensor",
	"HIFI",
	"Blood oxygen heart rate sensor",
	"Thermometer sensor"
};

struct productinfo_struct {
	int   used;
	char  productinfo_data[PRODUCTINFO_BUFF_LEN];
};

struct productinfo_struct productinfo_data[PRODUCTINFO_ID_NUM];
char *productinfo_data_ptr;

int productinfo_register(int id, const char *devname, const char *devinfo)
{
	int len = 0;

	if (id >= PRODUCTINFO_ID_NUM)
		return -ENOMEM;

	if (!deviceclassname[id])
		return -ENOMEM;

	len = strlen(deviceclassname[id]);
	if (devname)
		len += strlen(devname);

	if (devinfo)
		len += strlen(devinfo);

	if (len >= PRODUCTINFO_BUFF_LEN - 5)
		return -ENOMEM;

	memset(productinfo_data[id].productinfo_data, 0,
			sizeof(productinfo_data[id].productinfo_data));
	productinfo_data_ptr = productinfo_data[id].productinfo_data;
	productinfo_data[id].used = 1;
	strlcat(productinfo_data_ptr, deviceclassname[id], PRODUCTINFO_BUFF_LEN);
	if (devname) {
		strlcat(productinfo_data_ptr, ": ", PRODUCTINFO_BUFF_LEN);
		strlcat(productinfo_data_ptr, devname, PRODUCTINFO_BUFF_LEN);
	}
	if (devinfo) {
		strlcat(productinfo_data_ptr, "--", PRODUCTINFO_BUFF_LEN);
		strlcat(productinfo_data_ptr, devinfo, PRODUCTINFO_BUFF_LEN);
	}
	strlcat(productinfo_data_ptr, "\n", PRODUCTINFO_BUFF_LEN);

	return 0;
}
EXPORT_SYMBOL(productinfo_register);

int productinfo_dump(char *buf, int offset)
{
	int i = 0;
	int len = 0;
	char *ptr = NULL;

	for (i = 0; i < PRODUCTINFO_ID_NUM; i++) {
		if (productinfo_data[i].used) {
			ptr = productinfo_data[i].productinfo_data;
			len = strlen(ptr);
			memcpy(buf + offset, ptr, len);
			offset += len;
		}
	}

	return 0;
}
EXPORT_SYMBOL(productinfo_dump);

char *get_product_hw_info(int id)
{
	return productinfo_data[id].productinfo_data;
}

static int productinfo_proc_show(struct seq_file *m, void *v)
{
	int i;

	for (i = 0; i < PRODUCTINFO_ID_NUM; i++) {
		if (productinfo_data[i].used) {
			productinfo_data_ptr = productinfo_data[i].productinfo_data;
			seq_write(m, productinfo_data_ptr, strlen(productinfo_data_ptr));
		}
	}

	return 0;
}

static int productinfo_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, productinfo_proc_show, NULL);
}

static const struct file_operations productinfo_proc_fops = {
	.open       = productinfo_proc_open,
	.read       = seq_read,
	.llseek     = seq_lseek,
	.release    = single_release,
};

static int __init proc_productinfo_init(void)
{
	int ret = -EPERM;

	ret = his_create_procfs_file("productinfo", S_IRUGO,
			&productinfo_proc_fops);
	if (ret < 0)
		return -ENOMEM;

#ifdef CONFIG_MACH_HISENSE_SMARTPHONE
	productinfo_register(PRODUCTINFO_VENDOR_ID, CONFIG_HISENSE_VENDOR_NAME, NULL);
	productinfo_register(PRODUCTINFO_BOARD_ID, CONFIG_HISENSE_PRODUCT_NAME, NULL);
#endif /* CONFIG_MACH_HISENSE_SMARTPHONE */

	return 0;
}
module_init(proc_productinfo_init);

MODULE_DESCRIPTION("Hisense basic info");
MODULE_LICENSE("GPL v2");
