/*--------------------------------------------------------------------------
 AX22000.H

Header file for the AX22000.
Copyright (c) 2001 Keil Software, Inc.  All rights reserved.
--------------------------------------------------------------------------*/
/*------------------------------------------------
Make sure that this file gets included only once.
------------------------------------------------*/
#ifndef AX22000_HEADER_FILE
#define AX22000_HEADER_FILE


/*------------------------------------------------
Byte Registers
------------------------------------------------*/
sfr	P0			= 0x80;// Port 0
sfr	SP			= 0x81;// Stack Pointer
sfr	DPL0		= 0x82;// Data Pointer 0 Low Byte
sfr	DPH0		= 0x83;// Data Pointer 0 High Byte
sfr	DPL			= 0x82;// Data Pointer 0 Low Byte
sfr	DPH			= 0x83;// Data Pointer 0 High Byte
sfr	DPL1		= 0x84;// Data Pointer 1 Low Byte
sfr	DPH1		= 0x85;// Data Pointer 1 High Byte
sfr	DPS			= 0x86;// Data Pointers Select Register
sfr	PCON		= 0x87;// Power Configuration Register
sfr TCON		= 0x88;// Timer 0,1 Configuration Register
sfr TMOD		= 0x89;// Timer 0,1 Control Mode Register
sfr TL0			= 0x8A;// Timer 0 Low Byte Counter
sfr TL1			= 0x8B;// Timer 1 Low Byte Counter
sfr TH0			= 0x8C;// Timer 0 High Byte Counter
sfr TH1			= 0x8D;// Timer 1 High Byte Counter
sfr CKCON		= 0x8E;// Clock Control Register
/* FOR MAIN_CPU_SUBSYSTEM */
sfr CSREPR		= 0x8F;// Software Reset and External Program RAM Select Register
/* FOR WIFI_CPU_SUBSYSTEM */
sfr WBSR		= 0x8F;// WCPU Byte Swap Register
/* END */

sfr P1			= 0x90;// Port 1
sfr EIF			= 0x91;// Extended Interrupt Flag Register
sfr WTST		= 0x92;// Program Wait States Register
sfr DPX0		= 0x93;// Data Pointer Extended Register 0
sfr DPX			= 0x93;// Data Pointer Extended Register 0
/* FOR MAIN_CPU_SUBSYSTEM */
sfr	PSGR		= 0x94;// Program Status & GAP Register
/* END */
sfr DPX1		= 0x95;// Data Pointer Extended Register 1
/* FOR MAIN_CPU_SUBSYSTEM */
sfr I2CCIR		= 0x96;// I2C Command Index Register
sfr I2CDR		= 0x97;// I2C Data Register
/* END */
sfr SCON0		= 0x98;// UART 0 Configuration Register
sfr SBUF0		= 0x99;// UART 0 Buffer Register
/* FOR MAIN_CPU_SUBSYSTEM */
sfr EMDMR		= 0x9A;// External Memory DMA Mask Register
sfr OCGCR		= 0x9B;// Oscillator and Clock Generation Control Register
sfr WKUPSR		= 0x9C;// Wake-up Status Summary Register
/* END */
sfr ACON		= 0x9D;// Address Control Register
/* FOR MAIN_CPU_SUBSYSTEM */
sfr PISSR1		= 0x9E;// Peripheral Interrupt Status Summary Register 1
sfr PISSR2		= 0x9F;// Peripheral Interrupt Status Summary Register 2
/* FOR WIFI_CPU_SUBSYSTEM */
sfr SFFSCR		= 0x9E;// Sequence and Fragment Filtering SRAM Command Register
sfr SFFSDR		= 0x9F;// Sequence and Fragment Filtering SRAM Data Register
/* END */

