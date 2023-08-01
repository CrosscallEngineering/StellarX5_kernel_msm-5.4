/*
 * Copyright (C) 2016-2017 Hisense, Inc.
 *
 * Author:
 *   qiuxudong <qiuxudong@hisense.com>
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
#define pr_fmt(fmt) "Hisfuse: " fmt

#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/utsname.h>
#include <linux/export.h>
#include <linux/debugfs.h>
#include <linux/atomic.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/io.h>
#include <linux/productinfo.h>
#include <linux/his_debug_base.h>

#define SECBOOT_BUF_SIZE       32

struct cpu_fused_status {
	int cpu_id_reg;
	int fuse_reg;
	int cpuid_valid_num;
	int fuse_valid_num;
};

static struct cpu_fused_status fuse_status;

static uint cpu_reg_read(u32 add)
{
	void __iomem *base;
	uint r_value;

	base = ioremap(add, 4);
	if (!base) {
		pr_err("%s: Error read cpu register\n", __func__);
		return 0;
	}
	r_value = __raw_readl(base);
	iounmap(base);

	return r_value;
}
static int read_soc_cpuid(u32 *cpu_id)
{
	//u32 state = 0;
	u32 *cpuid_value = NULL;

	cpuid_value = cpu_id;
	//state = cpu_reg_read(fuse_status.cpu_id_reg);

	//*cpuid_value = fuse_status.cpuid_valid_num & state;
	*cpuid_value = fuse_status.cpu_id_reg;
	if (!(*cpuid_value)) {
		pr_err("%s: Error read cpu_id\n", __func__);
		return -ENXIO;
	}

	return 0;
}

#ifdef CONFIG_HISENSE_PRODUCT_DEVINFO
static void cpuid_set_into_productinfo(void)
{
	u32 cpu_id = 0;
	int ret = 0;
	char cpu_id_product_info[12];

	ret = read_soc_cpuid(&cpu_id);
	if (ret != 0)
		pr_err("%s: Error read cpu_id\n", __func__);

	snprintf(cpu_id_product_info, sizeof(cpu_id_product_info),
				"0x%08x", cpu_id);
	productinfo_register(PRODUCTINFO_CPU_ID, cpu_id_product_info, NULL);
}

static void fuse_set_into_productinfo(void)
{
	uint fuse = 0;
	bool fuse_is_exist = false;
	char fuse_id_product_info[6];
	u32 valid_num = fuse_status.fuse_valid_num;

	fuse = cpu_reg_read(fuse_status.fuse_reg);

	fuse_is_exist = (valid_num == fuse) ? true : false;
	if (fuse_is_exist)
		snprintf(fuse_id_product_info,
				sizeof(fuse_id_product_info), "True");
	else
		snprintf(fuse_id_product_info,
				sizeof(fuse_id_product_info), "False");

	productinfo_register(PRODUCTINFO_FUSE_ID, fuse_id_product_info, NULL);
}
#else  /* CONFIG_HISENSE_PRODUCT_DEVINFO */
static void cpuid_set_into_productinfo(void) { return; }
static void fuse_set_into_productinfo(void) { return; }
#endif /* CONFIG_HISENSE_PRODUCT_DEVINFO */

static ssize_t cpuid_dbgfs_read_cpuid(struct file *file, char __user *ubuf,
				       size_t count, loff_t *ppos)
{
	char *buf = NULL;
	u32 cpu_id = 0;
	int ret = 0;
	unsigned int buf_size = 0;
	unsigned int bytes_written = 0;

	buf = kzalloc(sizeof(char) * SECBOOT_BUF_SIZE, GFP_KERNEL);
	if (!buf) {
		pr_err("%s: Error allocating memory\n", __func__);
		return -ENOMEM;
	}

	ret = read_soc_cpuid(&cpu_id);
	if (ret != 0) {
		pr_err("%s: Error read cpu_id\n", __func__);
		kfree(buf);
		return -ENXIO;
	}
	buf_size = ksize(buf);
	bytes_written = scnprintf(buf, buf_size, "%08x", cpu_id);
	ret = simple_read_from_buffer(ubuf, count, ppos, buf, bytes_written);
	kfree(buf);

	return ret;
}

