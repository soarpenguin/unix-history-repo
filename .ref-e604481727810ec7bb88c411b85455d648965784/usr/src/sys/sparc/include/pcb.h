/*
 * Copyright (c) 1992 The Regents of the University of California.
 * All rights reserved.
 *
 * This software was developed by the Computer Systems Engineering group
 * at Lawrence Berkeley Laboratory under DARPA contract BG 91-66 and
 * contributed to Berkeley.
 *
 * All advertising materials mentioning features or use of this software
 * must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Lawrence Berkeley Laboratory.
 *
 * %sccs.include.redist.c%
 *
 *	@(#)pcb.h	7.3 (Berkeley) %G%
 *
 * from: $Header: pcb.h,v 1.6 92/11/26 02:04:39 torek Exp $
 */

#include <machine/reg.h>

#ifdef notyet
#define	PCB_MAXWIN	32	/* architectural limit */
#else
#define	PCB_MAXWIN	8	/* worried about u area sizes ... */
#endif

/*
 * SPARC Process Control Block.
 *
 * pcb_uw is positive if there are any user windows that are
 * are currently in the CPU windows rather than on the user
 * stack.  Whenever we are running in the kernel with traps
 * enabled, we decrement pcb_uw for each ``push'' of a CPU
 * register window into the stack, and we increment it for
 * each ``pull'' from the stack into the CPU.  (If traps are
 * disabled, or if we are in user mode, pcb_uw is junk.)
 *
 * To ease computing pcb_uw on traps from user mode, we keep track
 * of the log base 2 of the single bit that is set in %wim.
 *
 * If an overflow occurs while the associated user stack pages
 * are invalid (paged out), we have to store the registers
 * in a page that is locked in core while the process runs,
 * i.e., right here in the pcb.  We also need the stack pointer
 * for the last such window (but only the last, as the others
 * are in each window) and the count of windows saved.  We
 * cheat by having a whole window structure for that one %sp.
 * Thus, to save window pcb_rw[i] to memory, we write it at
 * pcb_rw[i + 1].rw_in[6].
 *
 * pcb_nsaved has three `kinds' of values.  If 0, it means no
 * registers are in the PCB (though if pcb_uw is positive,
 * there may be the next time you look).  If positive, it means
 * there are no user registers in the CPU, but there are some
 * saved in pcb_rw[].  As a special case, traps that needed
 * assistance to pull user registers from the stack also store
 * the registers in pcb_rw[], and set pcb_nsaved to -1.  This
 * special state is normally short-term: it can only last until the
 * trap returns, and it can never persist across entry to user code.
 */
struct pcb {
	int	pcb_sp;		/* sp (%o6) when swtch() was called */
	int	pcb_pc;		/* pc (%o7) when swtch() was called */
	int	pcb_psr;	/* %psr when swtch() was called */

	caddr_t	pcb_onfault;	/* for copyin/out */

	int	pcb_uw;		/* user windows inside CPU */
	int	pcb_wim;	/* log2(%wim) */
	int	pcb_nsaved;	/* number of windows saved in pcb */

#ifdef notdef
	int	pcb_winof;	/* number of window overflow traps */
	int	pcb_winuf;	/* number of window underflow traps */
#endif
	int	pcb_pad;	/* pad to doubleword boundary */

	/* the following MUST be aligned on a doubleword boundary */
	struct	rwindow pcb_rw[PCB_MAXWIN];	/* saved windows */
};

/*
 * The pcb is augmented with machine-dependent additional data for
 * core dumps.  Note that the trapframe here is a copy of the one
 * from the top of the kernel stack (included here so that the kernel
 * stack itself need not be dumped).
 */
struct md_coredump {
	struct	trapframe md_tf;
	struct	fpstate md_fpstate;
};

#ifdef KERNEL
extern struct pcb *cpcb;
#endif /* KERNEL */
