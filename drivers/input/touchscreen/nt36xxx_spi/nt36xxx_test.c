#include <linux/delay.h>
#include <linux/firmware.h>
#include "nt36xxx.h"
#include "nt36xxx_test.h"
#include <linux/gpio.h>

#if NVT_TOUCH_ESD_PROTECT
#include <linux/jiffies.h>
#endif /* #if NVT_TOUCH_ESD_PROTECT */
#ifdef CONFIG_HISENSE_PRODUCT_DEVINFO
#include <linux/productinfo.h>
#endif /* CONFIG_HISENSE_PRODUCT_DEVINFO */

extern struct nvt_ts_data *ts;

static int factory_get_fs_fw_version(struct device *dev, char *buf)
{
	int ret = 0, fw_ver = 0;

	// request bin file in "/etc/firmware"
	ret = update_firmware_request(BOOT_UPDATE_FIRMWARE_NAME);
	if (ret) {
		NVT_ERR("update_firmware_request failed. (%d)\n", ret);
		return -EAGAIN;
	}
	fw_ver = Get_FW_Ver();
	update_firmware_release();
	
	return snprintf(buf, PAGE_SIZE, "0x%02X\n", fw_ver);
}

static int factory_check_fw_update_need(struct device *dev)
{
	return 0;
}

static int factory_proc_fw_update(struct device *dev, bool force)
{
	return 0;
}

static int factory_proc_fw_bin_update(struct device *dev, const char *buf)
{
	return 0;
}

static int factory_get_calibration_ret(struct device *dev)
{
	return 1;
}

static int factory_get_ic_fw_version(struct device *dev, char *buf)
{
	struct nvt_ts_data *data = dev_get_drvdata(dev);

	return snprintf(buf, PAGE_SIZE, "0x%02X\n", data->fw_ver);
}

static int factory_get_module_id(struct device *dev, char *buf)
{
	struct nvt_ts_data *data = dev_get_drvdata(dev);
	
	return snprintf(buf, PAGE_SIZE, "0x%02X\n", data->pannel_id);
}

static int factory_get_rawdata_info(struct device *dev, char *buf)
{
	struct nvt_ts_data *data = dev_get_drvdata(dev);

	return snprintf(buf, PAGE_SIZE, "RX:%d TX:%d HIGH:%d LOW:%d\n",
			data->x_num, data->y_num, 0, 0);
}

static int factory_get_rawdata(struct device *dev, char *buf)
{
	//struct nvt_ts_data *data = dev_get_drvdata(dev);

	return nvt_hs_get_rawdata(buf);
}

static int factory_get_diff(struct device *dev, char *buf)
{
	struct nvt_ts_data *data = dev_get_drvdata(dev);

	return nvt_hs_get_diff(data, buf);
}

static int factory_proc_hibernate_test(struct device *dev)
{
	if (nvt_check_fw_reset_state(RESET_STATE_NORMAL_RUN))
	{
 		NVT_ERR("check fw reset state failed!\n");
		return 0;
	}
	return 1;
}

static int factory_get_factory_info(struct device *dev, char *buf)
{
	struct nvt_ts_data *data = dev_get_drvdata(dev);

	return snprintf(buf, PAGE_SIZE, "%s\n", data->factory_info);
}

static bool get_tp_enable_switch(struct device *dev)
{
	NVT_LOG("enable = %d", bTouchIsAwake);
	return bTouchIsAwake;
}

static int set_tp_enable_switch(struct device *dev, bool enable)
{
	int retval = 0;
	struct nvt_ts_data *data = dev_get_drvdata(dev);

	NVT_LOG("set_enable = %d", enable);
	if (enable) {
		retval = nvt_ts_resume(&data->client->dev);
		if (retval)
			NVT_ERR("failed to enter active power mode");
	} else {
		retval = nvt_ts_suspend(&data->client->dev);
		if (retval)
			NVT_ERR("failed to enter low power mode");
	}

	return retval;
}

#ifdef CONFIG_TOUCHSCREEN_NT36XXX_GESTURE
static unsigned int asic_to_hex(unsigned char val)
{
	if((val >= '0') && (val <= '9')){
		val -= '0';
		}
	else if((val >= 'a') && (val <= 'z')){
		val = val - 'a' + 10;
		}
	else if((val >= 'A') && (val <= 'Z')){
		val = val - 'A' + 10;
		}
	return (unsigned int)val;
}

