/*
 *
 *   Copyright (c) International Business Machines  Corp., 2001
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*
 * NAME
 * 	setregid02.c
 *
 * DESCRIPTION
 * 	Test that setregid() fails and sets the proper errno values when a
 *	non-root user attemps to change the real or effective group id to a 
 *	value other than the current gid or the current effective gid.
 *
 * ALGORITHM
 *
 *	Setup:
 *	  Setup signal handling
 *	  Get user information.
 *	  Pause for SIGUSER1 if option specified.
 *	Setup test values.
 *	Loop if the proper options are given.
 *	For each test set execute the system call
 *	  Check return code, if system call failed (return=-1)
 *		Log the errno and Issue a FAIL message.
 *	  Otherwise,
 *		Verify the Functionality of system call
 *		if successful,
 *			Issue Functionality-Pass message.
 *		Otherwise,
 *			Issue Functionality-Fail message.
 *	Cleanup:
 *	  Print errno log and/or timing stats if options given.
 *
 * USAGE:  <for command-line>
 *	setregid02 [-c n] [-e] [-f] [-i n] [-I x] [-P x] [-t]
 *	where,  -c n : Run n copies concurrently.
 *		-e   : Turn on errno logging.
 *		-f   : Turn off functionality Testing.
 *		-i n : Execute test n times.
 *		-I x : Execute test for x seconds.
 *		-P x : Pause for x seconds between iterations.
 *		-t   : Turn on syscall timing.
 * History
 *	07/2001 John George
 *		-Ported
 *
 * Restrictions
 * 	This test must be ran as adm.
 *	nobody must be a valid group.
 */


#include <pwd.h>
#include <grp.h>
#include <malloc.h>
#include <string.h>
#include <test.h>
#include <usctest.h>
#include <errno.h>

extern int Tst_count;

char *TCID = "setregid02";
gid_t nobody_gr_gid, root_gr_gid, bin_gr_gid;
int neg_one = -1;
int exp_enos[]={EPERM, 0};
int inval_user = 999999;

struct group nobody, root, bin;
struct passwd adm;

/*
 * The following structure contains all test data.  Each structure in the array
 * is used for a separate test.  The tests are executed in the for loop below.
 */

struct test_data_t {
	gid_t*	real_gid;
	gid_t*	eff_gid;
	int	exp_errno;
	struct group* exp_real_usr;
	struct group* exp_eff_usr;
	char *	test_msg;
} test_data[] = {
	{ &neg_one, &root_gr_gid, EPERM, &nobody, &nobody, "After setregid(-1, root)," },
	{ &neg_one, &bin_gr_gid, EPERM, &nobody, &nobody, "After setregid(-1, bin)" },
	{ &root_gr_gid, &neg_one, EPERM, &nobody, &nobody, "After setregid(root,-1)," },
	{ &bin_gr_gid, &neg_one, EPERM, &nobody, &nobody, "After setregid(bin, -1)," },
	{ &root_gr_gid, &bin_gr_gid, EPERM, &nobody, &nobody, "After setregid(root, bin)" },
	{ &bin_gr_gid, &root_gr_gid, EPERM, &nobody, &nobody, "After setregid(bin, root)," },
	{ &inval_user, &neg_one, EPERM, &nobody, &nobody, "After setregid(-1, invalid user)," },
	{ &neg_one, &inval_user, EPERM, &nobody, &nobody, "After setregid(-1, invalid user)," },
};

int TST_TOTAL = sizeof(test_data)/sizeof(test_data[0]);

void setup(void);			/* Setup function for the test */
void cleanup(void);			/* Cleanup function for the test */
void gid_verify(struct group *ru, struct group *eu, char *when);

main(int ac, char **av)
{
	int lc;				/* loop counter */
	char *msg;			/* message returned from parse_opts */

	/* parse standard options */
	if ((msg = parse_opts(ac, av, (option_t *)NULL, NULL)) !=
	    (char *)NULL) {
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);
		tst_exit();
		/*NOTREACHED*/
	}

	/* Perform global setup for test */
	setup();

	/* check looping state if -i option is given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {
		int i;

		/* reset Tst_count in case we are looping */
		Tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++) {
			/* Set the real or effective group id */
			TEST(setregid(*test_data[i].real_gid,
				*test_data[i].eff_gid));

			if (TEST_RETURN == -1) {
				TEST_ERROR_LOG(TEST_ERRNO);
				if (TEST_ERRNO == test_data[i].exp_errno) {
					tst_resm(TPASS, "setresuid(%d, %d) "
						"failed as expected.",
						*test_data[i].real_gid,
						*test_data[i].eff_gid);
				} else {
					tst_resm(TFAIL, "setresuid(%d, %d) "
						"failed but did not set the "
						"expected errno.",
						*test_data[i].real_gid,
						*test_data[i].eff_gid);
				}
			} else {
				tst_resm(TFAIL, "setresuid(%d, %d) "
					"did not fail as expected.",
					*test_data[i].real_gid,
					*test_data[i].eff_gid);
			}
			/*
			 * Perform functional verification if test
			 * executed without (-f) option.
			 */
			if (STD_FUNCTIONAL_TEST) {
				gid_verify(test_data[i].exp_real_usr,
					test_data[i].exp_eff_usr,
					test_data[i].test_msg);
			} else {
				tst_resm(TINFO, "Call succeeded.");
			}
		}
	}
	cleanup();
	/*NOTREACHED*/
}

/*
 * setup()
 *	performs all ONE TIME setup for this test
 */
void
setup(void)
{
	/* capture signals */
	tst_sig(FORK, DEF_HANDLER, cleanup);

	if (getpwnam("adm") == NULL) {
		tst_brkm(TBROK, NULL, "adm must be a valid user.");
		tst_exit();
		/*NOTREACHED*/
	}

	adm = *getpwnam("adm");

 	/* Check that the test process id is adm */
	if (geteuid() != adm.pw_uid) {
		tst_brkm(TBROK, NULL, "Must be adm for this test!");
		tst_exit();
	}

	/* set the expected errnos... */
	TEST_EXP_ENOS(exp_enos);

	root = *(getgrnam("root"));
	root_gr_gid = root.gr_gid;

	nobody = *( getgrnam("nobody"));
	nobody_gr_gid = nobody.gr_gid;

	bin = *(getgrnam("bin"));
	bin_gr_gid = bin.gr_gid;

	/* Pause if that option was specified
	 * TEST_PAUSE contains the code to fork the test with the -c option.
	 */
	TEST_PAUSE;
}

/*
 * cleanup()
 *	performs all ONE TIME cleanup for this test at
 *	completion or premature exit
 */
void
cleanup(void)
{
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	/* exit with return code appropriate for results */
	tst_exit();
	/*NOTREACHED*/
}

void
gid_verify(struct group *rg, struct group *eg, char *when)
{
	if ((getgid() != rg->gr_gid) || (getegid() != eg->gr_gid)) {
		tst_resm(TINFO, "ERROR: %s real gid = %d; effective gid = %d",
			 when, getgid(), getegid());
		tst_resm(TINFO, "Expected: real gid = %d; effective gid = %d",
			 rg->gr_gid, eg->gr_gid);
	}
}

