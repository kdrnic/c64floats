#include "6502.h"

#include <cstdio>

class C64Memory : public Memory
{
	public:
	uint8_t ram[64 * 1024];
	static const uint8_t rom_kernal[8 * 1024];
	static const uint8_t rom_basic[8 * 1024];
	virtual MemoryByte operator[](std::size_t address)
	{
		MemoryByte mb(this);
		mb.addr = address;
		mb.wptr = ram + address;
		mb.ptr = ram + address;
		
		switch(address){
			case 0xA000 ... 0xBFFF:
				mb.ptr = rom_basic + address - 0xA000;
				break;
			case 0xE000 ... 0xFFFF:
				mb.ptr = rom_kernal + address - 0xE000;
				break;
			default:
				break;
		}
		return mb;
	}
	
	virtual void Read(const MemoryByte&mb)
	{
	}
	
	virtual void Write(const MemoryByte&mb)
	{
	}
	
	C64Memory() : ram{}
	{
		//Copy CHRGET
		for(size_t src = 0xE3A2, dst = 0x73; src <= 0xE3BE; src++, dst++){
			ram[dst] = (*this)[src];
		}
		
		//Patch out multiply bug
		// https://www.c64-wiki.com/wiki/Multiply_bug
		//std::printf("Memory --------------------- %02x\n", (*this)[0xA000 + 0x1a4f].inspect());
		ram[0xA000 + 0x1A4F] = 0x5E;
	}
};