sfr P2			= 0xA0;// Port 2
/* FOR MAIN_CPU_SUBSYSTEM */
/* Local Bus Master Mode */
sfr LMSR		= 0xA1;// Local Bus Mode Setup Register
sfr LCR			= 0xA2;// Local Bus Command Register
sfr LSR			= 0xA3;// Local Bus Status Register
sfr LDALR		= 0xA4;// Local Bus Device Address Low Register
sfr LDAHR		= 0xA5;// Local Bus Device Address High Register
sfr LDCSR		= 0xA6;// Local Bus Device Chip Select Register
sfr LABLR		= 0xA7;// Local Bus Arbitrate Burst Length Register
/* Local Bus Slave Mode */
sfr LSAIER		= 0xA1;// Local Bus Slave mode Action and Interrupt Enable Register
sfr LSCR		= 0xA2;// Local Bus Slave mode Command Register
sfr LSSR		= 0xA3;// Local Bus Slave mode Status Register
sfr XRXNR		= 0xA4;// External Master Receive Notice Register of 8Bit
sfr XRXAR		= 0xA5;// External Master Receive Acknowledge Register of 8Bit
sfr XTXAR		= 0xA6;// External Master Transmit Acknowledge Register of 8Bit
sfr XTXRR		= 0xA7;// External Master Transmit Request Register of 8Bit
/* Local Bus Digiport Mode */
sfr DGMSR		= 0xA1;// Digiport Setup Register
sfr DGCR		= 0xA2;// Digiport Command Register
sfr DGSR		= 0xA3;// Digiport Status Register
sfr DGLLR		= 0xA4;// Digiport Length Low Register
sfr DGLMR		= 0xA5;// Digiport Length Medial Register
sfr DGLHR		= 0xA6;// Digiport Length High Register
sfr DGCNT		= 0xA7;// Digiport Frame Counter Register
/* FOR WIFI_CPU_SUBSYSTEM */
sfr PMALR		= 0xA1;// PCI Master Address Low Register
sfr PMAHR		= 0xA2;// PCI Master Address High Register
sfr	PMDR0		= 0xA3;// PCI Master Data Register 0
sfr	PMDR1		= 0xA4;// PCI Master Data Register 1
sfr	PMDR2		= 0xA5;// PCI Master Data Register 2
sfr	PMDR3		= 0xA6;// PCI Master Data Register 3
sfr	PBIER		= 0xA7;// PCI Interrupt Enable register
/* END */
sfr IE			= 0xA8;// Interrupt Enable register
/* FOR MAIN_CPU_SUBSYSTEM */
sfr LDLR		= 0xA9;// Local Bus Data Low Register
sfr LDHR		= 0xAA;// Local Bus Data High Register
sfr DMALR		= 0xAB;// Local Bus DMA Address Low Register
sfr DMAMR		= 0xAC;// Local Bus DMA Address Medium Register
sfr DMAHR		= 0xAD;// Local Bus DMA Address High Register
sfr TCIR		= 0xAE;// TOE Command Index Register
sfr	TDR			= 0xAF;// TOE Data Register
/* FOR WIFI_CPU_SUBSYSTEM */
sfr RHPLR		= 0xA9;
sfr RHPHR		= 0xAA;
sfr RSPLR		= 0xAB;
sfr RSPHR		= 0xAC;
sfr PBSR		= 0xAE;// PCI Bridge Status Register
sfr	PBISR		= 0xAF;// PCI Bridge Interrupt Status Register
/* END */

