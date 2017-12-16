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
 *	functionality of the Mbox.
 *
 *	Revision History:
 *
 *	V01.000		19-Jun-2017	Jonathan D. Belanger
 *	Initially written.
 */
#include "AXP_Configure.h"
#include "AXP_21264_Mbox.h"

/*
 * AXP_21264_Mbox_GetLQSlot
 *	This function is called to get the next available Load slot.  They are
 *	assigned in instruction order.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the structure containing the information needed to emulate 
 *		a single CPU.
 *
 * Output Parameters:
 *	cpu:
 *		The LQ slot index is incremented, if there is room for another load
 *		request.
 *
 * Return Value:
 *	The value of the slot to be used for the Load instruction.  If there are no
 *	slots available a value of the size of the LoadQueue will be returned.
 */
u32 AXP_21264_Mbox_GetLQSlot(AXP_21264_CPU *cpu)
{
	u32 retVal = AXP_MBOX_QUEUE_LEN;

	/*
	 * Let's make sure the index does not get updated while we are accessing
	 * it.
	 */
	pthread_mutex_lock(&cpu->lqMutex);

	/*
	 * If there is another slot available, get is to return to the caller and
	 * increment the index.  As loads are completed, the index will be reduced.
	 */
	if (cpu->lqNext < AXP_MBOX_QUEUE_LEN)
	{
		retVal = cpu->lqNext++;
		cpu->lq[retVal].state = Assigned;
	}

	/*
	 * Don't forget to unlock the LQ mutex.
	 */
	pthread_mutex_unlock(&cpu->lqMutex);

	/*
	 * Returned the assigned slot back to the caller.
	 */
	return(retVal);
}

/*
 * AXP_21264_Mbox_ReadMem
 *	This function is called to queue up a read from Dcache based on a virtual
 *	address, size of the data to be read and the instruction that is queued up
 *	to be completed in order.  This function works with the AXP_Mbox_WriteMem
 *	function to ensure correct Alpha memory reference behavior.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the structure containing the information needed to emulate 
 *		a single CPU.
 *	instr:
 *		A pointer to the decoded instruction.  When the read is complete, the
 *		value from memory is store in the instr->destv location and the
 *		instruction marked as WaitingForCompletion.
 *	slot:
 *		A value indicating the assigned Load Queue (LQ) where this read entry
 *		is to be stored.
 *	virtAddr:
 *		A value containing the virtual address, full 64-bits, where the value
 *		to be read is located.
 *
 * Output Parameters:
 *	instr:
 *		A pointer to the decoded instruction.  It is updated with the value
 *		read from the Dcache.
 *
 * Return Value:
 *	None.
 */
void AXP_21264_Mbox_ReadMem(AXP_21264_CPU *cpu,
							AXP_INSTRUCTION *instr,
							u32 slot,
							u64 virtAddr)
{

	/*
	 * Let's make sure the Mbox does not try to update or access the LQ while
	 * we are accessing it.
	 */
	pthread_mutex_lock(&cpu->mBoxMutex);

	/*
	 * Store the information in the
	 */
	cpu->lq[slot].virtAddress = virtAddr;
	cpu->lq[slot].instr = instr;
	cpu->lq[slot].state = Initial;

	/*
	 * Notify the Mbox that there is something to process and unlock the Mbox
	 * mutex so it can start performing the processing we just requested.
	 */
	pthread_cond_signal(&cpu->mBoxCondition, &cpu->mBoxMutex);
	pthread_mutex_unlock(&cpu->mBoxMutex);

	/*
	 * Return back to the caller.
	 */
	return;
 }
 
/*
 * AXP_21264_Mbox_GetSQSlot
 *	This function is called to get the next available Store slot.  They are
 *	assigned in instruction order.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the structure containing the information needed to emulate 
 *		a single CPU.
 *
 * Output Parameters:
 *	cpu:
 *		The SQ slot index is incremented, if there is room for another store
 *		request.
 *
 * Return Value:
 *	The value of the slot to be used for the Store instruction.  If there are no
 *	slots available a value of the size of the StoreQueue will be returned.
 */
