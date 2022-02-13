
// Nes_Snd_Emu 0.1.7. http://www.slack.net/~ant/

#include "Nes_Apu.h"

/* Copyright (C) 2003-2006 Shay Green. This module is free software; you
can redistribute it and/or modify it under the terms of the GNU Lesser
General Public License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version. This
module is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details. You should have received a copy of the GNU Lesser General
Public License along with this module; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA */

#include "blargg_source.h"

int const amp_range = 15;

Nes_Apu::Nes_Apu() :
	square1( &square_synth ),
	square2( &square_synth )
{
	dmc.apu = this;
	dmc.prg_reader = NULL;
	irq_notifier_ = NULL;

	oscs [0] = &square1;
	oscs [1] = &square2;
	oscs [2] = &triangle;
	oscs [3] = &noise;
	oscs [4] = &dmc;

	output( NULL );
	volume( 1.0 );
	reset( false );
}

Nes_Apu::~Nes_Apu()
{
}

void Nes_Apu::treble_eq( const blip_eq_t& eq )
{
	square_synth.treble_eq( eq );
	triangle.synth.treble_eq( eq );
	noise.synth.treble_eq( eq );
	dmc.synth.treble_eq( eq );
}

void Nes_Apu::enable_nonlinear( double v )
{
	dmc.nonlinear = true;
	square_synth.volume( 1.3 * 0.25751258 / 0.742467605 * 0.25 / amp_range * v );

	const double tnd = 0.48 / 202 * nonlinear_tnd_gain();
	triangle.synth.volume( 3.0 * tnd );
	noise.synth.volume( 2.0 * tnd );
	dmc.synth.volume( tnd );

	square1 .last_amp = 0;
	square2 .last_amp = 0;
	triangle.last_amp = 0;
	noise   .last_amp = 0;
	dmc     .last_amp = 0;
}

void Nes_Apu::volume( double v )
{
	dmc.nonlinear = false;
	square_synth.volume(   0.1128  / amp_range * v );
	triangle.synth.volume( 0.12765 / amp_range * v );
	noise.synth.volume(    0.0741  / amp_range * v );
	dmc.synth.volume(      0.42545 / 127 * v );
}

void Nes_Apu::output( Blip_Buffer* buffer )
{
}

void Nes_Apu::reset( bool pal_mode, int initial_dmc_dac )
{
	// to do: time pal frame periods exactly
	frame_period = pal_mode ? 8314 : 7458;
	dmc.pal_mode = pal_mode;

	square1.reset();
	square2.reset();
	triangle.reset();
	noise.reset();
	dmc.reset();

	last_time = 0;
	last_dmc_time = 0;
	osc_enables = 0;
	irq_flag = false;
	earliest_irq_ = no_irq;
	frame_delay = 1;
	write_register( 0, 0x4017, 0x00 );
	write_register( 0, 0x4015, 0x00 );

	for ( nes_addr_t addr = start_addr; addr <= 0x4013; addr++ )
		write_register( 0, addr, (addr & 3) ? 0x00 : 0x10 );

	dmc.dac = initial_dmc_dac;
	if ( !dmc.nonlinear )
		triangle.last_amp = 15;
	//if ( !dmc.nonlinear ) // to do: remove?
	//  dmc.last_amp = initial_dmc_dac; // prevent output transition
}

void Nes_Apu::irq_changed()
{
	nes_time_t new_irq = dmc.next_irq;
	if ( dmc.irq_flag | irq_flag ) {
		new_irq = 0;
	}
	else if ( new_irq > next_irq ) {
		new_irq = next_irq;
	}

	if ( new_irq != earliest_irq_ ) {
		earliest_irq_ = new_irq;
		if ( irq_notifier_ )
			irq_notifier_( irq_data );
	}
}

// frames

void Nes_Apu::run_until( nes_time_t end_time )
{
	last_dmc_time = end_time;
}

void Nes_Apu::run_until_( nes_time_t end_time )
{
 last_time = end_time;
 last_dmc_time = end_time;
}

template<class T>
inline void zero_apu_osc( T* osc, nes_time_t time )
{
}

void Nes_Apu::end_frame( nes_time_t end_time )
{
 last_time = end_time;
}

// registers

static const unsigned char length_table [0x20] = {
	0x0A, 0xFE, 0x14, 0x02, 0x28, 0x04, 0x50, 0x06,
	0xA0, 0x08, 0x3C, 0x0A, 0x0E, 0x0C, 0x1A, 0x0E, 
	0x0C, 0x10, 0x18, 0x12, 0x30, 0x14, 0x60, 0x16,
	0xC0, 0x18, 0x48, 0x1A, 0x10, 0x1C, 0x20, 0x1E
};

void Nes_Apu::write_register( nes_time_t time, nes_addr_t addr, int data )
{
}

int Nes_Apu::read_status( nes_time_t time )
{
	run_until_( time - 1 );

	int result = (dmc.irq_flag << 7) | (irq_flag << 6);
	
	for ( int i = 0; i < osc_count; i++ )
		if ( oscs [i]->length_counter )
			result |= 1 << i;

	run_until_( time );

	if ( irq_flag ) {
		irq_flag = false;
		irq_changed();
	}

	return result;
}

