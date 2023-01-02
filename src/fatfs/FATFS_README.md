# fastfs R0.13c Update for eLua

## General
The update of fatfs to R0.13c should be mostly compatible with the old version. The disk I/O interface in elua_mmc.c is still the same as before. There are some improvements, that have nothing to do with fatfs itself, see below

## Configuration parameters
The mmcfs section in the board configuration has some additional parmeters to allow configuration of fatfs:

Example:
```
mmcfs= { spi=0, codepage=850, 
         use_utf8_api=false, tiny=false, use_locking=true,
         files=8, lfn_length=255 
       }
```      

| Value        	| Default 	| Description                                                                       	| ffconf.h parameter    	| eLua define       	|
|--------------	|---------	|-----------------------------------------------------------------------------------	|-----------------------	|-------------------	|
| codepage     	| 437     	| Codepage where long file names are converted too (see ffconf.h for allowed values 	| FF_CODE_PAGE          	| MMCFS_CODEPAGE    	|
| use_utf8_api 	| false   	| Convert long file names to utf-8 encoding instead of codepag (true/false)         	| FF_LFN_UNICODE        	| MMMCFS_API_MODE   	|
| tiny         	| true    	| Use FF_FS_TINY parameter (see fatfs documentation) (true/false)                   	| FF_FS_TINY            	| MMFCFS_TINY       	|
| use_locking  	| true    	| Enable fatfs file locking (true/false)                                            	| FF_FS_LOCK            	| MMCFS_USE_LOCKING 	|
| files        	| 4       	| Max. number of open files (4-32)                                                  	| FF_FS_LOCK            	| MMCFS_MAX_FDS     	|
| lfn          	| 1       	| long file name mode - see ffconf.h for documentation (0,1,2)                      	| FF_USE_LFN            	| MMCFS_USE_LFN     	|
| lfn_length   	| 30      	| Maximum length of a long file name (30-255)                                       	| FF_MAX_LFN/FF_LFN_BUF 	| MMCFS_MAX_LFN     	|


## File locking
Enabling file locking is highly recommended, because otherwise fatfs can corrupt data on the file system, when a file is opened for writing more than once. 

## Long file name support
Setting lfn to 0 will limit fatfs to 8.3 filenames in upper case like old DOS versions. It only makes sense in very memory constrained enviornments. 
lfn=1 uses a static working buffer for the long file name in the bss segment, this is the mode fatfs 0.07 in elua has used
lfn=2 uses a buffer on the stack, so it only consumes memory when files are accessed. What is better may depend on your needs.
lnf=3 requires implementation of heap allocation functions for fatfs, this is the reason why the configurator currently does not allow this mode. 

Setting use_utf8_api will change the encoding/decoding of non-ASCII characters in long file names to UTF-8 instead of using a codepage. This is expermental. 

## ffunicode.c
The old file ccsbcs.c is replaced by ffunicode.c from fatfs 0.13c. It serves the same purpose.
The file is very big, but if a single code page is selected with the codepage parameter the compiled result will only contain the selected code page. 
The usage of DBCS codepages (9XX) is not tested. 

## Error code mapping
The function map_error in mmcfs.c maps fatfs error codes to newlib error codes. This will result in better error messages.

## Use of platform_spi_select on for chip select
When no gpio port/pin is set in the mmcfs section of the board configuration the platform_spi_select function is used to control the cs pin. 
