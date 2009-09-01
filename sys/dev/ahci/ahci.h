/*-
 * Copyright (c) 1998 - 2008 S�ren Schmidt <sos@FreeBSD.org>
 * Copyright (c) 2009 Alexander Motin <mav@FreeBSD.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer,
 *    without modification, immediately at the beginning of the file.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * $FreeBSD$
 */

/* ATA register defines */
#define ATA_DATA                        0       /* (RW) data */

#define ATA_FEATURE                     1       /* (W) feature */
#define         ATA_F_DMA               0x01    /* enable DMA */
#define         ATA_F_OVL               0x02    /* enable overlap */

#define ATA_COUNT                       2       /* (W) sector count */

#define ATA_SECTOR                      3       /* (RW) sector # */
#define ATA_CYL_LSB                     4       /* (RW) cylinder# LSB */
#define ATA_CYL_MSB                     5       /* (RW) cylinder# MSB */
#define ATA_DRIVE                       6       /* (W) Sector/Drive/Head */
#define         ATA_D_LBA               0x40    /* use LBA addressing */
#define         ATA_D_IBM               0xa0    /* 512 byte sectors, ECC */

#define ATA_COMMAND                     7       /* (W) command */

#define ATA_ERROR                       8       /* (R) error */
#define         ATA_E_ILI               0x01    /* illegal length */
#define         ATA_E_NM                0x02    /* no media */
#define         ATA_E_ABORT             0x04    /* command aborted */
#define         ATA_E_MCR               0x08    /* media change request */
#define         ATA_E_IDNF              0x10    /* ID not found */
#define         ATA_E_MC                0x20    /* media changed */
#define         ATA_E_UNC               0x40    /* uncorrectable data */
#define         ATA_E_ICRC              0x80    /* UDMA crc error */
#define		ATA_E_ATAPI_SENSE_MASK	0xf0	/* ATAPI sense key mask */

#define ATA_IREASON                     9       /* (R) interrupt reason */
#define         ATA_I_CMD               0x01    /* cmd (1) | data (0) */
#define         ATA_I_IN                0x02    /* read (1) | write (0) */
#define         ATA_I_RELEASE           0x04    /* released bus (1) */
#define         ATA_I_TAGMASK           0xf8    /* tag mask */

#define ATA_STATUS                      10      /* (R) status */
#define ATA_ALTSTAT                     11      /* (R) alternate status */
#define         ATA_S_ERROR             0x01    /* error */
#define         ATA_S_INDEX             0x02    /* index */
#define         ATA_S_CORR              0x04    /* data corrected */
#define         ATA_S_DRQ               0x08    /* data request */
#define         ATA_S_DSC               0x10    /* drive seek completed */
#define         ATA_S_SERVICE           0x10    /* drive needs service */
#define         ATA_S_DWF               0x20    /* drive write fault */
#define         ATA_S_DMA               0x20    /* DMA ready */
#define         ATA_S_READY             0x40    /* drive ready */
#define         ATA_S_BUSY              0x80    /* busy */

#define ATA_CONTROL                     12      /* (W) control */
#define         ATA_A_IDS               0x02    /* disable interrupts */
#define         ATA_A_RESET             0x04    /* RESET controller */
#define         ATA_A_4BIT              0x08    /* 4 head bits */
#define         ATA_A_HOB               0x80    /* High Order Byte enable */

/* SATA register defines */
#define ATA_SSTATUS                     13
#define         ATA_SS_DET_MASK         0x0000000f
#define         ATA_SS_DET_NO_DEVICE    0x00000000
#define         ATA_SS_DET_DEV_PRESENT  0x00000001
#define         ATA_SS_DET_PHY_ONLINE   0x00000003
#define         ATA_SS_DET_PHY_OFFLINE  0x00000004

#define         ATA_SS_SPD_MASK         0x000000f0
#define         ATA_SS_SPD_NO_SPEED     0x00000000
#define         ATA_SS_SPD_GEN1         0x00000010
#define         ATA_SS_SPD_GEN2         0x00000020
#define         ATA_SS_SPD_GEN3         0x00000040

