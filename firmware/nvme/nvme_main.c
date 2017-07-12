//////////////////////////////////////////////////////////////////////////////////
// nvme_main.c for Cosmos+ OpenSSD
// Copyright (c) 2016 Hanyang University ENC Lab.
// Contributed by Yong Ho Song <yhsong@enc.hanyang.ac.kr>
//				  Youngjin Jo <yjjo@enc.hanyang.ac.kr>
//				  Sangjin Lee <sjlee@enc.hanyang.ac.kr>
//				  Jaewook Kwak <jwkwak@enc.hanyang.ac.kr>
//				  Kibin Park <kbpark@enc.hanyang.ac.kr>
//
// This file is part of Cosmos+ OpenSSD.
//
// Cosmos+ OpenSSD is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3, or (at your option)
// any later version.
//
// Cosmos+ OpenSSD is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Cosmos+ OpenSSD; see the file COPYING.
// If not, see <http://www.gnu.org/licenses/>.
//////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////
// Company: ENC Lab. <http://enc.hanyang.ac.kr>
// Engineer: Sangjin Lee <sjlee@enc.hanyang.ac.kr>
//			 Jaewook Kwak <jwkwak@enc.hanyang.ac.kr>
//			 Kibin Park <kbpark@enc.hanyang.ac.kr>
//
// Project Name: Cosmos+ OpenSSD
// Design Name: Cosmos+ Firmware
// Module Name: NVMe Main
// File Name: nvme_main.c
//
// Version: v1.2.0
//
// Description:
//   - initializes FTL and NAND
//   - handles NVMe controller
//////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////
// Revision History:
//
// * v1.2.0
//   - header file for buffer is changed from "ia_lru_buffer.h" to "lru_buffer.h"
//   - Low level scheduler execution is allowed when there is no i/o command
//
// * v1.1.0
//   - DMA status initialization is added
//
// * v1.0.0
//   - First draft
//////////////////////////////////////////////////////////////////////////////////

#include "string.h"
#include "xil_printf.h"
#include "debug.h"
#include "io_access.h"

#include "nvme.h"
#include "host_lld.h"
#include "nvme_main.h"
#include "nvme_admin_cmd.h"
#include "nvme_io_cmd.h"

#include "../lru_buffer.h"
#include "../low_level_scheduler.h"

// gunjae added
#include "../memory_map.h"
#include "../util.h"
unsigned int g_tick;

volatile NVME_CONTEXT g_nvmeTask;

