/* Copyright (C) 2003-2006 Shay Green. This module is free software; you
can redistribute it and/or modify it under the terms of the GNU Lesser
General Public License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version. This
module is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details. You should have received l.a copy of the GNU Lesser General
Public License along with this module; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA */
// Nes_Emu 0.7.0. http://www.slack.net/~ant/nes-emu/

#include "Nes_Cpu.h"
#include <string.h>
#include <limits.h>
#include "blargg_endian.h"
#include "nes_cpu_io.h"
#include "blargg_source.h"

#ifdef BLARGG_ENABLE_OPTIMIZER
	#include BLARGG_ENABLE_OPTIMIZER
#endif

inline void Nes_Cpu::set_code_page( int i, uint8_t const* p )
{
	code_map [i] = p - (unsigned) i * page_size;
}

void Nes_Cpu::reset( void const* unmapped_page )
{
	r.status = 0;
	r.sp = 0;
	r.pc = 0;
	r.a = 0;
	r.x = 0;
	r.y = 0;
	
	error_count_ = 0;
	clock_count = 0;
	clock_limit = 0;
	irq_time_ = LONG_MAX / 2 + 1;
	end_time_ = LONG_MAX / 2 + 1;
	
	set_code_page( 0, low_mem );
	set_code_page( 1, low_mem );
	set_code_page( 2, low_mem );
	set_code_page( 3, low_mem );
	for ( int i = 4; i < page_count + 1; i++ )
		set_code_page( i, (uint8_t*) unmapped_page );
	
	#ifndef NDEBUG
		blargg_verify_byte_order();
	#endif
}

void Nes_Cpu::map_code( nes_addr_t start, unsigned size, const void* data )
{
	// address range must begin and end on page boundaries
	require( start % page_size == 0 );
	require( size % page_size == 0 );
	require( start + size <= 0x10000 );
	
	unsigned first_page = start / page_size;
	for ( unsigned i = size / page_size; i--; )
		set_code_page( first_page + i, (uint8_t*) data + i * page_size );
}

// Note: 'addr' is evaulated more than once in the following macros, so it
// must not contain side-effects.

//static void log_read( int opcode ) { LOG_FREQ( "read", 256, opcode ); }

#define READ_LIKELY_PPU( addr ) (NES_CPU_READ_PPU( this, (addr), (clock_count) ))
#define READ( addr )            (NES_CPU_READ( this, (addr), (clock_count) ))
#define WRITE( addr, data )     {NES_CPU_WRITE( this, (addr), (data), (clock_count) );}

#define READ_LOW( addr )        (low_mem [int (addr)])
#define WRITE_LOW( addr, data ) (void) (READ_LOW( addr ) = (data))

#define READ_PROG( addr )   (code_map [(addr) >> page_bits] [addr])
#define READ_PROG16( addr ) GET_LE16( &READ_PROG( addr ) )

#define SET_SP( v )     (l.sp = ((v) + 1) | 0x100)
#define GET_SP()        ((l.sp - 1) & 0xFF)
#define PUSH( v )       ((l.sp = (l.sp - 1) | 0x100), WRITE_LOW( l.sp, v ))

#ifdef BLARGG_ENABLE_OPTIMIZER
	#include BLARGG_ENABLE_OPTIMIZER
#endif

int Nes_Cpu::read( nes_addr_t addr )
{
	return READ( addr );
}

void Nes_Cpu::write( nes_addr_t addr, int value )
{
	WRITE( addr, value );
}


#ifndef NES_CPU_GLUE_ONLY

const unsigned char clock_table [256] = {
//  0 1 2 3 4 5 6 7 8 9 A B C D E F
	7,6,2,8,3,3,5,5,3,2,2,2,4,4,6,6,// 0
	3,5,2,8,4,4,6,6,2,4,2,7,4,4,7,7,// 1
	6,6,2,8,3,3,5,5,4,2,2,2,4,4,6,6,// 2
	3,5,2,8,4,4,6,6,2,4,2,7,4,4,7,7,// 3
	6,6,2,8,3,3,5,5,3,2,2,2,3,4,6,6,// 4
	3,5,2,8,4,4,6,6,2,4,2,7,4,4,7,7,// 5
	6,6,2,8,3,3,5,5,4,2,2,2,5,4,6,6,// 6
	3,5,2,8,4,4,6,6,2,4,2,7,4,4,7,7,// 7
	2,6,2,6,3,3,3,3,2,2,2,2,4,4,4,4,// 8
	3,6,2,6,4,4,4,4,2,5,2,5,5,5,5,5,// 9
	2,6,2,6,3,3,3,3,2,2,2,2,4,4,4,4,// A
	3,5,2,5,4,4,4,4,2,4,2,4,4,4,4,4,// B
	2,6,2,8,3,3,5,5,2,2,2,2,4,4,6,6,// C
	3,5,2,8,4,4,6,6,2,4,2,7,4,4,7,7,// D
	2,6,2,8,3,3,5,5,2,2,2,2,4,4,6,6,// E
	3,5,2,8,4,4,6,6,2,4,2,7,4,4,7,7 // F
};