u32 AXP_21264_Mbox_GetSQSlot(AXP_21264_CPU *cpu)
{
	u32 retVal = AXP_MBOX_QUEUE_LEN;

	/*
	 * Let's make sure the index does not get updated while we are accessing
	 * it.
	 */
	pthread_mutex_lock(&cpu->sqMutex);

	/*
	 * If there is another slot available, get is to return to the caller and
	 * increment the index.  As stores are completed, the index will be reduced.
	 */
	if (cpu->sqNext < AXP_MBOX_QUEUE_LEN)
	{
		retVal = cpu->sqNext++;
		cpu->sq[retVal].state = Assigned;
	}

	/*
	 * Don't forget to unlock the SQ mutex.
	 */
	pthread_mutex_unlock(&cpu->sqMutex);

	/*
	 * Returned the assigned slot back to the caller.
	 */
	return(retVal);
}

/*
 * AXP_21264_Mbox_WriteMem
 *	This function is called to queue up a write to the Dcache based on a
 *	virtual address, size of the data to be written, the value of the data and
 *	the instruction that is queued up to be completed in order.  This function
 *	works with the AXP_Mbox_ReadMem function to ensure correct Alpha memory
 *	reference behavior.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the structure containing the information needed to emulate 
 *		a single CPU.
 *	instr:
 *		A pointer to the decoded instruction.  When the write is completed, the
 *		instruction marked as WaitingForCompletion.
 *	slot:
 *		A value indicating the assigned Store Queue (SQ) where this write entry
 *		is to be stored.
 *	virtAddr:
 *		A value containing the virtual address, full 64-bits, where the value
 *		to be written is located.
 *	value:
 *		The value to be be written to the Dcache (and ultimately memory).
 *
 * Output Parameters:
 *	None.
 *
 * Return Value:
 *	None.
 */
void AXP_21264_Mbox_WriteMem(AXP_21264_CPU *cpu,
							 AXP_INSTRUCTION *instr,
							 u32 slot,
							 u64 virtAddr,
							 u64 value)
{

	/*
	 * Let's make sure the Mbox does not try to update or access the SQ while
	 * we are accessing it.
	 */
	pthread_mutex_lock(&cpu->mBoxMutex);
	
	/*
	 * Store the information in the
	 */
	cpu->sq[slot].value = value;
	cpu->sq[slot].virtAddress = virtAddr;
	cpu->sq[slot].instr = instr;
	cpu->sq[slot].state = Initial;

	/*
	 * Notify the Mbox that there is something to process and unlock the Mbox
	 * mutex so it can start performing the processing we just requested.
	 */
	pthread_cond_signal(&cpu->mBoxCondition, &cpu->mBoxMutex);
	pthread_mutex_unlock(&cpu->mBoxMutex);

	/*
	 * Return back to the caller.
	 */
	return;
}

/*
 * AXP_21264_Mbox_TryCaches
 *	This function is called to see if what we are looking to do with the cache
 *	can be done.  It checks the Dcache state and if acceptable, does the things
 *	needed for the Ibox to retire the associated instruction.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the structure containing the information needed to emulate
 *		a single CPU.
 *	entry:
 *		The value of the index into the LQ.
 *
 * Output Parameters:
 *	lqEntry:
 *		A pointer to the entry which has been set up so that the instruction
 *		can be retired (either when the data arrives or now).
 *
 * Return Value:
 *	None.
 *
 * NOTE: When we are called, the Mbox mutex is already locked.  No need to lock
 * it here.
 */
