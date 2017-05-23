/*
 * Copyright (C) Jonathan D. Belanger 2017.
 * All Rights Reserved.
 *
 * This software is furnished under a license and may be used and copied only
 * in accordance with the terms of such license and with the inclusion of the
 * above copyright notice.  This software or any other copies thereof may not
 * be provided or otherwise made available to any other person.  No title to
 * and ownership of the software is hereby transferred.
 *
 * The information in this software is subject to change without notice and
 * should not be construed as a commitment by the author or co-authors.
 *
 * The author and any co-authors assume no responsibility for the use or
 * reliability of this software.
 *
 * Description:
 *
 *	This header file contains the structures and definitions required to
 *	implement the instruction emulation for the Alpha 21264 (EV68) processor.
 *
 *	Revision History:
 *
 *	V01.000		04-May-2017	Jonathan D. Belanger
 *	Initially written.
 *	(May the 4th be with you).
 *
 *	V01.001		14-May-2017	Jonathan D. Belanger
 *	Included the AXP_Base_CPU header file and a location for the Program
 *	Counter (PC) in the CPU record.
 */
#ifndef _AXP_21264_CPU_DEFS_
#define _AXP_21264_CPU_DEFS_

#include "AXP_Utility.h"
#include "AXP_Base_CPU.h"
#include "AXP_21264_Predictions.h"
#include "AXP_21264_Instructions.h"

#define AXP_RESULTS_REG		41
#define AXP_NUM_FETCH_INS	4
#define AXP_IQ_LEN			20
#define AXP_FQ_LEN			15
#define AXP_SHADOW_REG		8
#define AXP_R04_SHADOW		(AXP_MAX_REGISTERS + 0)
#define AXP_R05_SHADOW		(AXP_MAX_REGISTERS + 1)
#define AXP_R06_SHADOW		(AXP_MAX_REGISTERS + 2)
#define AXP_R07_SHADOW		(AXP_MAX_REGISTERS + 3)
#define AXP_R20_SHADOW		(AXP_MAX_REGISTERS + 4)
#define AXP_R21_SHADOW		(AXP_MAX_REGISTERS + 5)
#define AXP_R22_SHADOW		(AXP_MAX_REGISTERS + 6)
#define AXP_R23_SHADOW		(AXP_MAX_REGISTERS + 7)
#define AXP_TB_LEN			128
#define AXP_ICB_INS_CNT		16

/*
 * TODO: 	We probably want to decode to determine things like, opcode type,
 *			functional unit (U0, L0, U1, L1, F0, F1), etc.
 */
typedef struct
{
	AXP_INS_FMT	instructions[AXP_NUM_FETCH_INS];
	u8			br_pred;
	u8			line_pred;
} AXP_INS_QUE;

