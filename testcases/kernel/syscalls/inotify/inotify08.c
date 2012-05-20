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
 *     negative testcases for inotify_rm_watch(2)
 *
 * ALGORITHM
 *     This negative test suite exercises the following ERROR states in
 *     inotify_rm_watch:
 *     - EBADF
 *     - EINVAL
 *
 *     - EBADF will be exercised with an fd == -1.
 *     - EINVAL will be exercised with...
 *       -- fd = fileno(stdin)
 *       -- wd = fd
 *
 ****************************************************************************/

#include "config.h"

#if defined(HAVE_SYS_INOTIFY_H)
#include <sys/inotify.h>
#endif
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include "test.h"
#include "usctest.h"
#include "linux_syscall_numbers.h"
#include "inotify.h"
#include "safe_macros.h"

char *TCID = "inotify08";

#if defined(HAVE_SYS_INOTIFY_H)

int TST_TOTAL = 3;

int fd_notify, test_wd;

static void cleanup(void)
{
	close(fd_notify);

	TEST_CLEANUP;

	tst_rmdir();
}

static void setup(void)
{

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	tst_tmpdir();

	fd_notify = myinotify_init();
	if (fd_notify == -1)
		tst_brkm(TBROK|TERRNO, cleanup, "inotify_init() failed");
	test_wd = inotify_add_watch(fd_notify, ".", IN_ATTRIB);
}

void
test_EBADF(void)
{
#define	TST_FMT	"inotify_rm_watch(-1, ...)"

	TEST(inotify_rm_watch(-1, test_wd));
	if (TEST_RETURN == -1 && TEST_ERRNO == EBADF)
		tst_resm(TPASS|TTERRNO,
			TST_FMT " failed as expected");
	else
		tst_resm(TFAIL|TTERRNO,
			TST_FMT " didn't fail as expected with EBADF");
#undef TST_FMT
}

#define	TST_FMT	"inotify_rm_watch(%d, %d)"

void
test_EINVAL1(void)
{
	int fd = fileno(stdin);
	int wd = test_wd;

	TEST(inotify_rm_watch(fd, wd));
	if (TEST_RETURN == -1 && TEST_ERRNO == EINVAL)
		tst_resm(TPASS|TTERRNO,
			TST_FMT " failed as expected",
			fd, wd);
	else
		tst_resm(TFAIL|TTERRNO,
			TST_FMT " didn't fail as expected with EINVAL",
			fd, wd);
}

void
test_EINVAL2(void)
{
	int fd = fd_notify;
	int wd = fileno(stdin);

	TEST(inotify_rm_watch(fd, wd));
	if (TEST_RETURN == -1 && TEST_ERRNO == EINVAL)
		tst_resm(TPASS|TTERRNO,
			TST_FMT " failed as expected",
			fd, wd);
	else
		tst_resm(TFAIL|TTERRNO,
			TST_FMT " didn't fail as expected with EINVAL",
			fd, wd);
}

#undef TST_FMT

int
main(int argc, char **argv)
{
	char *msg;

	msg = parse_opts(argc, argv, NULL, NULL);
	if (msg != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	setup();

	test_EBADF();

	test_EINVAL1();

	test_EINVAL2();

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
