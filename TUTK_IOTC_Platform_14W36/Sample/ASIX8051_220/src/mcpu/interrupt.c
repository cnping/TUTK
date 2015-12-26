/*
 *********************************************************************************
 *     Copyright (c) 2010	ASIX Electronic Corporation      All rights reserved.
 *
 *     This is unpublished proprietary source code of ASIX Electronic Corporation
 *
 *     The copyright notice above does not evidence any actual or intended
 *     publication of such source code.
 *********************************************************************************
 */
/*================================================================================
 * Module Name : interrupt.c
 * Purpose     : This file include peripheral interrupt service routine and
 *               the wake up interrupt service routine.
 * Author      : Robin Lee
 * Date        :
 * Notes       :
 * $Log: interrupt.c,v $
 * no message
 *
 *================================================================================
 */

/* INCLUDE FILE DECLARATIONS */
#include	"ax22000.h"
#include	"types.h"
#include	"mcpu_cfg.h"
#include	"mcpu.h"
#include	"interrupt.h"
#include	"mpsave.h"
#include	"mstimer.h"
#include	"mwif.h"

#if AX_LBI_INT_ENABLE
#include	"lbi.h"
#endif

#if AX_ETH_INT_ENABLE
#include	"stoe.h"
#include	"mac.h"
#endif

#if AX_I2C_INT_ENABLE
#include	"i2c.h"
#endif

#if AX_SPI_INT_ENABLE
#include	"spi.h"
#endif

#if AX_ONEWIRE_INT_ENABLE
#include	"onewire.h"
#endif

#if (AX_HSUART2_INT_ENABLE || AX_HSUART3_INT_ENABLE)
#include	"hsuart.h"
#endif

#if AX_HSUART2_INT_ENABLE
#include	"hsuart2.h"
#endif

#if AX_HSUART3_INT_ENABLE
#include	"hsuart3.h"
#endif

#if AX_I2S_INT_ENABLE
#include	"i2s.h"
#endif

#if AX_RTC_INT_ENABLE
#include	"rtc.h"
#endif

/* NAMING CONSTANT DECLARATIONS */


/* GLOBAL VARIABLE DECLARATIONS */


/* STATIC VARIABLE DECLARATIONS */
static U8_T	intr_WcpuWatchDogReset = 0;


/* LOCAL SUBPROGRAM DECLARATIONS */
static void intr_PeripherialISR(void);
static void intr_ExternalIntr5ISR(void);
static void intr_PmmWakeUpISR(void);
#if AX_WATCHDOG_INT_ENB
static void intr_WatchDogISR(void);
static void intr_WatchDogByWcpuInterruptService(void);
#endif //AX_WATCHDOG_INT_ENB


/* LOCAL SUBPROGRAM BODIES */

/*
 *--------------------------------------------------------------------------------
 * static void intr_PeripherialISR(void)
 * Purpose : The peripheral interface interrupt service routine of AX220xx MCPU.
 * Params  : None.
 * Returns : None.
 * Note    : None.
 *--------------------------------------------------------------------------------
 */