#define IS_NEG (nz & 0x880)

 #define CALC_STATUS( out ) do {             \
  out = status & (st_v | st_d | st_i);    \
  out |= (c >> 8) & st_c;                 \
  if ( IS_NEG ) out |= st_n;              \
  if ( !(nz & 0xFF) ) out |= st_z;        \
 } while ( 0 )

 #define SET_STATUS( in ) do {               \
  status = in & (st_v | st_d | st_i);     \
  c = in << 8;                            \
  nz = (in << 4) & 0x800;                 \
  nz |= ~in & st_z;                       \
 } while ( 0 )

 // Macros

 #define GET_OPERAND( addr )   page [addr]
 #define GET_OPERAND16( addr ) GET_LE16( &page [addr] )

 //#define GET_OPERAND( addr )   READ_PROG( addr )
 //#define GET_OPERAND16( addr ) READ_PROG16( addr )

 #define ADD_PAGE        (l.pc++, data += 0x100 * GET_OPERAND( l.pc ));
 #define GET_ADDR()      GET_OPERAND16( l.pc )

 #define HANDLE_PAGE_CROSSING( lsb ) clock_count += (lsb) >> 8;

 #define INC_DEC_XY( reg, n ) reg = uint8_t (nz = reg + n); goto loop;

 #define IND_Y(r,c)                                             \
   temp = READ_LOW( data ) + l.y;                        \
   data = temp + 0x100 * READ_LOW( uint8_t (data + 1) );   \
   if (c) HANDLE_PAGE_CROSSING( temp );                    \
   if (!(r) || (temp & 0x100))                             \
    READ( data - ( temp & 0x100 ) );                    \


 #define IND_X                                                  \
   temp = data + l.x;                                    \
   data = 0x100 * READ_LOW( uint8_t (temp + 1) ) + READ_LOW( uint8_t (temp) ); \


 #define ARITH_ADDR_MODES( op )          \
 case op - 0x04: /* (ind,l.x) */           \
  IND_X                               \
  goto ptr##op;                       \
 case op + 0x0C: /* (ind),l.y */           \
  IND_Y(true,true)                    \
  goto ptr##op;                       \
 case op + 0x10: /* zp,X */              \
  data = uint8_t (data + l.x);          \
 case op + 0x00: /* zp */                \
  data = READ_LOW( data );            \
  goto imm##op;                       \
 case op + 0x14: /* abs,Y */             \
  data += l.y;                          \
  goto ind##op;                       \
 case op + 0x18: /* abs,X */             \
  data += l.x;                          \
 ind##op:                               \
  HANDLE_PAGE_CROSSING( data );       \
  temp = data;                    \
  ADD_PAGE                            \
  if ( temp & 0x100 )                 \
    READ( data - 0x100 );          \
  goto ptr##op;                       \
 case op + 0x08: /* abs */               \
  ADD_PAGE                            \
 ptr##op:                                \
  data = READ( data );                \
 case op + 0x04: /* imm */               \
 imm##op:                                \

 #define ARITH_ADDR_MODES_PTR( op )      \
 case op - 0x04: /* (ind,l.x) */           \
  IND_X                               \
  goto imm##op;                       \
 case op + 0x0C:                         \
  IND_Y(false,false)                  \
  goto imm##op;                       \
 case op + 0x10: /* zp,X */              \
  data = uint8_t (data + l.x);          \
  goto imm##op;                       \
 case op + 0x14: /* abs,Y */             \
  data += l.y;                          \
  goto ind##op;                       \
 case op + 0x18: /* abs,X */             \
  data += l.x;                          \
 ind##op:                               \
  temp = data;                    \
  ADD_PAGE                            \
  READ( data - ( temp & 0x100 ) );    \
  goto imm##op;                       \
 case op + 0x08: /* abs */               \
  ADD_PAGE                            \
 case op + 0x00: /* zp */                \
 imm##op:                                \

 #define BRANCH( cond )      \
  l.pc++;                   \
  if ( (cond) == false ) { clock_count--; goto loop; }\
  offset = (int8_t) data;  \
  extra_clock = (l.pc & 0xFF) + offset; \
  l.pc += offset; \
  clock_count += (extra_clock >> 8) & 1;  \
  goto loop;          \

