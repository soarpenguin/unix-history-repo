/*
 * Copyright (c) 2003
 *	Bill Paul <wpaul@windriver.com>.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by Bill Paul.
 * 4. Neither the name of the author nor the names of any co-contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY Bill Paul AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL Bill Paul OR THE VOICES IN HIS HEAD
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/ctype.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/systm.h>
#include <sys/malloc.h>
#include <sys/lock.h>
#include <sys/mutex.h>

#include <sys/callout.h>
#include <sys/kernel.h>

#include <machine/atomic.h>
#include <machine/clock.h>
#include <machine/bus_memio.h>
#include <machine/bus_pio.h>
#include <machine/bus.h>
#include <machine/stdarg.h>

#include <sys/bus.h>
#include <sys/rman.h>

#include <compat/ndis/pe_var.h>
#include <compat/ndis/hal_var.h>
#include <compat/ndis/resource_var.h>
#include <compat/ndis/ndis_var.h>
#include <compat/ndis/ntoskrnl_var.h>

#define __regparm __attribute__((regparm(3)))

#define FUNC void(*)(void)

__stdcall static uint8_t ntoskrnl_unicode_equal(ndis_unicode_string *,
	ndis_unicode_string *, uint8_t);
__stdcall static void ntoskrnl_unicode_copy(ndis_unicode_string *,
	ndis_unicode_string *);
__stdcall static ndis_status ntoskrnl_unicode_to_ansi(ndis_ansi_string *,
	ndis_unicode_string *, uint8_t);
__stdcall static ndis_status ntoskrnl_ansi_to_unicode(ndis_unicode_string *,
	ndis_ansi_string *, uint8_t);
__stdcall static void *ntoskrnl_iobuildsynchfsdreq(uint32_t, void *,
	void *, uint32_t, uint32_t *, void *, void *);
__stdcall static uint32_t ntoskrnl_iofcalldriver(/*void *, void * */ void);
__stdcall static void ntoskrnl_iofcompletereq(/*void *, uint8_t*/ void);
__stdcall static uint32_t ntoskrnl_waitforobj(void *, uint32_t,
	uint32_t, uint8_t, void *);
__stdcall static void ntoskrnl_initevent(void *, uint32_t, uint8_t);
__stdcall static void ntoskrnl_writereg_ushort(uint16_t *, uint16_t);
__stdcall static uint16_t ntoskrnl_readreg_ushort(uint16_t *);
__stdcall static void ntoskrnl_writereg_ulong(uint32_t *, uint32_t);
__stdcall static uint32_t ntoskrnl_readreg_ulong(uint32_t *);
__stdcall static void ntoskrnl_writereg_uchar(uint8_t *, uint8_t);
__stdcall static uint8_t ntoskrnl_readreg_uchar(uint8_t *);
__stdcall static int64_t _allmul(int64_t, int64_t);
__stdcall static int64_t _alldiv(int64_t, int64_t);
__stdcall static int64_t _allrem(int64_t, int64_t);
__regparm static int64_t _allshr(int64_t, uint8_t);
__regparm static int64_t _allshl(int64_t, uint8_t);
__stdcall static uint64_t _aullmul(uint64_t, uint64_t);
__stdcall static uint64_t _aulldiv(uint64_t, uint64_t);
__stdcall static uint64_t _aullrem(uint64_t, uint64_t);
__regparm static uint64_t _aullshr(uint64_t, uint8_t);
__regparm static uint64_t _aullshl(uint64_t, uint8_t);
__stdcall static void *ntoskrnl_allocfunc(uint32_t, size_t, uint32_t);
__stdcall static void ntoskrnl_freefunc(void *);
__stdcall static void ntoskrnl_init_lookaside(paged_lookaside_list *,
	lookaside_alloc_func *, lookaside_free_func *,
	uint32_t, size_t, uint32_t, uint16_t);
__stdcall static void ntoskrnl_delete_lookaside(paged_lookaside_list *);
__stdcall static void ntoskrnl_init_nplookaside(npaged_lookaside_list *,
	lookaside_alloc_func *, lookaside_free_func *,
	uint32_t, size_t, uint32_t, uint16_t);
__stdcall static void ntoskrnl_delete_nplookaside(npaged_lookaside_list *);
__stdcall static slist_entry *ntoskrnl_push_slist(/*slist_entry *,
	slist_entry * */ void);
