/*
 * Copyright (c) 1992 The Regents of the University of California
 * Copyright (c) 1990, 1992 Jan-Simon Pendry
 * All rights reserved.
 *
 * This code is derived from software donated to Berkeley by
 * Jan-Simon Pendry.
 *
 * %sccs.include.redist.c%
 *
 *	@(#)fdesc.h	7.2 (Berkeley) %G%
 *
 * $Id: fdesc.h,v 1.8 1993/04/06 15:28:33 jsp Exp $
 */

#ifdef KERNEL
struct fdescmount {
	struct vnode	*f_root;	/* Root node */
};

#define FD_ROOT		2
#define FD_DEVFD	3
#define FD_STDIN	4
#define FD_STDOUT	5
#define FD_STDERR	6
#define FD_CTTY		7
#define FD_DESC		8
#define FD_MAX		12

typedef enum {
	Froot,
	Fdevfd,
	Fdesc,
	Flink,
	Fctty
} fdntype;

struct fdescnode {
	fdntype		fd_type;	/* Type of this node */
	unsigned	fd_fd;		/* Fd to be dup'ed */
	char		*fd_link;	/* Link to fd/n */
	int		fd_ix;		/* filesystem index */
};

#define VFSTOFDESC(mp)	((struct fdescmount *)((mp)->mnt_data))
#define	VTOFDESC(vp) ((struct fdescnode *)(vp)->v_data)

extern dev_t devctty;
extern int fdesc_allocvp __P((fdntype, int, struct mount *, struct vnode **));
extern int (**fdesc_vnodeop_p)();
extern struct vfsops fdesc_vfsops;
#endif /* KERNEL */
