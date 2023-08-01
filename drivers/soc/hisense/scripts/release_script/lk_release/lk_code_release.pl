#!/usr/bin/perl -w

use strict;
use File::Path;
use File::Copy;
use File::Glob;
use File::Basename;

if ($#ARGV < 0) {
	print "Error: no input parameters\n";
	print "Usage: perl lk_code_release.pl <projectname>\n";
	exit(1);
}

my $project = shift @ARGV;
my $compile_cmd = "make -C ../../../ aboot";

my @otherproject;
my $number = 0;

print "\nPlease configuration the android compile env, ready(y/N)?";
while (<STDIN>) {
	if(/^[Yy]/) {
		last;
	} else {
		print "\tStop script\n";
		exit 0;
	}
}

print "\nRun script will delete .git/.repo directory, are you sure(y/N)?";
while (<STDIN>) {
	if(/^[Yy]/) {
		print "\tRunning\n\n";
		last;
	} else {
		print "\tStop script\n";
		exit 0;
	}
}

# 1. compile lk project
system($compile_cmd);

# 2. copy *.o to app/vendorboot
system("cp ../../../out/target/product/$project/obj/EMMC_BOOTLOADER_OBJ/build-$project/app/vendorboot/*.o app/vendorboot/");

# 3. delete the source code
system("rm app/vendorboot/*.c");
#system("rm app/vendorboot/*.h");

# 4. modify make/build.mk
open BUILDMK, "make/build.mk" or die "open make/build.mk file fail.";
open BUILDBAK, "> make/build.mk.bak" or die "open build.mk.bak file fail.";
my $line_num = 0;
while (<BUILDMK>)
{
    $line_num ++;
    if ($line_num == 2) {
        print BUILDBAK "LIBGCC += \$(PREOBJS)\n"
    }

    print BUILDBAK $_;
}
close(BUILDMK);
close(BUILDBAK);
unlink("make/build.mk");
rename("make/build.mk.bak", "make/build.mk");

# 5. delete other project
opendir MKDIR, "project/" or die "open project directory fail.";
foreach (readdir MKDIR)
{
	my $name = $_;
	next if (!-f "project/$name");

	my ($fname, $fdir, $fsuffix) = fileparse($name, "\.mk");

	open FILEH, "project/$name" or die "open $name fail.";
	while(<FILEH>) {
		if(/HISENSE_BOOTCODE_COMPILE/) {
			if ($fname ne $project) {
				system("rm project/$name");
				$otherproject[$number] = $fname;
				$number ++;
			}
		}
	}
}
closedir(MKDIR);

# delete other project form target directory
foreach my $pname (@otherproject)
{
	system("rm -rf target/$pname");
}

# 6. modify the project makefile
open MKFILE, "project/$project.mk" or die "open $project.mk file fail.";
open TEMPFILE, "> project/$project.mk.bak" or die "open temp file fail.";
while (<MKFILE>)
{
	next if(/HISENSE_BOOTCODE_COMPILE/);
	print TEMPFILE $_;
}
close(MKFILE);
close(TEMPFILE);
unlink("project/$project.mk");
rename("project/$project.mk.bak", "project/$project.mk");

# 7. compile and verify
print "\n\n\n\n\n ------------------------ verify --------------------- \n\n\n\n\n";
system("rm -rf ../../../out/target/product/$project/obj/EMMC_BOOTLOADER_OBJ/");
system($compile_cmd);

# 8. delete .git or .repo directory
system("rm -rf .git .repo lk_code_release.pl");