__stdcall static slist_entry *ntoskrnl_pop_slist(/*slist_entry * */ void);
__stdcall static slist_entry *ntoskrnl_push_slist_ex(/*slist_entry *,
	slist_entry *,*/ kspin_lock *);
__stdcall static slist_entry *ntoskrnl_pop_slist_ex(/*slist_entry *,
	kspin_lock * */void);
__stdcall static void ntoskrnl_lock_dpc(/*kspin_lock * */ void);
__stdcall static void ntoskrnl_unlock_dpc(/*kspin_lock * */ void);
__stdcall static uint32_t
	ntoskrnl_interlock_inc(/*volatile uint32_t * */ void);
__stdcall static uint32_t
	ntoskrnl_interlock_dec(/*volatile uint32_t * */ void);
__stdcall static void ntoskrnl_freemdl(ndis_buffer *);
__stdcall static void *ntoskrnl_mmaplockedpages(ndis_buffer *, uint8_t);
__stdcall static void ntoskrnl_init_lock(kspin_lock *);
__stdcall static size_t ntoskrnl_memcmp(const void *, const void *, size_t);
__stdcall static void ntoskrnl_init_ansi_string(ndis_ansi_string *, char *);
__stdcall static void ntoskrnl_init_unicode_string(ndis_unicode_string *,
	uint16_t *);
__stdcall static void ntoskrnl_free_unicode_string(ndis_unicode_string *);
__stdcall static void ntoskrnl_free_ansi_string(ndis_ansi_string *);
__stdcall static ndis_status ntoskrnl_unicode_to_int(ndis_unicode_string *,
	uint32_t, uint32_t *);
static int atoi (const char *);
static long atol (const char *);
__stdcall static uint8_t ntoskrnl_wdmver(uint8_t, uint8_t);
__stdcall static void dummy(void);

static struct mtx *ntoskrnl_interlock;
extern struct mtx_pool *ndis_mtxpool;

int
ntoskrnl_libinit()
{
	ntoskrnl_interlock = mtx_pool_alloc(ndis_mtxpool);
	return(0);
}

int
ntoskrnl_libfini()
{
	return(0);
}

__stdcall static uint8_t 
ntoskrnl_unicode_equal(str1, str2, caseinsensitive)
	ndis_unicode_string	*str1;
	ndis_unicode_string	*str2;
	uint8_t			caseinsensitive;
{
	int			i;

	if (str1->nus_len != str2->nus_len)
		return(FALSE);

	for (i = 0; i < str1->nus_len; i++) {
		if (caseinsensitive == TRUE) {
			if (toupper((char)(str1->nus_buf[i] & 0xFF)) !=
			    toupper((char)(str2->nus_buf[i] & 0xFF)))
				return(FALSE);
		} else {
			if (str1->nus_buf[i] != str2->nus_buf[i])
				return(FALSE);
		}
	}

	return(TRUE);
}

__stdcall static void
ntoskrnl_unicode_copy(dest, src)
	ndis_unicode_string	*dest;
	ndis_unicode_string	*src;
{

	if (dest->nus_maxlen >= src->nus_len)
		dest->nus_len = src->nus_len;
	else
		dest->nus_len = dest->nus_maxlen;
	memcpy(dest->nus_buf, src->nus_buf, dest->nus_len);
	return;
}

__stdcall static ndis_status
ntoskrnl_unicode_to_ansi(dest, src, allocate)
	ndis_ansi_string	*dest;
	ndis_unicode_string	*src;
	uint8_t			allocate;
{
	char			*astr = NULL;

	if (dest == NULL || src == NULL)
		return(NDIS_STATUS_FAILURE);

	if (allocate == TRUE) {
		if (ndis_unicode_to_ascii(src->nus_buf, src->nus_len, &astr))
			return(NDIS_STATUS_FAILURE);
		dest->nas_buf = astr;
		dest->nas_len = dest->nas_maxlen = strlen(astr);
	} else {
		dest->nas_len = src->nus_len / 2; /* XXX */
		if (dest->nas_maxlen < dest->nas_len)
			dest->nas_len = dest->nas_maxlen;
		ndis_unicode_to_ascii(src->nus_buf, dest->nas_len * 2,
		    &dest->nas_buf);
	}
	return (NDIS_STATUS_SUCCESS);
}

