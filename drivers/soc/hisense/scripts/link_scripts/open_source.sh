#!/bin/bash

HS_SCRIPTS_DIR=drivers/soc/hisense/scripts

if [ $# -lt 3 ]; then
	echo "Usage:"
	echo "  ./open_source.sh project arch platform_base"
	echo "  ./open_source.sh t91 arm64 sdm845base"
else
	perl $HS_SCRIPTS_DIR/release_script/kernel_release.pl $1 $2 $3
fi
