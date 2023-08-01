LK代码开源处理脚本，使用方法：
1. 首先，拷贝release_code.sh和lk_code_release.pl脚本到LK根目录下
2. 执行代码处理脚本，方法如下（TARGET_PRODUCT为产品名称）：
   ./release_code.sh [TARGET_PRODUCT]
   该脚本会自动执行编译，然后将c文件删除替换为o文件

注意：
   该脚本会删除LK目录下的.repo/.git目录，如果调试的话，可以先屏蔽掉，方法：
   打开lk_code_release.pl，屏蔽掉最后一行：
   #system("rm -rf .git .repo lk_code_release.pl");
