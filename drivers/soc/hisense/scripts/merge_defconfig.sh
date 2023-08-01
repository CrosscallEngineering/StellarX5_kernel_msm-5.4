#!/bin/sh

# find the product defconfig
if [ -z $TARGET_PRODUCT ]; then
	echo Error: TARGET_PRODUCT is not defined
	exit 1
fi
echo TARGET_PRODUCT is ${TARGET_PRODUCT}

PRODUCT_CONFIG=`find ${srctree}/arch/arm64/configs -iname "${TARGET_PRODUCT}_defconfig" | xargs basename`
if [ -z $PRODUCT_CONFIG ]; then
	echo Error: PRODUCT_CONFIG is not finded
	exit 1
fi
echo PRODUCT_CONFIG is $PRODUCT_CONFIG

#=============================================================
DEFAULT_CONFIG=$1
if [ -z $DEFAULT_CONFIG ]; then
	echo Error: DEFAULT_CONFIG is not finded
	exit 1
fi
echo DEFAULT_CONFIG is ${DEFAULT_CONFIG}

BASE_CONF=${srctree}/arch/arm64/configs/${DEFAULT_CONFIG}
PROD_CONF=${srctree}/arch/arm64/configs/${PRODUCT_CONFIG}
DEST_CONF=${srctree}/arch/arm64/configs/.merged_defconfig

# start to merge defconfig
echo start to merge defconfig
cat ${BASE_CONF} > ${DEST_CONF}
echo -e "\n" >> ${DEST_CONF}
cat ${PROD_CONF} >> ${DEST_CONF}
echo merge end

