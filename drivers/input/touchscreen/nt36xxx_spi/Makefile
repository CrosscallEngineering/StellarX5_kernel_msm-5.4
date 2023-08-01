#
# Makefile for the Novatek NT36xxx touchscreen driver.
#

# Each configuration option enables a list of files.

obj-$(CONFIG_TOUCHSCREEN_NT36xxx_HOSTDL_SPI) += nt36xxx_spi.o

nt36xxx_spi-objs += nt36xxx.o 
nt36xxx_spi-objs += nt36xxx_fw_update.o 
nt36xxx_spi-objs += nt36xxx_ext_proc.o 
nt36xxx_spi-objs += nt36xxx_mp_ctrlram.o 
nt36xxx_spi-objs += nt36xxx_test.o 
nt36xxx_spi-objs += ../ts_func_test.o