void AXP_21264_Mbox_TryCaches(AXP_21264_CPU *cpu, u8 entry)
{
	AXP_MBOX_QUEUE	*lqEntry = &cpu->lq[entry];
	int				ii;
	u8				cacheStatus;
	u8				nextOlderStore;
	bool			olderStore = false;

	/*
	 * Loop through each of the SQ entries.
	 */
	for (ii = 0; ii < AXP_MBOX_QUEUE_LEN; ii++)
	{

		/*
		 * Look deeper at the SQ entries that are in process.
		 */
		if ((cpu->sq[ii].state == Initial) ||
			(cpu->sq[ii].state == SQWritePending) ||
			(cpu->sq[ii].state == SQComplete))
		{

			/*
			 * If the address for the store matches that for the load and
			 * the length for the store is greater than or equal to the load
			 * and the store is older than the load,, then we can source from
			 * the store (instead of the Dcache and/or Bcache).
			 */
			if ((lqEntry->virtAddress == cpu->sq[ii].virtAddress) &&
				(lqEntry->len <= cpu->sq[ii].len) &&
				(lqEntry->instr->uniqueID < cpu->sq[ii].instr->uniqueID))
			{

				/*
				 * OK, there may have been multiple stores to the same address,
				 * so get the most recent older store.  Otherwise, if this is
				 * the first one, set the index and the flag.
				 */
				if ((olderStore == true) &&
					(cpu->sq[ii].instr->uniqueID >
					 cpu->sq[nextOlderStore].instr->uniqueID))
					nextOlderStore = ii;
				else if (olderStore == false)
				{
					nextOlderStore = ii;
					olderStore = true;
				}
			}
		}
	}

	/*
	 * IF we didn't find an older store, we need to see if the information we
	 * need is in the Dcache or Bcache and in the proper state.
	 */
	if (olderStore == false)
	{
		bool	DcHit = false;

		/*
		 * Get the status for the Dcache for the current Va/PA pair.
		 */
		cacheStatus = AXP_Dcache_Status(
							cpu,
							lqEntry->virtAddress,
							lqEntry->physAddress);

		/*
		 * If we did not get a DcHit (See HRM Table 4-1), then we need to check
		 * the Bcache.
		 */
		if (cacheStatus != AXP_21264_CACHE_HIT)
		{

			/*
			 * Get the status for the Bcache for the current PA.
			 */
			cacheStatus = AXP_21264_Bcache_Status(
									cpu,
									lqEntry->physAddress);

			/*
			 * Missed both Caches (Dcache and Bcache).  Put an entry in the
			 * Missed Address File (MAF) for the Cbox to process.  There is
			 * nothing else for us to do here.
			 */
			if (cacheStatus != AXP_21264_CACHE_HIT)
			{
				AXP_21264_Add_MAF(
					cpu,
					LDx,
					lqEntry->physAddress,
					entry,
					NULL,
					lqEntry->len);
			}

			/*
			 * Hit in the Bcache, move the data to the Dcache, which may
			 * require evicting the current entry.
			 */
			else
			{

				/*
				 * We found what we were looking for in the Bcache.  We may
				 * need to evict the current block (possibly the same index and
				 * set, but not the same physical tag).
				 */
				AXP_21264_CopyBcacheToDcache(
										cpu,
										lqEntry->virtAddress,
										lqEntry->physAddress);
				DcHit = true;
			}
		}
		else
			DcHit = true;

		/*
		 * If we hit in the Dcache, them read the data out of it.
		 */
		if (DcHit == true)
		{
			bool	retVal;

			lqEntry->instr->destv = 0;
			retVal = AXP_DcacheRead(
						cpu,
						lqEntry->virtAddress,
						lqEntry->physAddress,
						lqEntry->len,
						&lqEntry->instr->destv,
						NULL);
			lqEntry->state = LQComplete;
		}
	}

	/*
	 * We found what we were looking for in a store that is older than the load
	 * currently being processed.
	 */
	else
	{
		u8	*dest8 = (u8 *) &lqEntry->instr->destv;
		u16	*dest16 = (u16 *) &lqEntry->instr->destv;
		u32	*dest32 = (u32 *) &lqEntry->instr->destv;
		u64	*dest64 = (u64 *) &lqEntry->instr->destv;

		lqEntry->instr->destv = 0;
		switch (lqEntry->len)
		{
			case 1:
				*dest8 = *((u8 *) &cpu->sq[nextOlderStore].value);
				break;

			case 2:
				*dest16 = *((u16 *) cpu->sq[nextOlderStore].value);
				break;

			case 4:
				*dest32 = *((u32 *) cpu->sq[nextOlderStore].value);
				break;

			case 8:
				*dest64 = *((u64 *) cpu->sq[nextOlderStore].value);
				break;
		}
		lqEntry->state = LQComplete;
	}
	return;
}

/*
 * AXP_21264_Mbox_LQ_Init
 *	This function is called to initialize the entry.  This is a new entry in
 *	in the LQ for processing.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the structure containing the information needed to emulate
 *		a single CPU.
 *	entry:
 *		The value of the index into the LQ.
 *
 * Output Parameters:
 *	lqEntry:
 *		A pointer to the entry which has been initialized.
 *
 * Return Value:
 *	None.
 *
 * NOTE: When we are called, the Mbox mutex is already locked.  No need to lock
 * it here.
 */