sfr P3			= 0xB0;// Port 3
/* FOR MAIN_CPU_SUBSYSTEM */
sfr CCAPL0      = 0xB1;// PCA Module 0 Compare/Capture Low Byte Register
sfr CCAPL1      = 0xB2;// PCA Module 1 Compare/Capture Low Byte Register
sfr CCAPL2      = 0xB3;// PCA Module 2 Compare/Capture Low Byte Register
sfr CCAPL3      = 0xB4;// PCA Module 3 Compare/Capture Low Byte Register
sfr CCAPL4      = 0xB5;// PCA Module 4 Compare/Capture Low Byte Register
sfr MCIR		= 0xB6;// MAC Command Index Register
sfr MDR			= 0xB7;// MAC Data Register
/* FOR WIFI_CPU_SUBSYSTEM */
sfr MTWPL		= 0xB1;// This register defines the current address that MII bridge is prepared to write(LOW).
sfr MTWPH		= 0xB2;// This register defines the current address that MII bridge is prepared to write(HIGH).
sfr CTRPL		= 0xB3;// This register defines the address of cpu reads from TX block(LOW).
sfr CTRPH		= 0xB4;// This register defines the address of cpu reads from TX block(HIGH).
sfr TXBS		= 0xB5;// MII Bridge TX Block Status Register.
/* END */
sfr IP			= 0xB8;// Interrupt Priority Register
/* FOR MAIN_CPU_SUBSYSTEM */
sfr CCAPH0      = 0xB9;// PCA Module 0 Compare/Capture High Byte Register
sfr CCAPH1      = 0xBA;// PCA Module 1 Compare/Capture High Byte Register
sfr CCAPH2      = 0xBB;// PCA Module 2 Compare/Capture High Byte Register
sfr CCAPH3      = 0xBC;// PCA Module 3 Compare/Capture High Byte Register
sfr CCAPH4      = 0xBD;// PCA Module 4 Compare/Capture High Byte Register
sfr EPCR		= 0xBE;// Ethernet PHY Command Index Register
sfr EPDR		= 0xBF;// Ethernet PHY Data Register
/* FOR WIFI_CPU_SUBSYSTEM */
sfr	MRPLL		= 0xB9;// This register defines the packet length of received packet(LOW).
sfr MRPLH		= 0xBA;// This register defines the packet length of received packet(HIGH).
sfr MRBLL		= 0xBB;// This register defines the block length of received packet for each block access(LOW).
sfr MRBLH		= 0xBC;// This register defines the block length of received packet for each block access(HIGH).
sfr MRBSAL		= 0xBD;// This register defines the starting address of RX block(LOW).
sfr MRBSAH		= 0xBE;// This register defines the starting address of RX block(HIGH).
sfr MRCR		= 0xBF;// MII Bridge RX Command Register.
/* END */

sfr SCON1		= 0xC0;// UART 1 Configuration Register
sfr SBUF1		= 0xC1;// UART 1 Buffer Register
/* FOR MAIN_CPU_SUBSYSTEM */
sfr CMOD		= 0xC2;// PCA Timer/Counter Mode Register
sfr CCON		= 0xC3;// PCA Timer/Counter Control Register
sfr CL			= 0xC4;// PCA Timer/Counter Low
sfr CH			= 0xC5;// PCA Timer/Counter High
sfr RTCIR		= 0xC6;// RTC Command Index Register
sfr RTCDR		= 0xC7;// RTC Data Register
/* FOR WIFI_CPU_SUBSYSTEM */
sfr MISR		= 0xC2;// Interrupt status.
sfr MIMR		= 0xC3;// Interrupt mask.
/* END */
sfr T2CON		= 0xC8;// Timer 2 Configuration Register
sfr T2IF		= 0xC9;// Timer 2 Interrupt Flags
sfr RLDL		= 0xCA;// Timer 2 Reload Low Byte
sfr RLDH		= 0xCB;// Timer 2 Reload High Byte
sfr TL2			= 0xCC;// Timer 2 Low Byte Counter
sfr TH2			= 0xCD;// Timer 2 High Byte Counter
/* FOR MAIN_CPU_SUBSYSTEM */
sfr SPICIR		= 0xCE;// SPI Command Index Register
sfr SPIDR		= 0xCF;// SPI Data Register
/* FOR WIFI_CPU_SUBSYSTEM */
sfr WI2CCIR		= 0xCE;// I2C Command Index Register
sfr WI2CDR		= 0xCF;// I2C Data Register
/* END */

