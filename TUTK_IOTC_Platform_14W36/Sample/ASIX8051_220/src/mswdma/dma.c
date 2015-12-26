/*
 ******************************************************************************
 *     Copyright (c) 2010	ASIX Electronic Corporation      All rights reserved.
 *
 *     This is unpublished proprietary source code of ASIX Electronic Corporation
 *
 *     The copyright notice above does not evidence any actual or intended
 *     publication of such source code.
 ******************************************************************************
 */
/*=============================================================================
 * Module Name : dma.c
 * Purpose     : AX220xx provides a DMA that is used by software.
 *               This module handles all DMA functions 
 * Author      : Robin Lee
 * Date        : 2010-11-16
 * Notes       : 
 * $Log: dma.c,v $
 * no message
 *
 *=============================================================================
 */

/* INCLUDE FILE DECLARATIONS */
#include "ax22000.h"
#include "types.h"
#include "dma.h"
#if (DMA_INCLUDE_ETHERNET_MODULE ==	DMA_YES)
 #include "stoe.h"
#endif


/* GLOBAL VARIABLE DECLARATIONS */


/* STATIC VARIABLE DECLARATIONS */


/* LOCAL SUBPROGRAM BODIES */

/*
 * ----------------------------------------------------------------------------
 * void DMA_ReadReg(U8_T regaddr, U8_T XDATA* pbuf, U8_T length)
 * Purpose: Read SW DMA register.
 * Params : regaddr: A register index address.
 *          pbuf: A pointer to indicate the register data.
 *          length: How many bytes will be read. 
 * Returns: None.
 * Note   : None.
 * ----------------------------------------------------------------------------
 */
void DMA_ReadReg(U8_T regaddr, U8_T XDATA* pbuf, U8_T length)
{
	BIT		oldEintBit = EA;

	EA = 0;
	DCIR = regaddr;
	while (length--)
		pbuf[length] = DDR;
	EA = oldEintBit;

} /* End of DMA_ReadReg */

/*
 * ----------------------------------------------------------------------------
 * void DMA_WriteReg(U8_T regaddr, U8_T XDATA* pbuf, U8_T length)
 * Purpose: Write SW DMA register.
 * Params : regaddr: A register index address.
 *          pbuf: A pointer to indicate the register data.
 *          length: How many bytes will be written. 
 * Returns: None.
 * Note   : None.
 * ----------------------------------------------------------------------------
 */
void DMA_WriteReg(U8_T regaddr, U8_T XDATA* pbuf, U8_T length)
{
	BIT		oldEintBit = EA;

	EA = 0;
	while (length--)
		DDR = pbuf[length];
	DCIR = regaddr;
	EA = oldEintBit;

} /* End of STOE_DMA_WriteReg */

/*
 * ----------------------------------------------------------------------------
 * void DMA_Init(void)
 * Purpose: Initial AX220xx SW DMA module.
 * Params :
 * Returns:
 * Note   :
 * ----------------------------------------------------------------------------
 */
void DMA_Init(void)
{
	/* set DMA burst size */
	TDR = DMA_BURST_SIZE;
	TCIR = 0x24;

} /* End of DMA_Init */

/*
 * ----------------------------------------------------------------------------
 * void DMA_Start(void)
 * Purpose: Start software-dma module by enabling the interrupt 5.
 * Params :
 * Returns:
 * Note   :
 * ----------------------------------------------------------------------------
 */
void DMA_Start(void)
{

} /* End of DMA_Start */

/*
 * ----------------------------------------------------------------------------
 * U8_T DMA_CheckStatus(void)
 * Purpose: Wait software-dma to complete, and check if dma error happened or not?
 * Params : None.
 * Returns: 0 : dma error had happened.
 *			1 :	dma had completed.
 * Note   : None.
 * ----------------------------------------------------------------------------
 */