__stdcall static ndis_status
ntoskrnl_ansi_to_unicode(dest, src, allocate)
	ndis_unicode_string	*dest;
	ndis_ansi_string	*src;
	uint8_t			allocate;
{
	uint16_t		*ustr = NULL;

	if (dest == NULL || src == NULL)
		return(NDIS_STATUS_FAILURE);

	if (allocate == TRUE) {
		if (ndis_ascii_to_unicode(src->nas_buf, &ustr))
			return(NDIS_STATUS_FAILURE);
		dest->nus_buf = ustr;
		dest->nus_len = dest->nus_maxlen = strlen(src->nas_buf) * 2;
	} else {
		dest->nus_len = src->nas_len * 2; /* XXX */
		if (dest->nus_maxlen < dest->nus_len)
			dest->nus_len = dest->nus_maxlen;
		ndis_ascii_to_unicode(src->nas_buf, &dest->nus_buf);
	}
	return (NDIS_STATUS_SUCCESS);
}

__stdcall static void *
ntoskrnl_iobuildsynchfsdreq(func, dobj, buf, len, off, event, status)
	uint32_t		func;
	void			*dobj;
	void			*buf;
	uint32_t		len;
	uint32_t		*off;
	void			*event;
	void			*status;
{
	return(NULL);
}
	
__stdcall static uint32_t
ntoskrnl_iofcalldriver(/*dobj, irp*/)
{
	void			*dobj;
	void			*irp;

	__asm__ __volatile__ ("" : "=c" (dobj), "=d" (irp));

	return(0);
}

__stdcall static void
ntoskrnl_iofcompletereq(/*irp, prioboost*/)
{
	void			*irp;
	uint8_t			prioboost;

	__asm__ __volatile__ ("" : "=c" (irp), "=d" (prioboost));

	return;
}

__stdcall static uint32_t
ntoskrnl_waitforobj(obj, reason, mode, alertable, timeout)
	void			*obj;
	uint32_t		reason;
	uint32_t		mode;
	uint8_t			alertable;
	void			*timeout;
{
	return(0);
}

__stdcall static void
ntoskrnl_initevent(event, type, state)
	void			*event;
	uint32_t		type;
	uint8_t			state;
{
	return;
}

__stdcall static void
ntoskrnl_writereg_ushort(reg, val)
	uint16_t		*reg;
	uint16_t		val;
{
	bus_space_write_2(NDIS_BUS_SPACE_MEM, 0x0, (bus_size_t)reg, val);
	return;
}

__stdcall static uint16_t
ntoskrnl_readreg_ushort(reg)
	uint16_t		*reg;
{
	return(bus_space_read_2(NDIS_BUS_SPACE_MEM, 0x0, (bus_size_t)reg));
}

__stdcall static void
ntoskrnl_writereg_ulong(reg, val)
	uint32_t		*reg;
	uint32_t		val;
{
	bus_space_write_4(NDIS_BUS_SPACE_MEM, 0x0, (bus_size_t)reg, val);
	return;
}

__stdcall static uint32_t
ntoskrnl_readreg_ulong(reg)
	uint32_t		*reg;
{
	return(bus_space_read_4(NDIS_BUS_SPACE_MEM, 0x0, (bus_size_t)reg));
}

__stdcall static uint8_t
ntoskrnl_readreg_uchar(reg)
	uint8_t			*reg;
{
	return(bus_space_read_1(NDIS_BUS_SPACE_MEM, 0x0, (bus_size_t)reg));
}

__stdcall static void
ntoskrnl_writereg_uchar(reg, val)
	uint8_t			*reg;
	uint8_t			val;
{
	bus_space_write_1(NDIS_BUS_SPACE_MEM, 0x0, (bus_size_t)reg, val);
	return;
}

__stdcall static int64_t
_allmul(a, b)
	int64_t			a;
	int64_t			b;
{
	return (a * b);
}

__stdcall static int64_t
_alldiv(a, b)
	int64_t			a;
	int64_t			b;
{
	return (a / b);
}

__stdcall static int64_t
_allrem(a, b)
	int64_t			a;
	int64_t			b;
{
	return (a % b);
}

__stdcall static uint64_t
_aullmul(a, b)
	uint64_t		a;
	uint64_t		b;
{
	return (a * b);
}

__stdcall static uint64_t
_aulldiv(a, b)
	uint64_t		a;
	uint64_t		b;
{
	return (a / b);
}

__stdcall static uint64_t
_aullrem(a, b)
	uint64_t		a;
	uint64_t		b;
{
	return (a % b);
}

