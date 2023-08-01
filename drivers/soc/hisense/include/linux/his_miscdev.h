
#ifndef __HIS_MISCDEV_H__
#define __HIS_MISCDEV_H__

extern struct kobject *his_register_miscdev_dir(const char *miscname);
extern int his_register_miscdev_attr(struct attribute *attr);

#endif /* __HIS_MISCDEV_H__ */

