#include "Nes_Mapper.h"

struct m156_state_t
{
	uint8_t prg_bank;
	uint8_t chr_banks [8];
};
BOOST_STATIC_ASSERT( sizeof (m156_state_t) == 9 );

class Mapper_156 : public Nes_Mapper, m156_state_t {
public:
	Mapper_156()
	{
		m156_state_t * state = this;
		register_state( state, sizeof * state );
	}

	void reset_state()
	{
		prg_bank = 0;
		for ( unsigned i = 0; i < 8; i++ ) chr_banks [i] = i;
		enable_sram();
		apply_mapping();
	}

	void apply_mapping()
	{
		mirror_single( 0 );
		set_prg_bank( 0x8000, bank_16k, prg_bank );

		for ( int i = 0; i < (int) sizeof chr_banks; i++ )
			set_chr_bank( i * 0x400, bank_1k, chr_banks [i] );
	}

	void write( nes_time_t, nes_addr_t addr, int data )
	{
		unsigned int reg = addr - 0xC000;
		if ( addr == 0xC010 )
		{
			prg_bank = data;
			set_prg_bank( 0x8000, bank_16k, data );
		}
		else if ( reg < 4 )
		{
			chr_banks [reg] = data;
			set_chr_bank( reg * 0x400, bank_1k, data );
		}
		else if ( ( reg - 8 ) < 4 )
		{
			reg -= 4;
			chr_banks [reg] = data;
			set_chr_bank( reg * 0x400, bank_1k, data );
		}
	}
};

class Mapper_78 : public Nes_Mapper {
	// lower 8 bits are the reg at 8000:ffff
	// next two bits are autodetecting type
	// 0 = unknown 1 = cosmo carrier 2 = holy diver
	int reg;
	void writeinternal(int data, int changed)
	{
		reg &= 0x300;
		reg |= data;

		if (changed & 0x07)
			set_prg_bank(0x8000, bank_16k, reg & 0x07);
		if (changed & 0xf0)
			set_chr_bank(0x0000, bank_8k, (reg >> 4) & 0x0f);
		if (changed & 0x08)
		{
			// set mirroring based on memorized board type
			if (reg & 0x100)
			{
				mirror_single((reg >> 3) & 1);
			}
			else if (reg & 0x200)
			{
				if (reg & 0x08)
					mirror_vert();
				else
					mirror_horiz();
			}
			else
			{
				// if you don't set something here, holy diver dumps with 4sc set will
				// savestate as 4k NTRAM.  then when you later set H\V mapping, state size mismatch.
				mirror_single(1);
			}
		}
	}

public:
	Mapper_78()
	{
		register_state(&reg, 4);
	}

	virtual void reset_state()
	{
		reg = 0;
	}

	virtual void apply_mapping()
	{
		writeinternal(reg, 0xff);
	}

	virtual void write( nes_time_t, nes_addr_t addr, int data)
	{
		// heuristic: if the first write ever to the register is 0,
		// we're on holy diver, otherwise, carrier.  it works for these two games...
		if (!(reg & 0x300))
		{
			reg |= data ? 0x100 : 0x200;
			writeinternal(data, 0xff);
		}
		else
		{
			writeinternal(data, reg ^ data);
		}
	}
};

// Magic Kid Googoo
class Mapper_190: public Nes_Mapper {
public:
	Mapper_190()
	{
	}

	virtual void reset_state()
	{
	}

	virtual void apply_mapping()
	{
		mirror_vert();
		enable_sram();
		set_prg_bank( 0xc000, bank_16k, 0);
	}

	virtual void write(nes_time_t, nes_addr_t addr, int data)
	{
		switch ( addr >> 12 )
		{
			case 0x8:
			case 0x9:
			case 0xc:
			case 0xd:
				set_prg_bank( 0x8000, bank_16k, ( ( ( addr >> 11 ) & 8 ) | ( data & 7 ) ) );
				break;
			case 0xa:
			case 0xb:
				switch ( addr & 3 )
				{
					case 0:
						set_chr_bank( 0x0000, bank_2k, data );
						break;
					case 1:
						set_chr_bank( 0x0800, bank_2k, data );
						break;
					case 2:
						set_chr_bank( 0x1000, bank_2k, data );
						break;
					case 3:
						set_chr_bank( 0x1800, bank_2k, data );
						break;
				}
				break;
		}
	}
};

void register_more_mappers();
void register_more_mappers()
{
	register_mapper< Mapper_156 > ( 156 );
	register_mapper< Mapper_78 > ( 78 );
	register_mapper< Mapper_190 > ( 190 );
}