__regparm static int64_t
_allshl(a, b)
	int64_t			a;
	uint8_t			b;
{
	return (a << b);
}

__regparm static uint64_t
_aullshl(a, b)
	uint64_t		a;
	uint8_t			b;
{
	return (a << b);
}

__regparm static int64_t
_allshr(a, b)
	int64_t			a;
	uint8_t			b;
{
	return (a >> b);
}

__regparm static uint64_t
_aullshr(a, b)
	uint64_t		a;
	uint8_t			b;
{
	return (a >> b);
}

__stdcall static void *
ntoskrnl_allocfunc(pooltype, size, tag)
	uint32_t		pooltype;
	size_t			size;
	uint32_t		tag;
{
	return(malloc(size, M_DEVBUF, M_NOWAIT));
}

__stdcall static void
ntoskrnl_freefunc(buf)
	void			*buf;
{
	free(buf, M_DEVBUF);
}

__stdcall static void
ntoskrnl_init_lookaside(lookaside, allocfunc, freefunc,
    flags, size, tag, depth)
	paged_lookaside_list	*lookaside;
	lookaside_alloc_func	*allocfunc;
	lookaside_free_func	*freefunc;
	uint32_t		flags;
	size_t			size;
	uint32_t		tag;
	uint16_t		depth;
{
	struct mtx		*mtx;

	lookaside->nll_l.gl_size = size;
	lookaside->nll_l.gl_tag = tag;
	if (allocfunc == NULL)
		lookaside->nll_l.gl_allocfunc = ntoskrnl_allocfunc;
	else
		lookaside->nll_l.gl_allocfunc = allocfunc;

	if (freefunc == NULL)
		lookaside->nll_l.gl_freefunc = ntoskrnl_freefunc;
	else
		lookaside->nll_l.gl_freefunc = freefunc;

	mtx = mtx_pool_alloc(ndis_mtxpool);
	lookaside->nll_obsoletelock = (kspin_lock)mtx;

	return;
}

__stdcall static void
ntoskrnl_delete_lookaside(lookaside)
	paged_lookaside_list   *lookaside;
{
	return;
}

__stdcall static void
ntoskrnl_init_nplookaside(lookaside, allocfunc, freefunc,
    flags, size, tag, depth)
	npaged_lookaside_list	*lookaside;
	lookaside_alloc_func	*allocfunc;
	lookaside_free_func	*freefunc;
	uint32_t		flags;
	size_t			size;
	uint32_t		tag;
	uint16_t		depth;
{
	struct mtx		*mtx;

	lookaside->nll_l.gl_size = size;
	lookaside->nll_l.gl_tag = tag;
	if (allocfunc == NULL)
		lookaside->nll_l.gl_allocfunc = ntoskrnl_allocfunc;
	else
		lookaside->nll_l.gl_allocfunc = allocfunc;

	if (freefunc == NULL)
		lookaside->nll_l.gl_freefunc = ntoskrnl_freefunc;
	else
		lookaside->nll_l.gl_freefunc = freefunc;

	mtx = mtx_pool_alloc(ndis_mtxpool);
	lookaside->nll_obsoletelock = (kspin_lock)mtx;

	return;
}

__stdcall static void
ntoskrnl_delete_nplookaside(lookaside)
	npaged_lookaside_list   *lookaside;
{
	return;
}

/*
 * Note: the interlocked slist push and pop routines are
 * declared to be _fastcall in Windows. gcc 3.4 is supposed
 * to have support for this calling convention, however we
 * don't have that version available yet, so we kludge things
 * up using some inline assembly.
 */

__stdcall static slist_entry *
ntoskrnl_push_slist(/*head, entry*/ void)
{
	slist_header		*head;
	slist_entry		*entry;
	slist_entry		*oldhead;

	__asm__ __volatile__ ("" : "=c" (head), "=d" (entry));

	mtx_pool_lock(ndis_mtxpool, ntoskrnl_interlock);
	oldhead = head->slh_list.slh_next;
	entry->sl_next = head->slh_list.slh_next;
	head->slh_list.slh_next = entry;
	mtx_pool_unlock(ndis_mtxpool, ntoskrnl_interlock);
	return(oldhead);
}