static bool get_gesture_switch(struct device *dev)
{
	struct nvt_ts_data *data = dev_get_drvdata(dev);

	NVT_LOG("gesture_enable = %d\n", data->gesture_enable);
	return data->gesture_enable;
}

static int set_gesture_switch(struct device *dev, const char *buf)
{
	struct nvt_ts_data *data = dev_get_drvdata(dev);
	unsigned char gesture[10], len;

	if(!bTouchIsAwake){
		NVT_LOG("Touch screen is suspending, can not set gesture!\n");
		return 0;
	}

	strlcpy(gesture, buf, sizeof(gesture));
	len = strlen(gesture);
	if (len > 0) {
		if((gesture[len-1] == '\n') || (gesture[len-1] == '\0')){
			len--;
		}
	}

	if (len == 1) {
		if (gesture[0] == '1')
			data->gesture_state = 0xffff;
		else if (gesture[0] == '0')
			data->gesture_state = 0x0;
	} else if(len == 4) {
		data->gesture_state = asic_to_hex(gesture[0])*0x1000
						+ asic_to_hex(gesture[1]) * 0x100
						+ asic_to_hex(gesture[2]) * 0x10
						+ asic_to_hex(gesture[3]);
	} else {
		NVT_ERR("[set_gesture_switch]write wrong cmd.");
		return 0;
	}
	if (!data->gesture_state)
		data->gesture_enable = false;
	else
		data->gesture_enable = true;

	NVT_LOG("gesture_enable = %d\n", data->gesture_enable);

	return 0;
}
#endif

#ifdef CONFIG_TOUCHSCREEN_NT36XXX_GLOVE
static bool get_glove_switch(struct device *dev)
{
	struct nvt_ts_data *ts = dev_get_drvdata(dev);

	NVT_LOG("glove_enable = %d\n", ts->glove_enable);
	return ts->glove_enable;
}

static int set_glove_switch(struct device *dev, bool enable)
{
	struct nvt_ts_data *ts = dev_get_drvdata(dev);

	NVT_LOG("glove_enable = %d\n", ts->glove_enable);

	return (nvt_set_glove_switch(enable));
}
#endif

static int factory_selftest(struct device *dev, char *buf)
{
	return nvt_hs_selftest_open(buf);
}

static int factory_get_fw_update_progress(struct device *dev)
{
	struct nvt_ts_data *data = dev_get_drvdata(dev);
	return data->fw_updating;
}

static int factory_get_chip_type(struct device *dev, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "nt36xxx\n");
}

static bool factory_get_test_config_need(struct device *dev)
{
	return false;
}

static int factory_set_test_config_path(struct device *dev, const char *buf)
{
	return false;
}

static int factory_get_chip_settings(struct device *dev, char *buf)
{
	struct nvt_ts_data *data = dev_get_drvdata(dev);
	char temp[400]= {0};
	char temp1[300] = {0};

	NVT_LOG("enter");
	strlcpy(temp, "\n### nt36xxx ###\n", sizeof(temp));
	snprintf(temp1, ARRAY_SIZE(temp1), "IC_FW_version=0x%02X\nIC_host_version=", data->fw_ver);
	strlcat(temp, temp1, sizeof(temp));
	factory_get_fs_fw_version(dev, temp1);
	strlcat(temp, temp1, sizeof(temp));
	strcpy(buf,temp);
	NVT_LOG(" leave");
	return snprintf(buf, PAGE_SIZE, "%s\n\n", buf);
}

#ifdef CONFIG_NVT_INCELL_CHIP
static int nvt_provide_reset_control(struct device *dev)
{
	struct nvt_ts_data *data;

	if (NT_Client == NULL)
		return -ENODEV;

	data = dev_get_drvdata(&NT_Client->dev);
	if (gpio_is_valid(data->reset_gpio)) {
		gpio_set_value_cansleep(data->reset_gpio, 0);
		msleep(5);
		gpio_set_value_cansleep(data->reset_gpio, 1);
	}
	return 0;
}

static int nvt_suspend_need_lcd_reset_high(void)
{
	struct nvt_ts_data *data;

	if (NT_Client == NULL)
		return -ENODEV;

	data = dev_get_drvdata(&NT_Client->dev);
	return data->keep_lcd_suspend_reset_high;
}

int nvt_need_lcd_power_reset_keep_flag_get(struct device *dev)
{
	struct nvt_ts_data *data;

	if (NT_Client == NULL)
		return -ENODEV;

	data = dev_get_drvdata(&NT_Client->dev);
	return data->gesture_enable;
}