static void INTR_PeripherialISR(void) interrupt 9 //use external interrupt 4 (0x4B)
{
	U8_T	intrStt1 = 0;
	U8_T	intrStt2 = 0;

	/* Interrupt type check */
	while (1)
	{
		intrStt1 = PISSR1;
		intrStt2 = PISSR2;

		if ((intrStt1 == 0) && (intrStt2 == 0))
			break;

#if AX_LBI_INT_ENABLE
		if (intrStt1 & LB_INT_STU)
		{
			LBI_IntrEntryFunc();
        }
#endif

#if AX_LBI_INT_ENABLE
		if (intrStt1 & LB_EXT_INT_STU)
		{
			LBI_ExtIntrEntryFunc();
		}
#endif

#if AX_ETH_INT_ENABLE
  #if (MAC_ARBIT_MODE & MAC_ARBIT_ETH)
    #if (MAC_GET_INTSTATUS_MODE == MAC_INTERRUPT_MODE)
		if (intrStt1 & ETH_INT_STU)
		{
			MAC_SetInterruptFlag();
		}
    #endif /* #if (MAC_GET_INTSTATUS_MODE == MAC_INTERRUPT_MODE) */
  #endif /* #if (MAC_ARBIT_MODE & MAC_ARBIT_ETH) */
  #if (STOE_GET_INTSTATUS_MODE == STOE_INTERRUPT_MODE)
		if (intrStt1 & TOE_INT_STU)
		{
			STOE_SetInterruptFlag();
		}
  #endif /*#if (STOE_GET_INTSTATUS_MODE == STOE_INTERRUPT_MODE) */
#endif /* #if AX_ETH_INT_ENABLE */

#if AX_I2C_INT_ENABLE
		if (intrStt1 & I2C_INT_STU)
		{
			I2C_IntrEntryFunc();
		}
#endif

#if AX_SPI_INT_ENABLE
		if (intrStt1 & SPI_INT_STU)
		{
			SPI_IntrEntryFunc();
		}
#endif

#if AX_HSUART2_INT_ENABLE
		if (intrStt2 & UART2_INT_STU)
		{
			HSUR2_IntrEntryFunc();
		}
#endif

#if AX_HSUART3_INT_ENABLE
		if (intrStt2 & UART3_INT_STU)
		{
			HSUR3_IntrEntryFunc();
		}
#endif

#if AX_I2S_INT_ENABLE
  #if (I2SPCM_SUPPORT_TXRX_TYPE == I2SPCM_SUPPORT_TX)
		if (intrStt2 & I2STX_INT_STU)
		{
			I2S_TxIntr();
		}
  #endif /*#if (I2SPCM_SUPPORT_TXRX_TYPE == I2SPCM_SUPPORT_TX) */
  #if (I2SPCM_SUPPORT_TXRX_TYPE == I2SPCM_SUPPORT_RX)
		if (intrStt2 & I2SRX_INT_STU)
		{
			I2S_RxIntr();
		}
  #endif /* (I2SPCM_SUPPORT_TXRX_TYPE == I2SPCM_SUPPORT_RX) */
#endif

#if AX_ONEWIRE_INT_ENABLE
		if (intrStt1 & OW_INT_STU)
		{
			ONEWIRE_IntrEntryFunc();
		}
#endif
	}
	
	if (PCON & PMMS_)
	{
		if (OCGCR & BIT3)
		{
			PCON = (PMM_ | PMMS_ | SWB_);
		}
	}
} /* End of INTR_PeripherialISR() */

/*
 *--------------------------------------------------------------------------------
 * static void INTR_ExternalIntr5ISR(void)
 * Purpose : The interface interrupt service routine of AX220xx MCPU interrupt 5.
 * Params  : None.
 * Returns : None.
 * Note    : None.
 *--------------------------------------------------------------------------------
 */
static void INTR_ExternalIntr5ISR(void) interrupt 10 //use external interrupt pin 5 (0x53)
{
	U8_T	intr5Stt = 0;

	EIF = INT5F;

	/* Interrupt type check */
	while (1)
	{
		intr5Stt = SDSTSR;
		if (intr5Stt == 0)
			break;
		
		if (intr5Stt & STT)
		{
			SWTIMER_Counter++;
		}
#if AX_RTC_INT_ENABLE
		if (intr5Stt & RTCT)
		{
			RTC_InterruptService();
		}
#endif
#if AX_WATCHDOG_INT_ENB
		if (intr5Stt & WWDT)
		{
			intr_WatchDogByWcpuInterruptService();
		}
#endif		
#if (MAC_ARBIT_MODE & MAC_ARBIT_WIFI)
		if (intr5Stt & WDRST)
		{
			MWIF_DocdReset();
		}
#endif
	}

	if (PCON & PMMS_)
	{
		if (OCGCR & BIT3)
		{
			PCON = (PMM_ | PMMS_ | SWB_);
		}
	}
} /* End of intr_ExternalIntr5ISR() */

/*
 *--------------------------------------------------------------------------------
 * static void INTR_PmmWakeUpISR(void)
 * Purpose : The interrupt service routine of the
 *           wake up event in power management mode.
 * Params  : None.
 * Returns : None.
 * Note    : None.
 *--------------------------------------------------------------------------------
 */