void AXP_21264_Mbox_LQ_Init(AXP_21264_CPU *cpu, u8 entry)
{
	AXP_MBOX_QUEUE *lqEntry = &cpu->lq[entry];
	u32		fault;
	bool 	_asm;

	/*
	 * First, determine the length of the load.
	 */
	switch (lqEntry->instr->opcode)
	{

		case LDBU:
			lqEntry->len = 1;
			break;

		case LDW_U:
			lqEntry->len = 2;
			break;

		case LDF:
		case LDS:
		case LDL:
		case LDL_L:
			lqEntry->len = 4;
			break;

		case LDA:
		case LDAH:
		case LDQ_U:
		case LDG:
		case LDT:
		case LDQ:
		case LDQ_L:
			lqEntry->len = 8;
			break;

		case HW_LD:
			if (lqEntry->instr->len_stall = AXP_HW_LD_LONGWORD)
				lqEntry->len = 4;
			else
				lqEntry->len = 8;
			break;
	}

	/*
	 * Then we need to do is translate the virtual address into its physical
	 * equivalent.
	 */
	lqEntry->physAddress = AXP_va2pa(
								cpu,
								lqEntry->virtAddress,
								lqEntry->instr->pc,
								true,	/* use the DTB */
								Read,
								&_asm,
								&fault);

	/*
	 * If a physical address was returned, then we have some more to do.
	 */
	if (lqEntry->physAddress != 0)
	{

		/*
		 * Set a flag indicating the the address is for an I/O device (and not
		 * memory)
		 */
		lqEntry->IOflag = AXP_21264_IS_IO_ADDR(lqEntry->physAddress);
		lqEntry->state = LQReadPending;

		/*
		 * At this point we have 2 options.  First, this is a load from memory.
		 * Second, this is a load from an I/O device.
		 */
		if (lqEntry->IOflag == false)
			AXP_21264_Mbox_TryCaches(cpu, entry);

		/*
		 * OK, this is a load from an I/O device.  We just send the request to
		 * the Cbox.
		 */
		else
			AXP_21264_Add_IOWB(
					cpu,
					lqEntry->physAddress,
					entry,
					NULL,
					lqEntry->len);
	}
	else
	{
		AXP_21264_Ibox_Event(
					cpu,
					fault,
					lqEntry->instr->pc,
					lqEntry->virtAddress,
					lqEntry->instr->opcode,
					lqEntry->instr->aDest,
					false,
					false);
	}

	return;
}

/*
 * AXP_21264_Mbox_LQ_Pending
 *	This function is called to check if a pending the entry in the LQ can be
 *	completed.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the structure containing the information needed to emulate
 *		a single CPU.
 *	entry:
 *		The value of the index into the LQ.
 *
 * Output Parameters:
 *	lqEntry:
 *		A pointer to the entry which has been completed.
 *
 * Return Value:
 *	None.
 *
 * NOTE: When we are called, the Mbox mutex is already locked.  No need to lock
 * it here.
 */
void AXP_21264_Mbox_LQ_Pending(AXP_21264_CPU *cpu, u8 entry)
{

	/*
	 * TODO: All of it.
	 */
	return;
}
/*
 * AXP_21264_Mbox_Process_Q
 *	This function is called because we just received and indication that one or
 *	more entries in the LQ and/or SQ require processed.  This function searches
 *	through all the entries and determines the next processing that is
 *	required.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the structure containing the information needed to emulate
 *		a single CPU.
 *
 * Output Parameters:
 *	None.
 *
 * Return Value:
 *	None.
 *
 * NOTE: When we are called, the Mbox mutex is already locked.  No need to lock
 * it here.
 */
void AXP_21264_Mbox_Process_Q(AXP_21264_CPU *cpu)
{
	u32 ii;

	/*
	 * First the Load Queue (LQ) entries.
	 */
	for (ii = 0; ii < AXP_MBOX_QUEUE_LEN; ii++)
	{
		switch (cpu->lq[ii].state)
		{
			case Initial:
				AXP_21264_Mbox_LQ_Init(cpu, ii);
				break;

			case LQReadPending:
				AXP_21264_Mbox_LQ_Pending(cpu, ii);
				break;

			default:
				/* nothing to be done */
				break;
		}

		/*
		 * Because the above calls can and do complete LQ entries by the time
		 * they return.  If the state of the entry is now Complete, then call
		 * the code to finish up with this request and get it back to the Ebox
		 * or Fbox.
		 */
		if (cpu->lq[ii].state == LQComplete)
			AXP_21264_Mbox_LQ_Complete(cpu, ii);
	}

	/*
	 * Last the store Queue (SQ) entries.
	 */
	for (ii = 0; ii < AXP_MBOX_QUEUE_LEN; ii++)
		switch (cpu->sq[ii].state)
		{
			case Initial:
				AXP_21264_Mbox_SQ_Init(cpu, ii);
				break;

			case LQReadPending:
				AXP_21264_Mbox_SQ_Pending(cpu, ii);
				break;

			case LQReadable:
				AXP_21264_Mbox_SQ_Complete(cpu, ii);
				break;

			default:
				/* nothing to be done */
				break;

		}
	return;
}