typedef struct
{
	/*
	 * This structure needs to be at the top of all data blocks/structures
	 * that need to be specifically allocated by the Blocks module.
	 */
	AXP_BLOCK_DSC header;

	/**************************************************************************
	 *	Ibox Definitions													  *
	 *																		  *
	 *	The Ibox is responsible for instuction processing.  It maintains the  *
	 *	VPC Queue, ITB, Branch Prediction, Instruction Predecode, Instruction *
	 *	decode and register renaming, Instruction Cache, Instruction		  *
	 *	Retirement, and the Integer and Floating-Point Instruction Queues.	  *
	 *																		  *
	 *	The Ibox interfaces with with the Cbox, Ebox, and Fbox.  The Cbox	  *
	 *	provides the next set of instructions when an Icache miss occurs.	  *
	 *	The set of instructions are provided to the Ibox for predecoding and  *
	 *	entry into the Icache.  The Ebox reads instructions of the Integer	  *
	 *	Issue Queue (IQ) into upto 4 integer processors.  The Fbox reads	  *
	 *	instructions from the FP Issue Queue (FQ) into upto 2 FP processors.  *
	 **************************************************************************/

	/*
	 * The following definitions are used by the branch prediction code.
	 */
	LHT			localHistoryTable;
	LPT			localPredictor;
	GPT			globalPredictor;
	CPT			choicePredictor;
	u16			globalPathHistory;

	/*
	 * Architectural (virtual) registers.
	 */
	u64			r[AXP_MAX_REGISTERS + AXP_SHADOW_REG];	/* Integer (Virt) Reg */
	u64 		f[AXP_MAX_REGISTERS];	/* Floating-point (Virtual) Registers */

	/*
	 * Virtual Program Counter Queue
	 */
	AXP_PC		vpc[AXP_IQ_LEN];
	int			vpcIdx;

	u8			itb[AXP_TB_LEN];

	/*
	 * Instruction Queues (Integer and Floating-Point).
	 */
	AXP_INS_QUE	iq[AXP_IQ_LEN];
	AXP_INS_QUE	fq[AXP_FQ_LEN];

	/*
	 * Ibox Internal Processor Registers (IPRs)
	 */
	AXP_IBOX_ITB_TAG	itbTag;		/* ITB tag array write					*/
	AXP_IBOX_ITB_IS		itbIs;		/* ITB invalidate single				*/
	AXP_IBOX_EXC_ADDR	exeAddr;	/* Exception address					*/
	AXP_IBOX_IVA_FORM	ivaForm;	/* Instruction VA format				*/
	AXP_IBOX_IER_CM		ierCm;		/* Interrupt enable and current mode	*/
	AXP_IBOX_SIRR		sirr;		/* Software interrupt request			*/
	AXP_IBOX_ISUM		iSum;		/* Interrupt summary					*/
	AXP_IBOX_HW_INT_CLR	hwIntClr;	/* Hardware interrupt clear				*/
	AXP_IBOX_EXC_SUM	excSum;		/* Exception summary					*/
	AXP_IBOX_PAL_BASE	palBase;	/* PAL base address						*/
	AXP_IBOX_I_CTL		iCtl;		/* Ibox control							*/
	AXP_IBOX_I_STAT		iStat;		/* Ibox status							*/
	AXP_IBOX_PCTX		pCtx;		/* Process context register				*/
	AXP_IBOX_PCTR_CTL	pCtrCtl;	/* Performance counter control			*/

	/**************************************************************************
	 *	Ebox Definitions													  *
	 *																		  *
	 *	The Ebox is responsible for processing instructions from the IQ.  It  *
	 *	maintains 2 sets of Physical Integer Registers, which are copies of	  *
	 *	one another.  It can handle upto 4 simultaneous instructions.		  *
	 *																		  *
	 *	The Ebox interfaces with the Ibox (see above for more information),	  *
	 *	the Fbox and the Mbox.  The Fbox and Ebox are allowed to move values  *
	 *	from a register in one to the other.  This is does for Integer/FP to  *
	 *	FP/Integer conversion and FP branch operations.  The Mbox provides	  *
	 *	data to the Ebox from memory, via that data cache (Dcache).			  *
	 **************************************************************************/

	/*
	 * Physical registers.
	 *
	 * There are 80 register file entries for integer registers.  This is the
	 * 31 Integer registers (R31 is not stored), plus the 8 shadow registers,
	 * plus the 41 results for instructions that can potentially have not been
	 * retired.
	 *
	 * Since the integer execution unit has 2 cluster, there are a set of 80
	 * registers for each.
	 */
	u64			pr0[AXP_MAX_REGISTERS + AXP_SHADOW_REG + AXP_RESULTS_REG - 1];
	u64			pr1[AXP_MAX_REGISTERS + AXP_SHADOW_REG + AXP_RESULTS_REG - 1];

	/*
	 * Ebox IPRs
	 */
	AXP_EBOX_CC			cc;			/* Cycle counter						*/
	AXP_EBOX_CC_CTL		ccCtl;		/* Cycle counter control				*/
	AXP_EBOX_VA			va;			/* Virtual address						*/
	AXP_EBOX_VA_CTL		vaCtl;		/* Virtual address control				*/
	AXP_EBOX_VA_FORM	vaForm;		/* Virtual address format				*/

	/**************************************************************************
	 *	Fbox Definitions													  *
	 *																		  *
	 *	The Fbox is responsible for processing instructions from the FQ.  It  *
	 *	maintains a sets of Physical Integer Registers.  It can handle upto 2 *
	 *	simultaneous instructions.											  *
	 *																		  *
	 *	The Fbox interfaces with the Ibox (see above for more information),	  *
	 *	the Ebox (See above for more information) and the Mbox.  The Mbox	  *
	 *	provides data to the Fbox from memory, via that data cache (Dcache).  *
	 **************************************************************************/

	/*
	 * Physical registers.
	 *
	 * There are 72 register file entries for the floating-point registers.
	 * This is the 31 Floating-point registers (F31 is not stored), plus the 41
	 * results for instructions that can potentially have not been retired.
	 *
	 * Since the floating-point execution unit only has 1 cluster, there is 
	 * just 1 set of 72 registers.
	 */
	u64			pf[AXP_MAX_REGISTERS + AXP_RESULTS_REG - 1];

	/**************************************************************************
	 *	Mbox Definitions													  *
	 *																		  *
	 *	The Mbox is responsible for providing data to the Ebox and Fbox.  The *
	 *	Mbox maintins a Load and Stor Queue, as well as a Miss Address File.  *
	 *																		  *
	 *	The Mbox interfaces with with the Cbox, Ebox, and Fbox (see above for *
	 *	more information on the last 2).  The Cbox provides data when a		  *
	 *	Dcache miss occurs.  The Mbox provides data to the Cbox to store in	  *
	 *	memory when a store operation occirs.								  *
	 **************************************************************************/
	u8			lq;
	u8			sq;
	u8			maf;
	u8			dtb[AXP_TB_LEN];

	/*
	 * Mbox IPRs
	 */
	AXP_MBOX_DTB_TAG		dtbTag0;	/* DTB tag array write 0			*/
	AXP_MBOX_DTB_TAG		dtbTag1;	/* DTB tag array write 1			*/
	AXP_MBOX_DTB_PTE		dtbPte0;	/* DTB PTE array write 0			*/
	AXP_MBOX_DTB_PTE		dtbPte1;	/* DTB PTE array write 1			*/
	AXP_MBOX_DTB_ALTMODE	dtbAltMode;	/* DTB alternate processor mode		*/
	AXP_MBOX_DTB_IS			dtbIs0;		/* DTB invalidate single 0			*/
	AXP_MBOX_DTB_IS			dtbIs1;		/* DTB invalidate single 1			*/
	AXP_MBOX_DTB_ASN		dtbAsn0;	/* DTB address space number 0		*/
	AXP_MBOX_DTB_ASN		dtbAsn1;	/* DTN address space number 1		*/
	AXP_MBOX_MM_STAT		mmStat;		/* Memory management status			*/
	AXP_MBOX_M_CTL			mCtl;		/* Mbox control						*/
	AXP_MBOX_DC_CTL			dcCtl;		/* Dcache control					*/
	AXP_MBOX_DC_STAT		dcStat;		/* Dcache status					*/

	/**************************************************************************
	 *	Cbox Definitions													  *
	 *																		  *
	 *	The Cbox is responsible for interfacing with the system.  It		  *
	 *	maintains a Probe Queue, Duplicate Tag Store, I/O Write Buffer		  *
	 *	(IOWB), Victim Buffer, and Arbiter.  It interfaces with the System	  *
	 *	(memory, disk drives, I/O devices, etc.), Ibox and Mbox (see above	  *
	 *	for more information about the last 2 items).						  *
	 *																		  *
	 *	The Cbox is responsible for the interfaces between the system and the *
	 *	CPU.																  *
	 **************************************************************************/
	u8			vaf;
	u8			vdf;
	u8			iowb;
	u8			pq;
	u8			dtag;

	/*
	 * Cbox IPRs
	 */
	AXP_CBOX_C_DATA		cData;		/* Cbox data							*/
	AXP_CBOX_C_SHFT		cShft;		/* Cbox shift control					*/

	/*
	 * Alpha AXP Architectural IPRs
	 */
	AXP_BASE_ASN		asn;		/* Address Space Number					*/
	AXP_BASE_ASTEN		astEn;		/* AST Enable							*/
	AXP_BASE_ASTSR		astSr;		/* AST Summary Register					*/
	AXP_BASE_DATFX		datFx;		/* Data Alignment Trap Fixup			*/
	AXP_BASE_ESP		esp;		/* Executive Stack Pointer				*/
	AXP_BASE_FEN		fen;		/* Floating-point Enable				*/
	AXP_BASE_IPIR		ipIr;		/* Interprocessor Interrupt Request		*/
	AXP_BASE_IPL		ipl;		/* Interrupt Priority Level				*/
	AXP_BASE_KSP		ksp;		/* Kernel Stack Pointer					*/
	AXP_BASE_MCES		mces;		/* Machine Check Error Summary			*/
	AXP_BASE_PCBB		pcbb;		/* Privileged Context Block Base		*/
	AXP_BAST_PRPB		prbr;		/* Processor Base Register				*/
	AXP_BAST_PTBR		ptbr;		/* Page Table Base Register				*/
	AXP_BASE_SCBB		scbb;		/* System Control Block Base			*/
	AXP_BASE_SISR		sisr;		/* Software Interrupt Summary Register	*/
	AXP_BASE_SSP		ssp;		/* Supervisor Stack Pointer				*/
	AXP_BASE_SYSPTBR	sysPtbr;	/* System Page Table Base				*/
	AXP_BASE TBCHK		tbChk;		/* TB Check								*/
	AXP_BASE_USP		usp;		/* User Stack Pointer					*/
	AXP_BASE_VIRBND		virBnd;		/* Virtual Address Boundary				*/
	AXP_BASE_VPTB		vptb;		/* Virtual Page Table Base				*/
	AXP_BASE_WHAMI		whami;		/* Who-Am-I								*/

} AXP_21264_CPU;
#endif /* _AXP_21264_CPU_DEFS_ */