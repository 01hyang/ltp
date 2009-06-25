/******************************************************************************/
/* Copyright (c) Crackerjack Project., 2007                                   */
/*                                                                            */
/* This program is free software;  you can redistribute it and/or modify      */
/* it under the terms of the GNU General Public License as published by       */
/* the Free Software Foundation; either version 2 of the License, or          */
/* (at your option) any later version.                                        */
/*                                                                            */
/* This program is distributed in the hope that it will be useful,            */
/* but WITHOUT ANY WARRANTY;  without even the implied warranty of            */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See                  */
/* the GNU General Public License for more details.                           */
/*                                                                            */
/* You should have received a copy of the GNU General Public License          */
/* along with this program;  if not, write to the Free Software               */
/* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA    */
/*                                                                            */
/******************************************************************************/
/******************************************************************************/
/*                                                                            */
/* File:        keyctl01.c                                   		      */
/*                                                                            */
/* Description: This tests the keyctl() syscall                		      */
/*		Manipulate the kernel's key management facility               */
/* Usage:  <for command-line>                                                 */
/* keyctl01 [-c n] [-e][-i n] [-I x] [-p x] [-t]                     	      */
/*      where,  -c n : Run n copies concurrently.                             */
/*              -e   : Turn on errno logging.                                 */
/*              -i n : Execute test n times.                                  */
/*              -I x : Execute test for x seconds.                            */
/*              -P x : Pause for x seconds between iterations.                */
/*              -t   : Turn on syscall timing.                                */
/*                                                                            */
/* Total Tests: 2                                                             */
/*                                                                            */
/* Test Name:   keyctl01                                             	      */
/* History:     Porting from Crackerjack to LTP is done by                    */
/*              Manas Kumar Nayak maknayak@in.ibm.com>                        */
/******************************************************************************/

#include <stdio.h>
#include <errno.h>
#include <linux/keyctl.h>
/* Harness Specific Include Files. */
#include "test.h"
#include "usctest.h"
#include "linux_syscall_numbers.h"

/* Extern Global Variables */
extern int Tst_count;           /* counter for tst_xxx routines.         */
extern char *TESTDIR;           /* temporary dir created by tst_tmpdir() */

/* Global Variables */
char *TCID = "keyctl01";  /* Test program identifier.*/
int  testno;
int  TST_TOTAL = 2;                   /* total number of tests in this file.   */

/* Extern Global Functions */
/******************************************************************************/
/*                                                                            */
/* Function:    cleanup                                                       */
/*                                                                            */
/* Description: Performs all one time clean up for this test on successful    */
/*              completion,  premature exit or  failure. Closes all temporary */
/*              files, removes all temporary directories exits the test with  */
/*              appropriate return code by calling tst_exit() function.       */
/*                                                                            */
/* Input:       None.                                                         */
/*                                                                            */
/* Output:      None.                                                         */
/*                                                                            */
/* Return:      On failure - Exits calling tst_exit(). Non '0' return code.   */
/*              On success - Exits calling tst_exit(). With '0' return code.  */
/*                                                                            */
/******************************************************************************/
extern void cleanup() {
        /* Remove tmp dir and all files in it */
        TEST_CLEANUP;
        tst_rmdir();

        /* Exit with appropriate return code. */
        tst_exit();
}

/* Local  Functions */
/******************************************************************************/
/*                                                                            */
/* Function:    setup                                                         */
/*                                                                            */
/* Description: Performs all one time setup for this test. This function is   */
/*              typically used to capture signals, create temporary dirs      */
/*              and temporary files that may be used in the course of this    */
/*              test.                                                         */
/*                                                                            */
/* Input:       None.                                                         */
/*                                                                            */
/* Output:      None.                                                         */
/*                                                                            */
/* Return:      On failure - Exits by calling cleanup().                      */
/*              On success - returns 0.                                       */
/*                                                                            */
/******************************************************************************/
void setup() {
        /* Capture signals if any */
        /* Create temporary directories */
        TEST_PAUSE;
        tst_tmpdir();
}

int main(int ac, char **av) {
	int lc;                 /* loop counter */
        char *msg;              /* message returned from parse_opts */
	
        /* parse standard options */
        if ((msg = parse_opts(ac, av, (option_t *)NULL, NULL)) != (char *)NULL){
             tst_brkm(TBROK, cleanup, "OPTION PARSING ERROR - %s", msg);
             tst_exit();
           }

        setup();

        /* Check looping state if -i option given */
        for (lc = 0; TEST_LOOPING(lc); ++lc) {
                Tst_count = 0;
                for (testno = 0; testno < TST_TOTAL; ++testno) {
                     TEST(syscall(__NR_keyctl, KEYCTL_GET_KEYRING_ID, KEY_SPEC_USER_SESSION_KEYRING));    //call keyctl() and Ask for a keyring's ID
                     if(TEST_RETURN != -1) {
        		tst_resm(TPASS,"KEYCTL_GET_KEYRING_ID succeed");
                     /*   cleanup(); */
                     }
                     else {
                 	   tst_resm(TFAIL, "(%s failed) KEYCTL_GET_KEYRING_ID fail with - errno = %d : %s", TCID, TEST_ERRNO, strerror(TEST_ERRNO));
                           cleanup();
	  	           tst_exit();
                     }


		     TEST(syscall(__NR_keyctl, KEYCTL_REVOKE, "MyKey"));    //call keyctl()
                     if(TEST_RETURN != -1) {
                        tst_resm(TFAIL,"KEYTL_REVOKE succeededs, but should fail");
                        cleanup();
			tst_exit();
                     }
                     else {
				if(TEST_ERRNO == ENOKEY)	//Check for correct error no.	
								//if no  matching key was found or an invalid key was specified. 
                           	{
					tst_resm(TPASS,"KEYCTL_REVOKE got expected errno:%ld",TEST_ERRNO);
                        		cleanup();
				}else{
					tst_resm(TFAIL,"KEYCTL_REVOKE got unexpected errno:%ld", TEST_ERRNO);
                        		cleanup();
					tst_exit();
				}
                     }

                }
        }	
        tst_exit();
}