#define         ATA_SS_IPM_MASK         0x00000f00
#define         ATA_SS_IPM_NO_DEVICE    0x00000000
#define         ATA_SS_IPM_ACTIVE       0x00000100
#define         ATA_SS_IPM_PARTIAL      0x00000200
#define         ATA_SS_IPM_SLUMBER      0x00000600

#define ATA_SERROR                      14
#define         ATA_SE_DATA_CORRECTED   0x00000001
#define         ATA_SE_COMM_CORRECTED   0x00000002
#define         ATA_SE_DATA_ERR         0x00000100
#define         ATA_SE_COMM_ERR         0x00000200
#define         ATA_SE_PROT_ERR         0x00000400
#define         ATA_SE_HOST_ERR         0x00000800
#define         ATA_SE_PHY_CHANGED      0x00010000
#define         ATA_SE_PHY_IERROR       0x00020000
#define         ATA_SE_COMM_WAKE        0x00040000
#define         ATA_SE_DECODE_ERR       0x00080000
#define         ATA_SE_PARITY_ERR       0x00100000
#define         ATA_SE_CRC_ERR          0x00200000
#define         ATA_SE_HANDSHAKE_ERR    0x00400000
#define         ATA_SE_LINKSEQ_ERR      0x00800000
#define         ATA_SE_TRANSPORT_ERR    0x01000000
#define         ATA_SE_UNKNOWN_FIS      0x02000000

#define ATA_SCONTROL                    15
#define         ATA_SC_DET_MASK         0x0000000f
#define         ATA_SC_DET_IDLE         0x00000000
#define         ATA_SC_DET_RESET        0x00000001
#define         ATA_SC_DET_DISABLE      0x00000004

#define         ATA_SC_SPD_MASK         0x000000f0
#define         ATA_SC_SPD_NO_SPEED     0x00000000
#define         ATA_SC_SPD_SPEED_GEN1   0x00000010
#define         ATA_SC_SPD_SPEED_GEN2   0x00000020
#define         ATA_SC_SPD_SPEED_GEN3   0x00000040

#define         ATA_SC_IPM_MASK         0x00000f00
#define         ATA_SC_IPM_NONE         0x00000000
#define         ATA_SC_IPM_DIS_PARTIAL  0x00000100
#define         ATA_SC_IPM_DIS_SLUMBER  0x00000200

#define ATA_SACTIVE                     16

#define AHCI_MAX_PORTS			32
#define AHCI_MAX_SLOTS			32

/* SATA AHCI v1.0 register defines */
#define AHCI_CAP                    0x00
#define		AHCI_CAP_NPMASK	0x0000001f
#define		AHCI_CAP_SXS	0x00000020
#define		AHCI_CAP_EMS	0x00000040
#define		AHCI_CAP_CCCS	0x00000080
#define		AHCI_CAP_NCS	0x00001F00
#define		AHCI_CAP_NCS_SHIFT	8
#define		AHCI_CAP_PSC	0x00002000
#define		AHCI_CAP_SSC	0x00004000
#define		AHCI_CAP_PMD	0x00008000
#define		AHCI_CAP_FBSS	0x00010000
#define		AHCI_CAP_SPM	0x00020000
#define		AHCI_CAP_SAM	0x00080000
#define		AHCI_CAP_ISS	0x00F00000
#define		AHCI_CAP_ISS_SHIFT	20
#define		AHCI_CAP_SCLO	0x01000000
#define		AHCI_CAP_SAL	0x02000000
#define		AHCI_CAP_SALP	0x04000000
#define		AHCI_CAP_SSS	0x08000000
#define		AHCI_CAP_SMPS	0x10000000
#define		AHCI_CAP_SSNTF	0x20000000
#define		AHCI_CAP_SNCQ	0x40000000
#define		AHCI_CAP_64BIT	0x80000000

#define AHCI_GHC                    0x04
#define         AHCI_GHC_AE         0x80000000
#define         AHCI_GHC_MRSM       0x00000004
#define         AHCI_GHC_IE         0x00000002
#define         AHCI_GHC_HR         0x00000001

#define AHCI_IS                     0x08
#define AHCI_PI                     0x0c
#define AHCI_VS                     0x10

