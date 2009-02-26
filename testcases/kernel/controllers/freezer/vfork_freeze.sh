#!/bin/bash

# Copyright (c) International Business Machines  Corp., 2008
# Author: Matt Helsley <matthltc@us.ibm.com>
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#

#
# This bash script tests freezer code by starting a process with vfork(2).
# vfork causes the freezer to wait until the vfork call "returns" to the
# parent.
#

# we need the vfork test binary -- ensure it's been built
make vfork || {
	echo "ERROR: Failed to build vfork test binary." 1>&2
	exit -1
}

. "${CGROUPS_TESTROOT}/libcgroup_freezer"
SETS_DEFAULTS="${TCID=vfork_freeze.sh} ${TST_COUNT=1} ${TST_TOTAL=1}"
declare -r TCID
declare -r TST_COUNT
declare -r TST_TOTAL
export TCID TST_COUNT TST_TOTAL

# We replace the normal sample process with a process which uses vfork to
# create new processes. The vfork'ed processes then sleep, causing the
# parent process ($sample_proc) to enter the TASK_UNINTERRUPTIBLE state
# for the duration of the sleep.
function vfork_sleep()
{
	# TODO use a proper temp file
	./vfork -s$sample_sleep 1 > /tmp/tmp.txt &
	local rc=$?
	export vfork_proc=$!
	read sample_proc < /tmp/tmp.txt
	rm -f /tmp/tmp.txt
	export sample_proc

	return $rc
}

running_cgroup_test
mount_freezer && {
make_sample_cgroup && {
assert_cgroup_freezer_state "THAWED" \
		"ERROR: cgroup freezer started in non-THAWED state" && {

vfork_sleep && {

while /bin/true ; do
	trap 'break' ERR

	add_sample_proc_to_cgroup
	"${CG_FILE_WRITE}" $vfork_proc >> tasks # should add to the same cgroup as above

	issue_freeze_cmd
	wait_until_frozen
	assert_sample_proc_is_frozen
	assert_task_is_frozen $vfork_proc

	issue_thaw_cmd
	wait_until_thawed
	assert_sample_proc_not_frozen
	assert_task_not_frozen $vfork_proc

	result=$FINISHED
	break
done
trap '' ERR
cleanup_cgroup_test
tst_resm TINFO " Cleaning up $0"

# We need to kill the sample process(es).
kill_sample_proc ; export sample_proc=$vfork_proc ; kill_sample_proc ; }

# no inverse op needed for assert
}

rm_sample_cgroup ; }
umount_freezer ; }

rm -f tmp.txt

# Failsafe cleanup
cleanup_freezer || /bin/true

exit $result