U8_T DMA_CheckStatus(void)
{
	U8_T XDATA	temp;

	while (1)
	{
		DMA_ReadReg(SW_DMA_CMD_REG, &temp, 1);
		if (temp & 0x37)
		{
			if (temp & DMA_ERROR_BIT)
				return 0;
		}
		else
		{
			return 1;
		}
	}

} /* End of DMA_CheckStatus */

/*
 * ----------------------------------------------------------------------------
 * BOOL DMA_DataToProgram(U32_T addrSour, U32_T addrDest, U16_T length)
 * Purpose: This function is responsible to copy data from external data memory
 *          to program memory. It will check DMA status automatically
 *          after DMA begin copying.
 * Params : addrSour¡GThe source address in external data memory.
 *          addrDest¡GThe destination address in program memory.
 *          length¡GThe total length that S/W DMA has need to copy.
 * Returns: 0 : DMA error had happened
 *			1 :	DMA had completed.
 * Note   :
 * ----------------------------------------------------------------------------
 */
BOOL DMA_DataToProgram(U32_T addrSour, U32_T addrDest, U16_T length)
{
	U32_T	cmd32b = 0;
	U8_T	cmd8b = 0;

    if (!length || addrSour == addrDest)
        return FALSE;

	cmd32b = (U32_T)addrSour << 8;
	DMA_WriteReg(SW_DMA_SOURCE_ADDR_REG, (U8_T *)&cmd32b, 3);

	cmd32b = (U32_T)addrDest << 8;
	DMA_WriteReg(SW_DMA_TARGET_ADDR_REG, (U8_T *)&cmd32b, 3);

	DMA_WriteReg(SW_DMA_BYTE_COUNT_REG, (U8_T *)&length, 2);

	cmd8b = (DMA_CMD_GO | DMA_TA_IN_PROG_MEM);

	DMA_WriteReg(SW_DMA_CMD_REG, (U8_T *)&cmd8b, 1);

	if (DMA_CheckStatus() == FALSE)
		return FALSE;
	return TRUE;
}

/*
 * ----------------------------------------------------------------------------
 * BOOL DMA_ProgramToProgram(U32_T addrSour, U32_T addrDest, U16_T length)
 * Purpose: This function is responsible to copy data from program memory
 *          to program memory. It will check DMA status automatically
 *          after DMA begin copying.
 * Params : addrSour¡GThe source address in program memory.
 *          addrDest¡GThe destination address in program memory.
 *          length¡GThe total length that S/W DMA has need to copy.
 * Returns: 0 : DMA error had happened
 *			1 :	DMA had completed.
 * Note   :
 * ----------------------------------------------------------------------------
 */
BOOL DMA_ProgramToProgram(U32_T addrSour, U32_T addrDest, U16_T length)
{
	U32_T	cmd32b = 0;
	U8_T	cmd8b = 0;

    if (!length || addrSour == addrDest)
        return FALSE;

	cmd32b = (U32_T)addrSour << 8;
	DMA_WriteReg(SW_DMA_SOURCE_ADDR_REG, (U8_T *)&cmd32b, 3);

	cmd32b = (U32_T)addrDest << 8;
	DMA_WriteReg(SW_DMA_TARGET_ADDR_REG, (U8_T *)&cmd32b, 3);

	DMA_WriteReg(SW_DMA_BYTE_COUNT_REG, (U8_T *)&length, 2);

	cmd8b = (DMA_CMD_GO | DMA_SA_IN_PROG_MEM | DMA_TA_IN_PROG_MEM);

	DMA_WriteReg(SW_DMA_CMD_REG, (U8_T *)&cmd8b, 1);

	if (DMA_CheckStatus() == FALSE)
		return FALSE;
	return TRUE;
}

/*
 * ----------------------------------------------------------------------------
 * BOOL DMA_ProgramToData(U32_T addrSour, U32_T addrDest, U16_T length)
 * Purpose: This function is responsible to copy data from program memory
 *          to external data memory. It will check DMA status automatically
 *          after DMA begin copying.
 * Params : addrSour¡GThe source address in program memory.
 *          addrDest¡GThe destination address in external data memory.
 *          length¡GThe total length that S/W DMA has need to copy.
 * Returns: 0 : DMA error had happened
 *			1 :	DMA had completed.
 * Note   :
 * ----------------------------------------------------------------------------
 */