sfr PSW			= 0xD0;// Program Status Word
/* FOR MAIN_CPU_SUBSYSTEM */
sfr CCAPM0		= 0xD1;// PCA Compare/Capture Module Mode Register 0
sfr CCAPM1		= 0xD2;// PCA Compare/Capture Module Mode Register 1
sfr CCAPM2		= 0xD3;// PCA Compare/Capture Module Mode Register 2
sfr CCAPM3		= 0xD4;// PCA Compare/Capture Module Mode Register 3
sfr CCAPM4		= 0xD5;// PCA Compare/Capture Module Mode Register 4
sfr OWCIR		= 0xD6;// OneWire Command Index Register
sfr OWDR		= 0xD7;// OneWire Data Register
/* FOR WIFI_CPU_SUBSYSTEM */
sfr WSDSR		= 0xD1;// WCPU Software DMA Setting Register
sfr WSDSAL		= 0xD2;// WCPU Software DMA Source Low Address Register
sfr WSDSAH		= 0xD3;// WCPU Software DMA Source High Address Register
sfr WSDDAL		= 0xD4;// WCPU Software DMA Destination Low Address Register
sfr WSDDAH		= 0xD5;// WCPU Software DMA Destination High Address Register
sfr WSDLR0		= 0xD6;// WCPU Software DMA Length Register 0
sfr WSDLR1		= 0xD7;// WCPU Software DMA Length Register 1
/* END */
sfr WDCON		= 0xD8;// Watchdog Configuration
/* FOR MAIN_CPU_SUBSYSTEM */
sfr SDSTSR		= 0xD9;// Software DMA and Software Timer complete interrupt status
sfr DCIR		= 0xDA;// DMA Command Index Register
sfr DDR			= 0xDB;// DMA Data Register
sfr MR			= 0xDE;// Miscellaneous Register
sfr CRR			= 0xDF;// Chip Revision Register
/* FOR WIFI_CPU_SUBSYSTEM */
sfr SINTMR		= 0xD9;// Sub-System Interrupt Mask Register
sfr HASHCR		= 0xDA;// HASH Command Register
sfr HASHDR		= 0xDB;// HASH Data Register
sfr SFFEDR0		= 0xDC;// Sequence and Fragment Filtering ENTRY DROP Register 0
sfr SFFEDR1		= 0xDD;// Sequence and Fragment Filtering ENTRY DROP Register 1
sfr SFFEDR2		= 0xDE;// Sequence and Fragment Filtering ENTRY DROP Register 2
sfr SFFEDR3		= 0xDF;// Sequence and Fragment Filtering ENTRY DROP Register 3
/* END */

sfr ACC			= 0xE0;// Accumulator
/* FOR MAIN_CPU_SUBSYSTEM */
sfr PWHR		= 0xE1;// Program Write High Byte Register
sfr FCCR		= 0xE2;// Flash Controller Command Register
sfr FCSR		= 0xE3;// Flash Controller Status Register
sfr I2STCR		= 0xE4;// I2S/PCM TX Command Index Register
sfr I2STDR		= 0xE5;// I2S/PCM TX Data Register
sfr I2SRCR		= 0xE6;// I2S/PCM RX Command Index Register
sfr	I2SRDR		= 0xE7;// I2S/PCM RX Data Register
/* FOR WIFI_CPU_SUBSYSTEM */
sfr SINTSR		= 0xE1;// Sub-System Interrupt Status Register
sfr	SSCIR		= 0xE2;// Sub-System Command Index Register
sfr	SSDR		= 0xE3;// Sub-System Data Register
sfr	MBCIR		= 0xE4;// The indirect addressing to MII Bridge other registers.
sfr	MBDR		= 0xE5;// The data port to or from MII bridge internal register.
sfr	PBCIR		= 0xE6;// PCI Bridge Command Index Register
sfr PBDR		= 0xE7;// PCI Bridge Data Register
/* END */
sfr EIE			= 0xE8;// External Interrupt Enable
sfr STATUS		= 0xE9;// Status Register
sfr MXAX		= 0xEA;// MOVX @Ri Extended Register
sfr TA			= 0xEB;// Timed Access Protection
/* FOR MAIN_CPU_SUBSYSTEM */
sfr U2CIR		= 0xEC;// UART2 Command Index Register
sfr U2DR		= 0xED;// UART2 Data Register
sfr U3CIR		= 0xEE;// UART3 Command Index Register
sfr U3DR		= 0xEF;// UART3 Data Register
/* FOR WIFI_CPU_SUBSYSTEM */
sfr MOP			= 0xEC;// Math Operation Register
sfr MD			= 0xED;// Math Data Register
sfr RNO			= 0xEE;// Random Number Operation Register
sfr RNDP		= 0xEF;// Random Number Data Port Register
/* END */

