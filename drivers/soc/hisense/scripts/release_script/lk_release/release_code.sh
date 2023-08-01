
PRODUCT_NAME=$1

cd ../../../

. build/envsetup.sh
choosecombo 1 $PRODUCT_NAME 2

cd -

perl lk_code_release.pl $PRODUCT_NAME

