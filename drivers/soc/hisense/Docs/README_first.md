### 1. 找主设计添加公共库到产品manifest.xml

```xml
<project name="hmct_platform/Kernel_BaseCode/Kernel_BaseCode.git" path="kernel/msm-4.14/drivers/soc/hisense"/>
```

### 2. 将公共库编译到当前内核

- 添加头文件Include路径(根目录下Makefile)

```makefile
LINUXINCLUDE += -I$(srctree)/drivers/soc/hisense/include
```
- 添加hisense目录编译（drivers/soc下）

```
Kconfig:
  source "drivers/soc/hisense/Kconfig"
Makefile:
  obj-$(CONFIG_MACH_HISENSE_SMARTPHONE) += hisense/
```

### 3. 产品和单编脚本添加

- 添加toolchain，使用当前android下的工具，注意使用git add -f命令添加
- 产品dtb添加，注意dtbo-base命名和base defconfig保持一致，如720T为sm6150base.dts/sm6150base_defconfig
- 单编脚本mkbootimg.sh添加，具体的打包参数通过make showcommands bootimage获得

### 4. 添加BSP公共代码编译

```
CONFIG_HISENSE_BOOT_INFO
CONFIG_SAVE_AWAKEN_EVENT
CONFIG_HISENSE_DEBUG_CTRL
CONFIG_HISENSE_SUSPEND_SYS_SYNC
```

### 5. 在产品dts中配置产品信息

```
// For example：
his_devinfo {
    status = "okay";

    /* The number of name and value MUST be same */
    dev,keymap-names = "POWER", "DOWN", "UP";
    dev,keymap-values = <116>, <114>, <115>;
    /* trustzone protect gpios */
    dev,prot-gpios = <0  1  2  3>;
};
```

### 6. 创建公共脚本软连接

将clone_project.sh、open_souce.sh、optimize_config.sh脚本软连接到内核代码根目录下

执行命令如下：
```shell
ln -s drivers/soc/hisense/scripts/link_scripts/clone_project.sh clone_project.sh
ln -s drivers/soc/hisense/scripts/link_scripts/open_souce.sh open_souce.sh
ln -s drivers/soc/hisense/scripts/link_scripts/optimize_config.sh optimize_config.sh
```

### 公共库代码合入规则

- 共用性，适用于所有产品
- 芯片、平台无关性
- 可移植行，能够在不同的内核版本上使用
- 可配置性，针对产品不同，可以通过dts或其他方式来配置

