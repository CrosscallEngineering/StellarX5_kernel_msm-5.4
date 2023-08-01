
#ifndef __AWAKEN_SYSTEM_EVENT_H__
#define __AWAKEN_SYSTEM_EVENT_H__

enum {
	AWAKEN_EVENT_NONE,
	AWAKEN_EVENT_PWRKEY,
	AWAKEN_EVENT_FP,
	AWAKEN_EVENT_TP_GESTURE,
	AWAKEN_EVENT_RESIN,
	AWAKEN_EVENT_PSENSOR,
	AWAKEN_EVENT_KEYAI,
	AWAKEN_EVENT_KEYUP,
	AWAKEN_EVENT_MAX,
	AWAKEN_EVENT_UNKNOWN
};

#define WAKEUP_TIME_COUNT        5

struct awaken_event {
	int event;
	/* timestamp in nanoseconds */
	u64 recv_nsec;
	u64 lcd_start_nsec;
	u64 lcd_end_nsec;
	u64 bl_on_nsec;
	u64 timebank[WAKEUP_TIME_COUNT];
};

extern void set_awaken_event_by_irqnum(int irq_number);
extern void clear_awaken_system_event(void);

extern int register_irqnum_and_event(int irq_number, int event);
extern int get_current_awaken_event(void);

extern void update_backlight_on_time(void);
extern void update_screen_init_time(int start);

#endif /* __AWAKEN_SYSTEM_EVENT_H__ */