Nes_Cpu::result_t Nes_Cpu::run( nes_time_t end )
{
	set_end_time_( end );
	clock_count = 0;
	
	volatile result_t result = result_cycles;
	
	// registers
	registers_t l;

	l.pc = r.pc;
	SET_SP( r.sp );
 l.a = r.a;
	l.x = r.x;
	l.y = r.y;
	
	// temporary values
 uint16_t temp;
 uint16_t addr;
 uint16_t msb;
 int8_t offset;
 int16_t extra_clock;
 uint8_t carry;
 int16_t ov;

	// status flags
	const uint8_t st_n = 0x80;
	const uint8_t st_v = 0x40;
	const uint8_t st_r = 0x20;
	const uint8_t st_b = 0x10;
	const uint8_t st_d = 0x08;
	const uint8_t st_i = 0x04;
	const uint8_t st_z = 0x02;
	const uint8_t st_c = 0x01;

	int status;
	uint16_t c;  // carry set if (c & 0x100) != 0
	uint16_t nz; // Z set if (nz & 0xFF) == 0, N set if (nz & 0x880) != 0
	temp = r.status;
	SET_STATUS( temp );

	loop:

	uint8_t const* page = code_map [l.pc >> page_bits];
	uint8_t opcode = page [l.pc];
	l.pc++;

	if ( clock_count >= clock_limit )	goto stop;
	clock_count += clock_table [opcode];
	uint16_t data;
	data = page [l.pc];

	switch ( opcode )
 {
  // Often-Used
  case 0xB5: // LDA zp,l.x
   data = uint8_t (data + l.x);

  case 0xA5: // LDA zp
   l.a = nz = READ_LOW( data );
   l.pc++;
   goto loop;

  case 0xD0: // BNE
   BRANCH( (uint8_t) nz );

  case 0x20:  // JSR
   temp = l.pc + 1;
   l.pc = GET_OPERAND16( l.pc );
   WRITE_LOW( 0x100 | (l.sp - 1), temp >> 8 );
   l.sp = (l.sp - 2) | 0x100;
   WRITE_LOW( l.sp, temp );
   goto loop;


  case 0x4C: // JMP abs
   l.pc = GET_OPERAND16( l.pc );
   goto loop;

  case 0xE8: INC_DEC_XY( l.x, 1 )  // INX

  case 0x10: // BPL
   BRANCH( !IS_NEG )

  ARITH_ADDR_MODES( 0xC5 ) // CMP
   nz = l.a - data;
   l.pc++;
   c = ~nz;
   nz &= 0xFF;
   goto loop;

  case 0x30: // BMI
   BRANCH( IS_NEG )

  case 0xF0: // BEQ
   BRANCH( !(uint8_t) nz );

  case 0x95: // STA zp,l.x
   data = uint8_t (data + l.x);
  case 0x85: // STA zp
   l.pc++;
   WRITE_LOW( data, l.a );
   goto loop;

  case 0xC8: INC_DEC_XY( l.y, 1 )  // INY

  case 0xA8: // TAY
   l.y = l.a;
  case 0x98: // TYA
   l.a = nz = l.y;
   goto loop;

  case 0xAD:// LDA abs
   addr = GET_ADDR();
   l.pc += 2;
   l.a = nz = READ_LIKELY_PPU( addr );
   goto loop;

  case 0x60: // RTS
   l.pc = 1 + READ_LOW( l.sp );
   l.pc += READ_LOW( 0x100 | (l.sp - 0xFF) ) * 0x100;
   l.sp = (l.sp - 0xFE) | 0x100;
   goto loop;

  case 0x99: // STA abs,Y
   data += l.y;
   goto sta_ind_common;

  case 0x9D: // STA abs,X
   data += l.x;
  sta_ind_common:
   temp = data;
   ADD_PAGE
   READ( data - ( temp & 0x100 ) );
   goto sta_ptr;

  case 0x8D: // STA abs
   ADD_PAGE
  sta_ptr:
   l.pc++;
   WRITE( data, l.a );
   goto loop;

  case 0xA9: // LDA #imm
   l.pc++;
   l.a = data;
   nz = data;
   goto loop;

  case 0xB9:// LDA abs,Y
   data += l.y;
   data -= l.x;

  case 0xBD:// LDA abs,X
   l.pc++;
   msb = GET_OPERAND( l.pc );
   data += l.x;
   // indexed common
   l.pc++;
   HANDLE_PAGE_CROSSING( data );
   temp = data;
   data += msb * 0x100;
   l.a = nz = READ_PROG( BOOST::uint16_t( data ) );
   if ( (unsigned) (data - 0x2000) >= 0x6000 )
    goto loop;
   if ( temp & 0x100 )
    READ( data - 0x100 );
   l.a = nz = READ( data );
   goto loop;


  case 0xB1:// LDA (ind),Y
   msb = READ_LOW( (uint8_t) (data + 1) );
   data = READ_LOW( data ) + l.y;
   // indexed common
   l.pc++;
   HANDLE_PAGE_CROSSING( data );
   temp = data;
   data += msb * 0x100;
   l.a = nz = READ_PROG( BOOST::uint16_t( data ) );
   if ( (unsigned) (data - 0x2000) >= 0x6000 )
    goto loop;
   if ( temp & 0x100 )
    READ( data - 0x100 );
   l.a = nz = READ( data );
   goto loop;


  case 0xA1: // LDA (ind,X)
   IND_X
   l.a = nz = READ( data );
   l.pc++;
   goto loop;


 // Branch

  case 0x50: // BVC
   BRANCH( !(status & st_v) )

  case 0x70: // BVS
   BRANCH( status & st_v )

  case 0xB0: // BCS
   BRANCH( c & 0x100 )

  case 0x90: // BCC
   BRANCH( !(c & 0x100) )

 // Load/store

  case 0x94: // STY zp,l.x
   data = uint8_t (data + l.x);
  case 0x84: // STY zp
   l.pc++;
   WRITE_LOW( data, l.y );
   goto loop;

  case 0x96: // STX zp,l.y
   data = uint8_t (data + l.y);
  case 0x86: // STX zp
   l.pc++;
   WRITE_LOW( data, l.x );
   goto loop;

  case 0xB6: // LDX zp,l.y
   data = uint8_t (data + l.y);
  case 0xA6: // LDX zp
   data = READ_LOW( data );
  case 0xA2: // LDX #imm
   l.pc++;
   l.x = data;
   nz = data;
   goto loop;

  case 0xB4: // LDY zp,l.x
   data = uint8_t (data + l.x);
  case 0xA4: // LDY zp
   data = READ_LOW( data );
  case 0xA0: // LDY #imm
   l.pc++;
   l.y = data;
   nz = data;
   goto loop;

  case 0x91: // STA (ind),Y
   IND_Y(false,false)
   goto sta_ptr;

  case 0x81: // STA (ind,X)
   IND_X
   goto sta_ptr;

  case 0xBC: // LDY abs,X
   data += l.x;
   HANDLE_PAGE_CROSSING( data );
  case 0xAC:// LDY abs
   l.pc++;
   addr = data + 0x100 * GET_OPERAND( l.pc );
   if ( data & 0x100 )
    READ( addr - 0x100 );
   l.pc++;
   l.y = nz = READ( addr );
   goto loop;


  case 0xBE: // LDX abs,l.y
   data += l.y;
   HANDLE_PAGE_CROSSING( data );
  case 0xAE:// LDX abs
   l.pc++;
   addr = data + 0x100 * GET_OPERAND( l.pc );
   l.pc++;
   if ( data & 0x100 )
    READ( addr - 0x100 );
   l.x = nz = READ( addr );
   goto loop;

   case 0x8C: // STY abs
   temp = l.y;
   goto store_abs;

  case 0x8E: // STX abs
   temp = l.x;
  store_abs:
   addr = GET_ADDR();
   WRITE( addr, temp );
   l.pc += 2;
   goto loop;

 // Compare

  case 0xEC:// CPX abs
   addr = GET_ADDR();
   l.pc++;
   data = READ( addr );
   goto cpx_data;

  case 0xE4: // CPX zp
   data = READ_LOW( data );
  case 0xE0: // CPX #imm
  cpx_data:
   nz = l.x - data;
   l.pc++;
   c = ~nz;
   nz &= 0xFF;
   goto loop;

  case 0xCC:// CPY abs
   addr = GET_ADDR();
   l.pc++;
   data = READ( addr );
   goto cpy_data;

  case 0xC4: // CPY zp
   data = READ_LOW( data );
  case 0xC0: // CPY #imm
  cpy_data:
   nz = l.y - data;
   l.pc++;
   c = ~nz;
   nz &= 0xFF;
   goto loop;

 // Logical

  ARITH_ADDR_MODES( 0x25 ) // AND
   nz = (l.a &= data);
   l.pc++;
   goto loop;

  ARITH_ADDR_MODES( 0x45 ) // EOR
   nz = (l.a ^= data);
   l.pc++;
   goto loop;

  ARITH_ADDR_MODES( 0x05 ) // ORA
   nz = (l.a |= data);
   l.pc++;
   goto loop;

  case 0x2C:// BIT abs
   addr = GET_ADDR();
   l.pc += 2;
   status &= ~st_v;
   nz = READ_LIKELY_PPU( addr );
   status |= nz & st_v;
   if ( l.a & nz )
    goto loop;
   // result must be zero, even if N bit is set
   nz = nz << 4 & 0x800;
   goto loop;

  case 0x24: // BIT zp
   nz = READ_LOW( data );
   l.pc++;
   status &= ~st_v;
   status |= nz & st_v;
   if ( l.a & nz )
    goto loop;
   // result must be zero, even if N bit is set
   nz = nz << 4 & 0x800;
   goto loop;

 // Add/subtract

  ARITH_ADDR_MODES( 0xE5 ) // SBC
  case 0xEB: // unofficial equivalent
   data ^= 0xFF;
   goto adc_imm;

  ARITH_ADDR_MODES( 0x65 ) // ADC
  adc_imm:
   carry = (c >> 8) & 1;
   ov = (l.a ^ 0x80) + carry + (int8_t) data; // sign-extend
   status &= ~st_v;
   status |= (ov >> 2) & 0x40;
   c = nz = l.a + data + carry;
   l.pc++;
   l.a = (uint8_t) nz;
   goto loop;

 // Shift/rotate

  case 0x4A: // LSR A
  lsr_a:
   c = 0;
  case 0x6A: // ROR A
   nz = (c >> 1) & 0x80; // could use bit insert macro here
   c = l.a << 8;
   nz |= l.a >> 1;
   l.a = nz;
   goto loop;

  case 0x0A: // ASL A
   nz = l.a << 1;
   c = nz;
   l.a = (uint8_t) nz;
   goto loop;

  case 0x2A: // ROL A
   nz = l.a << 1;
   temp = (c >> 8) & 1;
   c = nz;
   nz |= temp;
   l.a = (uint8_t) nz;
   goto loop;

  case 0x3E: // ROL abs,X
   data += l.x;
   goto rol_abs;

  case 0x1E: // ASL abs,X
   data += l.x;
  case 0x0E: // ASL abs
   c = 0;
  case 0x2E: // ROL abs
  rol_abs:
   temp = data;
   ADD_PAGE
   if ( opcode == 0x1E || opcode == 0x3E ) READ( data - ( temp & 0x100 ) );
   WRITE( data, temp = READ( data ) );
   nz = (c >> 8) & 1;
   nz |= (c = temp << 1);
     rotate_common:
   l.pc++;
   WRITE( data, (uint8_t) nz );
   goto loop;

  case 0x7E: // ROR abs,X
   data += l.x;
   goto ror_abs;

  case 0x5E: // LSR abs,X
   data += l.x;
  case 0x4E: // LSR abs
   c = 0;
  case 0x6E: // ROR abs
  ror_abs:
   temp = data;
   ADD_PAGE
   if ( opcode == 0x5E || opcode == 0x7E ) READ( data - ( temp & 0x100 ) );
   WRITE( data, temp = READ( data ) );
   nz = ((c >> 1) & 0x80) | (temp >> 1);
   c = temp << 8;
   goto rotate_common;

  case 0x76: // ROR zp,l.x
   data = uint8_t (data + l.x);
   goto ror_zp;

  case 0x56: // LSR zp,l.x
   data = uint8_t (data + l.x);
  case 0x46: // LSR zp
   c = 0;
  case 0x66: // ROR zp
  ror_zp:
   temp = READ_LOW( data );
   nz = ((c >> 1) & 0x80) | (temp >> 1);
   c = temp << 8;
   goto write_nz_zp;

  case 0x36: // ROL zp,l.x
   data = uint8_t (data + l.x);
   goto rol_zp;

  case 0x16: // ASL zp,l.x
   data = uint8_t (data + l.x);
  case 0x06: // ASL zp
   c = 0;
  case 0x26: // ROL zp
  rol_zp:
   nz = (c >> 8) & 1;
   nz |= (c = READ_LOW( data ) << 1);
   goto write_nz_zp;

 // Increment/decrement

  case 0xCA: INC_DEC_XY( l.x, -1 ) // DEX

  case 0x88: INC_DEC_XY( l.y, -1 ) // DEY

  case 0xF6: // INC zp,l.x
   data = uint8_t (data + l.x);
  case 0xE6: // INC zp
   nz = 1;
   goto add_nz_zp;

  case 0xD6: // DEC zp,l.x
   data = uint8_t (data + l.x);
  case 0xC6: // DEC zp
   nz = -1;
  add_nz_zp:
   nz += READ_LOW( data );
  write_nz_zp:
   l.pc++;
   WRITE_LOW( data, nz );
   goto loop;

  case 0xFE: // INC abs,l.x
   temp = data + l.x;
   data = l.x + GET_ADDR();
   READ( data - ( temp & 0x100 ) );
   goto inc_ptr;

  case 0xEE: // INC abs
   data = GET_ADDR();
  inc_ptr:
   nz = 1;
   goto inc_common;

  case 0xDE: // DEC abs,l.x
   temp = data + l.x;
   data = l.x + GET_ADDR();
   READ( data - ( temp & 0x100 ) );
   goto dec_ptr;

  case 0xCE: // DEC abs
   data = GET_ADDR();
  dec_ptr:
   nz = -1;
  inc_common:
   WRITE( data, temp = READ( data ) );
   nz += temp;
   l.pc += 2;
   WRITE( data, (uint8_t) nz );
   goto loop;

 // Transfer

  case 0xAA: // TAX
   l.x = l.a;
  case 0x8A: // TXA
   l.a = nz = l.x;
   goto loop;

  case 0x9A: // TXS
   SET_SP( l.x ); // verified (no flag change)
   goto loop;

  case 0xBA: // TSX
   l.x = nz = GET_SP();
   goto loop;

 // Stack

  case 0x48: // PHA
   PUSH( l.a ); // verified
   goto loop;

  case 0x68: // PLA
   l.a = nz = READ_LOW( l.sp );
   l.sp = (l.sp - 0xFF) | 0x100;
   goto loop;

  case 0x40: // RTI
    temp = READ_LOW( l.sp );
    l.pc   = READ_LOW( 0x100 | (l.sp - 0xFF) );
    l.pc  |= READ_LOW( 0x100 | (l.sp - 0xFE) ) * 0x100;
    l.sp = (l.sp - 0xFD) | 0x100;
    data = status;
    SET_STATUS( temp );

   if ( !((data ^ status) & st_i) )
    goto loop; // I flag didn't change
  i_flag_changed:
   //dprintf( "%6d %s\n", time(), (status & st_i ? "SEI" : "CLI") );
   this->r.status = status; // update externally-visible I flag
   // update clock_limit based on modified I flag
   clock_limit = end_time_;
   if ( end_time_ <= irq_time_ )
    goto loop;
   if ( status & st_i )
    goto loop;
   clock_limit = irq_time_;
   goto loop;

  case 0x28:// PLP
   temp = READ_LOW( l.sp );
   l.sp = (l.sp - 0xFF) | 0x100;
   data = status;
   SET_STATUS( temp );
   if ( !((data ^ status) & st_i) )
    goto loop; // I flag didn't change
   if ( !(status & st_i) )
    goto handle_cli;
   goto handle_sei;

  case 0x08: // PHP
   CALC_STATUS( temp );
   PUSH( temp | st_b | st_r );
   goto loop;

  case 0x6C: // JMP (ind)
   data = GET_ADDR();
   l.pc = READ( data );
   l.pc |= READ( (data & 0xFF00) | ((data + 1) & 0xFF) ) << 8;
   goto loop;

  case 0x00: // BRK
   l.pc++;
   WRITE_LOW( 0x100 | (l.sp - 1), l.pc >> 8 );
   WRITE_LOW( 0x100 | (l.sp - 2), l.pc );
   CALC_STATUS( temp );
   l.sp = (l.sp - 3) | 0x100;
   WRITE_LOW( l.sp, temp | st_b | st_r );
   l.pc = GET_LE16( &code_map [0xFFFE >> page_bits] [0xFFFE] );
   status |= st_i;
   goto i_flag_changed;

 // Flags

  case 0x38: // SEC
   c = ~0;
   goto loop;

  case 0x18: // CLC
   c = 0;
   goto loop;

  case 0xB8: // CLV
   status &= ~st_v;
   goto loop;

  case 0xD8: // CLD
   status &= ~st_d;
   goto loop;

  case 0xF8: // SED
   status |= st_d;
   goto loop;

  case 0x58: // CLI
   if ( !(status & st_i) )
    goto loop;
   status &= ~st_i;
  handle_cli:
   //dprintf( "%6d CLI\n", time() );
   this->r.status = status; // update externally-visible I flag
   if ( clock_count < end_time_ )
   {
    if ( end_time_ <= irq_time_ )
     goto loop; // irq is later
    if ( clock_count >= irq_time_ )
     irq_time_ = clock_count + 1; // delay IRQ until after next instruction
    clock_limit = irq_time_;
    goto loop;
   }

   // execution is stopping now, so delayed CLI must be handled by caller
   result = result_cli;
   goto end;

  case 0x78: // SEI
   if ( status & st_i )
    goto loop;
   status |= st_i;
  handle_sei:
   //dprintf( "%6d SEI\n", time() );
   this->r.status = status; // update externally-visible I flag
   clock_limit = end_time_;
   if ( clock_count < irq_time_ )
    goto loop;
   result = result_sei; // IRQ will occur now, even though I flag is set
   goto end;

 // Unofficial
  case 0x1C: case 0x3C: case 0x5C: case 0x7C: case 0xDC: case 0xFC: // SKW
   data += l.x;
   HANDLE_PAGE_CROSSING( data );
   addr = GET_ADDR() + l.x;
   if ( data & 0x100 )
    READ( addr - 0x100 );
   READ( addr );

  case 0x0C: // SKW
   l.pc++;
  case 0x74: case 0x04: case 0x14: case 0x34: case 0x44: case 0x54: case 0x64: // SKB
  case 0x80: case 0x82: case 0x89: case 0xC2: case 0xD4: case 0xE2: case 0xF4:
   l.pc++;
  case 0xEA: case 0x1A: case 0x3A: case 0x5A: case 0x7A: case 0xDA: case 0xFA: // NOP
   goto loop;

  ARITH_ADDR_MODES_PTR( 0xC7 ) // DCP
   WRITE( data, nz = READ( data ) );
   nz = uint8_t( nz - 1 );
   WRITE( data, nz );
   l.pc++;
   nz = l.a - nz;
   c = ~nz;
   nz &= 0xFF;
   goto loop;

  ARITH_ADDR_MODES_PTR( 0xE7 ) // ISC
   WRITE( data, nz = READ( data ) );
   nz = uint8_t( nz + 1 );
   WRITE( data, nz );
   data = nz ^ 0xFF;
   goto adc_imm;

  ARITH_ADDR_MODES_PTR( 0x27 )  // RLA
   WRITE( data, nz = READ( data ) );
   temp = c;
   c = nz << 1;
   nz = uint8_t( c ) | ( ( temp >> 8 ) & 0x01 );
   WRITE( data, nz );
   l.pc++;
   nz = l.a &= nz;
   goto loop;

  ARITH_ADDR_MODES_PTR( 0x67 ) // RRA
   WRITE( data, temp = READ( data ) );
   nz = ((c >> 1) & 0x80) | (temp >> 1);
   WRITE( data, nz );
   data = nz;
   c = temp << 8;
   goto adc_imm;

  ARITH_ADDR_MODES_PTR( 0x07 ) // SLO
   WRITE( data, nz = READ( data ) );
   c = nz << 1;
   nz = uint8_t( c );
   WRITE( data, nz );
   nz = (l.a |= nz);
   l.pc++;
   goto loop;

  ARITH_ADDR_MODES_PTR( 0x47 ) // SRE
   WRITE( data, nz = READ( data ) );
   c = nz << 8;
   nz >>= 1;
   WRITE( data, nz );
   nz = l.a ^= nz;
   l.pc++;
   goto loop;

  case 0x4B: // ALR
   nz = (l.a &= data);
   l.pc++;
   goto lsr_a;

  case 0x0B: // ANC
  case 0x2B:
   nz = l.a &= data;
   c = l.a << 1;
   l.pc++;
   goto loop;

  case 0x6B: // ARR
   nz = l.a = uint8_t( ( ( data & l.a ) >> 1 ) | ( ( c >> 1 ) & 0x80 ) );
   c = l.a << 2;
   status = ( status & ~st_v ) | ( ( l.a ^ l.a << 1 ) & st_v );
   l.pc++;
   goto loop;

  case 0xAB: // LXA
   l.a = data;
   l.x = data;
   nz = data;
   l.pc++;
   goto loop;

  case 0xA3: // LAX
   IND_X
   goto lax_ptr;

  case 0xB3:
   IND_Y(true,true)
   goto lax_ptr;

  case 0xB7:
   data = uint8_t (data + l.y);

  case 0xA7:
   data = READ_LOW( data );
   goto lax_imm;

  case 0xBF:
   data += l.y;
   HANDLE_PAGE_CROSSING( data );
   temp = data;
   ADD_PAGE;
   if ( temp & 0x100 )
    READ( data - 0x100 );
   goto lax_ptr;

  case 0xAF:
   ADD_PAGE

  lax_ptr:
   data = READ( data );
  lax_imm:
   nz = l.x = l.a = data;
   l.pc++;
   goto loop;

  case 0x83: // SAX
   IND_X
   goto sax_imm;

  case 0x97:
   data = uint8_t (data + l.y);
   goto sax_imm;

  case 0x8F:
   ADD_PAGE

  case 0x87:
  sax_imm:
   WRITE( data, l.a & l.x );
   l.pc++;
   goto loop;

  case 0xCB: // SBX
   data = ( l.a & l.x ) - data;
   c = ( data <= 0xFF ) ? 0x100 : 0;
   nz = l.x = uint8_t( data );
   l.pc++;
   goto loop;

  case 0x93: // SHA (ind),Y
   IND_Y(false,false)
   l.pc++;
   WRITE( data, uint8_t( l.a & l.x & ( ( data >> 8 ) + 1 ) ) );
   goto loop;

  case 0x9F: // SHA abs,Y
   data += l.y;
   temp = data;
   ADD_PAGE
   READ( data - ( temp & 0x100 ) );
   l.pc++;
   WRITE( data, uint8_t( l.a & l.x & ( ( data >> 8 ) + 1 ) ) );
   goto loop;

  case 0x9E: // SHX abs,Y
   data += l.y;
   temp = data;
   ADD_PAGE
   READ( data - ( temp & 0x100 ) );
   l.pc++;
   if ( !( temp & 0x100 ) )
    WRITE( data, uint8_t( l.x & ( ( data >> 8 ) + 1 ) ) );
   goto loop;

  case 0x9C: // SHY abs,X
   data += l.x;
   temp = data;
   ADD_PAGE
   READ( data - ( temp & 0x100 ) );
   l.pc++;
   if ( !( temp & 0x100) )
    WRITE( data, uint8_t( l.y & ( ( data >> 8 ) + 1 ) ) );
   goto loop;

  case 0x9B: // SHS abs,Y
   data += l.y;
   temp = data;
   ADD_PAGE
   READ( data - ( temp & 0x100 ) );
   l.pc++;
   SET_SP( l.a & l.x );
   WRITE( data, uint8_t( l.a & l.x & ( ( data >> 8 ) + 1 ) ) );
   goto loop;

  case 0xBB: // LAS abs,Y
   data += l.y;
   HANDLE_PAGE_CROSSING( data );
   temp = data;
   ADD_PAGE
   if ( temp & 0x100 )
    READ( data - 0x100 );
   l.pc++;
   l.a = GET_SP();
   l.x = l.a &= READ( data );
   SET_SP( l.a );
   goto loop;

 // Unimplemented

  case page_wrap_opcode: // HLT
   if ( l.pc > 0x10000 )
   {
    // handle wrap-around (assumes caller has put page of HLT at 0x10000)
    l.pc = (l.pc - 1) & 0xFFFF;
    clock_count -= 2;
    goto loop;
   }
   // fall through
  default:
   // skip over proper number of bytes
   static unsigned char const row [8] = { 0x95, 0x95, 0x95, 0xd5, 0x95, 0x95, 0xd5, 0xf5 };
   int len = row [opcode >> 2 & 7] >> (opcode << 1 & 6) & 3;
   if ( opcode == 0x9C )
    len = 3;
   l.pc += len - 1;
   error_count_++;
   goto loop;

   //result = result_badop; // TODO: re-enable
   goto stop;
	}

stop:
	l.pc--;
end:
	
	{
		int temp;
		CALC_STATUS( temp );
		r.status = temp;
	}
	
	this->clock_count = clock_count;
	r.pc = l.pc;
	r.sp = GET_SP();
	r.a = l.a;
	r.x = l.x;
	r.y = l.y;
	irq_time_ = LONG_MAX / 2 + 1;
	
	return result;
}

#endif