static void INTR_PmmWakeUpISR(void) interrupt 11 //use external interrupt 6 (0x5B)
{
	U8_T	wakeStatus = 0;
	U8_T	tempStatus = 0;
	U8_T	macIntEnbBitMap = 0, macWakeupFrameCmdBitMap = 0;

	EXTINT6(OFF);

	EIF = INT6F;	// Clear the flag of interrupt 6.

	wakeStatus = PCON;
	if (!(wakeStatus & SWB_))
	{
		PCON &= ~ (PMM_ | STOP_);
	}

	wakeStatus = WKUPSR;

#if POWER_SAVING
	/* to avoid the first timer, wcpu enter the sleep state in disable,
	fast and maximum power saving mode. */
	if (PS_Mode)
		MCPU_SleepFlag = 1;

	while (wakeStatus)
	{
#if AX_ETH_INT_ENABLE
		if (wakeStatus & MCPU_WAKEUP_BY_ETHERNET) // External wakeup, Ethernet Link-up, Wakeup Frame, Magic Packet.
		{
			/* record the SPWIE (MAC_STOP_PMM_INT_ENABLE_REG) */
			MCIR = MAC_WAKEUP_INT_ENABLE_REG;
			macIntEnbBitMap = MDR;
			
			/* record wakeup frame command */
			MCIR = MAC_WAKEUP_FRAME_COMMAND;
			macWakeupFrameCmdBitMap = MDR;

			/* clear SPWIE (MAC_STOP_PMM_INT_ENABLE_REG) */
			MDR = 0;
			MCIR = MAC_WAKEUP_INT_ENABLE_REG;
			
			/* clear wakeup frame command */
			MDR = macWakeupFrameCmdBitMap & ~(BIT0|BIT1|BIT2|BIT3);
			MCIR = MAC_WAKEUP_FRAME_COMMAND;

			/* read MAC status register for clear status */
			MCIR = MAC_WAKEUP_LINK_INT_STATUS_REG;
			tempStatus = MDR;
			tempStatus |= MCPU_WakeUpByMacFlag;

			if (MCPU_WakeUpEventTable & MCPU_WAKEUP_BY_ETHERNET)
			{
				if (tempStatus & MCPU_MacWakeUpEventTable)
				{
					/* clear PMMS mode */
					PCON &= ~(PMM_ | STOP_ | SWB_ | PMMS_);
					MCPU_SleepFlag = 0;
				}
			}
			
			if (tempStatus & STATUS_PRIMARY_LINK_CHANGE)
			{
				MCPU_WakeUpByMacFlag = 0;
				MAC_InterruptStatus |= STATUS_PRIMARY_LINK_CHANGE;
			}
			else if (tempStatus & STATUS_SECOND_LINK_CHANGE)
			{
				MCPU_WakeUpByMacFlag = 0;
				MAC_InterruptStatus |= STATUS_SECOND_LINK_CHANGE;
			}
			else
			{
				MAC_InterruptStatus = 0;
			}
			
			/* re-enable MAC wakeup interrupt */
			MDR = macIntEnbBitMap;
			MCIR = MAC_WAKEUP_INT_ENABLE_REG;

			/* re-enable wakeup frame command */
			MDR = macWakeupFrameCmdBitMap;
			MCIR = MAC_WAKEUP_FRAME_COMMAND;

			if (PS_State) /* deep sleep mode */
			{
				if (PS_State == PS_STATE_IN_DEEP_SLEEP)
				{
					PS_State = PS_STATE_EXIT_DEEP_SLEEP_REQ;
					P2 = 0x01;
				}
			}
		}

		if (wakeStatus & REVMII_WK)
		{
			if (MCPU_WakeUpEventTable & MCPU_WAKEUP_BY_REVMII)
			{
				/* clear MAC status register */
				MCIR = MAC_WAKEUP_LINK_INT_STATUS_REG;
				tempStatus = MDR;
				if (tempStatus & STATUS_TRIG_BY_REV_MII)
				{
					PCON &= ~ PMMS_;
					MCPU_SleepFlag = 0;
				}
			}
		}
#endif //AX_ETH_INT_ENABLE

#if AX_LBI_INT_ENABLE
		if (wakeStatus & LBS_WK)
		{
			if (MCPU_WakeUpEventTable & MCPU_WAKEUP_BY_LBS)
			{
				PCON &= ~ PMMS_;
				MCPU_SleepFlag = 0;
				LBI_WakeUpFunc();
			}
		}
#endif

#if AX_RTC_INT_ENABLE
		if (wakeStatus & RTC_WK)
		{
			if (MCPU_WakeUpEventTable & MCPU_WAKEUP_BY_RTC)
			{
				PCON &= ~ PMMS_;
				MCPU_SleepFlag = 0;
				RTC_WakeUpService();
			}
		}
#endif

#if AX_HSUART2_INT_ENABLE
		if (wakeStatus & UART2_WK)
		{
			HSUR2_WakeUpEvent();
			if (MCPU_WakeUpEventTable & MCPU_WAKEUP_BY_UART2)
			{
				PCON &= ~ PMMS_;
				MCPU_SleepFlag = 0;
			}
		}
#endif

#if AX_HSUART3_INT_ENABLE
		if (wakeStatus & UART3_WK)
		{
			HSUR3_WakeUpEvent();
			if (MCPU_WakeUpEventTable & MCPU_WAKEUP_BY_UART3)
			{
				PCON &= ~ PMMS_;
				MCPU_SleepFlag = 0;
			}
		}
#endif

#if AX_SPI_INT_ENABLE
		if (wakeStatus & SPI_WK) // For STOP mode, need to be cleared by software.
		{
			if (MCPU_WakeUpEventTable & MCPU_WAKEUP_BY_SPI)
			{
				PCON &= ~ PMMS_;
				MCPU_SleepFlag = 0;
				SPI_WakeUpEvent();
			}
		}
#endif		

		if (wakeStatus & MCPU_WAKEUP_BY_WIFI_PS_CHANGED)
		{
#if POWER_SAVING
			if (PS_Mode & 3)/* PS_Mode = 1, 2, 3 */
			{
				if (OCGCR & BIT3)
				{
					if (MCPU_WakeUpEventTable & MCPU_WAKEUP_BY_LBS)
					{
						PCON &= ~ PMMS_;
						MCPU_SleepFlag = 0;
					}
					else
					{
						/* continuous to sleep */
					}
				}
				else
				{
					MCPU_SleepFlag = 0;
				}
			}
			else
#endif //POWER_SAVING
			{
				PCON &= ~(PMM_ | STOP_ | SWB_ | PMMS_);
				MCPU_SleepFlag = 0;
			}
		}

		/* check the EIF for INTF6 again */
		if (EIF & INT6F)
		{
			EIF = INT6F;	// Clear the flag of interrupt 6.
			wakeStatus = WKUPSR;
		}
		else
		{
			break;
		}
	}

	if (MCPU_McpuNotSleepFlag)
	{
		PCON &= ~(PMM_ | STOP_ | SWB_ | PMMS_);
		MCPU_SleepFlag = 0;
	}

	if (MCPU_SleepFlag)
	{
	 	if (!PS_Mode || (PS_Mode && (OCGCR & BIT3)))
		{
#if (MAC_ARBIT_MODE & MAC_ARBIT_ETH)
			if ((MCPU_GetPowerSavingMode() & STOP_) || PS_State) /* stop mode or deep sleep */
			{
				/* disable mac receive */
				MCIR = MAC_MEDIUM_STATUS_MODE_REG;
				status = MDR;
				status &= 0xfb;
				MDR = status;
				MCIR = MAC_MEDIUM_STATUS_MODE_REG;
			}
#endif //(MAC_ARBIT_MODE & MAC_ARBIT_ETH)

			PCON &= ~(PMM_ | STOP_ | SWB_ | PMMS_);
			if (MCPU_GetPowerSavingMode() & STOP_)
				PCON |= STOP_;
			else
				PCON |= (MCPU_GetPowerSavingMode() | PMMS_);

			MCPU_SleepFlag = 1;

#if (MAC_ARBIT_MODE & MAC_ARBIT_ETH)
			if ((MCPU_GetPowerSavingMode() & STOP_) || PS_State) /* stop mode or deep sleep */
			{
				/* enable mac receive */
				MCIR = MAC_MEDIUM_STATUS_MODE_REG;
				status = MDR;
				status |= 4;
				MDR = status;
				MCIR = MAC_MEDIUM_STATUS_MODE_REG;
			}
#endif //(MAC_ARBIT_MODE & MAC_ARBIT_ETH)
		}
		else
		{
			MCPU_SleepFlag = 0;
		}
	}
#endif
	EXTINT6(ON);
} /* End of INTR_PmmWakeUpISR() */

