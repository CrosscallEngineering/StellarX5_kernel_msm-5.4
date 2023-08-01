#!/bin/bash

if [ $# -lt 2 ]; then
	echo "Usage:"
	echo "  ./clone_project.sh origin target"
else
	perl ./drivers/soc/hisense/scripts/clone_kernel_project.pl $1 $2
fi
