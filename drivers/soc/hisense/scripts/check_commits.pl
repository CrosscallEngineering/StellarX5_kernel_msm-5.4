
use strict;

my $start = 0;
my $end   = 0;
my $project;
my @c_files;
my $build_script;

sub is_correct_commit_id()
{
	my ($commit_id) = @_;

	return 1 if ($commit_id =~ /\w{40}/);
	return 0;
}

sub get_modified_sourcefile_list()
{
	my ($since, $until) = @_;

	system("git log $since..$until --name-only > gitlog.log");
	open LOG, "< gitlog.log" or die "open gitlog.log file failed!";
	while(<LOG>)
	{
		if (/^(.*\.c)$/) {
			my $src_file = $1;
			$src_file =~ s/msm-\d.\d+\///g;
			push(@c_files, $src_file);
		}
	}
	close LOG;
	system("rm -rf gitlog.log");
}

if ($#ARGV < 1) {
	print "\n\tthe number of argv is error\n\n";
	exit 0;
}

$project = shift @ARGV;
$start = shift @ARGV;
$end = shift @ARGV;

$build_script=`ls build_*$project.sh`;
chomp($build_script);

# input parameter check
if (not -e $build_script) {
	print "\n\tproject: $build_script is not exsit\n\n";
	exit(0);
}

if (!&is_correct_commit_id($start)) {
	print "\n\tThe start commit id is error\n\n";
	exit 0;
}

&get_modified_sourcefile_list($start, $end);

system("rm -f warns.txt");
system("echo -e \"check commit: $start .. $end\n\n\" > warns.txt");
foreach my $file (@c_files) {
	print "\n\t\t===========================================================\n";
	print "\t\tStart Check: $file\n";
	print "\t\t===========================================================\n";
	system(". ./$build_script check --output-file $file");
}