#define AHCI_CCCC                   0x14
#define		AHCI_CCCC_TV_MASK	0xffff0000
#define		AHCI_CCCC_TV_SHIFT	16
#define		AHCI_CCCC_CC_MASK	0x0000ff00
#define		AHCI_CCCC_CC_SHIFT	8
#define		AHCI_CCCC_INT_MASK	0x000000f8
#define		AHCI_CCCC_INT_SHIFT	3
#define		AHCI_CCCC_EN		0x00000001
#define AHCI_CCCP                   0x18

#define AHCI_CAP2                   0x24
#define		AHCI_CAP2_BOH	0x00000001
#define		AHCI_CAP2_NVMP	0x00000002
#define		AHCI_CAP2_APST	0x00000004

#define AHCI_OFFSET                 0x100
#define AHCI_STEP                   0x80

#define AHCI_P_CLB                  0x00
#define AHCI_P_CLBU                 0x04
#define AHCI_P_FB                   0x08
#define AHCI_P_FBU                  0x0c
#define AHCI_P_IS                   0x10
#define AHCI_P_IE                   0x14
#define         AHCI_P_IX_DHR       0x00000001
#define         AHCI_P_IX_PS        0x00000002
#define         AHCI_P_IX_DS        0x00000004
#define         AHCI_P_IX_SDB       0x00000008
#define         AHCI_P_IX_UF        0x00000010
#define         AHCI_P_IX_DP        0x00000020
#define         AHCI_P_IX_PC        0x00000040
#define         AHCI_P_IX_DI        0x00000080

#define         AHCI_P_IX_PRC       0x00400000
#define         AHCI_P_IX_IPM       0x00800000
#define         AHCI_P_IX_OF        0x01000000
#define         AHCI_P_IX_INF       0x04000000
#define         AHCI_P_IX_IF        0x08000000
#define         AHCI_P_IX_HBD       0x10000000
#define         AHCI_P_IX_HBF       0x20000000
#define         AHCI_P_IX_TFE       0x40000000
#define         AHCI_P_IX_CPD       0x80000000

#define AHCI_P_CMD                  0x18
#define         AHCI_P_CMD_ST       0x00000001
#define         AHCI_P_CMD_SUD      0x00000002
#define         AHCI_P_CMD_POD      0x00000004
#define         AHCI_P_CMD_CLO      0x00000008
#define         AHCI_P_CMD_FRE      0x00000010
#define         AHCI_P_CMD_CCS_MASK 0x00001f00
#define         AHCI_P_CMD_CCS_SHIFT 8
#define         AHCI_P_CMD_ISS      0x00002000
#define         AHCI_P_CMD_FR       0x00004000
#define         AHCI_P_CMD_CR       0x00008000
#define         AHCI_P_CMD_CPS      0x00010000
#define         AHCI_P_CMD_PMA      0x00020000
#define         AHCI_P_CMD_HPCP     0x00040000
#define         AHCI_P_CMD_ISP      0x00080000
#define         AHCI_P_CMD_CPD      0x00100000
#define         AHCI_P_CMD_ATAPI    0x01000000
#define         AHCI_P_CMD_DLAE     0x02000000
#define         AHCI_P_CMD_ALPE     0x04000000
#define         AHCI_P_CMD_ASP      0x08000000
#define         AHCI_P_CMD_ICC_MASK 0xf0000000
#define         AHCI_P_CMD_NOOP     0x00000000
#define         AHCI_P_CMD_ACTIVE   0x10000000
#define         AHCI_P_CMD_PARTIAL  0x20000000
#define         AHCI_P_CMD_SLUMBER  0x60000000

#define AHCI_P_TFD                  0x20
#define AHCI_P_SIG                  0x24
#define AHCI_P_SSTS                 0x28
#define AHCI_P_SCTL                 0x2c
#define AHCI_P_SERR                 0x30
#define AHCI_P_SACT                 0x34
#define AHCI_P_CI                   0x38
#define AHCI_P_SNTF                 0x3C
#define AHCI_P_FBS                  0x40