BOOL DMA_ProgramToData(U32_T addrSour, U32_T addrDest, U16_T length)
{
	U32_T	cmd32b = 0;
	U8_T	cmd8b = 0;

    if (!length || addrSour == addrDest)
        return FALSE;

	cmd32b = (U32_T)addrSour << 8;
	DMA_WriteReg(SW_DMA_SOURCE_ADDR_REG, (U8_T *)&cmd32b, 3);

	cmd32b = (U32_T)addrDest << 8;
	DMA_WriteReg(SW_DMA_TARGET_ADDR_REG, (U8_T *)&cmd32b, 3);

	DMA_WriteReg(SW_DMA_BYTE_COUNT_REG, (U8_T *)&length, 2);

#if (DMA_INCLUDE_ETHERNET_MODULE == DMA_YES)
	if ((addrDest >= (U16_T)PBDP->XmtStartAddr) && (addrDest <= (U16_T)PBDP->XmtEndAddr))
		cmd8b = (DMA_CMD_GO | DMA_SA_IN_PROG_MEM | DMA_TO_TPBR);
	else
#endif //(DMA_INCLUDE_ETHERNET_MODULE == DMA_YES)
		cmd8b = (DMA_CMD_GO | DMA_SA_IN_PROG_MEM);

	DMA_WriteReg(SW_DMA_CMD_REG, (U8_T *)&cmd8b, 1);

	if (DMA_CheckStatus() == FALSE)
		return FALSE;
	return TRUE;
}

#if (DMA_INCLUDE_ETHERNET_MODULE == DMA_YES)
/*
* -----------------------------------------------------------------------------
 * Function Name: DMA_GrantXdata
 * Purpose: Use sw-dma doing memory copy. The scope of source and destination
 *				target must	be smaller than 64k sram memory.
 *			If the scope of either source or destination target is large than
 *				64k memory,	the programmers can call DMA_Grant() instead of
 *				this function.
 *			DMA_GrantXdata() can run faster than DMA_Grant() function.
 * Params: 
 * Returns: A address of XDATA memory that point to the next address of
 *				the end address of the scope of destination target.
 * Note:
 * ----------------------------------------------------------------------------
 */
U8_T XDATA* DMA_GrantXdata(U8_T XDATA* pdest, U8_T XDATA* psour, U16_T length)
{
	U8_T XDATA		temp[3];
	U16_T XDATA* 	ptempshort = (U16_T XDATA*)&temp[1];
	U8_T 			dmatype = 0;

    if (!length || pdest == psour)
        return 0;

	temp[0] = 0;
	*ptempshort = psour;
	DMA_WriteReg(SW_DMA_SOURCE_ADDR_REG, temp, 3);
	*ptempshort = pdest;
	DMA_WriteReg(SW_DMA_TARGET_ADDR_REG, temp, 3);
	*ptempshort = length;
	DMA_WriteReg(SW_DMA_BYTE_COUNT_REG, temp + 1, 2);

	if ((psour >= PBDP->RcvStartAddr) && (psour <= PBDP->RcvEndAddr))
		dmatype = DMA_FROM_RPBR;
	if ((pdest >= PBDP->XmtStartAddr) && (pdest <= PBDP->XmtEndAddr))
		dmatype |= DMA_TO_TPBR;

	temp[0] = (DMA_CMD_GO | dmatype);
	DMA_WriteReg(SW_DMA_CMD_REG, temp, 1);

	if (dmatype | DMA_TO_TPBR)
	{
		pdest += length;
		if (pdest > PBDP->XmtEndAddr)
			pdest -= STOE_XMIT_BUF_SIZE;
	}

	/* check software dma had completed. */
	if (DMA_CheckStatus())
		return pdest;
	else
		return 0;

} /* DMA_GrantXdata */

