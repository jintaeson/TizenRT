/************************************************************************************
 * arch/arm/src/stm32l4/stm32l4_usbhost.h
 *
 *   Copyright (C) 2012, 2014 Gregory Nutt. All rights reserved.
 *   Author: Gregory Nutt <gnutt@nuttx.org>
 *           dev@ziggurat29.com
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name NuttX nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ************************************************************************************/

#ifndef __ARCH_ARM_SRC_STM32L4_STM32L4_USBHOST_H
#define __ARCH_ARM_SRC_STM32L4_STM32L4_USBHOST_H

/************************************************************************************
 * Included Files
 ************************************************************************************/

#include <tinyara/config.h>
#include <tinyara/usb/usbhost.h>
#include <tinyara/usb/usbhost_trace.h>
#include <stdint.h>

#include "chip.h"

#if defined(CONFIG_STM32L4_OTGFS) && defined(CONFIG_USBHOST)

#if defined(CONFIG_STM32L4_STM32L4X5)
#  include "chip/stm32l4x5xx_otgfs.h"
#elif defined(CONFIG_STM32L4_STM32L4X6)
#  include "chip/stm32l4x6xx_otgfs.h"
#else
#  error "Unsupported STM32L4 chip"
#endif

/************************************************************************************
 * Public Types
 ************************************************************************************/

#ifdef HAVE_USBHOST_TRACE
enum usbhost_trace1codes_e
{
  __TRACE1_BASEVALUE = 0,           /* This will force the first value to be 1 */

  OTGFS_TRACE1_DEVDISCONN,           /* OTGFS ERROR: Host Port Device disconnected */
  OTGFS_TRACE1_IRQATTACH,            /* OTGFS ERROR: Failed to attach IRQ */
  OTGFS_TRACE1_TRNSFRFAILED,         /* OTGFS ERROR: Host Port Transfer Failed */
  OTGFS_TRACE1_SENDSETUP,            /* OTGFS ERROR: sendsetup() failed with: */
  OTGFS_TRACE1_SENDDATA,             /* OTGFS ERROR: senddata() failed with: */
  OTGFS_TRACE1_RECVDATA,             /* OTGFS ERROR: recvdata() failed with: */

#ifdef HAVE_USBHOST_TRACE_VERBOSE

  OTGFS_VTRACE1_CONNECTED,           /* OTGFS Host Port connected */
  OTGFS_VTRACE1_DISCONNECTED,        /* OTGFS Host Port disconnected */
  OTGFS_VTRACE1_GINT,                /* OTGFS Handling Interrupt. Entry Point */
  OTGFS_VTRACE1_GINT_SOF,            /* OTGFS Handle the start of frame interrupt */
  OTGFS_VTRACE1_GINT_RXFLVL,         /* OTGFS Handle the RxFIFO non-empty interrupt */
  OTGFS_VTRACE1_GINT_NPTXFE,         /* OTGFS Handle the non-periodic TxFIFO empty interrupt */
  OTGFS_VTRACE1_GINT_PTXFE,          /* OTGFS Handle the periodic TxFIFO empty interrupt */
  OTGFS_VTRACE1_GINT_HC,             /* OTGFS Handle the host channels interrupt */
  OTGFS_VTRACE1_GINT_HPRT,           /* OTGFS Handle the host port interrupt */
  OTGFS_VTRACE1_GINT_HPRT_POCCHNG,   /* OTGFS  HPRT: Port Over-Current Change*/
  OTGFS_VTRACE1_GINT_HPRT_PCDET,     /* OTGFS  HPRT: Port Connect Detect */
  OTGFS_VTRACE1_GINT_HPRT_PENCHNG,   /* OTGFS  HPRT: Port Enable Changed */
  OTGFS_VTRACE1_GINT_HPRT_LSDEV,     /* OTGFS  HPRT: Low Speed Device Connected */
  OTGFS_VTRACE1_GINT_HPRT_FSDEV,     /* OTGFS  HPRT: Full Speed Device Connected */
  OTGFS_VTRACE1_GINT_HPRT_LSFSSW,    /* OTGFS  HPRT: Host Switch: LS -> FS */
  OTGFS_VTRACE1_GINT_HPRT_FSLSSW,    /* OTGFS  HPRT: Host Switch: FS -> LS */
  OTGFS_VTRACE1_GINT_DISC,           /* OTGFS Handle the disconnect detected interrupt */
  OTGFS_VTRACE1_GINT_IPXFR,          /* OTGFS Handle the incomplete periodic transfer */

#endif

  __TRACE1_NSTRINGS,                 /* Separates the format 1 from the format 2 strings */

  OTGFS_TRACE2_CLIP,                 /* OTGFS CLIP: chidx:  buflen: */

#ifdef HAVE_USBHOST_TRACE_VERBOSE