/* Just to be sure, if building as module. */
#if MAXPHYS < 512 * 1024
#undef MAXPHYS
#define MAXPHYS				512 * 1024
#endif
/* Pessimistic prognosis on number of required S/G entries */
#define AHCI_SG_ENTRIES	(roundup(btoc(MAXPHYS) + 1, 8))
/* Command list. 32 commands. First, 1Kbyte aligned. */
#define AHCI_CL_OFFSET              0
#define AHCI_CL_SIZE                32
/* Command tables. Up to 32 commands, Each, 128byte aligned. */
#define AHCI_CT_OFFSET              (AHCI_CL_OFFSET + AHCI_CL_SIZE * AHCI_MAX_SLOTS)
#define AHCI_CT_SIZE                (128 + AHCI_SG_ENTRIES * 16)
/* Total main work area. */
#define AHCI_WORK_SIZE              (AHCI_CT_OFFSET + AHCI_CT_SIZE * ch->numslots)

struct ahci_dma_prd {
    u_int64_t                   dba;
    u_int32_t                   reserved;
    u_int32_t                   dbc;            /* 0 based */
#define AHCI_PRD_MASK		0x003fffff      /* max 4MB */
#define AHCI_PRD_MAX		(AHCI_PRD_MASK + 1)
#define AHCI_PRD_IPC		(1 << 31)
} __packed;

struct ahci_cmd_tab {
    u_int8_t                    cfis[64];
    u_int8_t                    acmd[32];
    u_int8_t                    reserved[32];
    struct ahci_dma_prd         prd_tab[AHCI_SG_ENTRIES];
} __packed;

struct ahci_cmd_list {
    u_int16_t                   cmd_flags;
#define AHCI_CMD_ATAPI		0x0020
#define AHCI_CMD_WRITE		0x0040
#define AHCI_CMD_PREFETCH		0x0080
#define AHCI_CMD_RESET		0x0100
#define AHCI_CMD_BIST		0x0200
#define AHCI_CMD_CLR_BUSY		0x0400

    u_int16_t                   prd_length;     /* PRD entries */
    u_int32_t                   bytecount;
    u_int64_t                   cmd_table_phys; /* 128byte aligned */
} __packed;

/* misc defines */
#define ATA_IRQ_RID                     0
#define ATA_INTR_FLAGS                  (INTR_MPSAFE|INTR_TYPE_BIO|INTR_ENTROPY)

struct ata_dmaslot {
    bus_dmamap_t                data_map;       /* data DMA map */
    int				nsegs;		/* Number of segs loaded */
};

/* structure holding DMA related information */
struct ata_dma {
    bus_dma_tag_t               work_tag;       /* workspace DMA tag */
    bus_dmamap_t                work_map;       /* workspace DMA map */
    uint8_t                     *work;          /* workspace */
    bus_addr_t                  work_bus;       /* bus address of work */
    bus_dma_tag_t               rfis_tag;       /* RFIS list DMA tag */
    bus_dmamap_t                rfis_map;       /* RFIS list DMA map */
    uint8_t                     *rfis;          /* FIS receive area */
    bus_addr_t                  rfis_bus;       /* bus address of rfis */
    bus_dma_tag_t               data_tag;       /* data DMA tag */
    u_int64_t                   max_address;    /* highest DMA'able address */
};

enum ahci_slot_states {
	AHCI_SLOT_EMPTY,
	AHCI_SLOT_LOADING,
	AHCI_SLOT_RUNNING,
	AHCI_SLOT_WAITING
};

struct ahci_slot {
    device_t                    dev;            /* Device handle */
    u_int8_t			slot;           /* Number of this slot */
    enum ahci_slot_states	state;          /* Slot state */
    union ccb			*ccb;		/* CCB occupying slot */
    struct ata_dmaslot          dma;            /* DMA data of this slot */
    struct callout              timeout;        /* Execution timeout */
};

/* structure describing an ATA channel */
struct ahci_channel {
	device_t		dev;            /* Device handle */
	int			unit;           /* Physical channel */
	struct resource		*r_mem;		/* Memory of this channel */
	struct resource		*r_irq;         /* Interrupt of this channel */
	void			*ih;            /* Interrupt handle */
	struct ata_dma		dma;            /* DMA data */
	struct cam_sim		*sim;
	struct cam_path		*path;
	uint32_t		caps;		/* Controller capabilities */
	uint32_t		caps2;		/* Controller capabilities */
	int			numslots;	/* Number of present slots */
	int			pm_level;	/* power management level */
	int			sata_rev;	/* Maximum allowed SATA generation */

