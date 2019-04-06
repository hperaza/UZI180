/***************************************************************
       UZI (Unix Z80 Implementation) Kernel:  hdconf.h
  Hard Disk Partitioning Physical and Logical data Definitions
----------------------------------------------------------------
            Copyright (C) 1998 by Harold F. Bower
****************************************************************/
/* Revisions:
 */

/* ! ! ! ! !  Define ONLY ONE of the following types:  ! ! ! ! ! */
/* ! ! ! ! !            UnDefine all others            ! ! ! ! ! */
#define  SCSI
#undef   GIDE

#define LUN   0    /* Only needed for SCSI, but no adverse effect on GIDE */

/* Set Read and Write Command Bytes for appropriate Disk System */

#ifdef SCSI
#define RDCMD 0x08
#define WRCMD 0x0a
#endif

#ifdef GIDE
#define RDCMD 0x20
#define WRCMD 0x30
#endif

/* For GIDE and Unique data, Insert Drive geometry here (as an example).
 * These values are used in the low-level driver to compute the
 * physical CHS address for Read/Write operations.
 */
#ifdef GIDE
#define  HD_Cyls   998     /* for Toshiba MK4118 2.5" 82 MB */
#define  HD_Heads   10
#define  HD_Sector  17
#endif

/* Initial Track Offsets, and Number of Tracks in each partition. */
/***************************************************************************
 * NOTE: For Hard drives, each "track" equates to 16 512-byte blocks (8 KB).
 *       The Low-level assembly driver should multiply the offset by 16
 *       before adding to the computed block number passed from devwd.c.
 ***************************************************************************
 */

#define HD0_Start 2000    /* 1st Drive Start.  CP/M uses first 2000 "tracks" */
#define HD0_Size  1900    /* Number of "tracks" in HD0 (1900 * 8 = 15.2 MB) */

#define HD1_Start 4000    /* 2nd Drive Start. */
#define HD1_Size  1700    /* Number of "tracks" in HD1 (2700 * 8 = 21.6 MB) */

/* Swap space is needed for the original UZI system configuration.
 * It needs to be defined until replaced or redefined.
 */
#define SWAP_Start 3900     /* Swap space Start. (until Swap not needed) */
#define SWAP_Size   100     /* Number of "tracks" in Swap (100 * 8 = 800 KB) */
