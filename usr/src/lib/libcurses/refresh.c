/*
 * Copyright (c) 1981 Regents of the University of California.
 * All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#ifndef lint
static char sccsid[] = "@(#)refresh.c	5.9 (Berkeley) %G%";
#endif /* not lint */

#include <curses.h>
#include <string.h>

static int curwin;
static short ly, lx;

WINDOW *_win;

static void	domvcur __P((int, int, int, int));
static int	makech __P((WINDOW *, int));

/*
 * wrefresh --
 *	Make the current screen look like "win" over the area coverd by
 *	win.
 */
int
wrefresh(win)
	register WINDOW *win;
{
	register int retval;
	register short wy;

	/* Make sure were in visual state. */
	if (__endwin) {
		tputs(VS, 0, __cputchar);
		tputs(TI, 0, __cputchar);
		__endwin = 0;
	}

	/* Initialize loop parameters. */

	ly = curscr->cury;
	lx = curscr->curx;
	wy = 0;
	_win = win;
	curwin = (win == curscr);

	if (win->flags & __CLEAROK || curscr->flags & __CLEAROK || curwin) {
		if ((win->flags & __FULLWIN) || curscr->flags & __CLEAROK) {
			tputs(CL, 0, __cputchar);
			ly = 0;
			lx = 0;
			if (!curwin) {
				curscr->flags &= ~__CLEAROK;
				curscr->cury = 0;
				curscr->curx = 0;
				werase(curscr);
			}
			touchwin(win);
		}
		win->flags &= ~__CLEAROK;
	}
	if (!CA) {
		if (win->curx != 0)
			putchar('\n');
		if (!curwin)
			werase(curscr);
	}
#ifdef DEBUG
	__TRACE("wrefresh: (%0.2o): curwin = %d\n", win, curwin);
	__TRACE("wrefresh: \tfirstch\tlastch\n");
#endif
	for (wy = 0; wy < win->maxy; wy++) {
#ifdef DEBUG
		__TRACE("%d\t%d\t%d\n",
		    wy, win->lines[wy]->firstch, win->lines[wy]->lastch);
#endif
		if (win->lines[wy]->flags & __ISDIRTY)
			if (makech(win, wy) == ERR)
				return (ERR);
			else {
				if (win->lines[wy]->firstch >= win->ch_off)
					win->lines[wy]->firstch = win->maxx +
					    win->ch_off;
				if (win->lines[wy]->lastch < win->maxx +
				    win->ch_off)
					win->lines[wy]->lastch = win->ch_off;
				if (win->lines[wy]->lastch < 
				    win->lines[wy]->firstch)
					win->lines[wy]->flags &= ~__ISDIRTY;
			}
#ifdef DEBUG
		__TRACE("\t%d\t%d\n", win->lines[wy]->firstch, 
			win->lines[wy]->lastch);
#endif
	}
	
	__TRACE("refresh: ly=%d, lx=%d\n", ly, lx);
	if (win == curscr)
		domvcur(ly, lx, win->cury, win->curx);
	else {
		if (win->flags & __LEAVEOK) {
			curscr->cury = ly;
			curscr->curx = lx;
			ly -= win->begy;
			lx -= win->begx;
			if (ly >= 0 && ly < win->maxy && lx >= 0 &&
			    lx < win->maxx) {
				win->cury = ly;
				win->curx = lx;
			} else
				win->cury = win->curx = 0;
		} else {
			domvcur(ly, lx, win->cury + win->begy,
			    win->curx + win->begx);
			curscr->cury = win->cury + win->begy;
			curscr->curx = win->curx + win->begx;
		}
	}
	retval = OK;

	_win = NULL;
	(void)fflush(stdout);
	return (retval);
}

/*
 * makech --
 *	Make a change on the screen.
 */