/*******************************************************
Description:
	Novatek touchscreen driver suspend function.

return:
	Executive outcomes. 0---succeed.
*******************************************************/
int32_t nvt_suspend_for_lcd_async_use(struct device *dev)
{
	uint8_t buf[4] = {0};

	mutex_lock(&ts->lock);

	NVT_LOG("start\n");

	if (ts->resume_is_running) {
		printk("%s: TP is in work mode, no need do TP async syspend\n", __func__);
		mutex_unlock(&ts->lock);
		return 0;
	}
	bTouchIsAwake = 0;

	if ((WAKEUP_GESTURE == 1) && (ts->gesture_enable == true)) {
		//---write i2c command to enter "wakeup gesture mode"---
		buf[0] = EVENT_MAP_HOST_CMD;
		buf[1] = 0x13;

		CTP_SPI_WRITE(ts->client, buf, 2);

		NVT_LOG("Enabled touch wakeup gesture\n");
	} else {
		//---write i2c command to enter "deep sleep mode"---
		buf[0] = EVENT_MAP_HOST_CMD;
		buf[1] = 0x11;
		CTP_SPI_WRITE(ts->client, buf, 2);
	}
	msleep(50);

	mutex_unlock(&ts->lock);

	NVT_LOG("end\n");

	return 0;
}
#endif

static int nt36xxx_set_erase_flash_test(struct device *dev,bool enable)
{
	return 0;
}

#ifdef CONFIG_TOUCHSCREEN_WORK_MODE
static int get_tp_work_mode(struct device *dev, char *buf)
{
	u8 reg_val;
	u8 reg_addr = 0x4C;
	uint8_t buff[2] = {0};
	int err = 0;
	struct nvt_ts_data *data = dev_get_drvdata(dev);

	if(!bTouchIsAwake)
	{
		return snprintf(buf, PAGE_SIZE, "%s\n", "IDLE");
	} else {
		buff[0] = reg_addr;
		err = CTP_SPI_READ(data->client, buff, 2);
		if (err < 0)
			return snprintf(buf, PAGE_SIZE, "%s\n", "READ MODE ERROR");
		reg_val = buff[1];
		NVT_LOG("read 0x21C4C reg value = %#x\n", reg_val);

		if (reg_val == 0x03)
			return snprintf(buf, PAGE_SIZE, "%s\n", "OPERATING");
		else if (reg_val == 0x04)
			return snprintf(buf, PAGE_SIZE, "%s\n", "MONITOR");
		else
			return snprintf(buf, PAGE_SIZE, "%s\n", "UNKNOWN ERROR");
	}
}

static int set_tp_work_mode(struct device *dev, const char *mode)
{
	u8 reg_val = 0x00;
	uint8_t buf[4] = {0};
	int err = 0;
	struct nvt_ts_data *data = dev_get_drvdata(dev);

	if (strncmp(mode, "IDLE", 4)==0) {
		if(bTouchIsAwake)
			set_tp_enable_switch(dev, false);
	} else {
		if (strncmp(mode, "OPERATING", 9)==0)
			reg_val = 0xB8;
		else if (strncmp(mode, "MONITOR", 7)==0)
			reg_val = 0xBD;
		else
			return -EFAULT;

		if(!bTouchIsAwake)
			set_tp_enable_switch(dev, true);

		buf[0] = EVENT_MAP_HOST_CMD;
		buf[1] = reg_val;
		err = CTP_SPI_WRITE(data->client, buf, 2);
		if (err < 0) {
			NVT_ERR("write reg failed, [err]=%d\n", err);
			return -EFAULT;
		}
	}
	NVT_LOG("write %#x, tp set to %s work mode\n", reg_val, mode);

	return 0;
}
#endif/*CONFIG_TOUCHSCREEN_WORK_MODE*/

