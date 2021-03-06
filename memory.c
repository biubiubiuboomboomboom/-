/* ΦW */

#include "bootpack.h"

#define EFLAGS_AC_BIT		0x00040000
#define CR0_CACHE_DISABLE	0x60000000

unsigned int memtest(unsigned int start, unsigned int end)
{
	char flg486 = 0;
	unsigned int eflg, cr0, i;

	/* ??CPU₯386?₯486ΘγI */
	eflg = io_load_eflags();
	eflg |= EFLAGS_AC_BIT; /* AC-bit = 1 */
	io_store_eflags(eflg);
	eflg = io_load_eflags();
	if ((eflg & EFLAGS_AC_BIT) != 0) { 
	/* @Κ₯386C¦g?θAC=1CACI??ο©?ρ0 */
		flg486 = 1;
	}
	eflg &= ~EFLAGS_AC_BIT; /* AC-bit = 0 */
	io_store_eflags(eflg);

	if (flg486 != 0) {
		cr0 = load_cr0();
		cr0 |= CR0_CACHE_DISABLE; /* Φ~?Ά */
		store_cr0(cr0);
	}

	i = memtest_sub(start, end);

	if (flg486 != 0) {
		cr0 = load_cr0();
		cr0 &= ~CR0_CACHE_DISABLE; /* ς??Ά */
		store_cr0(cr0);
	}

	return i;
}

void memman_init(struct MEMMAN *man)
{
	man->frees = 0;			/* ΒpM§Ϊ */
	man->maxfrees = 0;		/* freesIΕε? */
	man->lostsize = 0;		/* Έ?IΰΆε¬?a */
	man->losts = 0;			/* ?ϊΈ? */
}

unsigned int memman_total(struct MEMMAN *man)
/* ?σ]ΰΆε¬I? */
{
	unsigned int i, t = 0;
	for (i = 0; i < man->frees; i++) {
		t += man->free[i].size;
	}
	return t;
}

unsigned int memman_alloc(struct MEMMAN *man, unsigned int size)
/* ͺz */
{
	unsigned int i, a;
	for (i = 0; i < man->frees; i++) {
		if (man->free[i].size >= size) {
			/* QΉ«?εIΰΆ */
			a = man->free[i].addr;
			man->free[i].addr += size;
			man->free[i].size -= size;
			if (man->free[i].size == 0) {
				/* @Κfree[i]?¬Ή0CA?{κπΒpM§ */
				man->frees--;
				for (; i < man->frees; i++) {
					man->free[i] = man->free[i + 1]; /* γό??Μ */
				}
			}
			return a;
		}
	}
	return 0; /*ΩΒpσ? */
}

int memman_free(struct MEMMAN *man, unsigned int addr, unsigned int size)
/* ?ϊ */
{
	int i, j;
	/* ?Φ°??ΰΆC«free[]ΒΖaddrI?rρ */
	/* ΘCζrθ??ϊέ?’ */
	for (i = 0; i < man->frees; i++) {
		if (man->free[i].addr > addr) {
			break;
		}
	}
	/* free[i - 1].addr < addr < free[i].addr */
	if (i > 0) {
		/* OΚLΒpΰΆ */
		if (man->free[i - 1].addr + man->free[i - 1].size == addr) {
			/* ΒΘ^OΚIΒpΰΆ??κN */
			man->free[i - 1].size += size;
			if (i < man->frees) {
				/* @ΚηL */
				if (addr + size == man->free[i].addr) {
					/* ηΒΘ^@ΚIΒpΰΆ??κN */
					man->free[i - 1].size += man->free[i].size;
					/* man->free[i]? */
					/* free[i]?¬0@??OΚ */
					man->frees--;
					for (; i < man->frees; i++) {
						man->free[i] = man->free[i + 1]; /* ??Μ?? */
					}
				}
			}
			return 0; /* ¬χ */
		}
	}
	/* s\^OΚIΒpσ???κN */
	if (i < man->frees) {
		/* @Κ?L */
		if (addr + size == man->free[i].addr) {
			/* ΒΘ^@ΚIΰe??κN */
			man->free[i].addr = addr;
			man->free[i].size += size;
			return 0; /* ¬χ */
		}
	}
	/* ωs\^OΚ??κNCηs\^@Κ??κN */
	if (man->frees < MEMMAN_FREES) {
		/* free[i]V@ICό@Ϊ?C?oκ_Βpσ? */
		for (j = man->frees; j > i; j--) {
			man->free[j] = man->free[j - 1];
		}
		man->frees++;
		if (man->maxfrees < man->frees) {
			man->maxfrees = man->frees; /* XVΕε? */
		}
		man->free[i].addr = addr;
		man->free[i].size = size;
		return 0; /* ¬χ */
	}
	/* s\@Ϊ? */
	man->losts++;
	man->lostsize += size;
	return -1;  /* Έ? */
}

unsigned int memman_alloc_4k(struct MEMMAN *man, unsigned int size)
{
	unsigned int a;
	size = (size + 0xfff) & 0xfffff000;
	a = memman_alloc(man, size);
	return a;
}

int memman_free_4k(struct MEMMAN *man, unsigned int addr, unsigned int size)
{
	int i;
	size = (size + 0xfff) & 0xfffff000;
	i = memman_free(man, addr, size);
	return i;
}