#if AX_WATCHDOG_INT_ENB
/*
 *--------------------------------------------------------------------------------
 * void INTR_WatchDogISR(void)
 * Purpose : Service the watchdog interrupt. Clear the interrupt status by watchdog timeout.
 * Params  : None
 * Returns : None
 * Note    : None
 *--------------------------------------------------------------------------------
 */
void INTR_WatchDogISR(void) interrupt 12
{
	if (EWDI)
	{
		if (EWT)
		{
			TA = 0xAA;
			TA = 0x55;
			WDIF = 0; // clear WatchDog Interrupt Flag.
			TA = 0xAA;
			TA = 0x55;
			WTRF = 0; // clear the Watchdog Timer Reset Flag.
		}
		else
		{
			TA = 0xAA;
			TA = 0x55;
			WDIF = 0; // clear WatchDog Interrupt Flag.
		}
	}
} /* End of INTR_WatchDogISR() */

/*
 *--------------------------------------------------------------------------------
 * void intr_WatchDogByWcpuInterruptService(void)
 * Purpose : When a watchdog timeout from WCPU, this function can service this event.
 * Params  : None.
 * Returns : None.
 * Note    : None.
 *--------------------------------------------------------------------------------
 */
static void intr_WatchDogByWcpuInterruptService(void)
{
	U8_T	rstTime = 33;
	
	if (intr_WcpuWatchDogReset)
	{
		CSREPR |= WF_RST;
		while (rstTime --)
		{
		}
		CSREPR &= ~WF_RST;
	}
	else // just show a notice
	{
	}

} /* End of intr_WatchDogByWcpuInterruptService() */
#endif //AX_WATCHDOG_INT_ENB

