; Z180 Register Mnemonics

CNTLA0	EQU	00H		; ASCI Control Reg A Ch 0
CNTLA1	EQU	01H		; ASCI Control Reg A Ch 1
CNTLB0	EQU	02H		; ASCI Control Reg B Ch 0
CNTLB1	EQU	03H		; ASCI Control Reg B Ch 1
STAT0	EQU	04H		; ASCI Status Reg Ch 0
STAT1	EQU	05H		; ASCI Status Reg Ch 1
TDR0	EQU	06H		; ASCI Tx Data Reg Ch 0
TDR1	EQU	07H		; ASCI Tx Data Reg Ch 1
RDR0	EQU	08H		; ASCI Rx Data Reg Ch 0
RDR1	EQU	09H		; ASCI Rx Data Reg Ch 1

CNTR	EQU	0AH		; CSI/O Control Reg
TRDR	EQU	0DH		; CSI/O Tx/Rx Data Reg

TMDR0L	EQU	0CH		; Timer Data Reg Ch0-Low
TMDR0H	EQU	0DH		; Timer Data Reg Ch0-High
RLDR0L	EQU	0EH		; Timer Reload Reg Ch0-Low
RLDR0H	EQU	0FH		; Timer Reload Reg Ch0-High
TCR	EQU	10H		; Timer Control Reg
TMDR1L	EQU	14H		; Timer Data Reg Ch1-Low
TMDR1H	EQU	15H		; Timer Data Reg Ch1-High
RLDR1L	EQU	16H		; Timer Reload Reg Ch1-Low
RLDR1H	EQU	17H		; Timer Reload Reg Ch1-High
FRC	EQU	18H		; Free-Running Counter

CCR	EQU	1FH		; CPU Control Reg (Z8S180 & higher Only)

SAR0L	EQU	20H		; DMA Source Addr Reg Ch0-Low
SAR0H	EQU	21H		; DMA Source Addr Reg Ch0-High
SAR0B	EQU	22H		; DMA Source Addr Reg Ch0-Bank
DAR0L	EQU	23H		; DMA Dest Addr Reg Ch0-Low
DAR0H	EQU	24H		; DMA Dest Addr Reg Ch0-High
DAR0B	EQU	25H		; DMA Dest ADDR REG CH0-Bank
BCR0L	EQU	26H		; DMA Byte Count Reg Ch0-Low
BCR0H	EQU	27H		; DMA Byte Count Reg Ch0-High
MAR1L	EQU	28H		; DMA Memory Addr Reg Ch1-Low
MAR1H	EQU	29H		; DMA Memory Addr Reg Ch1-High
MAR1B	EQU	2AH		; DMA Memory Addr Reg Ch1-Bank
IAR1L	EQU	2BH		; DMA I/O Addr Reg Ch1-Low
IAR1H	EQU	2CH		; DMA I/O Addr Reg Ch2-High
BCR1L	EQU	2EH		; DMA Byte Count Reg Ch1-Low
BCR1H	EQU	2FH		; DMA Byte Count Reg Ch1-High
DSTAT	EQU	30H		; DMA Status Reg
DMODE	EQU	31H		; DMA Mode Reg
DCNTL	EQU	32H		; DMA/Wait Control Reg

IL	EQU	33H		; INT Vector Low Reg
ITC	EQU	34H		; INT/TRAP Control Reg
RCR	EQU	36H		; Refresh Control Reg
CBR	EQU	38H		; MMU Common Base Reg
BBR	EQU	39H		; MMU Bank Base Reg
CBAR	EQU	3AH		; MMU COmmon/Bank Area Reg
OMCR	EQU	3EH		; Operation Mode Control Reg
ICR	EQU	3FH		; I/O Control Reg

; Z182 Additional Registers (uncomment to define)

;BRK0	EQU	12H		; Break Control Reg Ch 0
;BRK1	EQU	13H		; Break Control Reg Ch 1

; Some bit definitions used with the Z-180 on-chip peripherals:

TDRE	EQU	02H		; ACSI Transmitter Buffer Empty
RDRF	EQU	80H		; ACSI Received Character available

;++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
; Extended Features of Z80182 for P112