#if (DMA_SRAM_RANGE == DMA_COPY_LARGE_THAN_64K)
/*
* -----------------------------------------------------------------------------
 * Function Name: DMA_Grant
 * Purpose: Use sw-dma doing memory copy. 
 * Params: 
 * Returns: A address of sram memory that point to the next address of
 *				the end address of the scope of destination target.
 * Note:
 * ----------------------------------------------------------------------------
 */
U8_T* DMA_Grant(U8_T* pdest, U8_T* psour, U16_T length)
{
	U8_T XDATA		sourtemp[4];
	U8_T XDATA		desttemp[4];
	U32_T XDATA*	psourtemplong = (U32_T XDATA*)&sourtemp[0];
	U32_T XDATA*	pdesttemplong = (U32_T XDATA*)&desttemp[0];
	U16_T XDATA*	ptempshort;
	U8_T			dmatype = 0;
	U8_T*			ptemp;

    if (!length || pdest == psour)
        return 0;

	*psourtemplong = (U32_T)psour;
	*pdesttemplong = (U32_T)pdest;

	sourtemp[1]--;	/* for Keil C memory type */
	DMA_WriteReg(SW_DMA_SOURCE_ADDR_REG, &sourtemp[1], 3);

	if (sourtemp[1] == 0)
	{
		ptempshort = (U16_T XDATA*)&sourtemp[2];
		if ((*ptempshort >= (U16_T)PBDP->RcvStartAddr) && (*ptempshort <= (U16_T)PBDP->RcvEndAddr))
			dmatype = DMA_FROM_RPBR;
	}

	desttemp[1]--;	/* for Keil C memory type */
	DMA_WriteReg(SW_DMA_TARGET_ADDR_REG, &desttemp[1], 3);

	if (desttemp[1] == 0)
	{
		ptempshort = (U16_T XDATA*)&desttemp[2];
		if ((*ptempshort >= (U16_T)PBDP->XmtStartAddr) && (*ptempshort <= (U16_T)PBDP->XmtEndAddr))
			dmatype |= DMA_TO_TPBR;
	}

	ptempshort = (U16_T XDATA*)&sourtemp[0];
	*ptempshort = length;
	DMA_WriteReg(SW_DMA_BYTE_COUNT_REG, sourtemp, 2);

	sourtemp[0] = (DMA_CMD_GO | dmatype);
	DMA_WriteReg(SW_DMA_CMD_REG, sourtemp, 1);

	if (dmatype | DMA_TO_TPBR)
	{
		ptemp = pdest;
		pdest += length;
/* for KEIL */
		if (ptemp > pdest)
			pdest += 0x10000;
/* end */
		ptempshort = (U16_T XDATA*)&desttemp[2];
		(*ptempshort) += length;
		if (*ptempshort > (U16_T)PBDP->XmtEndAddr)
			pdest -= STOE_XMIT_BUF_SIZE;
	}

	/* check software dma had completed. */
	if (DMA_CheckStatus())
		return pdest;
	else
		return 0;

} /* DMA_Grant */
#endif /* End of #if (DMA_SRAM_RANGE == DMA_COPY_LARGE_THAN_64K)*/
#else
/*
* -----------------------------------------------------------------------------
 * Function Name: DMA_GrantXdata
 * Purpose: Use sw-dma doing memory copy. The scope of source and destination
 *				target must	be smaller than 64k sram memory.
 *			If the scope of either source or destination target is large than
 *				64k memory,	the programmers can call DMA_Grant() instead of
 *				this function.
 *			DMA_GrantXdata() can run faster than DMA_Grant() function.
 * Params: 
 * Returns:  A address of XDATA memory that point to the next address of
 *				the end address of the scope of destination target.
 * Note:
 * ----------------------------------------------------------------------------
 */
