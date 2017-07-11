/*
 * =====================================================================================
 *
 *       Filename:  util.c
 *    Description:  Utility functions for NVMe
 *
 *        Version:  1.0
 *        Created:  07/07/2017 04:20:00 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Gunjae Koo (gunjae.koo@gmail.com)
 *   Organization:  
 *
 * =====================================================================================
 */

#include "util.h"

inline unsigned char parse_flags_from_nvme_io_cmd(NVME_IO_COMMAND *nvmeIOCmd)
{
	/// gunjae: For the past project, we exploited <FLAGS> field (next byte to OpCode)
	//          to indentify newly defined commands
	//          Here, <FLAGS> field is split int <PDST, RESERVED[4:0], FUSE[1:0]>
	unsigned char flags = 0;
	flags |= nvmeIOCmd->FUSE & 0x03;
	flags |= ((nvmeIOCmd->reserved0) & 0x1F) << 2;
	flags |= ((nvmeIOCmd->PSDT) & 0x01) << 7;

	return (flags);
}