__stdcall static slist_entry *
ntoskrnl_pop_slist(/*head*/ void)
{
	slist_header		*head;
	slist_entry		*first;

	__asm__ __volatile__ ("" : "=c" (head));

	mtx_pool_lock(ndis_mtxpool, ntoskrnl_interlock);
	first = head->slh_list.slh_next;
	if (first != NULL)
		head->slh_list.slh_next = first->sl_next;
	mtx_pool_unlock(ndis_mtxpool, ntoskrnl_interlock);
	return(first);
}

__stdcall static slist_entry *
ntoskrnl_push_slist_ex(/*head, entry,*/ lock)
	kspin_lock		*lock;
{
	slist_header		*head;
	slist_entry		*entry;
	slist_entry		*oldhead;

	__asm__ __volatile__ ("" : "=c" (head), "=d" (entry));

	mtx_pool_lock(ndis_mtxpool, (struct mtx *)*lock);
	oldhead = head->slh_list.slh_next;
	entry->sl_next = head->slh_list.slh_next;
	head->slh_list.slh_next = entry;
	mtx_pool_unlock(ndis_mtxpool, (struct mtx *)*lock);
	return(oldhead);
}

__stdcall static slist_entry *
ntoskrnl_pop_slist_ex(/*head, lock*/ void)
{
	slist_header		*head;
	kspin_lock		*lock;
	slist_entry		*first;

	__asm__ __volatile__ ("" : "=c" (head), "=d" (lock));

	mtx_pool_lock(ndis_mtxpool, (struct mtx *)*lock);
	first = head->slh_list.slh_next;
	if (first != NULL)
		head->slh_list.slh_next = first->sl_next;
	mtx_pool_unlock(ndis_mtxpool, (struct mtx *)*lock);
	return(first);
}

__stdcall static void
ntoskrnl_lock_dpc(/*lock*/ void)
{
	kspin_lock		*lock;

	__asm__ __volatile__ ("" : "=c" (lock));

	mtx_pool_lock(ndis_mtxpool, (struct mtx *)*lock);

	return;
}

__stdcall static void
ntoskrnl_unlock_dpc(/*lock*/ void)
{
	kspin_lock		*lock;

	__asm__ __volatile__ ("" : "=c" (lock));

	mtx_pool_unlock(ndis_mtxpool, (struct mtx *)*lock);

	return;
}

__stdcall static uint32_t
ntoskrnl_interlock_inc(/*addend*/ void)
{
	volatile uint32_t	*addend;

	__asm__ __volatile__ ("" : "=c" (addend));

	atomic_add_long((volatile u_long *)addend, 1);
	return(*addend);
}

__stdcall static uint32_t
ntoskrnl_interlock_dec(/*addend*/ void)
{
	volatile uint32_t	*addend;

	__asm__ __volatile__ ("" : "=c" (addend));

	atomic_subtract_long((volatile u_long *)addend, 1);
	return(*addend);
}

__stdcall static void
ntoskrnl_freemdl(mdl)
	ndis_buffer		*mdl;
{
	ndis_buffer		*head;

	if (mdl == NULL || mdl->nb_process == NULL)
		return;

        head = mdl->nb_process;

        if (head->nb_flags != 0x1)
                return;

        mdl->nb_next = head->nb_next;
        head->nb_next = mdl;

        return;
}

__stdcall static void *
ntoskrnl_mmaplockedpages(buf, accessmode)
	ndis_buffer		*buf;
	uint8_t			accessmode;
{
	return(MDL_VA(buf));
}

/*
 * The KeInitializeSpinLock(), KefAcquireSpinLockAtDpcLevel()
 * and KefReleaseSpinLockFromDpcLevel() appear to be analagous
 * to splnet()/splx() in their use. We can't create a new mutex
 * lock here because there is no complimentary KeFreeSpinLock()
 * function. Instead, we grab a mutex from the mutex pool.
 */
__stdcall static void
ntoskrnl_init_lock(lock)
	kspin_lock		*lock;
{
	*lock = (kspin_lock)mtx_pool_alloc(ndis_mtxpool);

	return;
}

__stdcall static size_t
ntoskrnl_memcmp(s1, s2, len)
	const void		*s1;
	const void		*s2;
	size_t			len;
{
	size_t			i, total = 0;
	uint8_t			*m1, *m2;

	m1 = __DECONST(char *, s1);
	m2 = __DECONST(char *, s2);

	for (i = 0; i < len; i++) {
		if (m1[i] == m2[i])
			total++;
	}
	return(total);
}