sfr B			= 0xF0;// B Working Register
sfr	RC0			= 0xF1;// Read Command Register 0
sfr	RC1			= 0xF2;// Read Command Register 1
sfr	RC2			= 0xF3;// Read Command Register 2
sfr	RC3			= 0xF4;// Read Command Register 3
sfr	RC4			= 0xF5;// Read Command Register 4
sfr PCCR		= 0xF6;// Power Saving Control Register
sfr	PCDR		= 0xF7;// Power Saving Data Register
sfr EIP			= 0xF8;// Extended Interrupt Priority Register
sfr WC0			= 0xF9;// Write Command Register 0
sfr WC1			= 0xFA;// Write Command Register 1
sfr WC2			= 0xFB;// Write Command Register 2
sfr WC3			= 0xFC;// Write Command Register 3
sfr WC4			= 0xFD;// Write Command Register 4
sfr CCTRL		= 0xFE;// Command Control Register
sfr CSSR		= 0xFF;// Command Status Register


/*------------------------------------------------
P0 (0x80) Bit Registers
------------------------------------------------*/
sbit P0_0		= 0x80;
sbit P0_1		= 0x81;
sbit P0_2		= 0x82;
sbit P0_3		= 0x83;
sbit P0_4		= 0x84;
sbit P0_5		= 0x85;
sbit P0_6		= 0x86;
sbit P0_7		= 0x87;

/*------------------------------------------------
DPS (0x86) Bit Values
------------------------------------------------*/
#define SEL_	0x01
#define TSL_	0x20
#define ID0_	0x40
#define ID1_	0x80

/*------------------------------------------------
PCON (0x87) Bit Values
------------------------------------------------*/
#define PMM_	0x01
#define STOP_	0x02
#define SWB_	0x04
#define PMMS_	0x08
#define PWE_	0x10
#define	SMOD1_	0x40
#define SMOD0_	0x80


/*------------------------------------------------
TCON (0x88) Bit Registers
------------------------------------------------*/
sbit	IT0		= 0x88;
sbit	IE0		= 0x89;
sbit	IT1		= 0x8A;
sbit	IE1		= 0x8B;
sbit	TR0		= 0x8C;
sbit	TF0		= 0x8D;
sbit	TR1		= 0x8E;
sbit	TF1		= 0x8F;

/*------------------------------------------------
TMOD (0x89) Bit Values
------------------------------------------------*/
#define	T0_M0_		0x01
#define	T0_M1_		0x02
#define	T0_CT_		0x04
#define	T0_GATE_	0x08
#define	T1_M0_		0x10
#define	T1_M1_		0x20
#define	T1_CT_		0x40
#define	T1_GATE_	0x80

#define T1_MASK_	0xF0
#define T0_MASK_	0x0F

/*------------------------------------------------
CKCON (0x8E) Bit Values
------------------------------------------------*/
#define MD0_	0x01
#define MD1_	0x02
#define MD2_	0x04
#define	T0M_	0x08
#define	T1M_	0x10
#define	T2M_	0x20
#define WD0_	0x40
#define WD1_	0x80

/* FOR MAIN_CPU_SUBSYSTEM */
/*------------------------------------------------
CSREPR (0x8F) Bit Values
------------------------------------------------*/
#define	SW_RST				0x01
#define	SW_RBT				0x02
#define	FARM				0x04
#define	FAES				0x08
#define WF_RST				0x10	// WiFi CPU reset
#define ICD					0x20
#define PMS					0x40
#define	SCS_80M				0x80
#define	SCS_40M				0x00
/* END */

/*------------------------------------------------
P1 (0x90) Bit Registers
------------------------------------------------*/
sbit P1_0		= 0x90;
sbit P1_1		= 0x91;
sbit P1_2		= 0x92;
sbit P1_3		= 0x93;
sbit P1_4		= 0x94;
sbit P1_5		= 0x95;
sbit P1_6		= 0x96;
sbit P1_7		= 0x97;

/*------------------------------------------------
EIF (0x91) Bit Values
------------------------------------------------*/
#define INT2F		0x01
#define INT3F		0x02
#define INT4F		0x04
#define INT5F		0x08
#define INT6F		0x10

/*------------------------------------------------
PSGR (0x94) Bit Registers
------------------------------------------------*/
#define FCWR_BSY	0x80

/*------------------------------------------------
EMDMR (0x9A) Bit Registers
------------------------------------------------*/
#define	LB_MK		0x01
#define	SPI_MK		0x02
#define	I2S_MK		0x04
#define	UART2_MK	0x08
#define	UART3_MK	0x10
#define	SWDMA_MK	0x20
#define	ADC_MK		0x40