/*
 * AXP_21264_Mbox_Init
 *	This function is called by the Cbox to initialize the Mbox items.  These
 *	items are:
 *
 *		- Dcache
 *		- Dcache Tag
 *		- Duplicate Tag
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the structure containing the information needed to emulate
 *		a single CPU.
 *
 * Output Parameters:
 *	None.
 *
 * Return Value:
 *	true:	Failed to initialize the Mbox
 *	false:	Successful normal completion.
 */
bool AXP_21264_Mbox_Init(AXP_21264_CPU *cpu)
{
	bool retVal = false;
	int ii, jj;

	for (ii = 1; ii < AXP_CACHE_ENTRIES; ii++)
	{
		for (jj = 0; jj < AXP_2_WAY_CACHE; jj++)
		{
			memset(cpu->dCache[ii][jj].data, 0, AXP_DCACHE_DATA_LEN);
			cpu->dCache[ii][jj].physTag = 0;
			cpu->dCache[ii][jj].valid = false;
			cpu->dCache[ii][jj].dirty = false;
			cpu->dCache[ii][jj].shared = false;
			cpu->dCache[ii][jj].modified = false;
			cpu->dCache[ii][jj].set_0_1 = false;;
			cpu->dCache[ii][jj].locked = false;;
			cpu->dCache[ii][jj].state = Invalid;
		}
	}
	for (ii = 1; ii < AXP_CACHE_ENTRIES; ii++)
	{
		for (jj = 0; jj < AXP_2_WAY_CACHE; jj++)
		{
			cpu->dtag[ii][jj].physTag = 0;
			cpu->dtag[ii][jj].ctagIndex = AXP_CACHE_ENTRIES;
			cpu->dtag[ii][jj].ctagSet = AXP_2_WAY_CACHE;
			cpu->dtag[ii][jj].valid = false;
		}
	}

	for (ii = 0; ii < AXP_MBOX_QUEUE_LEN; ii++)
	{
		cpu->lq[ii].value = 0;
		cpu->lq[ii].virtAddress = 0;
		cpu->lq[ii].instr = NULL;
		cpu->lq[ii].state = QNotInUse;
		cpu->lq[ii].IOflag = false;
		cpu->lq[ii].lockCond = false;
	}
	cpu->lqNext = 0;
	for (ii = 0; ii < AXP_MBOX_QUEUE_LEN; ii++)
	{
		cpu->sq[ii].value = 0;
		cpu->sq[ii].virtAddress = 0;
		cpu->sq[ii].instr = NULL;
		cpu->sq[ii].state = QNotInUse;
		cpu->sq[ii].IOflag = false;
		cpu->sq[ii].lockCond = false;
	}
	cpu->sqNext = 0;
	for (ii = 0; ii < AXP_TB_LEN; ii++)
	{
		cpu->dtb[ii].virtAddr = 0;
		cpu->dtb[ii].physAddr = 0;
		cpu->dtb[ii].matchMask = 0;
		cpu->dtb[ii].keepMask = 0;
		cpu->dtb[ii].kre = 0;
		cpu->dtb[ii].ere = 0;
		cpu->dtb[ii].sre = 0;
		cpu->dtb[ii].ure = 0;
		cpu->dtb[ii].kwe = 0;
		cpu->dtb[ii].ewe = 0;
		cpu->dtb[ii].swe = 0;
		cpu->dtb[ii].uwe = 0;
		cpu->dtb[ii].faultOnRead = 0;
		cpu->dtb[ii].faultOnWrite = 0;
		cpu->dtb[ii].faultOnExecute = 0;
		cpu->dtb[ii].res_1 = 0;
		cpu->dtb[ii].asn = 0;
		cpu->dtb[ii]._asm = false;
		cpu->dtb[ii].valid = false;
	}
	cpu->nextDTB = 0;
	for (ii = 0; ii < AXP_21264_MAF_LEN; ii++)
	{
		cpu->maf[ii].type = MAFNotInUse;
		cpu->maf[ii].rq = NOPcmd;
		cpu->maf[ii].rsp = NOPsysdc;
		cpu->maf[ii].pa = 0;
		cpu->maf[ii].complete = false;
	}
	cpu->tbMissOutstanding = false;;
	cpu->dtbTag0.res_1 = 0;
	cpu->dtbTag0.va = 0;
	cpu->dtbTag0.res_2 = 0;
	cpu->dtbTag1.res_1 = 0;
	cpu->dtbTag1.va = 0;
	cpu->dtbTag1.res_2 = 0;
	cpu->dtbPte0.res_1 = 0;
	cpu->dtbPte0.pa = 0;
	cpu->dtbPte0.res_2 = 0;
	cpu->dtbPte0.uwe = 0;
	cpu->dtbPte0.swe = 0;
	cpu->dtbPte0.ewe = 0;
	cpu->dtbPte0.kwe = 0;
	cpu->dtbPte0.ure = 0;
	cpu->dtbPte0.sre = 0;
	cpu->dtbPte0.ere = 0;
	cpu->dtbPte0.kre = 0;
	cpu->dtbPte0.res_3 = 0;
	cpu->dtbPte0.gh = 0;
	cpu->dtbPte0._asm = 0;
	cpu->dtbPte0.res_4 = 0;
	cpu->dtbPte0.fow = 0;
	cpu->dtbPte0._for = 0;
	cpu->dtbPte0.res_5 = 0;
	cpu->dtbPte1.res_1 = 0;
	cpu->dtbPte1.pa = 0;
	cpu->dtbPte1.res_2 = 0;
	cpu->dtbPte1.uwe = 0;
	cpu->dtbPte1.swe = 0;
	cpu->dtbPte1.ewe = 0;
	cpu->dtbPte1.kwe = 0;
	cpu->dtbPte1.ure = 0;
	cpu->dtbPte1.sre = 0;
	cpu->dtbPte1.ere = 0;
	cpu->dtbPte1.kre = 0;
	cpu->dtbPte1.res_3 = 0;
	cpu->dtbPte1.gh = 0;
	cpu->dtbPte1._asm = 0;
	cpu->dtbPte1.res_4 = 0;
	cpu->dtbPte1.fow = 0;
	cpu->dtbPte1._for = 0;
	cpu->dtbPte1.res_5 = 0;
	cpu->dtbAltMode.alt_mode = AXP_MBOX_ALTMODE_KERNEL;
	cpu->dtbAltMode.res = 0;
	cpu->dtbIs0.res_1 = 0;
	cpu->dtbIs0.inval_itb = 0;
	cpu->dtbIs0.res_2 = 0;
	cpu->dtbIs1.res_1 = 0;
	cpu->dtbIs1.inval_itb = 0;
	cpu->dtbIs1.res_2 = 0;
	cpu->dtbAsn0.res_1 = 0;
	cpu->dtbAsn0.asn = 0;
	cpu->dtbAsn0.res_2 = 0;
	cpu->dtbAsn1.res_1 = 0;
	cpu->dtbAsn1.asn = 0;
	cpu->dtbAsn1.res_2 = 0;
	cpu->mmStat.res = 0;
	cpu->mmStat.dc_tag_perr = 0;
	cpu->mmStat.opcodes = 0;
	cpu->mmStat.fow = 0;
	cpu->mmStat._for = 0;
	cpu->mmStat.acv = 0;
	cpu->mmStat.wr = 0;
	cpu->mCtl.res_1 = 0;
	cpu->mCtl.spe = 0;
	cpu->mCtl.res_2 = 0;
	cpu->dcCtl.dcdat_err_en = 0;
	cpu->dcCtl.dctag_par_en = 0;
	cpu->dcCtl.f_bad_decc = 0;
	cpu->dcCtl.f_bad_tpar = 0;
	cpu->dcCtl.f_hit = 0;
	cpu->dcCtl.set_en = 3;				/* use both Dcache sets */
	cpu->dcStat.res = 0;
	cpu->dcStat.seo = 0;
	cpu->dcStat.ecc_err_ld = 0;
	cpu->dcStat.ecc_err_st = 0;
	cpu->dcStat.tperr_p1 = 0;
	cpu->dcStat.tperr_p1 = 0;

	/*
	 * All done, return to the caller.
	 */
	return(retVal);
}