	struct ahci_slot	slot[AHCI_MAX_SLOTS];
	union ccb		*hold[AHCI_MAX_SLOTS];
	struct mtx		mtx;		/* state lock */
	int			devices;        /* What is present */
	int			pm_present;	/* PM presence reported */
	uint32_t		rslots;		/* Running slots */
	uint32_t		aslots;		/* Slots with atomic commands  */
	int			numrslots;	/* Number of running slots */
	int			numtslots;	/* Number of tagged slots */
	int			readlog;	/* Our READ LOG active */
	int			lastslot;	/* Last used slot */
	int			taggedtarget;	/* Last tagged target */
	union ccb		*frozen;	/* Frozen command */
	struct callout		pm_timer;	/* Power management events */
};

/* structure describing a AHCI controller */
struct ahci_controller {
	device_t		dev;
	int			r_rid;
	struct resource		*r_mem;
	struct rman		sc_iomem;
	struct ahci_controller_irq {
		struct ahci_controller	*ctlr;
		struct resource		*r_irq;
		void			*handle;
		int			r_irq_rid;
		int			mode;
#define	AHCI_IRQ_MODE_ALL	0
#define	AHCI_IRQ_MODE_AFTER	1
#define	AHCI_IRQ_MODE_ONE	2
	} irqs[16];
	uint32_t		caps;		/* Controller capabilities */
	uint32_t		caps2;		/* Controller capabilities */
	int			numirqs;
	int			channels;
	int			ichannels;
	int			ccc;		/* CCC timeout */
	int			cccv;		/* CCC vector */
	struct {
		void			(*function)(void *);
		void			*argument;
	} interrupt[AHCI_MAX_PORTS];
};

enum ahci_err_type {
	AHCI_ERR_NONE,		/* No error */
	AHCI_ERR_INVALID,	/* Error detected by us before submitting. */
	AHCI_ERR_INNOCENT,	/* Innocent victim. */
	AHCI_ERR_TFE,		/* Task File Error. */
	AHCI_ERR_SATA,		/* SATA error. */
	AHCI_ERR_TIMEOUT,	/* Command execution timeout. */
	AHCI_ERR_NCQ,		/* NCQ command error. CCB should be put on hold
				 * until READ LOG executed to reveal error. */
};

/* macros to hide busspace uglyness */
#define ATA_INB(res, offset) \
	bus_read_1((res), (offset))
#define ATA_INW(res, offset) \
	bus_read_2((res), (offset))
#define ATA_INL(res, offset) \
	bus_read_4((res), (offset))
#define ATA_INSW(res, offset, addr, count) \
	bus_read_multi_2((res), (offset), (addr), (count))
#define ATA_INSW_STRM(res, offset, addr, count) \
	bus_read_multi_stream_2((res), (offset), (addr), (count))
#define ATA_INSL(res, offset, addr, count) \
	bus_read_multi_4((res), (offset), (addr), (count))
#define ATA_INSL_STRM(res, offset, addr, count) \
	bus_read_multi_stream_4((res), (offset), (addr), (count))
#define ATA_OUTB(res, offset, value) \
	bus_write_1((res), (offset), (value))
#define ATA_OUTW(res, offset, value) \
	bus_write_2((res), (offset), (value))
#define ATA_OUTL(res, offset, value) \
	bus_write_4((res), (offset), (value))
#define ATA_OUTSW(res, offset, addr, count) \
	bus_write_multi_2((res), (offset), (addr), (count))
#define ATA_OUTSW_STRM(res, offset, addr, count) \
	bus_write_multi_stream_2((res), (offset), (addr), (count))
#define ATA_OUTSL(res, offset, addr, count) \
	bus_write_multi_4((res), (offset), (addr), (count))
#define ATA_OUTSL_STRM(res, offset, addr, count) \
	bus_write_multi_stream_4((res), (offset), (addr), (count))
