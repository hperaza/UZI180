/***************************************************************
UZI (Unix Z80 Implementation) Kernel:  devhd.c
   Unique Hardware driver contained in hdasm.asz
----------------------------------------------------------------
 Adapted from UZI By Doug Braun, and UZI280 by Stefan Nitschke
            Copyright (C) 1998 by Harold F. Bower
       Portions Copyright (C) 1995 by Stefan Nitschke
****************************************************************/
/* Revisions:
 * 1.1.98 - Added IDE and SCSI configurations.		HFB
 * 14.7.98 - Finalized config via HDCONF.H w/variables.	HFB
 */

#include <unix.h>
#include <config.h>
#include <extern.h>
#include <hdconf.h>      /* Add configuration-specific data */

#ifdef GIDE
extern unsigned char hdspt, hdhds;
#endif

    /* All Hard Disk Drivers pass Commands and Data via the cmdblk array.
     * The definition of each byte in the array varies as:
     *
     * SCSI:  Drivers use the short (6 byte) SCSI Read/Write Commands
     * Byte 0 - SCSI Command (08H = Read, 0AH = Write, 03H = Read Sense)
     *      1 - B7..5 = LUN, B4..0 = High 5 bits of Block Number
     *      2 -   Middle 8 bits of Block Number
     *      3 -     Low 8 bits of Block Number
     *      4 - Number of 512-byte Blocks to Read
     *      5 - Control Flags (unused, set to 0)
     * IDE:  Drivers use data in similar manner as SCSI as:
     * Byte 0 - IDE Command (20H = Read, 30H = Write)
     *      1 - High bits of Block Number
     *      2 -   Middle 8 bits of Block Number
     *      3 -     Low 8 bits of Block Number
     *      4 - Number of 512-byte Blocks to Read
     *      5 -  (unused, set to 0)
     */
char cmdblk[6] = { 0, LUN<<5, 0, 0, 0, 0 };    /* GLOBAL.  Used in hdasm.asz */
unsigned hd_offset = 0;      /* Track offset for Selected Unit, in hdasm.asz */

char  *dptr  = NULL;        /* Ptr to Data Buffer for I/O */
int    dlen  = 0;           /* Length of Data I/O Transfer */
char  *cptr  = NULL;        /* Ptr to Code Buffer for I/O */
int    busid = 1;           /* Hardware Unit (SCSI = 0,2,4,..64.  Unit Bit) */

extern int scsiop();

wd_read (minor, rawflag)
unsigned minor;
int rawflag;
{
    int setup();

    cmdblk[0] = RDCMD;
    if (setup (minor, rawflag))
        return (0);

    chkstat (scsiop(), 1);
    return (cmdblk[4] << 9);
}


wd_write (minor, rawflag)
unsigned minor;
int rawflag;
{
    int setup();

    cmdblk[0] = WRCMD;
    if (setup (minor, rawflag))
        return (0);

    chkstat (scsiop(), 0);
    return (cmdblk[4] << 9);
}


setup (minor, rawflag)
unsigned minor;
int rawflag;
{
    register blkno_t block;

    cptr = cmdblk;
    busid = 1;
    cmdblk[5] = 0;       /* Clear Flags */
#ifdef GIDE
    hdspt = HD_Sector;           /* Set physical drive params if IDE */
    hdhds = HD_Heads;
#endif
    if (rawflag)
    {
        if (rawflag == 2)
        {
            cmdblk[4] = swapcnt >> 9;
            dlen = swapcnt;
            dptr = swapbase;
            block = swapblk;
        }
        else
        {
            cmdblk[4] = udata.u_count >> 9;
            dlen = udata.u_count;
            dptr = udata.u_base;
            block = udata.u_offset.o_blkno;
        }       
    }
    else
    {
        cmdblk[4] = 1;
        dlen = 512;
        dptr = udata.u_buf->bf_data;
        block = udata.u_buf->bf_blk;
    }

    switch (minor)			/* Also need to insure that ending
					   block # is valid on drive. HFB
					*/
    {
        case 0: hd_offset = HD0_Start;
                if (block < (HD0_Size << 4))
                    goto setCDB;
                else break;
        case 1: hd_offset = HD1_Start;
                if (block < (HD1_Size << 4))
                    goto setCDB;
                else break;
        case 2: hd_offset = SWAP_Start;
                if (block < (SWAP_Size << 4))
                    goto setCDB;
                else break;
        default: ;
    }
    udata.u_error = ENXIO;
    return (1);

setCDB:
    cmdblk[3] = block;
    cmdblk[2] = block >> 8;
    cmdblk[1] = LUN << 5;
    return (0);
}


/* Check results of operation returning (int)"stat".
 *  rdflag = 1 for Read, 0 for Write.
 */
chkstat (stat, rdflag)
int stat;
int rdflag;
{
    if (stat)
    {
        kprintf ("HD %s failure stat: %x", rdflag ? "Read": "Write", stat);
        panic ("");
    }
}


wd_open (minor)
int minor;
{
    return (0);
}
