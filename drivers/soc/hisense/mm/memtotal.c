#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/memory.h>
#include <linux/mm.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/his_debug_base.h>

unsigned long get_hs_total_ram(void)
{
	long ram_size;

	ram_size = dev_bi.ddr_size;
	if ((ram_size > SZ_1G) && ((ram_size % SZ_1G) > 0))
		ram_size = ((ram_size / SZ_1G) + 1) * SZ_1G;

	pr_debug("Device ddr size is %ld\n", ram_size);
	return ram_size;
}
EXPORT_SYMBOL(get_hs_total_ram);

static int memostotal_proc_show(struct seq_file *m, void *v)
{
	__kernel_ulong_t total = totalram_pages();

#ifdef CONFIG_QCOM_MEM_OFFLINE
	total= get_totalram_pages_count_inc_offlined();
#endif
	/*
	 * Tagged format, for easy grepping and expansion.
	 */
	seq_printf(m, "MemOsTotal:       %8lu kB\n",
			((total) << (PAGE_SHIFT - 10)));

	return 0;
}

static int memostotal_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, memostotal_proc_show, NULL);
}

static const struct file_operations memostotal_proc_fops = {
	.open           = memostotal_proc_open,
	.read           = seq_read,
	.llseek         = seq_lseek,
	.release        = single_release,
};

static int __init proc_memtotal_init(void)
{
	int ret = -EPERM;

	ret = his_create_procfs_file("memostotal", S_IRUGO, &memostotal_proc_fops);
	if (ret < 0)
		printk("proc create memostotal failed !\n");
	return ret;
}
fs_initcall(proc_memtotal_init);

