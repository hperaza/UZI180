Boot Loader for UZI180 floppies (P112 version)
----------------------------------------------

The block 0 on a UZI filesystem is the boot block, and is reserved for the
bootloader code. However, existing UZI implementations rely on CP/M in order
to boot the system: the kernel is started as a normal CP/M application and
then it takes control over the system. It would be more desirable, however,
to have standalone bootable disks.

This package contains a simple UZI boot loader. It consists of the boot
program code and a boot installer.


Files
-----

  README.TXT   - this file

  insboot      - boot installer (UZI180 executable)
  insboot.c    - source code for the boot installer
  loader       - floppy boot loader in binary format
  loader.asz   - source code for the boot loader

  stdio.h      - required include file
  unix.h       - required include file (from the UZI180 distribution)
  z180.i       - required include file

  makeboot.sub - SUB file to compile the whole thing
  linkldr      - boot loader link command file
  linkins      - insboot link command file
  Makefile     - makefile to build insboot under Linux using the
                 zxcc emulator

Note that all the text files are supplied in Unix format (lines end with
a single linefeed). To compile the loader you still need the UZI180
version of the HiTech libc runtime library and runtime header file 
(uzilibc.lib and crt.obj respectively), available separatedly.


How to use it
-------------

Once you compiled your UZI kernel, copy it to your floppy disk, the location
and name is not important, then copy the boot installer and the boot loader
binaries ('insboot' and 'loader' respectively) to some directory, both of
them must be in the same directory. Make sure the 'insboot' program has
execute permissions (chmod 0755 insboot). Start UZI if you're not already
running it, then invoke insboot specifyng the name of the device to be made
bootable and the full path to the kernel.

For example, suppose your kernel is named vmuzi and is in the root
directory, and the boot utilities are in the /boot directory, the command
sequence will be the following: 

   cd /boot 
   ./insboot /dev/fd0 /vmuzi 


How it works
------------

The boot installer program first reads the block allocation map for the
specified file, then writes to the boot block of the specified device the
boot code from the 'loader' file and appends the block allocation map.

When the system is booted, the loader uses the block allocation map to
figure out where the kernel resides, loads it block by block starting from
address 100h and passes control to it.

With this scheme it does not matter where the kernel is physically located
or whether it is fragmented or not. You should, however, be aware that the
insboot program must be run again every time the location or the size of the
kernel file changes, for example, if you replaced the file with a new
version (even if you copied it to the same place and using the same name) or
if you copied the file to another directory and then erased the original
file. In the other hand, insboot does not need to be re-run if you used the
mv command to rename the kernel file or to move it from one directory to
another, since in that case the physical location is preserved.

Note that the simple boot loader suupplied will work only with the P112,
since it uses the ROM monitor disk services. However, the whole idea can be
applied to any other systems as well, you'll just have to write a suitable
loader program and to modify the installer appropiately. In those cases
where the boot code is so large that the kernel allocation map would not
fit, you can use the following workaround: save the kernel allocation map
into a separate file instead (that file will occupy just a single block),
and pass that block number to the bootloader. During boot, the bootloader
would fetch first the block containing the kernel allocation map, and then
use it to load the kernel.