static ssize_t fuse_dbgfs_read_fuse(struct file *file, char __user *ubuf,
				       size_t count, loff_t *ppos)
{
	char *buf = NULL;
	uint fuse = 0;
	int ret = 0;
	bool fuse_is_exist = false;
	unsigned int buf_size = 0;
	unsigned int bytes_written = 0;
	u32 valid_num = fuse_status.fuse_valid_num;

	buf = kzalloc(sizeof(char) * SECBOOT_BUF_SIZE, GFP_KERNEL);
	if (!buf) {
		pr_err("%s: Error allocating memory\n", __func__);
		return -ENOMEM;
	}

	fuse = cpu_reg_read(fuse_status.fuse_reg);
	fuse_is_exist = (valid_num == fuse) ? true : false;

	buf_size = ksize(buf);
	bytes_written = scnprintf(buf, buf_size, "%d\n", fuse_is_exist);
	ret = simple_read_from_buffer(ubuf, count, ppos, buf, bytes_written);
	kfree(buf);

	return ret;
}

const struct file_operations cpuid_dbgfs_cpuid_ops = {
	.read = cpuid_dbgfs_read_cpuid,
};
const struct file_operations fuse_dbgfs_fuse_ops = {
	.read = fuse_dbgfs_read_fuse,
};

int cpuid_procfs_init(void)
{
	int ret = -EPERM;

	cpuid_set_into_productinfo();
	ret = his_create_procfs_file("cpu_id", 0444, &cpuid_dbgfs_cpuid_ops);
	if (ret < 0)
		return -ENOMEM;

	return 0;
}

int fuse_procfs_init(void)
{
	int ret = -EPERM;

	fuse_set_into_productinfo();
	ret = his_create_procfs_file("fuse_status", 0444, &fuse_dbgfs_fuse_ops);
	if (ret < 0)
		return -ENOMEM;

	return 0;
}

static int cpu_fused_parse_dt(struct device *dev)
{
	int ret = 0;
	struct device_node *pnode;

	pnode = dev->of_node;
	ret = of_property_read_u32(pnode, "qcom,cpuid-reg",
			&fuse_status.cpu_id_reg);
	if (ret && ret != -EINVAL) {
		pr_err("%s: read cpuid-reg error\n", __func__);
		goto error;
	}

	ret = of_property_read_u32(pnode, "qcom,fuse-reg",
			&fuse_status.fuse_reg);
	if (ret && ret != -EINVAL) {
		pr_err("%s: read fuse-reg error\n", __func__);
		goto error;
	}

	ret = of_property_read_u32(pnode, "qcom,cpuid-valid-num",
			&fuse_status.cpuid_valid_num);
	if (ret && ret != -EINVAL) {
		pr_err("%s: read cpuid-valid-num error\n", __func__);
		goto error;
	}

	ret = of_property_read_u32(pnode, "qcom,fuse-valid-num",
			&fuse_status.fuse_valid_num);
	if (ret && ret != -EINVAL) {
		pr_err("%s: read fuse-valid-num error\n", __func__);
		goto error;
	}

	return 0;
error:
	return ret;
}

static int cpu_fused_probe(struct platform_device *pdev)
{
	int ret = 0;

	pr_err("enter %s\n", __func__);
	if (!pdev->dev.of_node) {
		pr_err("can not find of_node of device\n");
		return -EINVAL;
	}

	ret = cpu_fused_parse_dt(&pdev->dev);
	if (ret < 0)
		return -EINVAL;

	ret = cpuid_procfs_init();
	if (ret)
		return -EPERM;

	ret = fuse_procfs_init();
	if (ret)
		return -EPERM;

	return 0;
}

static int cpu_fused_remove(struct platform_device *pdev)
{
	pr_info("done fuse_status_exit exit\n");

	return 0;
}

#ifdef CONFIG_OF
static const struct of_device_id cpu_fused_of_match[] = {
	{ .compatible = "qcom,cpu-fused", },
	{},
};
#endif

static struct platform_driver cpu_fused_driver = {
	.probe = cpu_fused_probe,
	.remove = cpu_fused_remove,
	.driver = {
		.name = "fused_status_driver",
#ifdef CONFIG_OF
		.of_match_table	= of_match_ptr(cpu_fused_of_match),
#endif
	}
};

static int __init fuse_status_init(void)
{
	int ret = 0;

	ret = platform_driver_register(&cpu_fused_driver);
	return ret;
}

static void fuse_status_exit(void)
{
	platform_driver_unregister(&cpu_fused_driver);
}

module_init(fuse_status_init);
module_exit(fuse_status_exit);

MODULE_DESCRIPTION("Hisense efuse info");
MODULE_LICENSE("GPL v2");
