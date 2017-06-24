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
 *	This source file contains the functions needed to implement the
 *	Load and Store functionality of the Ebox.
 *
 *	Revision History:
 *
 *	V01.000		19-Jun-2017	Jonathan D. Belanger
 *	Initially written.
 */
#include "AXP_21264_Ebox_LoadStore.h"

/*
 * IMPLEMENTATION NOTES:
 *
 * 		1)	If R31 is a destination register, then the code that selects the
 * 			instruction for executionfrom the IQ, will determine this and just
 * 			move the instruction state to WaitingRetirement.  The exception to
 * 			this are the LDL and LDQ instructions, where these instructions
 * 			become PREFETCH and PREFETCH_EN, respectively.
 * 		2)	When these functions are called, the instruction state is set to
 * 			Executing prior to the call.
 */

/*
 * AXP_LDA
 *	This function implements the Load Address instruction of the Alpha AXP
 *	processor.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the structure containing the information needed to emulate
 *		a single CPU.
 * 	instr:
 * 		A pointer to a structure containing the information needed to execute
 * 		this instruction.
 *
 * Output Parameters:
 * 	instr:
 * 		The contents of this structure are updated, as needed.
 *
 * Return Value:
 * 	An exception indicator.
 */
AXP_EXCEPTIONS AXP_LDA(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{

	/*
	 * Implement the instruction.
	 */
	instr->destv = instr->src1v + instr->displacement;

	/*
	 * Indicate that the instruction is ready to be retired.
	 */
	instr->state = WaitingRetirement;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(NoException);
}

/*
 * AXP_LDAH
 *	This function implements the Load Address High instruction of the Alpha AXP
 *	processor.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the structure containing the information needed to emulate
 *		a single CPU.
 * 	instr:
 * 		A pointer to a structure containing the information needed to execute
 * 		this instruction.
 *
 * Output Parameters:
 * 	instr:
 * 		The contents of this structure are updated, as needed.
 *
 * Return Value:
 * 	An exception indicator.
 */
AXP_EXCEPTIONS AXP_LDAH(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{

	/*
	 * Implement the instruction.
	 */
	instr->destv = instr->src1v + (instr->displacement * AXP_LDAH_MULT);

	/*
	 * Indicate that the instruction is ready to be retired.
	 */
	instr->state = WaitingRetirement;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(NoException);
}

/*
 * AXP_LDBU
 *	This function implements the Load Zero-Extend Byte from Memory to Register
 *	instruction of the Alpha AXP processor.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the structure containing the information needed to emulate
 *		a single CPU.
 * 	instr:
 * 		A pointer to a structure containing the information needed to execute
 * 		this instruction.
 *
 * Output Parameters:
 * 	instr:
 * 		The contents of this structure are updated, as needed.
 *
 * Return Value:
 * 	An exception indicator.
 */
AXP_EXCEPTIONS AXP_LDBU(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	AXP_EXCEPTIONS retVal = NoException;
	u64 va, vaPrime;

	/*
	 * Implement the instruction.
	 */
	vaPrime = va = instr->src1v + instr->displacement;

	/*
	 * If we are executing in big-endian mode, then we need to do some address
	 * adjustment.
	 */
	if (cpu->vaCtl.b_endian == 1)
		vaPrime = AXP_BIG_ENDIAN_BYTE(va);
	instr->destv = AXP_ZEXT_BYTE((vaPrime));		// TODO: Load from mem/cache
	// TODO: Check to see if we had an access fault (Access Violation)
	// TODO: Check to see if we had a read fault (Fault on Read)
	// TODO: Check to see if we had a translation fault (Translation Not Valid)

	/*
	 * Indicate that the instruction is ready to be retired.
	 */
	instr->state = WaitingRetirement;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(retVal);
}

/*
 * AXP_LDWU
 *	This function implements the Load Zero-Extend Word from Memory to Register
 *	instruction of the Alpha AXP processor.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the structure containing the information needed to emulate
 *		a single CPU.
 * 	instr:
 * 		A pointer to a structure containing the information needed to execute
 * 		this instruction.
 *
 * Output Parameters:
 * 	instr:
 * 		The contents of this structure are updated, as needed.
 *
 * Return Value:
 * 	An exception indicator.
 */
AXP_EXCEPTIONS AXP_LDWU(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	AXP_EXCEPTIONS retVal = NoException;
	u64 va, vaPrime;

	/*
	 * Implement the instruction.
	 */
	vaPrime = va = instr->src1v + instr->displacement;

	/*
	 * If we are executing in big-endian mode, then we need to do some address
	 * adjustment.
	 */
	if (cpu->vaCtl.b_endian == 1)
		vaPrime = AXP_BIG_ENDIAN_WORD(va);
	instr->destv = AXP_ZEXT_WORD((vaPrime));		// TODO: Load from mem/cache
	// TODO: Check to see if we had an access fault (Access Violation)
	// TODO: Check to see if we had an alignment fault (Alignment)
	// TODO: Check to see if we had a read fault (Fault on Read)
	// TODO: Check to see if we had a translation fault (Translation Not Valid)

	/*
	 * Indicate that the instruction is ready to be retired.
	 */
	instr->state = WaitingRetirement;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(retVal);
}

/*
 * AXP_LDL
 *	This function implements the Load/Prefetch Sign-Extend Longword from Memory
 *	to Register/no-where instruction of the Alpha AXP processor.
 *
 *	If the destination register is R31, then this instruction becomes the
 *	PREFETCH instruction.
 *
 *	A prefetch is a hint to the processor that a cache block might be used in
 *	the future and should be brought into the cache now.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the structure containing the information needed to emulate
 *		a single CPU.
 * 	instr:
 * 		A pointer to a structure containing the information needed to execute
 * 		this instruction.
 *
 * Output Parameters:
 * 	instr:
 * 		The contents of this structure are updated, as needed.
 *
 * Return Value:
 * 	An exception indicator.
 */
AXP_EXCEPTIONS AXP_LDL(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	AXP_EXCEPTIONS retVal = NoException;
	u64 va, vaPrime;

	/*
	 * Implement the instruction.
	 */
	vaPrime = va = instr->src1v + instr->displacement;

	/*
	 * If we are executing in big-endian mode, then we need to do some address
	 * adjustment.
	 */
	if (cpu->vaCtl.b_endian == 1)
		vaPrime = AXP_BIG_ENDIAN_LONG(va);
	instr->destv = AXP_SEXT_LONG((vaPrime));		// TODO: Load from mem/cache
	// TODO: Check to see if we had an access fault (Access Violation)
	// TODO: Check to see if we had an alignment fault (Alignment)
	// TODO: Check to see if we had a read fault (Fault on Read)
	// TODO: Check to see if we had a translation fault (Translation Not Valid)

	/*
	 * Indicate that the instruction is ready to be retired.
	 */
	instr->state = WaitingRetirement;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(retVal);
}

/*
 * AXP_LDQ
 *	This function implements the Load/Prefetch Quadword from Memory to
 *	Register/no-where instruction of the Alpha AXP processor.
 *
 *	If the destination register is R31, then this instruction becomes the
 *	PREFETCH_EN instruction.
 *
 *	A prefetch, evict next, is a hint to the processor that a cache block
 *	should be brought into the cache now and marked for preferential eviction
 *	on future cache fills.  Such a prefetch is particularly useful with an
 *	associative cache, to prefetch data that is not repeatedly referenced --
 *	data that has a short temporal lifetime in the cache.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the structure containing the information needed to emulate
 *		a single CPU.
 * 	instr:
 * 		A pointer to a structure containing the information needed to execute
 * 		this instruction.
 *
 * Output Parameters:
 * 	instr:
 * 		The contents of this structure are updated, as needed.
 *
 * Return Value:
 * 	An exception indicator.
 */
AXP_EXCEPTIONS AXP_LDQ(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	AXP_EXCEPTIONS retVal = NoException;
	u64 va;

	/*
	 * Implement the instruction.
	 */
	va = instr->src1v + instr->displacement;

	instr->destv = (va);					// TODO: Load from mem/cache
	// TODO: Check to see if we had an access fault (Access Violation)
	// TODO: Check to see if we had an alignment fault (Alignment)
	// TODO: Check to see if we had a read fault (Fault on Read)
	// TODO: Check to see if we had a translation fault (Translation Not Valid)

	/*
	 * Indicate that the instruction is ready to be retired.
	 */
	instr->state = WaitingRetirement;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(retVal);
}

/*
 * AXP_LDQ_U
 *	This function implements the Unaligned Load Quadword from Memory to
 *	Register/no-where instruction of the Alpha AXP processor.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the structure containing the information needed to emulate
 *		a single CPU.
 * 	instr:
 * 		A pointer to a structure containing the information needed to execute
 * 		this instruction.
 *
 * Output Parameters:
 * 	instr:
 * 		The contents of this structure are updated, as needed.
 *
 * Return Value:
 * 	An exception indicator.
 */
AXP_EXCEPTIONS AXP_LDQ_U(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	AXP_EXCEPTIONS retVal = NoException;
	u64 va;

	/*
	 * Implement the instruction.
	 */
	va = (instr->src1v + instr->displacement) & ~0x7;

	instr->destv = (va);					// TODO: Load from mem/cache
	// TODO: Check to see if we had an access fault (Access Violation)
	// TODO: Check to see if we had a read fault (Fault on Read)
	// TODO: Check to see if we had a translation fault (Translation Not Valid)

	/*
	 * Indicate that the instruction is ready to be retired.
	 */
	instr->state = WaitingRetirement;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(retVal);
}

/*
 * TODO:	The 21264 does not contain a dedicated lock register, not are any
 * TODO:	system components required to do so.
 * TODO:
 * TODO:	When a load-lock instruction executed, data is accessed from the
 * TODO:	Dcache (or Bcache).  If there is a cache miss, data is access from
 * TODO:	memory with a RdBlk command.  Its associated cache line is filled
 * TODO:	into the Dcache in the clean state, if it's not already there.
 * TODO:
 * TODO:	When a store-conditional instruction executes, it is allowed to
 * TODO:	succeed if it's associated cache line is still present in the
 * TODO:	Dcache and can be made writable; otherwise it fails.
 * TODO:
 * TODO:	This algorithm is successful because another agent in the system
 * TODO:	writing to the cache line between the load-lock and
 * TODO:	store-conditional cache line would make the cache line invalid.
 * TODO:
 * TODO:	The following code does not take any of this into account and will
 * TODO:	need to be corrected.
 */

/*
 * AXP_LDL_L
 *	This function implements the Load Longword Memory Data into Integer
 *	Register Locked instruction of the Alpha AXP processor.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the structure containing the information needed to emulate
 *		a single CPU.
 * 	instr:
 * 		A pointer to a structure containing the information needed to execute
 * 		this instruction.
 *
 * Output Parameters:
 * 	instr:
 * 		The contents of this structure are updated, as needed.
 *
 * Return Value:
 * 	An exception indicator.
 */
AXP_EXCEPTIONS AXP_LDL_L(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	AXP_EXCEPTIONS retVal = NoException;
	u64 va, vaPrime;

	/*
	 * Implement the instruction.
	 */
	vaPrime = va = instr->src1v + instr->displacement;

	/*
	 * If we are executing in big-endian mode, then we need to do some address
	 * adjustment.
	 */
	if (cpu->vaCtl.b_endian == 1)
		vaPrime = AXP_BIG_ENDIAN_LONG(va);

	instr->lockFlagPending = true;
	instr->lockPhysAddrPending = va;	// TODO: Need to convert to a physical address
	instr->lockVirtAddrPending = va;

	instr->destv = AXP_SEXT_LONG((vaPrime));		// TODO: Load from mem/cache
	// TODO: Check to see if we had an access fault (Access Violation)
	// TODO: Check to see if we had an alignment fault (Alignment)
	// TODO: Check to see if we had a read fault (Fault on Read)
	// TODO: Check to see if we had a translation fault (Translation Not Valid)

	/*
	 * Indicate that the instruction is ready to be retired.
	 */
	instr->state = WaitingRetirement;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(retVal);
}

/*
 * AXP_LDQ_L
 *	This function implements the Load Quadword Memory Data into Integer
 *	Register Locked instruction of the Alpha AXP processor.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the structure containing the information needed to emulate
 *		a single CPU.
 * 	instr:
 * 		A pointer to a structure containing the information needed to execute
 * 		this instruction.
 *
 * Output Parameters:
 * 	instr:
 * 		The contents of this structure are updated, as needed.
 *
 * Return Value:
 * 	An exception indicator.
 */
AXP_EXCEPTIONS AXP_LDQ_L(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	AXP_EXCEPTIONS retVal = NoException;
	u64 va;

	/*
	 * Implement the instruction.
	 */
	va = instr->src1v + instr->displacement;

	instr->lockFlagPending = true;
	instr->lockPhysAddrPending = va;	// TODO: Need to convert to a physical address
	instr->lockVirtAddrPending = va;

	instr->destv = AXP_SEXT_LONG((va));		// TODO: Load from mem/cache
	// TODO: Check to see if we had an access fault (Access Violation)
	// TODO: Check to see if we had an alignment fault (Alignment)
	// TODO: Check to see if we had a read fault (Fault on Read)
	// TODO: Check to see if we had a translation fault (Translation Not Valid)

	/*
	 * Indicate that the instruction is ready to be retired.
	 */
	instr->state = WaitingRetirement;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(retVal);
}

/*
 * AXP_STL_C
 *	This function implements the Store Longword Integer Register into Memory
 *	Conditional instruction of the Alpha AXP processor.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the structure containing the information needed to emulate
 *		a single CPU.
 * 	instr:
 * 		A pointer to a structure containing the information needed to execute
 * 		this instruction.
 *
 * Output Parameters:
 * 	instr:
 * 		The contents of this structure are updated, as needed.
 *
 * Return Value:
 * 	An exception indicator.
 */
AXP_EXCEPTIONS AXP_STL_C(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	AXP_EXCEPTIONS retVal = NoException;
	u64 va, vaPrime;

	/*
	 * Implement the instruction.
	 */
	vaPrime = va = instr->src1v + instr->displacement;

	/*
	 * If we are executing in big-endian mode, then we need to do some address
	 * adjustment.
	 */
	if (cpu->vaCtl.b_endian == 1)
		vaPrime = AXP_BIG_ENDIAN_LONG(va);

	if (cpu->lockFlag == true)
	{
		(vaPrime) = instr->src1v;
		// TODO: Check to see if we had an access fault (Access Violation)
		// TODO: Check to see if we had an alignment fault (Alignment)
		// TODO: Check to see if we had a write fault (Fault on Write)
		// TODO: Check to see if we had a translation fault (Translation Not Valid)
		instr->destv = 1;
	}
	else
		instr->destv = 0;
	instr->clearLockPending = true;

	/*
	 * Indicate that the instruction is ready to be retired.
	 */
	instr->state = WaitingRetirement;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(retVal);
}

/*
 * AXP_STQ_C
 *	This function implements the Store Quadword Integer Register into Memory
 *	Conditional instruction of the Alpha AXP processor.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the structure containing the information needed to emulate
 *		a single CPU.
 * 	instr:
 * 		A pointer to a structure containing the information needed to execute
 * 		this instruction.
 *
 * Output Parameters:
 * 	instr:
 * 		The contents of this structure are updated, as needed.
 *
 * Return Value:
 * 	An exception indicator.
 */
AXP_EXCEPTIONS AXP_STQ_C(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	AXP_EXCEPTIONS retVal = NoException;
	u64 va;

	/*
	 * Implement the instruction.
	 */
	va = instr->src1v + instr->displacement;

	if (cpu->lockFlag == true)
	{
		(va) = instr->src1v;
		// TODO: Check to see if we had an access fault (Access Violation)
		// TODO: Check to see if we had an alignment fault (Alignment)
		// TODO: Check to see if we had a write fault (Fault on Write)
		// TODO: Check to see if we had a translation fault (Translation Not Valid)
		instr->destv = 1;
	}
	else
		instr->destv = 0;
	instr->clearLockPending = true;

	/*
	 * Indicate that the instruction is ready to be retired.
	 */
	instr->state = WaitingRetirement;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(retVal);
}

/*
 * AXP_STB
 *	This function implements the Store Byte Integer Register into Memory
 *	instruction of the Alpha AXP processor.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the structure containing the information needed to emulate
 *		a single CPU.
 * 	instr:
 * 		A pointer to a structure containing the information needed to execute
 * 		this instruction.
 *
 * Output Parameters:
 * 	instr:
 * 		The contents of this structure are updated, as needed.
 *
 * Return Value:
 * 	An exception indicator.
 */
AXP_EXCEPTIONS AXP_STB(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	AXP_EXCEPTIONS retVal = NoException;
	u64 va, vaPrime;

	/*
	 * Implement the instruction.
	 */
	vaPrime = va = instr->src1v + instr->displacement;

	/*
	 * If we are executing in big-endian mode, then we need to do some address
	 * adjustment.
	 */
	if (cpu->vaCtl.b_endian == 1)
		vaPrime = AXP_BIG_ENDIAN_BYTE(va);
	(vaPrime) = AXP_BYTE_MASK(instr->src1v);
		// TODO: Check to see if we had an access fault (Access Violation)
		// TODO: Check to see if we had an alignment fault (Alignment)
		// TODO: Check to see if we had a write fault (Fault on Write)
		// TODO: Check to see if we had a translation fault (Translation Not Valid)

	/*
	 * Indicate that the instruction is ready to be retired.
	 */
	instr->state = WaitingRetirement;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(retVal);
}

/*
 * AXP_STW
 *	This function implements the Store Word Integer Register into Memory
 *	instruction of the Alpha AXP processor.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the structure containing the information needed to emulate
 *		a single CPU.
 * 	instr:
 * 		A pointer to a structure containing the information needed to execute
 * 		this instruction.
 *
 * Output Parameters:
 * 	instr:
 * 		The contents of this structure are updated, as needed.
 *
 * Return Value:
 * 	An exception indicator.
 */
AXP_EXCEPTIONS AXP_STW(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	AXP_EXCEPTIONS retVal = NoException;
	u64 va, vaPrime;

	/*
	 * Implement the instruction.
	 */
	vaPrime = va = instr->src1v + instr->displacement;

	/*
	 * If we are executing in big-endian mode, then we need to do some address
	 * adjustment.
	 */
	if (cpu->vaCtl.b_endian == 1)
		vaPrime = AXP_BIG_ENDIAN_WORD(va);
	(vaPrime) = AXP_WORD_MASK(instr->src1v);
		// TODO: Check to see if we had an access fault (Access Violation)
		// TODO: Check to see if we had an alignment fault (Alignment)
		// TODO: Check to see if we had a write fault (Fault on Write)
		// TODO: Check to see if we had a translation fault (Translation Not Valid)

	/*
	 * Indicate that the instruction is ready to be retired.
	 */
	instr->state = WaitingRetirement;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(retVal);
}

/*
 * AXP_STL
 *	This function implements the Store Longword Integer Register into Memory
 *	instruction of the Alpha AXP processor.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the structure containing the information needed to emulate
 *		a single CPU.
 * 	instr:
 * 		A pointer to a structure containing the information needed to execute
 * 		this instruction.
 *
 * Output Parameters:
 * 	instr:
 * 		The contents of this structure are updated, as needed.
 *
 * Return Value:
 * 	An exception indicator.
 */
AXP_EXCEPTIONS AXP_STL(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	AXP_EXCEPTIONS retVal = NoException;
	u64 va, vaPrime;

	/*
	 * Implement the instruction.
	 */
	vaPrime = va = instr->src1v + instr->displacement;

	/*
	 * If we are executing in big-endian mode, then we need to do some address
	 * adjustment.
	 */
	if (cpu->vaCtl.b_endian == 1)
		vaPrime = AXP_BIG_ENDIAN_LONG(va);
	(vaPrime) = AXP_LONG_MASK(instr->src1v);
		// TODO: Check to see if we had an access fault (Access Violation)
		// TODO: Check to see if we had an alignment fault (Alignment)
		// TODO: Check to see if we had a write fault (Fault on Write)
		// TODO: Check to see if we had a translation fault (Translation Not Valid)

	/*
	 * Indicate that the instruction is ready to be retired.
	 */
	instr->state = WaitingRetirement;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(retVal);
}

/*
 * AXP_STQ
 *	This function implements the Store Quadword Integer Register into Memory
 *	instruction of the Alpha AXP processor.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the structure containing the information needed to emulate
 *		a single CPU.
 * 	instr:
 * 		A pointer to a structure containing the information needed to execute
 * 		this instruction.
 *
 * Output Parameters:
 * 	instr:
 * 		The contents of this structure are updated, as needed.
 *
 * Return Value:
 * 	An exception indicator.
 */
AXP_EXCEPTIONS AXP_STQ(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	AXP_EXCEPTIONS retVal = NoException;
	u64 va;

	/*
	 * Implement the instruction.
	 */
	va = instr->src1v + instr->displacement;

	(va) = instr->src1v;
		// TODO: Check to see if we had an access fault (Access Violation)
		// TODO: Check to see if we had an alignment fault (Alignment)
		// TODO: Check to see if we had a write fault (Fault on Write)
		// TODO: Check to see if we had a translation fault (Translation Not Valid)

	/*
	 * Indicate that the instruction is ready to be retired.
	 */
	instr->state = WaitingRetirement;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(retVal);
}

/*
 * AXP_STQ_U
 *	This function implements the Store Unaligned Quadword Integer Register into
 *	Memory instruction of the Alpha AXP processor.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the structure containing the information needed to emulate
 *		a single CPU.
 * 	instr:
 * 		A pointer to a structure containing the information needed to execute
 * 		this instruction.
 *
 * Output Parameters:
 * 	instr:
 * 		The contents of this structure are updated, as needed.
 *
 * Return Value:
 * 	An exception indicator.
 */
AXP_EXCEPTIONS AXP_STQ_U(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	AXP_EXCEPTIONS retVal = NoException;
	u64 va;

	/*
	 * Implement the instruction.
	 */
	va = (instr->src1v + instr->displacement) & ~0x7;

	(va) = instr->src1v;
		// TODO: Check to see if we had an access fault (Access Violation)
		// TODO: Check to see if we had a write fault (Fault on Write)
		// TODO: Check to see if we had a translation fault (Translation Not Valid)

	/*
	 * Indicate that the instruction is ready to be retired.
	 */
	instr->state = WaitingRetirement;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(retVal);
}
