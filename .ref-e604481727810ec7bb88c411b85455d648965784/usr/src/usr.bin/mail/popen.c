/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#ifndef lint
static char sccsid[] = "@(#)popen.c	5.19 (Berkeley) %G%";
#endif /* not lint */

#include "rcv.h"
#include <sys/wait.h>
#include <fcntl.h>
#include "extern.h"

#define READ 0
#define WRITE 1

struct fp {
	FILE *fp;
	int pipe;
	int pid;
	struct fp *link;
};
static struct fp *fp_head;

struct child {
	int pid;
	char done;
	char free;
	union wait status;
	struct child *link;
};
static struct child *child;
static struct child *findchild __P((int));
static void delchild __P((struct child *));

FILE *
Fopen(file, mode)
	char *file, *mode;
{
	FILE *fp;

	if ((fp = fopen(file, mode)) != NULL) {
		register_file(fp, 0, 0);
		(void) fcntl(fileno(fp), F_SETFD, 1);
	}
	return fp;
}

FILE *
Fdopen(fd, mode)
	int fd;
	char *mode;
{
	FILE *fp;

	if ((fp = fdopen(fd, mode)) != NULL) {
		register_file(fp, 0, 0);
		(void) fcntl(fileno(fp), F_SETFD, 1);
	}
	return fp;
}

int
Fclose(fp)
	FILE *fp;
{
	unregister_file(fp);
	return fclose(fp);
}

FILE *
Popen(cmd, mode)
	char *cmd;
	char *mode;
{
	int p[2];
	int myside, hisside, fd0, fd1;
	int pid;
	FILE *fp;

	if (pipe(p) < 0)
		return NULL;
	(void) fcntl(p[READ], F_SETFD, 1);
	(void) fcntl(p[WRITE], F_SETFD, 1);
	if (*mode == 'r') {
		myside = p[READ];
		fd0 = -1;
		hisside = fd1 = p[WRITE];
	} else {
		myside = p[WRITE];
		hisside = fd0 = p[READ];
		fd1 = -1;
	}
	if ((pid = start_command(cmd, 0, fd0, fd1, NOSTR, NOSTR, NOSTR)) < 0) {
		close(p[READ]);
		close(p[WRITE]);
		return NULL;
	}
	(void) close(hisside);
	if ((fp = fdopen(myside, mode)) != NULL)
		register_file(fp, 1, pid);
	return fp;
}

int
Pclose(ptr)
	FILE *ptr;
{
	int i;
	int omask;

	i = file_pid(ptr);
	unregister_file(ptr);
	(void) fclose(ptr);
	omask = sigblock(sigmask(SIGINT)|sigmask(SIGHUP));
	i = wait_child(i);
	sigsetmask(omask);
	return i;
}

void
close_all_files()
{

	while (fp_head)
		if (fp_head->pipe)
			(void) Pclose(fp_head->fp);
		else
			(void) Fclose(fp_head->fp);
}

void
register_file(fp, pipe, pid)
	FILE *fp;
	int pipe, pid;
{
	struct fp *fpp;

	if ((fpp = (struct fp *) malloc(sizeof *fpp)) == NULL)
		panic("Out of memory");
	fpp->fp = fp;
	fpp->pipe = pipe;
	fpp->pid = pid;
	fpp->link = fp_head;
	fp_head = fpp;
}

void
unregister_file(fp)
	FILE *fp;
{
	struct fp **pp, *p;

	for (pp = &fp_head; p = *pp; pp = &p->link)
		if (p->fp == fp) {
			*pp = p->link;
			free((char *) p);
			return;
		}
	panic("Invalid file pointer");
}

file_pid(fp)
	FILE *fp;
{
	struct fp *p;

	for (p = fp_head; p; p = p->link)
		if (p->fp == fp)
			return (p->pid);
	panic("Invalid file pointer");
	/*NOTREACHED*/
}