static int
makech(win, wy)
	register WINDOW *win;
	int wy;
{
	register int nlsp, clsp;		/* Last space in lines. */
	register short wx, lch, y;
	register char *nsp, *csp, *ce;

	if (!(win->lines[wy]->flags & __ISDIRTY))
		return (OK);
	wx = win->lines[wy]->firstch - win->ch_off;
	if (wx >= win->maxx)
		return (OK);
	else if (wx < 0)
		wx = 0;
	lch = win->lines[wy]->lastch - win->ch_off;
	if (lch < 0)
		return (OK);
	else if (lch >= win->maxx)
		lch = win->maxx - 1;
	y = wy + win->begy;

	if (curwin)
		csp = " ";
	else
		csp = &curscr->lines[wy + win->begy]->line[wx + win->begx];

	nsp = &win->lines[wy]->line[wx];
	if (CE && !curwin) {
		for (ce = &win->lines[wy]->line[win->maxx - 1]; 
		     *ce == ' '; ce--)
			if (ce <= win->lines[wy]->line)
				break;
		nlsp = ce - win->lines[wy]->line;
	}
	if (!curwin)
		ce = CE;
	else
		ce = NULL;

	while (wx <= lch) {
		if (*nsp == *csp) {
			if (wx <= lch) {
				while (*nsp == *csp && wx <= lch) {
					nsp++;
					if (!curwin)
						csp++;
					++wx;
				}
				continue;
			}
			break;
		}
		domvcur(ly, lx, y, wx + win->begx);
#ifdef DEBUG
		__TRACE("makech: 1: wx = %d, lx = %d, newy = %d, newx = %d\n", 
		    wx, lx, y, wx + win->begx);
#endif
		ly = y;
		lx = wx + win->begx;
		while (*nsp != *csp && wx <= lch) {
			if (ce != NULL && wx >= nlsp && *nsp == ' ') {
				/* Check for clear to end-of-line. */
				ce = &curscr->lines[ly]->line[COLS - 1];
				while (*ce == ' ')
					if (ce-- <= csp)
						break;
				clsp = ce - curscr->lines[ly]->line - 
				       win->begx;
#ifdef DEBUG
			__TRACE("makech: clsp = %d, nlsp = %d\n", clsp, nlsp);
#endif
				if (clsp - nlsp >= strlen(CE) &&
				    clsp < win->maxx) {
#ifdef DEBUG
					__TRACE("makech: using CE\n");
#endif
					tputs(CE, 0, __cputchar);
					lx = wx + win->begx;
					while (wx++ <= clsp)
						*csp++ = ' ';
					return (OK);
				}
				ce = NULL;
			}

			/* Enter/exit standout mode as appropriate. */
			if (SO && (*nsp & __STANDOUT) !=
			    (curscr->flags & __STANDOUT)) {
				if (*nsp & __STANDOUT) {
					tputs(SO, 0, __cputchar);
					curscr->flags |= __WSTANDOUT;
				} else {
					tputs(SE, 0, __cputchar);
					curscr->flags &= ~__WSTANDOUT;
				}
			}

			wx++;
			if (wx >= win->maxx && wy == win->maxy - 1)
				if (win->flags & __SCROLLOK) {
					if (curscr->flags & __WSTANDOUT
					    && win->flags & __ENDLINE)
						if (!MS) {
							tputs(SE, 0,
							    __cputchar);
							curscr->flags &=
							    ~__WSTANDOUT;
						}
					if (!curwin)
						putchar((*csp = *nsp) & 0177);
					else
						putchar(*nsp & 0177);
					if (win->flags & __FULLWIN && !curwin)
						scroll(curscr);
					ly = win->begy + wy;
					lx = win->begx + wx;
					return (OK);
				} else
					if (win->flags & __SCROLLWIN) {
						lx = --wx;
						return (ERR);
					}
			if (!curwin)
				putchar((*csp++ = *nsp) & 0177);
			else
				putchar(*nsp & 0177);
#ifdef DEBUG
			__TRACE("makech: putchar(%c)\n", *nsp & 0177);
#endif
			if (UC && (*nsp & __STANDOUT)) {
				putchar('\b');
				tputs(UC, 0, __cputchar);
			}
			nsp++;
		}
#ifdef DEBUG
		__TRACE("makech: 2: wx = %d, lx = %d\n", wx, lx);
#endif
		if (lx == wx + win->begx)	/* If no change. */
			break;
		lx = wx + win->begx;
		if (lx >= COLS && AM) {
			lx = 0;
			ly++;
			/*
			 * xn glitch: chomps a newline after auto-wrap.
			 * we just feed it now and forget about it.
			 */
			if (XN) {
				putchar('\n');
				putchar('\r');
			}
		}
#ifdef DEBUG
		__TRACE("makech: 3: wx = %d, lx = %d\n", wx, lx);
#endif
	}
	return (OK);
}

/*
 * domvcur --
 *	Do a mvcur, leaving standout mode if necessary.
 */
static void
domvcur(oy, ox, ny, nx)
	int oy, ox, ny, nx;
{
	if (curscr->flags & __WSTANDOUT && !MS) {
		tputs(SE, 0, __cputchar);
		curscr->flags &= ~__WSTANDOUT;
	}
	mvcur(oy, ox, ny, nx);
}
