
enum {
	FW_UPGRADE_FAILED = 0,
	FW_UPGRADE_SUCCESS,
	FW_IS_UPDATETING,
};
extern uint8_t bTouchIsAwake;
extern int nt36xxx_factory_ts_func_test_register(struct nvt_ts_data *data);
extern void nt36xxx_ts_register_productinfo(struct nvt_ts_data *ts_data);
//extern int nvt_hs_short_test(struct nvt_ts_data *data);
//extern int nvt_hs_open_test(struct nvt_ts_data *data);
extern int nvt_hs_get_rawdata(char *buf);
//extern int nvt_hs_get_rawdata_cc(struct nvt_ts_data *data,  char *buf);
//extern int nvt_hs_get_noise(struct nvt_ts_data *data,  char *buf);
extern int nvt_hs_get_diff(struct nvt_ts_data *data,  char *buf);
extern int32_t nvt_hs_selftest_open(char *buf);
extern int32_t update_firmware_request(char *filename);
//extern int32_t update_firmware_request_with_given_bin(char *filename);
//extern int32_t Check_CheckSum(void);
//extern int32_t Update_Firmware(void);
//extern int32_t Update_Firmware_with_given_bin(void);
//extern int32_t Init_BootLoader(void);
//extern int32_t Resume_PD(void);
//extern int32_t Erase_Flash(void);
//extern int32_t Check_FW_Ver(void);
extern void update_firmware_release(void);
//extern void update_firmware_release_with_given_bin(void);
//extern int32_t nvt_check_flash_end_flag(void);
extern int Get_FW_Ver(void);//
extern int32_t nvt_ts_suspend(struct device *dev);//
extern int32_t nvt_ts_resume(struct device *dev);//
//extern int32_t nvt_glove_enable(uint8_t enable);
extern int32_t nvt_set_glove_switch(uint8_t glove_switch);

extern int32_t nvt_get_fw_info(void);//
