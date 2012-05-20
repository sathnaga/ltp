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
 *     negative testcases for inotify_add_watch(2)
 *
 * ALGORITHM
 *     This negative test suite exercises the following ERROR states in
 *     inotify_add_watch:
 *     - EBADF
 *     - EFAULT
 *     - EINVAL
 *     - ENOENT
 *
 *     - EBADF will be exercised with an fd == -1.
 *     - EFAULT will be exercised with pathname == NULL.
 *     - EINVAL will be exercised with...
 *       -- fd = fileno(stdin)
 *       -- attrib = ~IN_ALL_EVENTS
 *     - ENOENT will be exercised via /nonexistent and with a symlink to
 *       /idonotexist
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

char *TCID = "inotify05";

#if defined(HAVE_SYS_INOTIFY_H)

int TST_TOTAL = 6;

#define	TEST_BROKEN_DIR	"test_bdir"
#define	TEST_DIR	"test_dir"
#define	TEST_SYMLINK	"test_symlink"

int fd_notify;

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

	SAFE_MKDIR(cleanup, TEST_DIR, 00700);

	SAFE_MKDIR(cleanup, TEST_BROKEN_DIR, 00700);

	if (symlink(TEST_BROKEN_DIR, TEST_SYMLINK) == -1)
		tst_brkm(TERRNO|TBROK, cleanup, "symlink failed");

	/* Break the symlink */
	if (rmdir(TEST_BROKEN_DIR) == -1)
		tst_brkm(TERRNO|TBROK, cleanup, "rmdir failed");
}

void
test_EBADF(void)
{
#define	TST_FMT	"inotify_add_watch(-1, ...)"

	TEST(inotify_add_watch(-1, TEST_DIR, IN_ATTRIB));
	if (TEST_RETURN == -1 && TEST_ERRNO == EBADF)
		tst_resm(TPASS|TTERRNO,
			TST_FMT " failed as expected");
	else
		tst_resm(TFAIL|TTERRNO,
			TST_FMT " didn't fail as expected with EBADF");
#undef TST_FMT
}

void
test_EFAULT(void)
{
#define	TST_FMT	"inotify_add_watch(..., NULL, ...)"

	TEST(inotify_add_watch(fd_notify, NULL, IN_ATTRIB));
	if (TEST_RETURN == -1 && TEST_ERRNO == EFAULT)
		tst_resm(TPASS|TTERRNO,
			TST_FMT " failed as expected");
	else
		tst_resm(TFAIL|TTERRNO,
			TST_FMT " didn't fail as expected with EFAULT");
#undef TST_FMT
}

#define	TST_FMT	"inotify_add_watch(%d, ..., %0x)"

void
test_EINVAL1(void)
{
	int fd = fileno(stdin);
	int mask = IN_ATTRIB;

	TEST(inotify_add_watch(fd, TEST_DIR, mask));
	if (TEST_RETURN == -1 && TEST_ERRNO == EINVAL)
		tst_resm(TPASS|TTERRNO,
			TST_FMT " failed as expected",
			fd, mask);
	else
		tst_resm(TFAIL|TTERRNO,
			TST_FMT " didn't fail as expected with EINVAL",
			fd, mask);
}

void
test_EINVAL2(void)
{
	int fd = fd_notify;
	int mask = ~IN_ALL_EVENTS;

	TEST(inotify_add_watch(fd, TEST_DIR, mask));
	if (TEST_RETURN == -1 && TEST_ERRNO == EINVAL)
		tst_resm(TPASS|TTERRNO,
			TST_FMT " failed as expected",
			fd, mask);
	else
		tst_resm(TFAIL|TTERRNO,
			TST_FMT " didn't fail as expected with EINVAL",
			fd, mask);
}

#undef TST_FMT
#define	TST_FMT	"inotify_add_watch(..., \"%s\", ...)"

void
test_ENOENT1(void)
{
	const char *pathname = "/nonexistent";

	TEST(inotify_add_watch(fd_notify, pathname, IN_ATTRIB));
	if (TEST_RETURN == -1 && TEST_ERRNO == ENOENT)
		tst_resm(TPASS|TTERRNO,
			TST_FMT " failed as expected",
			pathname);
	else
		tst_resm(TFAIL|TTERRNO,
			TST_FMT " didn't fail as expected with ENOENT",
			pathname);
}

void
test_ENOENT2(void)
{
	const char *pathname = TEST_SYMLINK;

	TEST(inotify_add_watch(fd_notify, pathname, IN_ATTRIB));
	if (TEST_RETURN == -1 && TEST_ERRNO == ENOENT)
		tst_resm(TPASS|TTERRNO,
			TST_FMT " failed as expected",
			pathname);
	else
		tst_resm(TFAIL|TTERRNO,
			TST_FMT " didn't fail as expected with ENOENT",
			pathname);
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

	test_EFAULT();

	test_EINVAL1();

	test_EINVAL2();

	test_ENOENT1();

	test_ENOENT2();

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
