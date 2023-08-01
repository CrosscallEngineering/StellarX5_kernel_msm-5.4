
#ifndef __KDEBUG_PARTI_LAYOUT_H__
#define __KDEBUG_PARTI_LAYOUT_H__

#define SYS_DEBUG_PARTITION          "kdebug"

/* kdebug partition offset definition by module use */
#define CRASH_INFO_OFFSET_IN_KDEBUG  (0)
#define CRASH_INFO_SIZE_IN_KDEBUG    (0x100000)
#define RS_RECORDER_OFFSET_IN_KDEBUG (CRASH_INFO_OFFSET_IN_KDEBUG +\
                                      CRASH_INFO_SIZE_IN_KDEBUG)
#define PON_REASON_OFFSET_IN_KDEBUG  (0x500000)
#define STATS_INFO_OFFSET_IN_KDEBUG  (0x800000)
#define STATS_INFO_SIZE_IN_KDEBUG    (0x100000)

#endif /* __KDEBUG_PARTI_LAYOUT_H__ */

