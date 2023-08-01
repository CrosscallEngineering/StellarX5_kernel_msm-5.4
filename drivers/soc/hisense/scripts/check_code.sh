#!/bin/bash

DEFCONFIG=$1
HS_SCRIPTS_DIR=drivers/soc/hisense/scripts

shift
CHECK_TARGET=$*
if [ "$1" = "--output-file" ]; then
	REAL_TARGET=$2
else
	REAL_TARGET=$1
fi

if [ -f $REAL_TARGET ]; then
	touch $REAL_TARGET
elif [ -d $REAL_TARGET ]; then
	rm -f warns.txt
	rm -rf $OUTDIR/$REAL_TARGET
elif [ $REAL_TARGET = "all" ]; then
	rm -f warns.txt
	rm -rf $OUTDIR && mkdir -p $OUTDIR
else
	echo "Please input check target!"
fi

if [ "$REAL_TARGET" = "all" ]; then
	make O=$OUTDIR ARCH=$TARGET_ARCH $DEFCONFIG
	. $HS_SCRIPTS_DIR/smatch/smatch_scripts/check_all_kernel.sh
else
	. $HS_SCRIPTS_DIR/smatch/smatch_scripts/kchecker $CHECK_TARGET
fi

