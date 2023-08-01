#!/usr/bin/perl -w
#
# Created by wangxufeng
# Date: 2017-07-25
#
use File::Basename;

my $build_sh = shift @ARGV;

my $result;
my @modify_files;
my @ignore_files;
my @spec_configs;

my $enable_style_check = 0;
my $enable_code_static_check = 1;
my $enable_ignore_list = 1;

my $scripts_dir = "drivers/soc/hisense/scripts";
my $check_cmd = "./scripts/checkpatch.pl -f ";
my $ignore_list = "$scripts_dir/checkCodestyle/ignore_check_files";

sub get_modified_files_list {
	my @get_files;

	@get_files=`git status -s -uno`;
	foreach (@get_files) {
		chomp;
		if (/\s*[AM]\s+(.*\.[ch])$/) {
			push(@modify_files, $1);
		}
	}
}

sub get_ignore_files_list {
	# enable ignore function
	return if ($enable_ignore_list != 1);

	open IGNORE, "< $ignore_list" or die "open $ignore_list failed\n";

	foreach (<IGNORE>) {
		chomp;
		push(@ignore_files, $_);
	}
	close IGNORE;
}

sub check_in_ignore_list {
	my $ret = 0;
	my ($file) = @_;

	foreach (@ignore_files) {
		my $line = $_;

		if ($file =~ m/$line/) {
			$ret = 1;
			last;
		}
	}

	return ($ret);
}

sub check_linux_code_style {
	my $ret = 0;
	my $output = "result.txt";

	foreach (@modify_files) {
		$cur_file = $_;

		next if (check_in_ignore_list($cur_file) > 0);

		$run_cmd = $check_cmd.$cur_file." > ".$output;

		#print $run_cmd."\n";
		system($run_cmd);

		# check result.txt
		open RESULT, " < $output" or die "open $output failed\n";
		foreach (<RESULT>) {
			if (/^total:\s+(\d+)\s+errors.*$/) {
				if ($1 > 0) {
					$ret = 1;
					close RESULT;
					return ($ret);
				}
			}
		}

		system("rm -f $output");
		close RESULT;
	}

	return ($ret);
}

