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
 *     ENOSPC negative testcase for inotify_add_watch(2)
 *
 * ALGORITHM
 *     This negative test suite exercises the following ERROR states in
 *     inotify_add_watch:
 *     - ENOSPC
 *
 *     - ENOSPC will require first determining the per-process inotify watch
 *       limit, then try to extend beyond that limit. The limit can be
 *       determined by reading /proc/sys/fs/inotify/max_user_watches,
 *       according to inotify(7).
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

char *TCID = "inotify06";

#if defined(HAVE_SYS_INOTIFY_H)

int TST_TOTAL = 1;

int fd_notify, watch_limit;

static void cleanup(void)
{

	close(fd_notify);

	TEST_CLEANUP;

	tst_rmdir();
}

static void setup(void)
{
	char buf[BUFSIZ];
	int fd, i;

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	tst_tmpdir();

	fd_notify = myinotify_init();
	if (fd_notify == -1)
		tst_brkm(TBROK|TERRNO, cleanup, "inotify_init() failed");

#define	TST_FMT	"inotify_add_watch with more than %d watches"

	/* Read in the per user watch limit */
	fd = SAFE_OPEN(cleanup, "/proc/sys/fs/inotify/max_user_watches", O_RDONLY);
	SAFE_READ(cleanup, 0, fd, buf, sizeof(buf));

	watch_limit = (int)strtoll(buf, NULL, 10);

	SAFE_CLOSE(cleanup, fd);

	for (i = 0; i < (watch_limit + 20); i++) {
		snprintf(buf, sizeof(buf), "test_%d", i);
		SAFE_MKDIR(cleanup, buf, 00700);
	}
}

void
test_ENOSPC(void)
{
	char buf[BUFSIZ];
	int i;

	for (i = 0; i < (watch_limit + 20); i++) {

		snprintf(buf, sizeof(buf), "test_%d", i);

		TEST(inotify_add_watch(fd_notify, buf, IN_ATTRIB));
		if (TEST_RETURN == -1) {
			if (TEST_ERRNO == ENOSPC) {
				/* Early termination? */
				break;
			} else
				tst_brkm(TBROK|TERRNO, cleanup,
					"An unexpected error occurred when "
					"calling inotify_add_watch");
		}
	}

	if (watch_limit <= i && TEST_ERRNO == ENOSPC)
		tst_resm(TPASS|TTERRNO,
			"Adding more than %d watches failed as expected "
			"(added %d watches)",
			watch_limit, i);
	else
		tst_resm(TFAIL|TTERRNO,
			"Adding more than %d watches didn't fail as expected "
			"with ENOSPC (added %d watches)",
			watch_limit, i);
}

int
main(int argc, char **argv)
{
	char *msg;

	msg = parse_opts(argc, argv, NULL, NULL);
	if (msg != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	setup();

	test_ENOSPC();

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