/*------------------------------------------------
SCON0 (0x98) Bit Registers
------------------------------------------------*/
sbit RI0			= 0x98;
sbit TI0			= 0x99;
sbit RB08			= 0x9A;
sbit TB08			= 0x9B;
sbit REN0			= 0x9C;
sbit SM02			= 0x9D;
sbit SM01			= 0x9E;
sbit SM00			= 0x9F;

/*------------------------------------------------
OCGCR (0x9B) Bit Registers
------------------------------------------------*/
#define	TOF40M_STP			0x01
#define	TOF40M_WIFI			0x02
#define	TOF40M_LBSS			0x04
#define	WIFI_PWSV			0x08
#define	TOF32K_STP			0x10
#define	TOF32K_FP			0x20
#define	WIFI_CKEN			0x40
#define	X32K_USED			0x80

/*------------------------------------------------
WKUPSR (0x9C) Bit Values
------------------------------------------------*/
#define	ETH_WK				0x01
#define	REVMII_WK			0x02
#define	WPS_SC_IE			0x04
#define	WPS_SC				0x04
#define	RTC_WK				0x08
#define	UART2_WK			0x10
#define	UART3_WK			0x20
#define	SPI_WK				0x40
#define	LBS_WK				0x80

/*------------------------------------------------
ACON (0x9D) Bit Values
------------------------------------------------*/
#define	FLAT_MODE			0x02
#define	LARGE_MODE			0x00

/* FOR MAIN_CPU_SUBSYSTEM */
/*------------------------------------------------
PISSR1 (0x9E) Bit Values
------------------------------------------------*/
#define	LB_INT_STU			0x01
#define	LB_EXT_INT_STU		0x02
#define	ETH_INT_STU			0x04
#define	TOE_INT_STU			0x08
#define	OW_INT_STU			0x10
#define	SPI_INT_STU			0x20
#define	I2C_INT_STU			0x40

/*------------------------------------------------
PISSR2 (0x9F) Bit Values
------------------------------------------------*/
#define	UART2_INT_STU		0x01
#define	UART3_INT_STU		0x02
#define	I2STX_INT_STU		0x04
#define	I2SRX_INT_STU		0x08
/* END */

/*------------------------------------------------
P2 (0xA0) Bit Registers
------------------------------------------------*/
sbit P2_0		= 0xA0;
sbit P2_1		= 0xA1;
sbit P2_2		= 0xA2;
sbit P2_3		= 0xA3;
sbit P2_4		= 0xA4;
sbit P2_5		= 0xA5;
sbit P2_6		= 0xA6;
sbit P2_7		= 0xA7;

/*------------------------------------------------
IE (0xA8) Bit Registers
------------------------------------------------*/
sbit EX0		= 0xA8;       /* 1=Enable External interrupt 0 */
sbit ET0		= 0xA9;       /* 1=Enable Timer 0 interrupt */
sbit EX1		= 0xAA;       /* 1=Enable External interrupt 1 */
sbit ET1		= 0xAB;       /* 1=Enable Timer 1 interrupt */
sbit ES0		= 0xAC;       /* 1=Enable Serial port 0 interrupt */
sbit ET2		= 0xAD;       /* 1=Enable Timer 2 interrupt */
sbit ES1		= 0xAE;       /* 1=Enable Serial port 1 interrupt */
sbit EA			= 0xAF;       /* 0=Disable all interrupts */

/*------------------------------------------------
P3 (0xB0) Bit Registers (Mnemonics & Ports)
------------------------------------------------*/
sbit P3_0		= 0xB0;
sbit P3_1		= 0xB1;
sbit P3_2		= 0xB2;
sbit P3_3		= 0xB3;
sbit P3_4		= 0xB4;
sbit P3_5		= 0xB5;
sbit P3_6		= 0xB6;
sbit P3_7		= 0xB7;

sbit RXD		= 0xB0;       /* Serial data input */
sbit TXD		= 0xB1;       /* Serial data output */
sbit INT0		= 0xB2;       /* External interrupt 0 */
sbit INT1		= 0xB3;       /* External interrupt 1 */
sbit T0			= 0xB4;       /* Timer 0 external input */
sbit T1			= 0xB5;       /* Timer 1 external input */
sbit WR			= 0xB6;       /* External data memory write strobe */
sbit RD			= 0xB7;       /* External data memory read strobe */