sub read_spec_configs_list {
	my ($file) = @_;

	open CONFIG, " < $file" or die "open $file failed\n";
	foreach (<CONFIG>) {
		chomp;
		next if (/^\s*#.*/);
		if (/(\w+)\s+\d+/) {
			#print $1."\n";
			push(@spec_configs, $1);
		}
	}
	close CONFIG;
}

sub check_the_config_comment {
	my ($file, $cname) = @_;
	my $ifdef_flag = 0;
	my $ifdef_nest = 0;
	my $ret = 1;

	open INF, "< $file" or die "open $file failed\n";
	foreach (<INF>) {
		my $line = $_;

		if ($ifdef_flag == 0) {
			if (($line =~ /\s*#ifdef\s+$cname\b/)
				or ($line =~ /\s*#ifndef\s+$cname\b/)
				or ($line =~ /\s*#if\s+defined\s*\(?\s*$cname\b\s*\)?/)
				or ($line =~ /\s*#if\s+!\s*defined\s*\(?\s*$cname\b\s*\)?/)) {
				$ifdef_flag = 1;
				$ret = 1;
				next;
			}
		}
		else {
			if (($line =~ /\s*#ifdef\s+.*/)
				or ($line =~ /\s*#ifndef\s+.*/)
				or ($line =~ /\s*#if\s+defined\s*\(?\s*.*\s*\)?/)
				or ($line =~ /\s*#if\s+!\s*defined\s*\(?\s*.*\s*\)?/)) {
				$ifdef_nest = 1;
				next;
			}
			elsif ($line =~ /\s*#else.*/) {
				if ($ifdef_nest == 0) {
					if ($line !~ /\s*#else\s*\/\*\s*$cname\b.*/) {
						$ret = 1;
						last;
					}
				}
			}
			elsif ($line =~ /\s*#endif.*/) {
				if ($ifdef_nest == 0) {
					if ($line =~ /\s*#endif\s*\/\*\s*$cname\b.*/) {
						$ifdef_flag = 0;
						$ifdef_nest = 0;
						$ret = 0;
					}
				}
				else {
					$ifdef_nest = 0;
				}
			}
		}
	}
	close INF;

	return ($ret);
}

sub check_ifdef_endif_comment {
	my $ret = 0;
	my $spec_file = "$scripts_dir/release_script/special_config_name.lst";
	my $output = "find.txt";

	if (-e $spec_file) {
		read_spec_configs_list($spec_file);

		foreach (@spec_configs) {
			my $config = $_;

			foreach (@modify_files) {
				my $cur_file = $_;
				system("grep $config $cur_file > $output");
				if (-s $output) {
					$ret = check_the_config_comment($cur_file, $config);
					if ($ret != 0) {
						print "\n\n\t=== Error ===\n";
						print "\t\tFile: $cur_file\n";
						print "\t\tCONFIG: $config\n";
						print "\t\t  ifdef and endif comment is not match, please check\n\n";
						print "\tFor example:\n";
						print "\t\t#ifdef xxx_xxx\n";
						print "\t\t#else  /* xxx_xxx */\n";
						print "\t\t#endif /* xxx_xxx */\n\n";
						return 1;
					}
				}
			}
		}

		system("rm -f $output")
	}
	else {
		print "\n\n\t=== WARNNING ===\n";
		print "\t\tOpensource scripts is not exsit, skip check\n";
		print "\t\tAfter 2s continue\n\n";
		sleep(2);
		return 0;
	}
}

sub check_static_check_result()
{
	my $ret = 0;
	# report error check rules
	my @check_rules = (
		"error:",
		"warn: variable dereferenced before check",
		"warn: inconsistent returns \'mutex:"
		);

	if (!-e "./warns.txt") {
		return 0;
	}

	open WARN, "< ./warns.txt" or die "open warns.txt failed";
LABEL:
	while (<WARN>) {
		$log = $_;
		for (my $i = 0; $i < scalar(@check_rules); $i++) {
			if ($log =~ /.*:\d+\s+.*\s+$check_rules[$i]\s*.*/) {
				$ret = 1;
				last LABEL;
			}
		}
	}
	close WARN;

	return $ret;
}

sub run_static_code_check()
{
	my $ret = 0;
	my $file_dir;

	system("rm -f warns.txt");

	foreach (@modify_files) {
		$src_file = $_;
		next if (check_in_ignore_list($src_file) > 0);

		if ($src_file =~ /.*\.c\s*$/) {
			$file_dir = dirname($src_file);
			if (-e "$file_dir/Makefile") {
				system("./$build_sh check $src_file");
			}
		}
	}

	$ret = check_static_check_result();
	return ($ret);
}

sub main {
	my $ret = 0;

	get_modified_files_list();

	get_ignore_files_list();

	# check #ifdef...#else...#endif comment
	$ret = check_ifdef_endif_comment();
	if ($ret != 0) {
		return 1;
	}

	# check code style, Rules see ./Documentation/CodingStyle
	if ($enable_style_check == 1) {
		$ret = check_linux_code_style();
		if ($ret != 0) {
			print "\n\n\t=== ERROR ===\n";
			print "\t\tCode style check error, Please check result.txt\n\n";
			print "\t\tThe rules see Documentation/CodingStyle. If the file no need\n";
			print "\t\tto fix, add to $ignore_list\n\n";
			return 1;
		}
	}

	if ($enable_code_static_check == 1) {
		$ret = run_static_code_check();
		if ($ret != 0) {
			print "\n\n\t=== ERROR ===\n";
			print "\t\tStatic code check error, Please check \"warns.txt\"\n\n";
			print "\t\t!!!The errors in warns.txt must fixed!!!\n\n";
			return 1;
		}
	}

	return ($ret);
}

$result = main();

exit($result);

