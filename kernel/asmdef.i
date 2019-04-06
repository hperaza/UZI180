; Conditional Flag Definitions.

FALSE	EQU	0
TRUE	EQU	1

; General-Purpose Ascii Character Equates.

BS	EQU	08H		; Ascii BackSpace
TAB	EQU	09H		; Ascii Horizontal Tab
LF	EQU	0AH		; Ascii Line Feed
CR	EQU	0DH		; Ascii Carriage Return
ESC	EQU	1BH		; Ascii ESCape Char

; !!!!!-- Ensure definitions match those in unix.h --!!!!!

TICKSPERSEC  EQU   50		; Needed to compute Timer reload Value

; Byte Offsets into User Data Structure (udata/ub).
; !!!!!-- Ensure definitions match udata struct in unix.h --!!!!!

OSYS	EQU	2		; Offset in _ub for insys flag (Byte)
OCALL	EQU	3		; Offset in _ub for Function Call No. (Byte)
ORET	EQU	4		; Offset in _ub for Fcn Return Loc'n (Word)
ORVAL	EQU	6		; Offset in _ub for Fcn Return Value (Word)
OERR	EQU	8		; Offset in _ub to Error Status (Word)
OSP	EQU	10		; Offset in _ub to Stack Pointer (Word)
OBC	EQU	12		;
OCURSIG	EQU	14		; Offset in _ub to Signal being caught (Word)
OARGN	EQU	16		; Offset in _ub to Argn Parameter (Word)
OSIGVEC	EQU	65		; Offset in _ub to the Array of Signal Vectors
OPAGE	EQU	121		; Offset in _ub to Process' BBR Reg base

; TTY Ioctl Functions for UZI180.  Used by EMU and TTYASM.
; !!!!!-- Ensure definitions match those in unix.h --!!!!!

TIOCGETP  EQU	0		; UZI ioctl Return tty Parms
TIOCSETP  EQU	1		; UZI ioctl Set tty Parms
TIOCSETN  EQU	2		; UZI ioctl Return buffer count
TIOCTLSET EQU	9		; UZI ioctl Unique - Disable Ctl Char Proc
TIOCTLRES EQU	10		; UZI ioctl Unique - Enable Ctl Char Proc

; File Descriptors needed by EMU for Console IO.

STDIN	  EQU   0		; file descriptor value of keyboard
STDOUT	  EQU	1		; file descriptor value of display

; Z180/Z182 Hardware configurations.  Set to provide initial base conditions.

HISPEED	EQU	TRUE		; Set TRUE if operating Z8S180 or Z80182 with
				;  "divide-by-1" vice default "divide-by-2"
MHZ	DEFL	16		; CPU Crystal Frequency, truncate to nearest
				;  even MegaHertx (this actually 18.432 MHz)
	COND	HISPEED .eq. FALSE
MHZ	DEFL	MHZ / 2		; At Low Speed (powerup default), Crystal
	ENDC			;  frequency is divided by 2

PRTCONS	EQU	50000 / TICKSPERSEC  ; 51200 constant value for systems at
				; 6.144/9.216/18.432/36.864 MHz (use 50000 for
				; 8.0/16.0 MHz systems)  Compute as:
				;   (Clock_freq / 20) / MHZ whole # constant
RELOAD	EQU	PRTCONS * MHZ	; Programmable Reload Timer value

MWAIT	EQU	1		; Additional Memory Wait States (0..3)
IOWAIT	EQU	2		; Additional IO Wait States (0..3)

; Stack definitions.  Need hard addresses to optimize data movements during
; Process swapping.

KSTACK	EQU	0F200H		; Kernel Stack at "_ub+512" working Down to ub
ISTACK	EQU	0F300H		; Timer Interrupt Stack at "_ub+768" working
				;  Down to Kernel Stack.

; A couple of CP/M-specific values used in EMU.

fcb	EQU	005CH		; Default CP/M FCB
buff	EQU	0080H		; Default CP/M Buffer

; ---- H/W Unique Definitions ----