/*------------------------------------------------
IP (0xB8) Bit Registers
------------------------------------------------*/
sbit PX0		= 0xB8;
sbit PT0		= 0xB9;
sbit PX1		= 0xBA;
sbit PT1		= 0xBB;
sbit PS0		= 0xBC;
sbit PT2		= 0xBD;
sbit PS1		= 0xBE;

/*------------------------------------------------
SCON1 (0xC0) Bit Registers
------------------------------------------------*/
sbit RI1		= 0xC0;
sbit TI1		= 0xC1;
sbit RB18		= 0xC2;
sbit TB18		= 0xC3;
sbit REN1		= 0xC4;
sbit SM12		= 0xC5;
sbit SM11		= 0xC6;
sbit SM10		= 0xC7;

/* FOR MAIN_CPU_SUBSYSTEM */
/*------------------------------------------------
CMOD (0xC2) Bit Values
------------------------------------------------*/
#define PCA_ECF			0x01
#define PCA_T0_OF		0x08
#define	PCA_EXT_CLK		0x0E
#define	PCA_GATE_OFF	0x80

/*------------------------------------------------
CCON (0xC3) Bit Values
------------------------------------------------*/
#define PCA_CCF0		0x01
#define PCA_CCF1		0x02
#define	PCA_CCF2		0x04
#define	PCA_CCF3		0x08
#define	PCA_CCF4		0x10
#define	PCA_CR			0x40
#define	PCA_CF			0x80
/* END */

/*------------------------------------------------
T2CON (0xC8) Bit Registers
------------------------------------------------*/
sbit CPRL2		= 0xC8;
sbit CT2		= 0xC9;
sbit TR2		= 0xCA;
sbit EXEN2		= 0xCB;
sbit TCLK		= 0xCC;
sbit RCLK		= 0xCD;
sbit EXF2		= 0xCE;
sbit TF2		= 0xCF;

/*------------------------------------------------
PSW (0xD0) Bit Registers
------------------------------------------------*/
sbit P			= 0xD0;
sbit F1			= 0xD1;
sbit OV			= 0xD2;
sbit RS0		= 0xD3;
sbit RS1		= 0xD4;
sbit F0			= 0xD5;
sbit AC			= 0xD6;
sbit CY			= 0xD7;

/* FOR MAIN_CPU_SUBSYSTEM */
/*------------------------------------------------
CCAPM0-4 (0xD1-0xD5) Bit Values
------------------------------------------------*/
#define ECCFs		0x01
#define PWMs		0x02
#define	TOGs		0x04
#define	MATs		0x08
#define	CAPNs		0x10
#define	CAPPs		0x20
#define	ECOMs		0x40
#define	CEXs		0x80
/* END */

/*------------------------------------------------
WDCON (0xD8) Bit Registers
------------------------------------------------*/
sbit RWT		= 0xD8;
sbit EWT		= 0xD9;
sbit WTRF		= 0xDA;
sbit WDIF		= 0xDB;

/*------------------------------------------------
SDSTSR (0xD9) Bit Registers
------------------------------------------------*/
#define	SDC					0x01
#define	STT					0x02
#define	RTCT				0x04
#define	FLASH_INT_STU		0x10
#define	WDRST				0x40
#define	WWDT				0x80

/*------------------------------------------------
MR (0xDE) Bit Registers
------------------------------------------------*/
#define	RST_EXT				0x01
#define	MF_BL_DONE			0x10
#define	CLK_32K_GOOD		0x80

/*------------------------------------------------
Interrupt Vectors:
Interrupt Address = (Number * 8) + 3
------------------------------------------------*/
#define IE0_VECTOR      0  /* 0x03 External Interrupt 0 */
#define TF0_VECTOR      1  /* 0x0B Timer 0 */
#define IE1_VECTOR      2  /* 0x13 External Interrupt 1 */
#define TF1_VECTOR      3  /* 0x1B Timer 1 */
#define SIO_VECTOR      4  /* 0x23 Serial port */

