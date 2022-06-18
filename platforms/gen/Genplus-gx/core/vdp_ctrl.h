/***************************************************************************************
 *  Genesis Plus
 *  Video Display Processor (68k & Z80 CPU interface)
 *
 *  Support for SG-1000 (TMS99xx & 315-5066), Master System (315-5124 & 315-5246), Game Gear & Mega Drive VDP
 *
 *  Copyright (C) 1998-2003  Charles Mac Donald (original code)
 *  Copyright (C) 2007-2017  Eke-Eke (Genesis Plus GX)
 *
 *  Redistribution and use of this code or any derivative works are permitted
 *  provided that the following conditions are met:
 *
 *   - Redistributions may not be sold, nor may they be used in a commercial
 *     product or activity.
 *
 *   - Redistributions that are modified from the original source must include the
 *     complete source code, including the source code for all components used by a
 *     binary built from the modified sources. However, as a special exception, the
 *     source code distributed need not include anything that is normally distributed
 *     (in either source or binary form) with the major components (compiler, kernel,
 *     and so on) of the operating system on which the executable runs, unless that
 *     component itself accompanies the executable.
 *
 *   - Redistributions must reproduce the above copyright notice, this list of
 *     conditions and the following disclaimer in the documentation and/or other
 *     materials provided with the distribution.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************************/

#ifndef _VDP_H_
#define _VDP_H_

/* VDP context */
extern __thread uint8 reg[0x20];
extern __thread uint8 sat[0x400];
extern __thread uint8 vram[0x10000];
extern __thread uint8 cram[0x80];
extern __thread uint8 vsram[0x80];
extern __thread uint8 hint_pending;
extern __thread uint8 vint_pending;
extern __thread uint16 status;
extern __thread uint32 dma_length;

/* Global variables */
extern __thread uint16 ntab;
extern __thread uint16 ntbb;
extern __thread uint16 ntwb;
extern __thread uint16 satb;
extern __thread uint16 hscb;
extern __thread uint8 bg_name_dirty[0x800];
extern __thread uint16 bg_name_list[0x800];
extern __thread uint16 bg_list_index;
extern __thread uint8 hscroll_mask;
extern __thread uint8 playfield_shift;
extern __thread uint8 playfield_col_mask;
extern __thread uint16 playfield_row_mask;
extern __thread uint8 odd_frame;
extern __thread uint8 im2_flag;
extern __thread uint8 interlaced;
extern __thread uint8 vdp_pal;
extern __thread uint8 h_counter;
extern __thread uint16 v_counter;
extern __thread uint16 vc_max;
extern __thread uint16 vscroll;
extern __thread uint16 lines_per_frame;
extern __thread uint16 max_sprite_pixels;
extern __thread int32 fifo_write_cnt;
extern __thread uint32 fifo_slots;
extern __thread uint32 hvc_latch;
extern __thread const uint8 *hctab;

/* Function pointers */
extern __thread void (*vdp_68k_data_w)(unsigned int data);
extern __thread void (*vdp_z80_data_w)(unsigned int data);
extern __thread unsigned int (*vdp_68k_data_r)(void);
extern __thread unsigned int (*vdp_z80_data_r)(void);

/* Function prototypes */
extern void vdp_init(void);
extern void vdp_reset(void);
extern int vdp_context_save(uint8 *state);
extern int vdp_context_load(uint8 *state);
extern void vdp_dma_update(unsigned int cycles);
extern void vdp_68k_ctrl_w(unsigned int data);
extern void vdp_z80_ctrl_w(unsigned int data);
extern void vdp_sms_ctrl_w(unsigned int data);
extern void vdp_tms_ctrl_w(unsigned int data);
extern unsigned int vdp_68k_ctrl_r(unsigned int cycles);
extern unsigned int vdp_z80_ctrl_r(unsigned int cycles);
extern unsigned int vdp_hvc_r(unsigned int cycles);
extern void vdp_test_w(unsigned int data);
extern int vdp_68k_irq_ack(int int_level);

#endif /* _VDP_H_ */