U8_T XDATA* DMA_GrantXdata(U8_T XDATA* pdest, U8_T XDATA* psour, U16_T length)
{
	U8_T XDATA		temp[3];
	U16_T XDATA* 	ptempshort = (U16_T XDATA*)&temp[1];

    if (!length || pdest == psour)
        return 0;
		
	temp[0] = 0;
	*ptempshort = psour;
	DMA_WriteReg(SW_DMA_SOURCE_ADDR_REG, temp, 3);
	*ptempshort = pdest;
	DMA_WriteReg(SW_DMA_TARGET_ADDR_REG, temp, 3);
	*ptempshort = length;
	DMA_WriteReg(SW_DMA_BYTE_COUNT_REG, temp + 1, 2);

	temp[0] = DMA_CMD_GO;
	DMA_WriteReg(SW_DMA_CMD_REG, temp, 1);

	pdest += length;

	/* check software dma had completed. */
	if (DMA_CheckStatus())
		return pdest;
	else
		return 0;

} /* DMA_GrantXdata */

#if (DMA_SRAM_RANGE == DMA_COPY_LARGE_THAN_64K)
/*
* -----------------------------------------------------------------------------
 * Function Name: DMA_Grant
 * Purpose: Use dma doing memory copy. 
 * Params:
 * Returns:  A address of sram memory that point to the next address of
 *				the end address of the scope of destination target.
 * Note:
 * ----------------------------------------------------------------------------
 */
U8_T* DMA_Grant(U8_T* pdest, U8_T* psour, U16_T length)
{
	U8_T XDATA		temp[4];
	U32_T XDATA*	ptemplong = (U32_T XDATA*)&temp[0];
	U16_T XDATA*	ptempshort;
	U8_T*			ptemp;

    if (!length || pdest == psour)
        return 0;

	*ptemplong = (U32_T)psour;
	temp[1]--;	/* for Keil C memory type */
	DMA_WriteReg(SW_DMA_SOURCE_ADDR_REG, &temp[1], 3);

	*ptemplong = (U32_T)pdest;
	temp[1]--;	/* for Keil C memory type */
	DMA_WriteReg(SW_DMA_TARGET_ADDR_REG, &temp[1], 3);

	ptempshort = (U16_T XDATA*)&temp[0];
	*ptempshort = length;
	DMA_WriteReg(SW_DMA_BYTE_COUNT_REG, temp, 2);

	temp[0] = DMA_CMD_GO;
	DMA_WriteReg(SW_DMA_CMD_REG, temp, 1);

	ptemp = pdest;
	pdest += length;
/* for KEIL */
	if (ptemp > pdest)
		pdest += 0x10000;
/* end */
	/* check software dma had completed. */
	if (DMA_CheckStatus())
		return pdest;
	else
		return 0;

} /* DMA_Grant */
#endif /* End of #if (DMA_SRAM_RANGE == DMA_COPY_LARGE_THAN_64K) */

#endif /* End of #if (DMA_INCLUDE_ETHERNET_MODULE == DMA_YES)*/

/*
 * ----------------------------------------------------------------------------
 * Function Name: DMA_IndirectIO
 * Purpose: dma indirect register read/write, only for debugging.
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
U8_T DMA_IndirectIO(U8_T rw, U16_T index, U8_T XDATA* pbuf)
{
	U8_T len;

	switch (index)
	{
	case SW_DMA_SOURCE_ADDR_REG:
	case SW_DMA_TARGET_ADDR_REG:
		len = 3;
		break;
	case SW_DMA_BYTE_COUNT_REG:
		len = 2;
		break;
	case SW_DMA_CMD_REG:
		len = 1;
		break;
	default:
		return 0xff;
	}

	if (rw == 0) /* read */
		DMA_ReadReg (index, pbuf, len);
	else if (rw == 1) /* write */
		DMA_WriteReg (index, pbuf, len);
	else
		return 0xff;

	return	len;

} /* End of DMA_IndirectIO() */


/* End of dma.c */