/*
 * Run a command without a shell, with optional arguments and splicing
 * of stdin and stdout.  The command name can be a sequence of words.
 * Signals must be handled by the caller.
 * "Mask" contains the signals to ignore in the new process.
 * SIGINT is enabled unless it's in the mask.
 */
/*VARARGS4*/
int
run_command(cmd, mask, infd, outfd, a0, a1, a2)
	char *cmd;
	int mask, infd, outfd;
	char *a0, *a1, *a2;
{
	int pid;

	if ((pid = start_command(cmd, mask, infd, outfd, a0, a1, a2)) < 0)
		return -1;
	return wait_command(pid);
}

/*VARARGS4*/
int
start_command(cmd, mask, infd, outfd, a0, a1, a2)
	char *cmd;
	int mask, infd, outfd;
	char *a0, *a1, *a2;
{
	int pid;

	if ((pid = vfork()) < 0) {
		perror("fork");
		return -1;
	}
	if (pid == 0) {
		char *argv[100];
		int i = getrawlist(cmd, argv, sizeof argv / sizeof *argv);

		if ((argv[i++] = a0) != NOSTR &&
		    (argv[i++] = a1) != NOSTR &&
		    (argv[i++] = a2) != NOSTR)
			argv[i] = NOSTR;
		prepare_child(mask, infd, outfd);
		execvp(argv[0], argv);
		perror(argv[0]);
		_exit(1);
	}
	return pid;
}

void
prepare_child(mask, infd, outfd)
	int mask, infd, outfd;
{
	int i;

	/*
	 * All file descriptors other than 0, 1, and 2 are supposed to be
	 * close-on-exec.
	 */
	if (infd >= 0)
		dup2(infd, 0);
	if (outfd >= 0)
		dup2(outfd, 1);
	for (i = 1; i <= NSIG; i++)
		if (mask & sigmask(i))
			(void) signal(i, SIG_IGN);
	if ((mask & sigmask(SIGINT)) == 0)
		(void) signal(SIGINT, SIG_DFL);
	(void) sigsetmask(0);
}

int
wait_command(pid)
	int pid;
{

	if (wait_child(pid) < 0) {
		printf("Fatal error in process.\n");
		return -1;
	}
	return 0;
}

static struct child *
findchild(pid)
	int pid;
{
	register struct child **cpp;

	for (cpp = &child; *cpp != NULL && (*cpp)->pid != pid;
	     cpp = &(*cpp)->link)
			;
	if (*cpp == NULL) {
		*cpp = (struct child *) malloc(sizeof (struct child));
		(*cpp)->pid = pid;
		(*cpp)->done = (*cpp)->free = 0;
		(*cpp)->link = NULL;
	}
	return *cpp;
}

static void
delchild(cp)
	register struct child *cp;
{
	register struct child **cpp;

	for (cpp = &child; *cpp != cp; cpp = &(*cpp)->link)
		;
	*cpp = cp->link;
	free((char *) cp);
}

void
sigchild(signo)
	int signo;
{
	int pid;
	union wait status;
	register struct child *cp;

	while ((pid =
	    wait3((int *)&status, WNOHANG, (struct rusage *)0)) > 0) {
		cp = findchild(pid);
		if (cp->free)
			delchild(cp);
		else {
			cp->done = 1;
			cp->status = status;
		}
	}
}

union wait wait_status;

/*
 * Wait for a specific child to die.
 */
int
wait_child(pid)
	int pid;
{
	int mask = sigblock(sigmask(SIGCHLD));
	register struct child *cp = findchild(pid);

	while (!cp->done)
		sigpause(mask);
	wait_status = cp->status;
	delchild(cp);
	sigsetmask(mask);
	return wait_status.w_status ? -1 : 0;
}

/*
 * Mark a child as don't care.
 */
void
free_child(pid)
	int pid;
{
	int mask = sigblock(sigmask(SIGCHLD));
	register struct child *cp = findchild(pid);

	if (cp->done)
		delchild(cp);
	else
		cp->free = 1;
	sigsetmask(mask);
}
