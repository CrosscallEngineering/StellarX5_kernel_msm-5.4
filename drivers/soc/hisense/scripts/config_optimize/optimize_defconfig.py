
import os
import re
from optparse import OptionParser

def run_optimize_defconfig(options):
    config_list = []
    file_data   = []

    fcfghandle = open(options.cfgfile, "r")
    for line in fcfghandle:
        line = line.strip('\n')
        config_list.append(line)
    fcfghandle.close()
    print("\nDisable config list:")
    print(config_list)

    fdesthandle = open(options.defconfig, "r")
    for line in fdesthandle:
        if line.find('CONFIG_PANIC_TIMEOUT') >= 0:
            line = "CONFIG_PANIC_TIMEOUT=1\n"
            file_data.append(line)
            continue

        for config in config_list:
            ret = line.find(config)
            if ret >= 0:
                line = "# " + config + " is not set\n"
                config_list.remove(config)
                break
        file_data.append(line) 
    fdesthandle.close()
    # write data to file
    foutputhandle = open(options.defconfig, "w+")
    foutputhandle.writelines(file_data)
    foutputhandle.close()

def main():
    usage = "usage: %prog [options]"
    parser = OptionParser(usage)
    parser.add_option('-f', '--file', dest='defconfig', help='defconfig file patch')
    parser.add_option('-c', '--config', dest='cfgfile', help='config file path')

    (options, args) = parser.parse_args()
    if not os.path.exists(options.defconfig):
        print("Error: file(%s) not exist" % options.defconfig)
        return

    if not os.path.exists(options.cfgfile):
        print("Error: file(%s) not exist" % options.cfgfile)
        return

    run_optimize_defconfig(options)
    print("\n====== optimize defconfig success ======")

if __name__ == '__main__':
    main()
