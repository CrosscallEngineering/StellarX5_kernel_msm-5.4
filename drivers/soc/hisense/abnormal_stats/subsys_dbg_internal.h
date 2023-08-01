
#ifndef __SUBSYS_DBG_INTERNAL_H__
#define __SUBSYS_DBG_INTERNAL_H__

#define SUBSYS_NAME_LEN      8
#define REASON_STR_LEN       128
#define KDEBUG_SAVED_MAGIC   0x5353ACAC

struct subsys_trap_info {
	char name[SUBSYS_NAME_LEN];
	char last_reason[REASON_STR_LEN];
	int count;
	int saved;
};

/* Do not record the subsys exception when debug version */
extern int hs_get_debug_flag(void);

static void set_modem_subsys_reset(char *reset_time) {}

#endif /* __SUBSYS_DBG_INTERNAL_H__ */

