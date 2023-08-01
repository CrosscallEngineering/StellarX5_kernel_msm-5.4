#!/bin/bash

if [ $# -lt 1 ]; then
	echo "Usage:"
	echo "  ./optimize_config.sh arch/arm64/configs/defconfig"
else
	python ./drivers/soc/hisense/scripts/config_optimize/optimize_defconfig.py \
		-f $1 \
		-c ./drivers/soc/hisense/scripts/config_optimize/config_optimize_list.txt
fi