/*------------------------------------------------
FCCR (0xE2) Bit Registers
------------------------------------------------*/
#define	FLASH_SN0		0x00
#define	FLASH_SN1		0x01
#define	FLASH_SN2		0x02
#define	FLASH_SN3		0x03
#define	FLASH_SN4		0x04
#define	FLASH_SN5		0x05
#define	FLASH_SN6		0x06
#define	FLASH_SN7		0x07
#define	FLASH_SN8		0x08
#define	FLASH_SN9		0x09
#define	FLASH_SN10		0x0A
#define	FLASH_SN11		0x0B
#define	FLASH_SN12		0x0C
#define	FLASH_SN13		0x0D
#define	FLASH_SN14		0x0E
#define	FLASH_SN15		0x0F
#define	FLASH_SN16		0x10
#define	FLASH_SN17		0x11
#define	FLASH_SN18		0x12
#define	FLASH_SN19		0x13
#define	FLASH_SN20		0x14
#define	FLASH_SN21		0x15
#define	FLASH_SN22		0x16
#define	FLASH_SN23		0x17
#define	FLASH_SN24		0x18
#define	FLASH_SN25		0x19
#define	FLASH_SN26		0x1A
#define	FLASH_SN27		0x1B
#define	FLASH_SN28		0x1C
#define	FLASH_SN29		0x1D
#define	FLASH_SN30		0x1E
#define	FLASH_SN31		0x1F
#define	FLASH_SN32		0x20
#define	FLASH_SN33		0x21
#define	FLASH_SN34		0x22
#define	FLASH_ALL		0x3F
#define	SEC_GO			0x40
#define	FAST_PROG_ENB	0x80

/*------------------------------------------------
FCSR (0xE3) Bit Registers
------------------------------------------------*/
#define	PWC_CS			0x01
#define	PWC_TO			0x02
#define	PWH_NR			0x04
#define	SW_PWC_TO		0x08
#define	SEC_CS			0x10
#define	SEC_TO			0x20
#define	FCF_INT_ENB		0x40
#define	FCS_INT_ENB		0x80

/*------------------------------------------------
EIE (0xE8) Bit Registers
------------------------------------------------*/
sbit EINT2		= 0xE8;
sbit EINT3		= 0xE9;
sbit EINT4		= 0xEA;
sbit EINT5		= 0xEB;
sbit EINT6		= 0xEC;
sbit EWDI		= 0xED;

/*------------------------------------------------
EIP (0xF8) Bit Registers
------------------------------------------------*/
sbit PINT2		= 0xF8;
sbit PINT3		= 0xF9;
sbit PINT4		= 0xFA;
sbit PINT5		= 0xFB;
sbit PINT6		= 0xFC;
sbit PWDI		= 0xFD;
/*------------------------------------------------
CCTRL (0xFE) Bit Registers
------------------------------------------------*/
#define	CMD_RDC		0x01
#define	WRCM		0x02
#define	CMD_RDY		0x08
#define	CMD_WRC		0x10
#define	RDCM		0x20
#define	RD_RDY		0x80

/*------------------------------------------------
CSSR (0xFF) Bit Registers
------------------------------------------------*/
#define	CMD_IRDY_INT_STU	0x01
#define	RD_IRDY_INT_STU		0x10

/*------------------------------------------------
The Same SFRs in MCPU and WCPU
------------------------------------------------*/
/* FOR MAIN_CPU_SUBSYSTEM */
  #define MRC0	RC0
  #define MRC1	RC1
  #define MRC2	RC2
  #define MRC3	RC3
  #define MRC4	RC4
  #define MPCCR	PCCR
  #define MPCDR	PCDR
  
  #define MWC0	WC0
  #define MWC1	WC1
  #define MWC2	WC2
  #define MWC3	WC3
  #define MWC4	WC4
  #define MCCTRL	CCTRL
  #define MCSSR	CSSR
/* FOR WIFI_CPU_SUBSYSTEM */
  #define WRC0	RC0
  #define WRC1	RC1
  #define WRC2	RC2
  #define WRC3	RC3
  #define WRC4	RC4
  #define WPCCR	PCCR
  #define WPCDR	PCDR
  
  #define WWC0	WC0
  #define WWC1	WC1
  #define WWC2	WC2
  #define WWC3	WC3
  #define WWC4	WC4
  #define WCCTRL	CCTRL
  #define WCSSR	CSSR
/* END */

/*------------------------------------------------
------------------------------------------------*/

#endif