int nt36xxx_factory_ts_func_test_register(struct nvt_ts_data *data)
{
	ts_gen_func_test_init();
	data->ts_test_dev.dev = &data->client->dev;
	data->ts_test_dev.check_fw_update_need = factory_check_fw_update_need;
	data->ts_test_dev.get_calibration_ret = factory_get_calibration_ret;
	data->ts_test_dev.get_fs_fw_version = factory_get_fs_fw_version;
	data->ts_test_dev.get_fw_update_progress = factory_get_fw_update_progress;
	data->ts_test_dev.get_ic_fw_version = factory_get_ic_fw_version;
	data->ts_test_dev.get_module_id = factory_get_module_id;
	data->ts_test_dev.get_rawdata = factory_get_rawdata;
	data->ts_test_dev.get_rawdata_info = factory_get_rawdata_info;
	//data->ts_test_dev.get_rawdata_cc = factory_get_rawdata_cc;
	//data->ts_test_dev.get_noise = factory_get_noise;
	data->ts_test_dev.get_diff = factory_get_diff;
	data->ts_test_dev.proc_fw_update = factory_proc_fw_update;
	data->ts_test_dev.proc_fw_update_with_given_file = factory_proc_fw_bin_update;
	data->ts_test_dev.proc_hibernate_test = factory_proc_hibernate_test;
	data->ts_test_dev.get_factory_info = factory_get_factory_info;
	data->ts_test_dev.get_tp_enable_switch = get_tp_enable_switch;
	data->ts_test_dev.set_tp_enable_switch = set_tp_enable_switch;
#ifdef CONFIG_TOUCHSCREEN_NT36XXX_GESTURE
	data->ts_test_dev.get_gesture_switch = get_gesture_switch;
	data->ts_test_dev.set_gesture_switch = set_gesture_switch;
#endif
#ifdef CONFIG_TOUCHSCREEN_NT36XXX_GLOVE
	data->ts_test_dev.get_glove_switch = get_glove_switch;
	data->ts_test_dev.set_glove_switch = set_glove_switch;
#endif

	//data->ts_test_dev.get_short_test = factory_short_test;
	//data->ts_test_dev.get_open_test = factory_open_test;
	data->ts_test_dev.get_chip_type = factory_get_chip_type;
	data->ts_test_dev.need_test_config = factory_get_test_config_need;
	data->ts_test_dev.set_test_config_path = factory_set_test_config_path;
	data->ts_test_dev.get_tp_settings_info = factory_get_chip_settings;
	data->ts_test_dev.get_selftest = factory_selftest;
#ifdef CONFIG_NVT_INCELL_CHIP
	//data->ts_test_dev.ts_async_suspend_for_lcd_use = nvt_suspend_for_lcd_async_use;
	data->ts_test_dev.ts_reset_for_lcd_use = nvt_provide_reset_control;
	data->ts_test_dev.ts_suspend_need_lcd_reset_high = nvt_suspend_need_lcd_reset_high;
#endif
#if defined(CONFIG_NVT_INCELL_CHIP) && defined(CONFIG_TOUCHSCREEN_NT36XXX_GESTURE)
	data->ts_test_dev.ts_suspend_need_lcd_power_reset_high = nvt_need_lcd_power_reset_keep_flag_get;
#endif
	data->ts_test_dev.set_erase_flash_test = nt36xxx_set_erase_flash_test;
#ifdef CONFIG_TOUCHSCREEN_WORK_MODE
	data->ts_test_dev.get_tp_work_mode = get_tp_work_mode;
	data->ts_test_dev.set_tp_work_mode = set_tp_work_mode;
#endif

	register_ts_func_test_device(&data->ts_test_dev);
	return 0;
}

void nt36xxx_ts_register_productinfo(struct nvt_ts_data *ts_data)
{
    /* format as flow: version:0x01 Module id:0x57 */
	char deviceinfo[64];
	ts_data->pannel_id = 0;

	snprintf(deviceinfo, ARRAY_SIZE(deviceinfo), "FW version:0x%02X Module id:0x%02x",
		ts_data->fw_ver, ts_data->pannel_id);

#ifdef CONFIG_HISENSE_PRODUCT_DEVINFO
	productinfo_register(PRODUCTINFO_CTP_ID, NULL, deviceinfo);
#endif /* CONFIG_HISENSE_PRODUCT_DEVINFO */
}

#ifdef CONFIG_HISENSE_PRODUCT_DEVINFO
void nt36xxx_ts_register_productinfo_wq(struct work_struct *work)
{
	char deviceinfo[64];
	ts->pannel_id = 0;

	snprintf(deviceinfo, ARRAY_SIZE(deviceinfo), "FW version:0x%02X Module id:0x%02x",
		ts->fw_ver, ts->pannel_id);

	productinfo_register(PRODUCTINFO_CTP_ID, NULL, deviceinfo);
	//NVT_LOG("LK--- FW version:0x%02X\n", ts->fw_ver);
}
#endif /* CONFIG_HISENSE_PRODUCT_DEVINFO */