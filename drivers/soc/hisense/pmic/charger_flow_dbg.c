/*
 * Copyright (C) 2020 Hisense, Inc.
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

#define pr_fmt(fmt) "his-chg: " fmt

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/of.h>
#include <linux/string.h>
#include <linux/input.h>
#include <linux/time64.h>
#include <linux/pm.h>
#include <linux/his_debug_base.h>
#include <linux/timekeeping.h>

#define RECORDER_INDEX_MAX  11

struct charge_recorder {
    long recorder_time;
    bool status;
};

struct charger_time {
    long start_time;
    long stop_time;
    struct charge_recorder chr_recorder[RECORDER_INDEX_MAX]; /*Recorde 0,10,20 ... soc time & status*/
};

struct charger_time chr_time = {0};

void charger_log_start_charge(int curr_cap)
{
	struct timespec64 chr_start_ts;
	char time_str[32] = {0};

	memset(&chr_time, 0, sizeof(struct charger_time));

	ktime_get_boottime_ts64(&chr_start_ts);

	chr_time.start_time = chr_start_ts.tv_sec;

	his_get_current_time(time_str, sizeof(time_str));

	pr_buf_err("%s start charge, capacity: %d, start time(s): %ld\n", time_str, curr_cap, chr_time.start_time);
}

void charger_log_stop_charge(int curr_cap)
{
	struct timespec64 chr_stop_ts;
	char time_str[32] = {0};
	long chr_diff_time = 0;

	ktime_get_boottime_ts64(&chr_stop_ts);

	chr_time.stop_time = chr_stop_ts.tv_sec;

	chr_diff_time = chr_time.stop_time - chr_time.start_time;

	his_get_current_time(time_str, sizeof(time_str));

	pr_buf_err("%s stop charge, capacity: %d, charge total time(s): %ld\n", time_str, curr_cap, chr_diff_time);

}

void charger_log_charger_type(char *chgr_type)
{
	pr_buf_err("charger type: %s\n", chgr_type);
}

void charger_log_finish_charge(void)
{
	char time_str[32] = {0};

	his_get_current_time(time_str, sizeof(time_str));
	pr_buf_err("%s finish charge.\n", time_str);
}

void charger_process_time_recorder(bool usb_present , int curr_cap)
{
    int quo = 0, rem = 0;
    struct timespec64 chr_recoder_ts;

    if(!usb_present) {
        return;
    }

    quo = curr_cap/10;
    rem = curr_cap%10;
    //pr_buf_err("debug: rem:%d, quo:%d, status:%d\n", rem, quo, chr_time.chr_recorder[quo].status);

    if(!rem && (chr_time.chr_recorder[quo].status == false)) {
        ktime_get_boottime_ts64(&chr_recoder_ts);
        chr_time.chr_recorder[quo].recorder_time = chr_recoder_ts.tv_sec - chr_time.start_time;
        chr_time.chr_recorder[quo].status = true;

        pr_buf_err("charge recorder, capacity: %d, clock: %ld, start time: %ld, diff time: %ld, index: %d\n",
            curr_cap, chr_recoder_ts.tv_sec, chr_time.start_time, chr_time.chr_recorder[quo].recorder_time, quo);
    }
}

static ssize_t charge_time_recorder_show(struct kobject *kobj,
		struct kobj_attribute *attr, char *buf)
{
    size_t size = 0;
    int i = 0;

    size += snprintf(buf + size, PAGE_SIZE,
            "Capacity   \t    Time(seconds)\n");

    for (i = 0; i < RECORDER_INDEX_MAX; i++) {
        size += snprintf(buf + size, PAGE_SIZE,
                        "%d0 \t  %ld\n",
                        i, chr_time.chr_recorder[i].recorder_time);
    }

    return size;
}
static struct kobj_attribute charge_time_recorder_attr = __ATTR_RO(charge_time_recorder);

static int __init charger_flow_dbg_init(void)
{
	int ret;

	ret = his_register_sysfs_attr(&charge_time_recorder_attr.attr);
	if (ret < 0) {
		pr_err("Error create pon_off_reason file %d\n", ret);
		return ret;
	}

	return 0;
}

late_initcall(charger_flow_dbg_init);