  OTGFS_VTRACE2_CHANWAKEUP_IN,       /* OTGFS IN Channel wake up with result */
  OTGFS_VTRACE2_CHANWAKEUP_OUT,      /* OTGFS OUT Channel wake up with result */
  OTGFS_VTRACE2_CTRLIN,              /* OTGFS CTRLIN */
  OTGFS_VTRACE2_CTRLOUT,             /* OTGFS CTRLOUT */
  OTGFS_VTRACE2_INTRIN,              /* OTGFS INTRIN */
  OTGFS_VTRACE2_INTROUT,             /* OTGFS INTROUT */
  OTGFS_VTRACE2_BULKIN,              /* OTGFS BULKIN */
  OTGFS_VTRACE2_BULKOUT,             /* OTGFS BULKOUT */
  OTGFS_VTRACE2_ISOCIN,              /* OTGFS ISOCIN */
  OTGFS_VTRACE2_ISOCOUT,             /* OTGFS ISOCOUT */
  OTGFS_VTRACE2_STARTTRANSFER,       /* OTGFS EP buflen */
  OTGFS_VTRACE2_CHANCONF_CTRL_IN,
  OTGFS_VTRACE2_CHANCONF_CTRL_OUT,
  OTGFS_VTRACE2_CHANCONF_INTR_IN,
  OTGFS_VTRACE2_CHANCONF_INTR_OUT,
  OTGFS_VTRACE2_CHANCONF_BULK_IN,
  OTGFS_VTRACE2_CHANCONF_BULK_OUT,
  OTGFS_VTRACE2_CHANCONF_ISOC_IN,
  OTGFS_VTRACE2_CHANCONF_ISOC_OUT,
  OTGFS_VTRACE2_CHANHALT,            /* Channel halted. chidx: , reason:  */

#endif

  __TRACE2_NSTRINGS                  /* Total number of enumeration values */
};

#  define TRACE1_FIRST     ((int)__TRACE1_BASEVALUE + 1)
#  define TRACE1_INDEX(id) ((int)(id) - TRACE1_FIRST)
#  define TRACE1_NSTRINGS  TRACE1_INDEX(__TRACE1_NSTRINGS)

#  define TRACE2_FIRST     ((int)__TRACE1_NSTRINGS + 1)
#  define TRACE2_INDEX(id) ((int)(id) - TRACE2_FIRST)
#  define TRACE2_NSTRINGS  TRACE2_INDEX(__TRACE2_NSTRINGS)

#endif

/************************************************************************************
 * Public Functions
 ************************************************************************************/
/*
 * STM32L4 USB OTG FS Host Driver Support
 *
 * Pre-requisites
 *
 *  CONFIG_USBHOST        - Enable general USB host support
 *  CONFIG_STM32L4_OTGFS  - Enable the STM32 USB OTG FS block
 *  CONFIG_STM32L4_SYSCFG - Needed
 *
 * Options:
 *
 *  CONFIG_STM32L4_OTGFS_RXFIFO_SIZE - Size of the RX FIFO in 32-bit words.
 *    Default 128 (512 bytes)
 *  CONFIG_STM32L4_OTGFS_NPTXFIFO_SIZE - Size of the non-periodic Tx FIFO
 *    in 32-bit words.  Default 96 (384 bytes)
 *  CONFIG_STM32L4_OTGFS_PTXFIFO_SIZE - Size of the periodic Tx FIFO in 32-bit
 *    words.  Default 96 (384 bytes)
 *  CONFIG_STM32L4_OTGFS_SOFINTR - Enable SOF interrupts.  Why would you ever
 *    want to do that?
 *
 *  CONFIG_STM32L4_USBHOST_REGDEBUG - Enable very low-level register access
 *    debug.  Depends on CONFIG_DEBUG.
 */

/************************************************************************************
 * Public Functions
 ************************************************************************************/

#ifndef __ASSEMBLY__

#undef EXTERN
#if defined(__cplusplus)
#define EXTERN extern "C"
extern "C"
{
#else
#define EXTERN extern
#endif

/***********************************************************************************
 * Name: stm32l4_usbhost_vbusdrive
 *
 * Description:
 *   Enable/disable driving of VBUS 5V output.  This function must be provided be
 *   each platform that implements the STM32L4 OTG FS host interface
 *
 *   "On-chip 5 V VBUS generation is not supported. For this reason, a charge pump
 *    or, if 5 V are available on the application board, a basic power switch, must
 *    be added externally to drive the 5 V VBUS line. The external charge pump can
 *    be driven by any GPIO output. When the application decides to power on VBUS
 *    using the chosen GPIO, it must also set the port power bit in the host port
 *    control and status register (PPWR bit in OTG_FS_HPRT).
 *
 *   "The application uses this field to control power to this port, and the core
 *    clears this bit on an over current condition."
 *
 * Input Parameters:
 *   iface - For future growth to handle multiple USB host interface.  Should be zero.
 *   enable - true: enable VBUS power; false: disable VBUS power
 *
 * Returned Value:
 *   None
 *
 ***********************************************************************************/

void stm32l4_usbhost_vbusdrive(int iface, bool enable);

#undef EXTERN
#if defined(__cplusplus)
}
#endif

#endif /* __ASSEMBLY__ */
#endif /* CONFIG_STM32L4_OTGFS && CONFIG_USBHOST */
#endif /* __ARCH_ARM_SRC_STM32L4_STM32L4_USBHOST_H */