__stdcall static void
ntoskrnl_init_ansi_string(dst, src)
	ndis_ansi_string	*dst;
	char			*src;
{
	ndis_ansi_string	*a;

	a = dst;
	if (a == NULL)
		return;
	if (src == NULL) {
		a->nas_len = a->nas_maxlen = 0;
		a->nas_buf = NULL;
	} else {
		a->nas_buf = src;
		a->nas_len = a->nas_maxlen = strlen(src);
	}

	return;
}

__stdcall static void
ntoskrnl_init_unicode_string(dst, src)
	ndis_unicode_string	*dst;
	uint16_t		*src;
{
	ndis_unicode_string	*u;
	int			i;

	u = dst;
	if (u == NULL)
		return;
	if (src == NULL) {
		u->nus_len = u->nus_maxlen = 0;
		u->nus_buf = NULL;
	} else {
		i = 0;
		while(src[i] != 0)
			i++;
		u->nus_buf = src;
		u->nus_len = u->nus_maxlen = i * 2;
	}

	return;
}

__stdcall ndis_status
ntoskrnl_unicode_to_int(ustr, base, val)
	ndis_unicode_string	*ustr;
	uint32_t		base;
	uint32_t		*val;
{
	uint16_t		*uchr;
	int			len, neg = 0;
	char			abuf[64];
	char			*astr;

	uchr = ustr->nus_buf;
	len = ustr->nus_len;
	bzero(abuf, sizeof(abuf));

	if ((char)((*uchr) & 0xFF) == '-') {
		neg = 1;
		uchr++;
		len -= 2;
	} else if ((char)((*uchr) & 0xFF) == '+') {
		neg = 0;
		uchr++;
		len -= 2;
	}

	if (base == 0) {
		if ((char)((*uchr) & 0xFF) == 'b') {
			base = 2;
			uchr++;
			len -= 2;
		} else if ((char)((*uchr) & 0xFF) == 'o') {
			base = 8;
			uchr++;
			len -= 2;
		} else if ((char)((*uchr) & 0xFF) == 'x') {
			base = 16;
			uchr++;
			len -= 2;
		} else
			base = 10;
	}

	astr = abuf;
	if (neg) {
		strcpy(astr, "-");
		astr++;
	}

	ndis_unicode_to_ascii(uchr, len, &astr);
	*val = strtoul(abuf, NULL, base);

	return(NDIS_STATUS_SUCCESS);
}

__stdcall static void
ntoskrnl_free_unicode_string(ustr)
	ndis_unicode_string	*ustr;
{
	if (ustr->nus_buf == NULL)
		return;
	free(ustr->nus_buf, M_DEVBUF);
	ustr->nus_buf = NULL;
	return;
}

__stdcall static void
ntoskrnl_free_ansi_string(astr)
	ndis_ansi_string	*astr;
{
	if (astr->nas_buf == NULL)
		return;
	free(astr->nas_buf, M_DEVBUF);
	astr->nas_buf = NULL;
	return;
}

static int
atoi(str)
	const char		*str;
{
	return (int)strtol(str, (char **)NULL, 10);
}

static long
atol(str)
	const char		*str;
{
	return strtol(str, (char **)NULL, 10);
}

__stdcall static uint8_t
ntoskrnl_wdmver(major, minor)
	uint8_t			major;
	uint8_t			minor;
{
	if (major == WDM_MAJOR && minor == WDM_MINOR_WINXP)
		return(TRUE);
	return(FALSE);
}

__stdcall static void
dummy()
{
	printf ("ntoskrnl dummy called...\n");
	return;
}


