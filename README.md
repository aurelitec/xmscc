# XMS Control Center
An XMS management &amp; information utility, with a simple text-based user interface. Written in Borland C++ in 1997.

![Screenshot of XMS Control Center running on Windows 7 in VirtualBox](https://github.com/aurelitec/xmscc/blob/master/assets/readme/xmscc-win7-vbox.png)

## Introduction

Extended memory is RAM above the 1-Megabyte boundary, and is available only on machines that have a 286 or greater CPU. In order for most applications to be able to use extended memory, an XMS driver must be installed to manage it. The most popular XMS driver is HIMEM.SYS from Microsoft.

A DOS application can access extended memory even when running in a DOS shell under Windows 3.x or under Windows 95.

XMS Control Center displays the current setup of your eXtended Memory Manager (XMM) and allows you to change the layout of extended memory by modifying the currently allocated Extended Memory Blocks (EMBs).

When a program allocates a block of extended memory it receives a handle to that block and the handle is used in all operations with the block. XMS Control Center is able to display information about each block currently allocated: its handle, size, linear address and lock count. More important, XMS Control Center allows you to free, lock, unlock and reallocate any allocated extended memory block or to allocate a new one.

## System Requirements

In order to run XMS Control Center you will need a 286 or later CPU with extended memory and an XMS driver that implements the Extended Memory Specification version 2.0 or above installed in your system.

## Using XMS Control Center

### Interactive Mode

XMS Control Center enters interactive mode if you call it without any parameter in the command-line or only with the `/XMS` or `/DV` switches.

The `/DV` switch forces XMSCC to use direct video RAM output in interactive mode. By default XMSCC uses video ROM calls that will work on any IBM BIOS-compatible system. Direct video output is faster but your system's video hardware must be identical to IBM display adapters.

See HIMEM.SYS v XMS CALLS for information about the `/XMS` switch.

The XMS Control Center interactive interface was designed to be as self-explanatory as possible. On the main screen there are two opened windows: one that contains the list of the currently allocated extended memory blocks (EMBs) and one that displays information about the XMS driver and extended memory statistics. The status line located at the bottom of the EMBs window lists the function key commands.

| Key | Function            | Description                                             |
| --- | ------------------- |---------------------------------------------------------|
| F2  | Free                | Frees the highlighted EMB from the EMB list box.        |
| F3  | Lock                | Locks the highlighted EMB from the EMB list box.        |
| F4  | Unlock              | Unlocks the highlighted EMB from the EMB list box.      |
| F5  | Reallocate          | Reallocates the highlighted EMB from the EMB list box.  |
| F6  | Allocate new block  | Allocates a new EMB.                                    |

Press `F10` to end the program.

### The Configuration File (XMSCC.INI)

When XMS Control Center starts in interactive-mode it looks for `XMSCC.INI` in the current directory and then in the directory where `XMSCC.EXE` resides. XMS Control Center reads the contents of the configuration file and then configures itself accordingly. You can make XMS Control Center to ignore the .INI file by using the `/NOCFG` command-line parameter. You can also make XMS Control Center not to report any error found in the .INI file by using the `/I` command-line parameter. XMS Control Center reports a configuration error only when a key contains invalid values. It does not report missing configuration keys.

The `XMSCC.INI` file is a text file and can be edited with any text editor. XMS Control Center does not modify or create it. It only reads the configuration from it. So you are advised not to remove any configuration keys from the file. It is best to keep a copy of the original .INI file.

For more information on configuring XMS Control Center see the comments from `XMSCC.INI`.

### Command-Line Mode

Use `XMSCC/?` for help.

To print an XMS summary to the standard output call XMS Control Center with the following syntax: `XMSCC /P [/XMS]`. The summary is made up of information about the XMS driver and extended memory statistics and the list of all currently allocated extended memory blocks. You can redirect the output to a file or printer. (See HIMEM.SYS v XMS CALLS for information about the `/XMS` switch)

To modify the currently allocated extended memory blocks use the following syntax:

 ```
 XMSCC [/F<handle>] [/L<handle>] [/U<handle>] [/R<handle>,<Ksize>] [/A<Ksize>]

  /F<handle>            Free the extended memory block (EMB) addressed by handle.
  /L<handle>            Lock the EMB addressed by handle.
  /U<handle>            Unlock the EMB addressed by handle.
  /R<handle>,<Ksize>    Reallocate the EMB addressed by handle. Ksize is the new size of the block in K-bytes.
  /A<Ksize>             Allocate a new EMB of Ksize K-bytes.
```

You can do more than one change with only one command-line.

**Example:**

```
XMSCC /F43202 /R43212,77 /L43242 /A0
```

This will free the EMB addressed by handle 43202, reallocate the EMB addressed by handle 43212 to 77 K-bytes, lock the EMB addressed by handle 43242 and allocate a new EMB of 0 K-bytes (use this to reserve a handle).

## HIMEM.SYS v XMS Calls

By default, if HIMEM.SYS version 3.09 or above is detected XMS Control Center will retrieve information about extended memory blocks directly from HIMEM.SYS. When using HIMEM.SYS XMS Control Center will list not only the currently allocated extended memory blocks but also the free ones (the ones that are currently not used). As HIMEM.SYS is an XMS driver that has a fixed number of extended memory handles, the free handles in the system will also be displayed.

If a previous version of HIMEM.SYS or other XMS driver is detected the information will be retrieved using XMS calls. This method is slower and will list only the currently allocated extended memory blocks.

Use the `/XMS` switch to force XMS Control Center to use the second method (XMS calls) even if HIMEM.SYS version 3.09 or above is detected.

Note: HIMEM.SYS version 3.09 is a part of MS-DOS 6.00.

## Modifying Extended Memory Blocks

Modifying extended memory blocks used by other applications is generally NOT a very good idea. Modifying (freeing, reallocating) the wrong block may lead the system into an unstable state and may even damage files on your disk! Remember, many disk cache utilities use extended memory. Other programs could do a lot of damage as well if you mess with their allocated blocks.

The main purpose of the free function of XMS Control Center is to help you free EMBs that were not freed by their owner program when it ended (programs that crashed and did not clean after themselves or programs that you are developing).

Also use the reallocation function with care. If you reallocate an extended memory block to a smaller size than the old block's size, all data at the upper end of the old block WILL BE LOST.

With XMS drivers that have a fixed number of extended memory handles (like HIMEM.SYS) you can reserve a handle by allocating a block of 0 K-bytes.

## Technical Information

### Exit Codes

XMS Control Center will return the following result codes after execution:

| Code | Description                                                                    |
| ---- | ------------------------------------------------------------------------------ |
| 0    | Normal                                                                         |
| 1    | No XMS driver was found                                                        |
| 2    | A VDISK device was detected. VDISK devices are incompatible with XMS drivers.  |
| 3    | Out of memory (if the information about extended memory blocks is larger than the available memory - a very extreme case, unlikely to appear) |

XMS Control Center was written using the XMS Interface for Borland C/C++ and Borland Pascal (XMSBCP) Version 1.1 (freeware) developed by Aurelitec. This library is available by anonymous ftp from Simtel and its mirrors (msdos/pgmutl/xmsbcp11.zip).

XMS Control Center has been tested under MS-DOS 5.0, 6.00, 6.2, 6.22, Microsoft Windows 3.1, Microsoft Windows 95 with HIMEM.SYS.

## Release Information

Prefixes:

  '->' Indicates a major improvement that require special attention.
  '+'  Indicates a change or improvement that requires user attention.
  '-'  Indicates a change or improvement that does not require user attention.

Version 1.5 (July 21 1997)

    -> Added the possibility of customizing the interactive-mode of XMS
       Control Center through a text .INI configuration file that can be
       edited by the user with any text editor. Various options can be
       changed (even the colors).
     + Added 2 new command-line switches: /NOCFG (disable configuration read)
       and /I (ignore any errors in the configuration file).
     + Added license, disclaimer and contact information in the program's
       help screen (invoked by /? or /H).
     - Improved documentation.
     - Minor improvements in code (also fixed a small bug).

Version 1.1 (June 18 1997)

     + Added a new command-line switch (/DV) to force XMSCC to use direct
       video RAM output in interactive mode.
     - Removed a previous undocumented command-line switch (/M) that was
       intensively used in the debugging versions of XMSCC and was left by
       mistake in the final release.
     - Minor improvements in the documentation.

Version 1.0 (May 9 1997)

    First official release.

## Future Plans

An advanced feature of viewing/editing extended memory.

I'm always looking for suggestions!


## Contacting the Author

For technical support, questions, comments, problems or incompatibilities, suggestions, enhancements, write to

contact@aurelitec.com

or visit

https://www.aurelitec.com


Note: If you are using other eXtended Memory Manager than HIMEM.SYS and you'd like XMS Control Center to support it directly, contact me.

Note: If you include XMS Control Center on a CD-ROM package feel free to send a copy to the author.

The more feedback I get the more likely I am to continue improving XMS Control Center and writing other freeware utilities.

## Contributing
[Pull requests](https://github.com/aurelitec/xmscc/pulls) are welcome. For major changes, please open an [issue](https://github.com/aurelitec/xmscc/issues) first to discuss what you would like to change.

## License

Copyright (c) Aurelitec. All rights reserved.

Licensed under the [MIT](LICENSE) license.