WSGCS	EQU	0D8H		; Wait-State Generator CS
ENH182	EQU	0D9H		; Z80182 Enhancements Register
PINMUX	EQU	0DFH		; Interrupt Edge/Pin Mux Register
RAMUBR	EQU	0E6H		; RAM End Boundary
RAMLBR	EQU	0E7H		; RAM Start Boundary
ROMBR	EQU	0E8H		; ROM Boundary
FIFOCTL	EQU	0E9H		; FIFO Control Register
RTOTC	EQU	0EAH		; RX Time-Out Time Constant
TTOTC	EQU	0EBH		; TX Time-Out Time Constant
FCR	EQU	0ECH		; FIFO Register
SCR	EQU	0EFH		; System Pin Control
RBR	EQU	0F0H		; MIMIC RX Buffer Register	(R)
THR	EQU	0F0H		; MIMIN TX Holding Register	(W)
IER	EQU	0F1H		; Interrupt Enable Register
LCR	EQU	0F3H		; Line Control Register
MCR	EQU	0F4H		; Modem Control Register
LSR	EQU	0F5H		; Line Status Register
MDMSR	EQU	0F6H		; Modem Status Register
MSCR	EQU	0F7H		; MIMIC Scratch Register
DLATL	EQU	0F8H		; Divisor Latch (Low)
DLATM	EQU	0F9H		; Divisor Latch (High)
TTCR	EQU	0FAH		; TX Time Constant
RTCR	EQU	0FBH		; RX Time Constant
IVEC	EQU	0FCH		; MIMIC Interrupt Vector
MIMIE	EQU	0FDH		; MIMIC Interrupt Enable Register
IUSIP	EQU	0FEH		; MIMIC Interrupt Under-Service Register
MMCR	EQU	0FFH		; MIMIC Master Control Register

; Z80182 PIO Registers

DDRA	EQU	0EDH		; Data Direction Register A
DRA	EQU	0EEH		; Port A Data
DDRB	EQU	0E4H		; Data Direction Register B
DRB	EQU	0E5H		; Data B Data
DDRC	EQU	0DDH		; Data Direction Register C
DRC	EQU	0DEH		; Data C Data

;++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
; ESCC Registers on Z80182

SCCACNT	EQU	0E0H		; ESCC Control Channel A
SCCAD	EQU	0E1H		; ESCC Data Channel A
SCCBCNT	EQU	0E2H		; ESCC Control Channel B
SCCBD	EQU	0E3H		; ESCC Data Channel B

; [E]SCC Internal Register Definitions

RR0	EQU	00H
RR1	EQU	01H
RR2	EQU	02H
RR3	EQU	03H
RR6	EQU	06H
RR7	EQU	07H
RR10	EQU	0AH
RR12	EQU	0CH
RR13	EQU	0DH
RR15	EQU	0FH

WR0	EQU	00H
WR1	EQU	01H
WR2	EQU	02H
WR3	EQU	03H
WR4	EQU	04H
WR5	EQU	05H
WR6	EQU	06H
WR7	EQU	07H
WR9	EQU	09H
WR10	EQU	0AH
WR11	EQU	0BH
WR12	EQU	0CH
WR13	EQU	0DH
WR14	EQU	0EH
WR15	EQU	0FH

; FDC37C665/6 Parallel Port in Standard AT Mode

DPORT	EQU	8CH		; Data Port
SPORT	EQU	8DH		; Status Port
CPORT	EQU	8EH		; Control Port

; FDC37C665/6 Configuration Control (access internal registers)

CFCNTL	EQU	90H		; Configuration control port
CFDATA	EQU	91H		; Configuration data port

; FDC37C665/6 Floppy Controller on P112  (Intel 80277 compatible)

DCR	EQU	92H		; Drive Control Register (Digital Output)
MSR	EQU	94H		; Main Status Register
DR	EQU	95H		; Data/Command Register
DRR	EQU	97H		; Data Rate Register/Disk Changed Bit in B7

_DMA	EQU	0A0H		; Diskette DMA Address

; FDC37C665/6 Serial Port  (National 16550 compatible)

_RBR	EQU	98H		;R	Receiver Buffer
_THR	EQU	98H		;W	Transmit Holding Reg
_IER	EQU	99H		;RW	Interrupt-Enable Reg
_IIR	EQU	9AH		;R	Interrupt Ident. Reg
_FCR	EQU	9AH		;W	FIFO Control Reg
_LCR	EQU	9BH		;RW	Line Control Reg
_MCR	EQU	9CH		;RW	Modem Control Reg
_LSR	EQU	9DH		;RW	Line Status Reg
_MMSR	EQU	9EH		;RW	Modem Status Reg
_SCR	EQU	9FH		;N/A	Scratch Reg. (not avail in XT)
_DDL	EQU	98H		;RW	Divisor LSB | wih DLAB
_DLM	EQU	99H		;RW	Divisor MSB | set High
