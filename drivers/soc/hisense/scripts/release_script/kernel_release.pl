#!/usr/bin/perl -w

use strict;
use File::Find;
use File::Basename;
use Cwd;

if (@ARGV < 3)
{
	print "\n\tError:\n";
	print "\t   Please input the project name, arch platform. e.g.\n";
	print "\t   ./kernel_release.pl t91 arm64 sdm845base\n\n";
	exit(1);
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

# start release code flow
my $project = lc(shift @ARGV);
# arm or arm64
my $arch = lc(shift @ARGV);
my $platbase = lc(shift @ARGV);
my $defconfig = "";
my $release_defconfig = "";
my $product_defconfig = "";
my $kout = "./kout";
my $scripts_dir = "./drivers/soc/hisense/scripts/release_script";

my @deletecomment;
my %special_config_list;
my @objconfig_list;

sub read_release_list_file {
	open TMPLIST, "< $scripts_dir/delete_comment_file.lst" or
				die "open delete_comment_file.lst failed\n";
	while(<TMPLIST>) {
		chomp;
		next if (/^\s*#.*/);

		if (/[^\s]/) {
			push(@deletecomment, $_);
		}
	}
	close TMPLIST;

	open TMPLIST, "< $scripts_dir/special_config_name.lst" or
				die "open special_config_name.lst file failed\n";
	while(<TMPLIST>) {
		chomp;
		next if (/^\s*#.*/);

		if (/(\w+)\s+(\d+)/) {
			$special_config_list{"$1"} = $2;
		}
	}
	close TMPLIST;

	open TMPLIST, "< $scripts_dir/release_obj_config.lst" or
				die "open release_obj_file.lst failed\n";
	while(<TMPLIST>) {
		chomp;
		next if (/^\s*#.*/);

		if (/[^\s]/) {
			push(@objconfig_list, $_);
		}
	}
	close TMPLIST;
}

# -----------------------------------------------------------------------------
# log record for flow
# -----------------------------------------------------------------------------
open LOGFILE, "> code_release.log" or die "open log file failed\n";

sub log_print {
	my ($logstr) = @_;

	print $logstr;
#	print LOGFILE $logstr;
}

# -----------------------------------------------------------------------------
# common function: find string from the path
# -----------------------------------------------------------------------------
sub find_string_in_path {
	my ($name, $path) = @_;
	system("grep -rlE --exclude-dir=.git --exclude-dir=.repo \"$name\" $path > tmpfilelist");
}

sub find_string_in_Makefile {
	my ($name) = @_;
	my $ignore = "-name .repo -prune -o -name .git -prune -o -path ./kout -prune -o";

	system("find . $ignore -name 'Makefile' -type f -exec grep $name {} + > tmpfilelist");
}

# -----------------------------------------------------------------------------
# find the product defconfig
# -----------------------------------------------------------------------------
sub find_product_defconfig {
	log_print("\tProduct defconfig\n");
	$defconfig = `find ./arch/$arch/configs/vendor -name ${platbase}_defconfig`;
	chomp($defconfig);
	log_print("\t".$defconfig."\n");
	$release_defconfig = `find ./arch/$arch/configs/vendor -name ${platbase}-perf_defconfig`;
	chomp($release_defconfig);
	log_print("\t".$release_defconfig."\n\n");
	$product_defconfig = `find ./arch/$arch/configs -name ${project}_defconfig`;
	chomp($product_defconfig);
	log_print("\t".$product_defconfig."\n\n");
}

# -----------------------------------------------------------------------------
# remove other unrelated project
# -----------------------------------------------------------------------------
sub remove_other_defconfig {
	find_string_in_path("CONFIG_HISENSE_PRODUCT_NAME", "./arch/$arch/configs/");
	open TMPLIST, "< tmpfilelist" or die "open defconfig tmpfilelist failed\n";
	while (<TMPLIST>)
	{
		chomp;
		next if (/$project/);
		next if (/$platbase/);
		unlink($_);
	}
	close TMPLIST;
}

sub remove_other_project {
	my $dtspath = "./arch/$arch/boot/dts/qcom/hisense";

	log_print("\tremove other project start\n");
	opendir DTS, "$dtspath" or die "open dts failed\n";
	foreach (readdir DTS) {
		my $name = $_;
		next if ($name eq ".");
		next if ($name eq "..");
		next if ($name eq "Makefile");
		next if (/$project/);
		next if (/$platbase/);

		system("rm -rf $dtspath/$_");
	}
	closedir DTS;

	remove_other_defconfig();

	# delete build*.sh scripts
	system("find ./ -maxdepth 1 -name \"build*$project.sh\" -print | xargs -I \'{}\' mv \'{}\' temp_build.sh");
	system("find ./ -maxdepth 1 -name \"build*.sh\" -print | xargs rm -f");
	system("mv temp_build.sh build_$project.sh");

	# delete bootimg directory files
	system("rm -rf bootimg/adb* bootimg/fastboot* bootimg/Adb*");

	log_print("\tremove other project end\n\n");
}

# -----------------------------------------------------------------------------
# remove file comments from special file
# -----------------------------------------------------------------------------
sub remove_file_comments {
	my (@flist) = @_;

	log_print("\tremove file comments start\n");
	foreach (@flist) {
		my $fname = $_;
		my $first = 1;
		my $mflag = 0; # multi line comment flag

		log_print("\t\tdelete comment from $fname\n");
		open INFILE, "< $fname" or die "open $fname failed\n";
		open OUTFILE, "> $fname.bak" or die "open $fname.bak failed\n";
		while (<INFILE>) {
			my $line = $_;
			# first comment not delete, maybe copyright
			if ($first == 1) {
				print OUTFILE $line;
				$first = 0 if ($line =~ /.*\*\//);
				next;
			}

			if ($mflag == 1) {
				$mflag = 0 if ($line =~ /.*\*\//);
				next;
			} else {
				if ($line =~ /\/\*.*\*\//) {
					$line =~ s/\s*\/\*.*\*\///g;
				} elsif ($line =~ /\/\*.*[^\/]\s*$/) {
					$mflag = 1;
					$line =~ s/\s*\/*.*//g;
				} else {
					$line =~ s/\s*\/\/.*//g;
				}
			}
			print OUTFILE $line;
		}
		close OUTFILE;
		close INFILE;

		rename("$fname.bak", $fname);
	}

	log_print("\tremove file comments end\n\n");
}

# -----------------------------------------------------------------------------
# remove config infomation from code file (*.c *.h)
# -----------------------------------------------------------------------------
sub remove_config_context {
	my $ifdef_flag = 0;
	my $ifndef_flag = 0;
	my $has_else = 0;
	my ($fname, $cname, $state) = @_;

	log_print("\t\tdelete $cname related code from $fname\n");
	open IN, "< $fname" or die "open code file failed\n";
	open OUT, "> $fname.bak" or die "open code out file failed\n";
	while (<IN>) {
		my $line = $_;

		if ($ifdef_flag == 0 and $ifndef_flag == 0) {
			if ($line =~ /\s*#ifdef\s+$cname\b/) {
				$ifdef_flag = 1;
				next;
			} elsif ($line =~ /\s*#ifndef\s+$cname\b/) {
				$ifndef_flag = 1;
				next;
			} elsif ($line =~ /\s*#if\s+defined\s*\(?\s*$cname\b\s*\)?/) {
				$ifdef_flag = 1;
				next;
			} elsif ($line =~ /\s*#if\s+!\s*defined\s*\(?\s*$cname\b\s*\)?/) {
				$ifndef_flag = 1;
				next;
			} else {
				print OUT $line;
			}
		} elsif (($ifdef_flag == 1 and $state == 1) or ($ifndef_flag == 1 and $state == 0)) {
			if ($has_else == 0 and $line =~ /\s*#else\s*\/\*\s*$cname\b.*/) {
				$has_else = 1;
			} elsif (/\s*#endif\s*\/\*\s*$cname\b.*/) {
				$ifdef_flag = 0;
				$ifndef_flag = 0;
				$has_else = 0;
			} elsif ($has_else == 1) {
				print OUT $line;
			}
		} else {
			if ($has_else == 0 and $line =~ /\s*#else\s*\/\*\s*$cname\b.*/) {
				$has_else = 1;
			} elsif (/\s*#endif\s*\/\*\s*$cname\b.*/) {
				$ifdef_flag = 0;
				$ifndef_flag = 0;
				$has_else = 0;
			} else {
				next if ($has_else == 1);
				print OUT $line;
			}
		}
	}
	close OUT;
	close IN;

	rename ("$fname.bak", $fname);
}

sub remove_config_in_code {
	my (%clist) = @_;

	log_print("\tremove config in code start\n");
	foreach my $config (keys %clist) {
		my $state = $clist{$config};

		find_string_in_path("ifn?(def)?\\s+!?(defined\\s*)?\\(?$config\\b", "./");
		open TMPLIST, "< tmpfilelist" or die "open tmpfilelist fail\n";
		while (<TMPLIST>) {
			chomp;
			remove_config_context($_, $config, $state);
		}
		close TMPLIST;
	}

	log_print("\tremove config in code end\n\n");
}

# -----------------------------------------------------------------------------
# remove config infomation from defconfig
# -----------------------------------------------------------------------------
sub remove_config_in_defconfig {
	my (@clist) = @_;

	log_print("\tremove config in defconfig start\n");
	foreach my $config (@clist) {
		log_print("\t\tdelete $config from defconfig\n");
		open IN, "< $defconfig" or die "remove_config_in_defconfig in failed\n";
		open OUT, "> $defconfig.bak" or die "remove_config_in_defconfig out failed\n";
		while (<IN>) {
			next if (/$config/);
			print OUT $_;
		}
		close OUT;
		close IN;

		rename("$defconfig.bak", $defconfig);
	}

	log_print("\tremove config in defconfig end\n\n");
}

sub remove_config_in_release_defconfig {
	my (@clist) = @_;

	log_print("\tremove config in release_defconfig start\n");
	foreach my $config (@clist) {
		log_print("\t\tdelete $config from release_defconfig\n");
		open IN, "< $release_defconfig" or die "remove_config_in_release_defconfig in failed\n";
		open OUT, "> $release_defconfig.bak" or die "remove_config_in_release_defconfig out failed\n";
		while (<IN>) {
			next if (/$config/);
			print OUT $_;
		}
		close OUT;
		close IN;

		rename("$release_defconfig.bak", $release_defconfig);
	}

	log_print("\tremove config in release_defconfig end\n\n");
}

sub remove_config_in_product_defconfig {
	my (@clist) = @_;

	log_print("\tremove config in product_defconfig start\n");
	foreach my $config (@clist) {
		log_print("\t\tdelete $config from product_defconfig\n");
		open IN, "< $product_defconfig" or die "remove_config_in_product_defconfig in failed\n";
		open OUT, "> $product_defconfig.bak" or die "remove_config_in_product_defconfig out failed\n";
		while (<IN>) {
			next if (/$config/);
			print OUT $_;
		}
		close OUT;
		close IN;

		rename("$product_defconfig.bak", $product_defconfig);
	}

	log_print("\tremove config in product_defconfig end\n\n");
}
# -----------------------------------------------------------------------------
# remove config infomation from Kconfig
# -----------------------------------------------------------------------------
sub remove_config_define {
	my ($fname, $config) = @_;
	my $findit = 0;

	log_print("\t\tDelete $config from $fname\n");
	open IN, "< $fname" or die "open $fname failed\n";
	open OUT, "> $fname.bak" or die "open $fname.bak failed\n";
	while (<IN>) {
		my $line = $_;

		if ($line =~ /^\s*config\s+$config\s*$/) {
			$findit = 1;
			next;
		}
		elsif ($findit == 1)
		{
			if ($line =~ /^config\s*.*/ or $line =~ /^endif/
				or $line =~ /^menuconfig/ or $line =~ /^if/
				or $line =~ /^menu/ or $line =~ /endmenu/
				or $line =~ /^source\s+/) {
				print OUT $line;
				$findit = 0;
			}
			next;
		}

		print OUT $line;
	}

	close OUT;
	close IN;

	rename("$fname.bak", $fname);
}

# -----------------------------------------------------------------------------
# replace config infomation from Makefile
# -----------------------------------------------------------------------------
sub replace_config_to_y_in_Makefile {
	my ($fname, $config) = @_;
	my $findit = 0;

	log_print("\t\tReplace $config from $fname\n");
	open IN, "< $fname" or die "open $fname failed\n";
	open OUT, "> $fname.bak" or die "open $fname.bak failed\n";
	while (<IN>) {
		my $line = $_;
		$line =~ s/\$\($config\)/y/;
		print OUT $line;
		next;
	}

	close OUT;
	close IN;

	rename("$fname.bak", $fname);
}

sub remove_config_in_Kconfig {
	my (@clist) = @_;

	log_print("\tremove config in Kconfig start\n");
	foreach (@clist) {
		s/CONFIG_//;
		my $config = $_;

		find_string_in_path("config\\s+$config", "./");
		open TMPLIST, "tmpfilelist" or die "open tmpfilelist fail\n";
		while (<TMPLIST>) {
			chomp;
			remove_config_define($_, $config);
		}
		close TMPLIST;
	}

	log_print("\tremove config in Kconfig end\n\n");
}

sub delete_include_kconfig {
	my ($fpath, $name) = @_;

	open KCFG, "< $fpath/Kconfig" or die "open last Kconfig failed.";
	open KOUT, "> $fpath/Kconfig.bak" or die "open Kconfig.bak failed\n";
	while (<KCFG>) {
		next if (/$name\/Kconfig/);

		print KOUT $_;
	}
	close KOUT;
	close KCFG;

	rename ("$fpath/Kconfig.bak", "$fpath/Kconfig");
}

# -----------------------------------------------------------------------------
# delete config infomation from Makefile
# -----------------------------------------------------------------------------
sub remove_file_or_dir {
	my ($fname, $config) = @_;
	my $fpath = dirname($fname);

	# check target exsit
	if (-e $fname) {
		open IN, "< $fname" or die "open $fname failed\n";
		open OUT, "> $fname.bak" or die "open $fname.bak failed\n";
		while (<IN>) {
			if (/$config.*=\s*([\w-]+)\/([\w-]+)\.o/) {
				log_print("\t\trm file $fpath/$1/$2.c\n");
				system("rm -f $fpath/$1/$2.c");
			} elsif (/$config.*=\s*([\w-]+)\//) {
				log_print("\t\trm directory $fpath/$1\n");
				system("rm -fr $fpath/$1");
				delete_include_kconfig($fpath, $1);
			} elsif (/$config.*=\s+([\w-]+)\.o/) {
				log_print("\t\trm file $fpath/$1.c\n");
				system("rm -f $fpath/$1.c");
			} else {
				print OUT $_;
			}
		}
		close OUT;
		close IN;

		rename ("$fname.bak", $fname);
	}
}

sub remove_config_in_Makefile {
	my (%clist) = @_;

	log_print("\tremove config in Makefile start\n");
	foreach my $config (keys %clist) {
		my $state = $clist{$config};
		if ($state == 1) {
			find_string_in_path("\\($config\\)", "./");
			open TMPLIST, "tmpfilelist" or die "open tmpfilelist fail\n";
			while (<TMPLIST>) {
				chomp;
				remove_file_or_dir($_, $config);
			}

			close TMPLIST;
		} else {
			find_string_in_path("\\($config\\)", "./");
			open TMPLIST, "tmpfilelist" or die "open tmpfilelist fail\n";
			while (<TMPLIST>) {
				chomp;
				replace_config_to_y_in_Makefile($_, $config);
			}

			close TMPLIST;
		}
	}

	log_print("\tremove config in Makefile end\n\n");
}

# -----------------------------------------------------------------------------
# delete special config by special_config_list
# -----------------------------------------------------------------------------
sub remove_special_config {
	my @clist = keys %special_config_list;

	#remove_config_in_defconfig(@clist);

	#remove_config_in_release_defconfig(@clist);

	#remove_config_in_product_defconfig(@clist);

	remove_config_in_Kconfig(@clist);

	remove_config_in_Makefile(%special_config_list);

	remove_config_in_code(%special_config_list);
}

# ------------------------------------------------------------------------------
# build project and use obj to replace source
# ------------------------------------------------------------------------------
sub replace_src_use_obj {
	my ($result) = @_;
	my @source;

	if($result =~ /^(.*)\/Makefile:\s*obj.*\s*+=\s*([a-zA-Z0-9_-]+)\.o\s*$/) {
		print "$1 ==> $2\n";
		system("cp $kout/$1/$2.o $1/$2.obj");
		system("rm -f $1/$2.c");
	} elsif ($result =~ /^(.*)\/Makefile:\s*obj.*\s*+=\s*([a-zA-Z0-9_-]+)\//) {
		print "$1 ==> $2\n";
		@source =`ls $1/$2/*.c`;
		foreach (@source) {
			my $name = $_;
			if ($name =~ /(.*)\.c/) {
				system("cp $kout/$1.o $1.obj");
				system("rm -f $1.c");
			}
		}
	}
}

sub release_obj_files {
	if (@objconfig_list > 0) {
		system("./build_$project.sh");

		foreach (@objconfig_list) {
			find_string_in_Makefile($_);
			open TMPLIST, "tmpfilelist" or die "open tmpfilelist fail\n";
			while (<TMPLIST>) {
				chomp;
				replace_src_use_obj($_);
			}

			close TMPLIST;
		}
	}
}

# -----------------------------------------------------------------------------
# delete version control system, .git or .repo
# -----------------------------------------------------------------------------
sub remove_vcs_directory {
	log_print("\tremove .git or .repo directory\n\n");
	system("rm -rf .git .repo");
	system("rm -rf ".$scripts_dir);
}

# -----------------------------------------------------------------------------
# main function
# -----------------------------------------------------------------------------
sub main {
	log_print("\nStart release code for $project\n");

	#find_product_defconfig();

	read_release_list_file();

	#remove_other_project();

	remove_special_config();

	remove_file_comments(@deletecomment);

	release_obj_files();

	#remove_vcs_directory();

	log_print("kernel code release complete, Please compile and check\n\n");
}

main();

system("rm -f tmpfilelist");
close LOGFILE;