/* EXPORTED SUBPROGRAM BODIES */

/*
 *--------------------------------------------------------------------------------
 * void INTR_PmmAutoSet(void)
 * Purpose : Check the power management mode and configure the AX220xx into 
 *           the previous power management mode. If the wake-up event doesn't match
 *           with applications setting, this function should enter the PMM again.
 * Params  : None.
 * Returns : None.
 * Note    : None.
 *--------------------------------------------------------------------------------
 */
void INTR_PmmAutoSet(void)
{
	U8_T	bitEa;

	bitEa = EA;
	EA = 0;
	if ((PCON & PMMS_) || (MCPU_SleepFlag == 1))
	{
		if (MCPU_PowerSavingMode == (PMM_ | SWB_))
		{
			PCON &= ~STOP_;
			PCON |= (PMM_ | SWB_ | PMMS_);
		}
		else if (MCPU_PowerSavingMode == PMM_)
		{
			PCON &= ~(STOP_ | SWB_);
			PCON |= (PMM_ | PMMS_);
		}
		else if (MCPU_PowerSavingMode == STOP_)
		{
			PCON &= ~(PMM_ | SWB_ | PMMS_);
			PCON |= STOP_;
		}
	}
	EA = bitEa;
}

#if AX_WATCHDOG_INT_ENB
/*
 *--------------------------------------------------------------------------------
 * void INTR_SetWatchDogFromWcpu(U8_T mode)
 * Purpose : Configure the behavior when get a WCPU watchdog event
 * Params  : mode: Set 1 to do a reset function when WCPU watchdog happening
 * Returns : None.
 * Note    : None.
 *--------------------------------------------------------------------------------
 */
void INTR_SetWatchDogFromWcpu(U8_T mode)
{
	if (mode == BIT0) // need MCPU issue a "reset" to WCPU
	{
		intr_WcpuWatchDogReset = BIT0;
	}
	else // just show a notice
	{
		intr_WcpuWatchDogReset = 0;
	}

} /* End of INTR_SetWatchDogFromWcpu() */
#endif //AX_WATCHDOG_INT_ENB


/* End of interrupt.c */
