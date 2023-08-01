#!/usr/bin/perl -w
#
# Usage:
#   perl clone_kernel_project.pl origin new
#

use strict;
use File::Find;

my $origin = shift @ARGV;
my $new = shift @ARGV;
my $origin_uc = uc $origin;
my $new_uc = uc $new;

my @find_files;
my $find_dir;
my $qcom_new_dts_path = "arch/arm64/boot/dts/vendor/qcom";
my $qcom_lcd_conf_path = "techpack/display/config";
my $qcom_audio_conf_path = "techpack/audio/config";

sub find_defconfig() {
	if (/^${origin}_\w+\.config/) {
		$find_dir = $File::Find::dir;
		push(@find_files, $_);
	}
}

sub modify_file() {
	my ($infile) = @_;

	open IN, "< $infile" or die "open $infile failed\n";
	open OUT, "> temp.file" or die "open temp.file failed\n";
	while (<IN>) {
		my $line = $_;
		if ($line =~ /$origin/) {
			$line =~ s/$origin/$new/;
		} elsif ($line =~ /$origin_uc/) {
			$line =~ s/$origin_uc/$new_uc/;
		}

		print OUT $line;
	}
	close OUT;
	close IN;
	system("mv temp.file $infile");
}

# copy defconfig
find(\&find_defconfig, "arch/arm64/configs/vendor");
foreach my $fname (@find_files) {
	print $fname."\n";
	my $oldname = $fname;
	$fname =~ s/$origin/$new/;
	system("cp $find_dir/$oldname $find_dir/$fname");
	&modify_file("$find_dir/$fname");
}

# copy device tree
if( -e "$qcom_new_dts_path/${origin}-overlay.dts") {
	print "NEW QCOM PROJECT CLONE.\n";
	system("cp -rf $qcom_new_dts_path/$origin $qcom_new_dts_path/$new");
	system("cp $qcom_new_dts_path/${origin}-overlay.dts $qcom_new_dts_path/${new}-overlay.dts");
	&modify_file("$qcom_new_dts_path/${new}-overlay.dts");
}

# copy techpack display config
if ( -e "$qcom_lcd_conf_path/$origin"."disp.conf")
{
	print "QCOM DISPLAY CONF FILE CLONE.\n";
	system("cp -rf $qcom_lcd_conf_path/${origin}disp.conf  $qcom_lcd_conf_path/${new}disp.conf");
	&modify_file("$qcom_lcd_conf_path/${new}disp.conf");
}

if ( -e "$qcom_lcd_conf_path/$origin"."dispconf.h")
{
	print "QCOM DISPLAY CONF.h FILE CLONE.\n";
	system("cp -rf $qcom_lcd_conf_path/${origin}dispconf.h  $qcom_lcd_conf_path/${new}dispconf.h");
	&modify_file("$qcom_lcd_conf_path/${new}dispconf.h");
}

# copy techpack audio config
if ( -e "$qcom_audio_conf_path/$origin"."auto.conf")
{
	print "QCOM AUDIO CONF FILE CLONE.\n";
	system("cp -rf $qcom_audio_conf_path/${origin}auto.conf  $qcom_audio_conf_path/${new}auto.conf");
	&modify_file("$qcom_audio_conf_path/${new}auto.conf");
}

if ( -e "$qcom_audio_conf_path/$origin"."autoconf.h")
{
	print "QCOM AUDIO CONF.h FILE CLONE.\n";
	system("cp -rf $qcom_audio_conf_path/${origin}autoconf.h  $qcom_audio_conf_path/${new}autoconf.h");
	&modify_file("$qcom_audio_conf_path/${new}autoconf.h");
}

print "\n\t 派生完成，请验证！！！\n\n";