image_patch_table ntoskrnl_functbl[] = {
	{ "RtlCompareMemory",		(FUNC)ntoskrnl_memcmp },
	{ "RtlEqualUnicodeString",	(FUNC)ntoskrnl_unicode_equal },
	{ "RtlCopyUnicodeString",	(FUNC)ntoskrnl_unicode_copy },
	{ "RtlUnicodeStringToAnsiString", (FUNC)ntoskrnl_unicode_to_ansi },
	{ "RtlAnsiStringToUnicodeString", (FUNC)ntoskrnl_ansi_to_unicode },
	{ "RtlInitAnsiString",		(FUNC)ntoskrnl_init_ansi_string },
	{ "RtlInitUnicodeString",	(FUNC)ntoskrnl_init_unicode_string },
	{ "RtlFreeAnsiString",		(FUNC)ntoskrnl_free_ansi_string },
	{ "RtlFreeUnicodeString",	(FUNC)ntoskrnl_free_unicode_string },
	{ "RtlUnicodeStringToInteger",	(FUNC)ntoskrnl_unicode_to_int },
	{ "sprintf",			(FUNC)sprintf },
	{ "vsprintf",			(FUNC)vsprintf },
	{ "DbgPrint",			(FUNC)printf },
	{ "strncmp",			(FUNC)strncmp },
	{ "strcmp",			(FUNC)strcmp },
	{ "strncpy",			(FUNC)strncpy },
	{ "strcpy",			(FUNC)strcpy },
	{ "strlen",			(FUNC)strlen },
	{ "memcpy",			(FUNC)memcpy },
	{ "memset",			(FUNC)memset },
	{ "IofCallDriver",		(FUNC)ntoskrnl_iofcalldriver },
	{ "IofCompleteRequest",		(FUNC)ntoskrnl_iofcompletereq },
	{ "IoBuildSynchronousFsdRequest", (FUNC)ntoskrnl_iobuildsynchfsdreq },
	{ "KeWaitForSingleObject",	(FUNC)ntoskrnl_waitforobj },
	{ "KeInitializeEvent",		(FUNC)ntoskrnl_initevent },
	{ "_allmul",			(FUNC)_allmul },
	{ "_alldiv",			(FUNC)_alldiv },
	{ "_allrem",			(FUNC)_allrem },
	{ "_allshr",			(FUNC)_allshr },
	{ "_allshl",			(FUNC)_allshl },
	{ "_aullmul",			(FUNC)_aullmul },
	{ "_aulldiv",			(FUNC)_aulldiv },
	{ "_aullrem",			(FUNC)_aullrem },
	{ "_aullshr",			(FUNC)_aullshr },
	{ "_aullshl",			(FUNC)_aullshl },
	{ "atoi",			(FUNC)atoi },
	{ "atol",			(FUNC)atol },
	{ "WRITE_REGISTER_USHORT",	(FUNC)ntoskrnl_writereg_ushort },
	{ "READ_REGISTER_USHORT",	(FUNC)ntoskrnl_readreg_ushort },
	{ "WRITE_REGISTER_ULONG",	(FUNC)ntoskrnl_writereg_ulong },
	{ "READ_REGISTER_ULONG",	(FUNC)ntoskrnl_readreg_ulong },
	{ "READ_REGISTER_UCHAR",	(FUNC)ntoskrnl_readreg_uchar },
	{ "WRITE_REGISTER_UCHAR",	(FUNC)ntoskrnl_writereg_uchar },
	{ "ExInitializePagedLookasideList", (FUNC)ntoskrnl_init_lookaside },
	{ "ExDeletePagedLookasideList", (FUNC)ntoskrnl_delete_lookaside },
	{ "ExInitializeNPagedLookasideList", (FUNC)ntoskrnl_init_nplookaside },
	{ "ExDeleteNPagedLookasideList", (FUNC)ntoskrnl_delete_nplookaside },
	{ "InterlockedPopEntrySList",	(FUNC)ntoskrnl_pop_slist },
	{ "InterlockedPushEntrySList",	(FUNC)ntoskrnl_push_slist },
	{ "ExInterlockedPopEntrySList",	(FUNC)ntoskrnl_pop_slist_ex },
	{ "ExInterlockedPushEntrySList",(FUNC)ntoskrnl_push_slist_ex },
	{ "KefAcquireSpinLockAtDpcLevel", (FUNC)ntoskrnl_lock_dpc },
	{ "KefReleaseSpinLockFromDpcLevel", (FUNC)ntoskrnl_unlock_dpc },
	{ "InterlockedIncrement",	(FUNC)ntoskrnl_interlock_inc },
	{ "InterlockedDecrement",	(FUNC)ntoskrnl_interlock_dec },
	{ "IoFreeMdl",			(FUNC)ntoskrnl_freemdl },
	{ "MmMapLockedPages",		(FUNC)ntoskrnl_mmaplockedpages },
	{ "KeInitializeSpinLock",	(FUNC)ntoskrnl_init_lock },
	{ "IoIsWdmVersionAvailable",	(FUNC)ntoskrnl_wdmver },

	/*
	 * This last entry is a catch-all for any function we haven't
	 * implemented yet. The PE import list patching routine will
	 * use it for any function that doesn't have an explicit match
	 * in this table.
	 */

	{ NULL, (FUNC)dummy },

	/* End of list. */

	{ NULL, NULL },
};
