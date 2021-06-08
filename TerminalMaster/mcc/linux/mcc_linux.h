/*
 * Copyright 2013 Freescale Semiconductor
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
#ifndef __MCC_LINUX_H__
#define __MCC_LINUX_H__

#include <linux/ioctl.h>

#define MCC_DRIVER_NAME ("mcc")

// TODO is this where this should go?
#define SHARED_IRAM_START (MVF_IRAM_BASE_ADDR + 0x00040000)
#define SHARED_IRAM_SIZE (64*1024)
#define MCC_RESERVED_QUEUE_NUMBER  (0)

/* sets the load adress and subsequent writes will be to load data there */
struct mqx_boot_info_struct {
	unsigned int phys_load_addr;
	unsigned int phys_start_addr;
};

struct mcc_queue_info_struct {
	MCC_ENDPOINT endpoint;
	int current_queue_length;
};

#define MCC_READ_MODE_UNDEFINED (0)
#define MCC_READ_MODE_COPY (1)
#define MCC_READ_MODE_NOCOPY (2)
typedef unsigned int MCC_READ_MODE;

// ioctls
#define MCC_CREATE_ENDPOINT						_IOW('M', 1, MCC_ENDPOINT)
#define MCC_DESTROY_ENDPOINT						_IOW('M', 2, MCC_ENDPOINT)
#define MCC_SET_RECEIVE_ENDPOINT					_IOW('M', 3, MCC_ENDPOINT)
#define MCC_SET_SEND_ENDPOINT						_IOW('M', 4, MCC_ENDPOINT)
#define MCC_SET_TIMEOUT							_IOW('M', 5, unsigned int)
#define MCC_GET_INFO							_IOR('M', 6, MCC_INFO_STRUCT)
#define MCC_SET_READ_MODE						_IOW('M', 7, MCC_READ_MODE)
#define MCC_SET_MODE_LOAD_MQX_IMAGE					_IOW('M', 8, struct mqx_boot_info_struct)
#define MCC_BOOT_MQX_IMAGE						_IO('M', 9)
#define MCC_FREE_RECEIVE_BUFFER						_IOW('M', 10, unsigned int)
#define MCC_GET_QUEUE_INFO						_IOR('M', 11, struct mcc_queue_info_struct)
#define MCC_GET_NODE							_IOR('M', 12, MCC_NODE)
#define MCC_SET_NODE							_IOW('M', 13, MCC_NODE)
#define MCC_CHECK_ENDPOINT_EXISTS                                       _IOR('M', 14, MCC_ENDPOINT)

// for interrupts
#define MAX_MVF_CPU_TO_CPU_INTERRUPTS (4)
#define MSCM_IRCPnIR_INT0_MASK		(0x00000001)
#define MSCM_IRCPnIR	((CPU_LOGICAL_NUMBER == 0x0) ? 0x800 : 0x804)
#define CPU_LOGICAL_NUMBER			(MCC_CORE_NUMBER)
#define MSCM_IRCPGIR_CPUTL_SHIFT	(16)
#define MSCM_WRITE(data, offset)	writel(data, mscm_base + offset)
#define MSCM_IRCPGIR	0x820
#define MCC_INTERRUPT(n) ((n == 0 ? 1 : 2) << MSCM_IRCPGIR_CPUTL_SHIFT)

// for semaphores
#define MAX_SEMA4_GATES 16
#define SEMA4_GATEn_READ(gate)		readb(MVF_IO_ADDRESS(MVF_SEMA4_BASE_ADDR) + gate)
#define SEMA4_GATEn_WRITE(lock, gate) 	writeb(lock, MVF_IO_ADDRESS(MVF_SEMA4_BASE_ADDR) + gate)

#endif /* __MCC_LINUX_H__ */
