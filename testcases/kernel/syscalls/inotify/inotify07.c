/*
 * Copyright (c) 2012 Linux Test Project.  All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Further, this software is distributed without any warranty that it is
 * free of the rightful claim of any third person regarding infringement
 * or the like.  Any license provided herein, whether implied or
 * otherwise, applies only to this software file.  Patent licenses, if
 * any, provided herein do not apply to combinations of this program with
 * other software, or any other product whatsoever.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc., 59
 * Temple Place - Suite 330, Boston MA 02111-1307, USA.
 *
 * Garrett Cooper, May 2012
 */

/****************************************************************************
 * DESCRIPTION
 *     EACCES negative testcase for inotify_add_watch(2)
 *
 * ALGORITHM
 *     This negative test suite exercises the following ERROR states in
 *     inotify_add_watch:
 *     - EACCES
 *
 *     - First, a file will be created with 0600 permissions.
 *     - First, the test app will be setuid to "bin".
 *     - Finally, the test will try adding a watch to the file.
 *
 ****************************************************************************/

#include "config.h"

#if defined(HAVE_SYS_INOTIFY_H)
#include <sys/inotify.h>
#endif
#include <pwd.h>
#include <errno.h>
#include "test.h"
#include "usctest.h"
#include "linux_syscall_numbers.h"
#include "inotify.h"
#include "safe_macros.h"

char *TCID = "inotify07";

#if defined(HAVE_SYS_INOTIFY_H)

int TST_TOTAL = 1;

#define	TEST_FILE	"test_file"
#define TEST_USER	"bin"

int fd_notify;

static void cleanup(void)
{

	close(fd_notify);

	//SAFE_SETUID(NULL, 0);

	TEST_CLEANUP;

	tst_rmdir();
}

static void setup(void)
{
	struct passwd *pwd;

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	tst_tmpdir();

	fd_notify = myinotify_init();
	if (fd_notify == -1)
		tst_brkm(TBROK|TERRNO, cleanup, "inotify_init() failed");

	SAFE_CREAT(cleanup, TEST_FILE, 0600);

	pwd = SAFE_GETPWNAM(cleanup, TEST_USER);

	SAFE_SETUID(cleanup, pwd->pw_uid);
}

void
test_EACCES(void)
{
#define	TST_FMT	"inotify_add_watch(..., %s, ...)"

	TEST(inotify_add_watch(fd_notify, TEST_FILE, IN_ATTRIB));
	if (TEST_RETURN == -1 && TEST_ERRNO == EACCES)
		tst_resm(TPASS|TTERRNO,
			TST_FMT " failed as expected",
			TEST_FILE);
	else
		tst_resm(TFAIL|TTERRNO,
			TST_FMT " didn't fail as expected with EACCES",
			TEST_FILE);
}

int
main(int argc, char **argv)
{
	char *msg;

	msg = parse_opts(argc, argv, NULL, NULL);
	if (msg != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	setup();

	test_EACCES();

	cleanup();
	tst_exit();
}
#else

int TST_TOTAL;

int main(void)
{
	tst_brkm(TCONF, NULL, "system doesn't have required inotify support");
}
#endif /* defined(HAVE_SYS_INOTIFY_H) */
