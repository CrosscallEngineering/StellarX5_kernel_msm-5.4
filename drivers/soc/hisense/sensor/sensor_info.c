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

static int ProductInfoRegister(void)
{
#if defined(CONFIG_HS_SNS_ADSP_ACC)
	productinfo_register(PRODUCTINFO_SENSOR_ACCELEROMETER_ID,  CONFIG_ACC_SENSOR_TYPE,CONFIG_ACC_SENSOR_VENDOR);
#endif
#if defined(CONFIG_HS_SNS_ADSP_GYRO)
	productinfo_register(PRODUCTINFO_SENSOR_GYRO_ID, CONFIG_GYRO_SENSOR_TYPE,CONFIG_GYRO_SENSOR_VENDOR);
#endif
#if defined(CONFIG_HS_SNS_ADSP_MAG)
	productinfo_register(PRODUCTINFO_SENSOR_COMPASS_ID, CONFIG_MAG_SENSOR_TYPE, CONFIG_MAG_SENSOR_VENDOR);
#endif
#if defined(CONFIG_HS_SNS_ADSP_ALSPS)
	productinfo_register(PRODUCTINFO_SENSOR_ALSPS_ID, CONFIG_ALSP_SENSOR_TYPE, CONFIG_ALSP_SENSOR_VENDOR);
#endif
#if defined(CONFIG_HS_ADSP_HALL)
	productinfo_register(PRODUCTINFO_SENSOR_HALL_ID, CONFIG_HALL_SENSOR_TYPE, CONFIG_HALL_SENSOR_VENDOR);
#endif
#if defined(CONFIG_HS_SNS_ADSP_ALS)
	productinfo_register(PRODUCTINFO_SENSOR_ALS_ID, CONFIG_ALS_SENSOR_TYPE, CONFIG_ALS_SENSOR_VENDOR);
#endif
#if defined(CONFIG_HS_SNS_ADSP_PS)
	productinfo_register(PRODUCTINFO_SENSOR_PS_ID, CONFIG_PS_SENSOR_TYPE, CONFIG_PS_SENSOR_VENDOR);
#endif
#if defined(CONFIG_HS_SNS_ADSP_BARO)
	productinfo_register(PRODUCTINFO_SENSOR_BARO_ID, CONFIG_BARO_SENSOR_TYPE, CONFIG_BARO_SENSOR_VENDOR);
#endif
#if defined(CONFIG_HS_SNS_ADSP_TEM_HUM)
	productinfo_register(PRODUCTINFO_SENSOR_TEM_HUM_ID, CONFIG_TEM_HUM_SENSOR_TYPE, CONFIG_TEM_HUM_SENSOR_VENDOR);
#endif
#if defined(CONFIG_HS_SNS_ADSP_HR)
	productinfo_register(PRODUCTINFO_SENSOR_HR_ID, CONFIG_HR_SENSOR_TYPE, CONFIG_HR_SENSOR_VENDOR);
#endif
#if defined(CONFIG_HS_SNS_ADSP_SAR)
	productinfo_register(PRODUCTINFO_SENSOR_SAR_ID, CONFIG_SAR_SENSOR_TYPE, CONFIG_SAR_SENSOR_VENDOR);
#endif
	return 0;
}

static int __init sensor_productinfo_init(void)
{
	ProductInfoRegister();
	return 0;
}
module_init(sensor_productinfo_init);

