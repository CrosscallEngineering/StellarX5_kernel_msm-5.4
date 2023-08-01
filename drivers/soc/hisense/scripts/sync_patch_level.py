
import re
import os

check_file = "../../build/make/core/version_defaults.mk"
update_file = "./scripts/mkbootimg.sh"
os_version = re.compile("PLATFORM_VERSION\.\w+\s+:=\s+((\d)(\.\d)?(\.\d)?)")
patch_level = re.compile("\s+PLATFORM_SECURITY_PATCH\s+:=\s+(\d+-\d+-\d+)")

new_version = ""
new_level = ""

def update_kernel_info():
    filedata = ""
    with open(update_file, "r+") as f:
        for line in f:
            if line.find("ANDROID_INFO") == 0:
                line = "ANDROID_INFO=\" --os_version " + new_version + " --os_patch_level " + new_level
                line = line + " --header_version 1 " + "\"\n";
            filedata += line

        f.close()

    with open(update_file, "w+") as f:
        f.writelines(filedata)
        f.close()

def parse_current_info():
    global new_version
    global new_level
    ret = os.path.exists(check_file)
    if ret:
        filp = open(check_file, "r")
        for line in filp:
            result1 = os_version.match(line)
            if result1:
                new_version = result1.group(1)

            result2 = patch_level.match(line)
            if result2:
                new_level = result2.group(1)

        filp.close()

    print("Current version: " + new_version)
    print("Current patch level: " + new_level)

def main():
    parse_current_info()

    update_kernel_info()

if __name__ == '__main__':
    main()

