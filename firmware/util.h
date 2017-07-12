/*
 * =====================================================================================
 *
 *       Filename:  util.h
 *    Description:  functions and definitions
 *
 *        Version:  1.0
 *        Created:  07/06/2017 04:02:22 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Gunjae Koo (gunjae.koo@gmail.com)
 *   Organization:  
 *
 * =====================================================================================
 */

#ifndef _UTIL_H_
#define _UTIL_H_

#include "nvme/nvme.h"
#include "nvme/host_lld.h"
#include "nvme/io_access.h"
#include "xil_printf.h"	// for printf()

// Special code for in-SSD functions
#define SPC_PAGE_FILTERED_OUT 9999

#define DMA_DIRECT_TEST 1

// logging on/off (yunho's)
//#define FI_PRINT 0

//oyh
//FI on/off (1:on, 0:off)
//#define FI_ON 0

#define VERBOSE 1

// gunjae's functions for monitoring
#if (VERBOSE==1)
	#define GK_PRINT(args...) xil_printf(args)
	#define GK_INIT_PRINT(args...) xil_printf(args)
	#define GK_CMD_PRINT(args...) xil_printf(args)
	#define GK_BUF_PRINT(args...) xil_printf(args)
	#define GK_DMA_PRINT(args...) xil_printf(args)
	#define GK_FTL_PRINT(args...) xil_printf(args)
#else
	#define GK_PRINT(args...) xil_printf(args)
	#define GK_INIT_PRINT(args...)
	#define GK_CMD_PRINT(args...)
	#define GK_BUF_PRINT(args...)
	#define GK_DMA_PRINT(args...)
	#define GK_FTL_PRINT(args...)
#endif	// VERBOSE

inline unsigned char parse_flags_from_nvme_io_cmd(NVME_IO_COMMAND *nvmeIOCmd);
inline unsigned char get_flags_from_cmd_slot_tag(unsigned int cmdSlotTag);

#endif	// _UTIL_H_