void nvme_main()
{
	unsigned int exeLlr;
	g_nvmeTask.status = NVME_TASK_IDLE;
	g_nvmeTask.cacheEn = 0;

	g_hostDmaStatus.fifoTail.autoDmaRx = 0;
	g_hostDmaStatus.autoDmaRxCnt = 0;
	g_hostDmaStatus.fifoTail.autoDmaTx = 0;
	g_hostDmaStatus.autoDmaTxCnt = 0;

	g_hostDmaAssistStatus.autoDmaRxOverFlowCnt = 0;
	g_hostDmaAssistStatus.autoDmaTxOverFlowCnt = 0;

	xil_printf("!!! Wait until FTL reset complete !!! \r\n");

	LRUBufInit();
	InitChCtlReg();
	InitDieReqQueue();
	InitDieStatusTable();
	InitNandReset();
	EmptyLowLevelQ(SUB_REQ_QUEUE);

	InitFtlMapTable();

	// gunjae: display memory map information
	GK_INIT_PRINT("mmap (MB): buffer_addr: 0x%X\r\n", BUFFER_ADDR >> 20);
	GK_INIT_PRINT("mmap (MB): spare_addr: 0x%X\r\n", SPARE_ADDR >> 20);
	GK_INIT_PRINT("mmap (MB): gc_buffer_addr: 0x%X\r\n", GC_BUFFER_ADDR >> 20);
	GK_INIT_PRINT("mmap (MB): complete_table_addr: 0x%X\r\n", COMPLETE_TABLE_ADDR >> 20);
	GK_INIT_PRINT("mmap (MB): buffer_map_addr: 0x%X\r\n", BUFFER_MAP_ADDR >> 20);
	GK_INIT_PRINT("mmap (MB): page_map_addr: 0x%X\r\n", PAGE_MAP_ADDR >> 20);
	GK_INIT_PRINT("mmap (MB): req_queue_addr: 0x%X\r\n", REQ_QUEUE_ADDR >> 20);
	GK_INIT_PRINT("mmap (MB): die_status_table_addr: 0x%X\r\n", DIE_STATUS_TABLE_ADDR >> 20);
	GK_INIT_PRINT("mmap (MB): way_priority_table_addr (last): 0x%X\r\n", WAY_PRIORITY_TABLE_ADDR >> 20);

	// gunjae: tick, this value is reset here, and accumulated by every while loop
	g_tick = 0;

	xil_printf("\r\nFTL reset complete!!! \r\n");
	xil_printf("Turn on the host PC \r\n");

	while(1)
	{
		exeLlr = 1;

		if(g_nvmeTask.status == NVME_TASK_WAIT_CC_EN)
		{
			unsigned int ccEn;
			ccEn = check_nvme_cc_en();
			if(ccEn == 1)
			{
				set_nvme_admin_queue(1, 1, 1);
				set_nvme_csts_rdy(1);
				g_nvmeTask.status = NVME_TASK_RUNNING;
				xil_printf("\r\nNVMe ready!!!\r\n");
			}
		}
		else if(g_nvmeTask.status == NVME_TASK_RUNNING)
		{
			NVME_COMMAND nvmeCmd;
			unsigned int cmdValid;

			cmdValid = get_nvme_cmd(&nvmeCmd.qID, &nvmeCmd.cmdSlotTag, &nvmeCmd.cmdSeqNum, nvmeCmd.cmdDword);

			if(cmdValid == 1)
			{
				if(nvmeCmd.qID == 0)
				{
					handle_nvme_admin_cmd(&nvmeCmd);
				}
				else
				{
					handle_nvme_io_cmd(&nvmeCmd);
					exeLlr = 0;
				}
			}
		}
		else if(g_nvmeTask.status == NVME_TASK_SHUTDOWN)
		{
			NVME_STATUS_REG nvmeReg;
			nvmeReg.dword = IO_READ32(NVME_STATUS_REG_ADDR);
			if(nvmeReg.ccShn != 0)
			{
				unsigned int qID;
				set_nvme_csts_shst(1);

				for(qID = 0; qID < 8; qID++)
				{
					set_io_cq(qID, 0, 0, 0, 0, 0, 0);
					set_io_sq(qID, 0, 0, 0, 0, 0);
				}

				set_nvme_admin_queue(0, 0, 0);
				g_nvmeTask.cacheEn = 0;
				set_nvme_csts_shst(2);
				g_nvmeTask.status = NVME_TASK_WAIT_RESET;
				xil_printf("\r\nNVMe shutdown!!!\r\n");
			}
		}
		else if(g_nvmeTask.status == NVME_TASK_WAIT_RESET)
		{
			unsigned int ccEn;
			ccEn = check_nvme_cc_en();
			if(ccEn == 0)
			{
				g_nvmeTask.cacheEn = 0;
				set_nvme_csts_shst(0);
				set_nvme_csts_rdy(0);
				g_nvmeTask.status = NVME_TASK_IDLE;
				xil_printf("\r\nNVMe disable!!!\r\n");
			}
		}
		else if(g_nvmeTask.status == NVME_TASK_RESET)
		{
			unsigned int qID;
			for(qID = 0; qID < 8; qID++)
			{
				set_io_cq(qID, 0, 0, 0, 0, 0, 0);
				set_io_sq(qID, 0, 0, 0, 0, 0);
			}
			g_nvmeTask.cacheEn = 0;
			set_nvme_admin_queue(0, 0, 0);
			set_nvme_csts_shst(0);
			set_nvme_csts_rdy(0);
			g_nvmeTask.status = NVME_TASK_IDLE;
			xil_printf("\r\nNVMe reset!!!\r\n");
		}

		if(exeLlr && reservedReq)
			ExeLowLevelReq(SUB_REQ_QUEUE);

		g_tick++;
	}
}


