#include "6502.h"

#include <string.h>
#include <stdio.h>

#include <iomanip>
#include <sstream>

#define bit0  0x1u
#define bit1  0x2u
#define bit2  0x4u
#define bit3  0x8u
#define bit4  0x10u
#define bit5  0x20u
#define bit6  0x40u
#define bit7  0x80u
#define bit8  0x100u
#define bit9  0x200u
#define bit10 0x400u
#define bit11 0x800u
#define bit12 0x1000u
#define bit13 0x2000u
#define bit14 0x4000u
#define bit15 0x8000u

static unsigned int HexToInt(std::string s)
{
	unsigned int i;
	std::stringstream ss(s.c_str());
	ss >> std::hex >> i;
	return i;
}

void Memory::Load(std::istream &is)
{
	while(true)
	{
		std::string line;
		std::getline(is, line);
		if(line == "") break;
		std::string line2;
		for(unsigned int i = 0; i < line.length(); i++)
		{
			if(line[i] != ' ') line2.push_back(line[i]);
		}

		uint16_t address = HexToInt(line2.substr(0, line2.find_first_of(":")));

		line2 = line2.substr(line2.find_first_of(":") + 1);
		for(unsigned int i = 0; i < line2.length(); i += 2)
		{
			uint8_t value = HexToInt(line2.substr(i, 2));
			(*this)[address++] = value;
		}
	}
}

void Memory::Dump(std::ostream &os)
{
	for(unsigned int address = 0x00; address < 0x1000; address += 0x10)
	{
		os << std::setfill('0') << std::setw(4) << std::hex << address << ": ";
		for(unsigned int address2 = address; address2 < address + 0x10; address2 += 2)
		{
			os << std::setfill('0') << std::setw(2) << std::hex << int((*this)[address2]);
			os << std::setfill('0') << std::setw(2) << std::hex << int((*this)[address2 + 1]);
			os << ' ';
		}
		os << "\n";
	}
}

void Machine::Dump(std::ostream &os)
{
	os << "A  :" << std::setfill('0') << std::setw(2) << std::hex << int(registers.a) << '\n';
	os << "X  :" << std::setfill('0') << std::setw(2) << std::hex << int(registers.x) << '\n';
	os << "Y  :" << std::setfill('0') << std::setw(2) << std::hex << int(registers.y) << '\n';
	os << "S  :" << std::setfill('0') << std::setw(2) << std::hex << int(registers.s) << '\n';
	os << "P  :" << std::setfill('0') << std::setw(2) << std::hex << int(registers.p) << '\n';
	os << "PC :" << std::setfill('0') << std::setw(4) << std::hex << int(registers.pc) << '\n';
}

unsigned int Machine::DoStep()
{
	uint8_t opCode = memory[registers.pc];
	uint16_t address = 0u;
	uint8_t value = 0u;
	bool pageBoundaryCrossed = false;
	unsigned int cycles = 0;
	unsigned int length = 0;
	char log_line[128] = "";
	uint8_t operandA = 0u, operandB = 0u, temp8;
	Registers oldRegisters = registers;
	
	cycles += opCodes[opCode].cycles;
	length += 1;
	
	if(log_file) sprintf(log_line + strlen(log_line), "   %04x  %02X ", registers.pc, opCode);
	
	switch(opCodes[opCode].addressing){
		case Addressing::Accumulator:
		case Addressing::Implicit:
			break;
		
		case Addressing::ZeroPage:
		case Addressing::ZeroPageX:
		case Addressing::ZeroPageY:
		case Addressing::IndirectX:
		case Addressing::IndirectY:
		case Addressing::Relative:
		case Addressing::Immediate:
			operandA = memory[registers.pc + 1u];
			length += 1u;
			if(log_file) sprintf(log_line + strlen(log_line), "%02X", operandA);
			break;
		
		case Addressing::AbsoluteX:
		case Addressing::AbsoluteY:
		case Addressing::Indirect:
		case Addressing::Absolute:
			operandA = memory[registers.pc + 1u];
			operandB = memory[registers.pc + 2u];
			length += 2u;
			if(log_file) sprintf(log_line + strlen(log_line), "%02X %02X", operandA, operandB);
			break;
		
		case Addressing::Unknown:
			break;
	}
	
	if(log_file){
		while(strlen(log_line) < 21) strcat(log_line, " ");
		strcat(log_line, Instruction::toString[opCodes[opCode].instruction]);
	}
	
	//AVOIDS: reading value to cause instructions such as STA $PPUDATA,
	//that wouldn't READ the memory, to do so
	bool lazy_value = false;
	
	switch(opCodes[opCode].addressing){
		case Addressing::Accumulator:
			value = registers.a;
			break;

		case Addressing::Immediate:
			value = operandA;
			break;

		case Addressing::Relative:
			address = registers.pc + 2u;
			value = operandA;
			break;

		case Addressing::Implicit:
			break;

		case Addressing::Absolute:
			address = operandA | (operandB << 8u);
			/*value = memory[address];*/lazy_value = true;
			break;

		case Addressing::ZeroPage:
			address = operandA;
			/*value = memory[address];*/lazy_value = true;
			
			break;

		case Addressing::Indirect:
			address = operandA | (operandB << 8u);
			address =
				memory[address] |
				(memory[(address & 0xFF00u) | ((address + 1u) & 0xFFu)] << 8u);
			break;

		case Addressing::AbsoluteX:
			address = operandA | (operandB << 8u);
			if(temp8 = ~(address & 0xFFu), temp8 <= registers.x) pageBoundaryCrossed = true;
			address += registers.x;
			/*value = memory[address];*/lazy_value = true;
			break;

		case Addressing::AbsoluteY:
			address = operandA | (operandB << 8u);
			if(temp8 = ~(address & 0xFFu), temp8 <= registers.y) pageBoundaryCrossed = true;
			address += registers.y;
			/*value = memory[address];*/lazy_value = true;
			break;

		case Addressing::ZeroPageX:
			address = (operandA + registers.x) & 0xFFu;
			/*value = memory[address];*/lazy_value = true;
			break;

		case Addressing::ZeroPageY:
			address = (operandA + registers.y) & 0xFFu;
			/*value = memory[address];*/lazy_value = true;
			break;

		case Addressing::IndirectX:
			address =
				memory[(operandA + registers.x) & 0xFFu] |
				(memory[(operandA + registers.x + 1u) & 0xFFu] << 8u);
			/*value = memory[address];*/lazy_value = true;
			break;

		case Addressing::IndirectY:
			address = operandA;
			address =
				memory[address] |
				(memory[(address & 0xFF00u) | ((address + 1u) & 0xFFu)] << 8u);
			if(temp8 = ~(address & 0xFFu), temp8 <= registers.y) pageBoundaryCrossed = true;
			address += registers.y;
			/*value = memory[address];*/lazy_value = true;
			break;

		case Addressing::Unknown:
			break;
	}
	
	if(log_file){
		char strOperand[256];
		strcat(log_line, " ");
		sprintf(
			strOperand,
			Addressing::toString[opCodes[opCode].addressing],
			(opCodes[opCode].addressing != Addressing::Relative) ? operandA + (operandB << 8u) : address
		);
		strcat(log_line, strOperand);
	}

	if(pageBoundaryCrossed && opCodes[opCode].pageBoundaryPenalty) cycles++;
	registers.pc += length;

	uint16_t temp;
	switch(opCodes[opCode].instruction){
		case Instruction::ADC:
			if(lazy_value) value = memory[address];
			temp = registers.a + value + (registers.p & 1u);
			registers.p &= ~(Flags::Carry | Flags::Zero | Flags::Overflow | Flags::Sign);
			if(temp > 0xFFu) registers.p |= Flags::Carry;
			if(((~(registers.a ^ value)) & (registers.a ^ temp) & 0x80u) != 0) registers.p |= Flags::Overflow;
			if((temp & 0xFFu) == 0) registers.p |= Flags::Zero;
			if(temp & 128u) registers.p |= Flags::Sign;
			registers.a = temp & 0xFFu;
			break;

		case Instruction::AND:
			if(lazy_value) value = memory[address];
			temp = registers.a & value;
			registers.p &= ~(Flags::Zero | Flags::Sign);
			if((temp & 0xFFu) == 0) registers.p |= Flags::Zero;
			if(temp & 128u) registers.p |= Flags::Sign;
			registers.a = temp & 0xFFu;
			break;
			
		case Instruction::ATX:
			if(lazy_value) value = memory[address];
			//https://forums.nesdev.com/viewtopic.php?f=3&t=10698#p121064
			//temp = registers.a & value;
			temp = value;
			registers.p &= ~(Flags::Zero | Flags::Sign);
			if((temp & 0xFFu) == 0) registers.p |= Flags::Zero;
			if(temp & 128u) registers.p |= Flags::Sign;
			registers.a = temp & 0xFFu;
			registers.x = registers.a;
			break;
		
		case Instruction::AXS:
			if(lazy_value) value = memory[address];
			temp = (registers.a & registers.x) - value;
			registers.p &= ~(Flags::Zero | Flags::Sign | Flags::Carry);
			if(!(temp > 0xFFu)) registers.p |= Flags::Carry;
			if((temp & 0xFFu) == 0) registers.p |= Flags::Zero;
			if(temp & 128u) registers.p |= Flags::Sign;
			registers.x = temp & 0xFFu;
			break;
		
		case Instruction::ASR:
			//AND #i
			if(lazy_value) value = memory[address];
			temp = registers.a & value;
			registers.p &= ~(Flags::Zero | Flags::Sign);
			if((temp & 0xFFu) == 0) registers.p |= Flags::Zero;
			if(temp & 128u) registers.p |= Flags::Sign;
			registers.a = temp & 0xFFu;
			//LSR
			value = registers.a;
			temp = value >> 1u;
			registers.p &= ~(Flags::Sign | Flags::Zero | Flags::Carry);
			if((value & 1u) != 0) registers.p |= (Flags::Carry);
			if((temp & 0xFFu) == 0) registers.p |= Flags::Zero;
			if(temp & 128u) registers.p |= Flags::Sign;
			registers.a = temp & 0xFF;
			break;
		
		case Instruction::ARR:
			//AND #i
			if(lazy_value) value = memory[address];
			temp = registers.a & value;
			registers.p &= ~(Flags::Zero | Flags::Sign);
			if((temp & 0xFFu) == 0) registers.p |= Flags::Zero;
			if(temp & 128u) registers.p |= Flags::Sign;
			registers.a = temp & 0xFFu;
			//ROR
			value = registers.a;
			temp = (value >> 1u) | ((registers.p & Flags::Carry) << 7u);
			registers.p &= ~(Flags::Sign | Flags::Zero | Flags::Carry | Flags::Overflow);
			
			if((temp & bit6) != 0) registers.p |= Flags::Carry;
			if(((temp & bit6) >> 1u) ^ (temp & bit5)) registers.p |= Flags::Overflow;
			
			if((temp & 0xFFu) == 0) registers.p |= Flags::Zero;
			if((temp & 0x80u) != 0) registers.p |= Flags::Sign;
			registers.a = temp & 0xFF;
			break;
			
		case Instruction::AAC:
			if(lazy_value) value = memory[address];
			temp = registers.a & value;
			registers.p &= ~(Flags::Zero | Flags::Sign | Flags::Carry);
			if((temp & 0xFFu) == 0) registers.p |= Flags::Zero;
			if(temp & 128u){
				registers.p |= Flags::Sign;
				registers.p |= Flags::Carry;
			}
			registers.a = temp & 0xFFu;
			break;

		case Instruction::ASL:
			if(lazy_value) value = memory[address];
			temp = value << 1u;
			registers.p &= ~(Flags::Sign | Flags::Zero | Flags::Carry);
			if((temp & 0x100u) != 0u) registers.p |= (Flags::Carry);
			if((temp & 0xFFu) == 0u) registers.p |= Flags::Zero;
			if(temp & 128u) registers.p |= Flags::Sign;
			//registers.a = temp & 0xFF;
			goto PutValue;
		
		case Instruction::SLO:
			//ASL
			if(lazy_value) value = memory[address];
			temp = value << 1u;
			registers.p &= ~(Flags::Sign | Flags::Zero | Flags::Carry);
			if((temp & 0x100u) != 0u) registers.p |= (Flags::Carry);
			if((temp & 0xFFu) == 0u) registers.p |= Flags::Zero;
			if(temp & 128u) registers.p |= Flags::Sign;
			if(opCodes[opCode].addressing == Addressing::Accumulator) registers.a = temp & 0xFFu;
			else memory[address] = temp & 0xFFu;
			//ORA
			registers.a |= value;
			registers.p &= ~(Flags::Sign | Flags::Zero);
			if((registers.a & 0x80u) != 0) registers.p |= Flags::Sign;
			if(registers.a == 0) registers.p |= Flags::Zero;
			break;
			
		case Instruction::BIT:
			if(lazy_value) value = memory[address];
			temp = registers.a & value;
			registers.p &= ~(Flags::Zero | Flags::Sign | Flags::Overflow);
			if((temp & 0xFFu) == 0) registers.p |= Flags::Zero;
			if(value & Flags::Sign) registers.p |= Flags::Sign;
			if(value & Flags::Overflow) registers.p |= Flags::Overflow;
			break;

		case Instruction::BPL:
			if((registers.p & Flags::Sign) == 0) goto Branch;
			break;

		case Instruction::BMI:
			if((registers.p & Flags::Sign) != 0) goto Branch;
			break;

		case Instruction::BVC:
			if((registers.p & Flags::Overflow) == 0) goto Branch;
			break;

		case Instruction::BVS:
			if((registers.p & Flags::Overflow) != 0) goto Branch;
			break;

		case Instruction::BCC:
			if((registers.p & Flags::Carry) == 0) goto Branch;
			break;

		case Instruction::BCS:
			if((registers.p & Flags::Carry) != 0) goto Branch;
			break;

		case Instruction::BNE:
			if((registers.p & Flags::Zero) == 0) goto Branch;
			break;

		case Instruction::BEQ:
			if((registers.p & Flags::Zero) != 0) goto Branch;
			break;

		Branch:
			if(lazy_value) value = memory[address];
			cycles += 1;											// +1 cycle for branches taken
			if((value & 0x80u) == 0) temp = address + value;			// Handle positive offsets
			else temp = address - (0x100u - value);					// Handle negative offsets
			registers.pc = temp;
			if((temp & 0xFF00u) != (address & 0xFF00u)) cycles += 1;		// Special page boundary penalty for branches
			break;

		case Instruction::BRK:
			registers.pc++;
			BRK();
			break;

		case Instruction::CMP:
			temp = registers.a;
			goto Compare;

		case Instruction::CPX:
			temp = registers.x;
			goto Compare;

		case Instruction::CPY:
			temp = registers.y;
			goto Compare;

		Compare:
			if(lazy_value) value = memory[address];
			registers.p &= ~(Flags::Sign | Flags::Zero | Flags::Carry);
			if(temp >= value) registers.p |= Flags::Carry;
			if(temp == value) registers.p |= Flags::Zero;
			temp8 = temp & 0xFFu;
			temp8 -= value;
			if((temp8 & Flags::Sign) != 0) registers.p |= Flags::Sign;
			break;

		case Instruction::DEC:
			if(lazy_value) value = memory[address];
			registers.p &= ~(Flags::Sign | Flags::Zero);
			value -= 1u;
			if((value & 0x80u) != 0) registers.p |= Flags::Sign;
			if(value == 0) registers.p |= Flags::Zero;
			memory[address] = value;
			break;

		case Instruction::EOR:
			if(lazy_value) value = memory[address];
			temp = registers.a ^ value;
			registers.p &= ~(Flags::Sign | Flags::Zero);
			if((temp & 0x80u) != 0) registers.p |= Flags::Sign;
			if((temp & 0xFFu) == 0) registers.p |= Flags::Zero;
			registers.a = temp & 0xFFu;
			break;

		case Instruction::CLC:
			registers.p &= ~Flags::Carry;
			break;

		case Instruction::SEC:
			registers.p |= Flags::Carry;
			break;

		case Instruction::CLI:
			registers.p &= ~Flags::Interrupt;
			break;

		case Instruction::SEI:
			registers.p |= Flags::Interrupt;
			break;

		case Instruction::CLV:
			registers.p &= ~Flags::Overflow;
			break;

		case Instruction::CLD:
			registers.p &= ~Flags::Decimal;
			break;

		case Instruction::SED:
			registers.p |= Flags::Decimal;
			break;

		case Instruction::INC:
			if(lazy_value) value = memory[address];
			registers.p &= ~(Flags::Sign | Flags::Zero);
			value += 1u;
			if((value & 0x80u) != 0) registers.p |= Flags::Sign;
			if(value == 0) registers.p |= Flags::Zero;
			memory[address] = value;
			break;

		case Instruction::JMP:
			registers.pc = address;
			break;

		case Instruction::JSR:
			registers.pc--;
			memory[0x0100u | (registers.s--)] = (registers.pc & 0xFF00u) >> 8u;
			memory[0x0100u | (registers.s--)] = registers.pc & 0xFFu;
			registers.pc = address;
			break;

		case Instruction::LDA:
			if(lazy_value) value = memory[address];
			registers.a = value;
			registers.p &= ~(Flags::Sign | Flags::Zero);
			if((registers.a & 0x80u) != 0) registers.p |= Flags::Sign;
			if(registers.a == 0) registers.p |= Flags::Zero;
			break;

		case Instruction::LDX:
			if(lazy_value) value = memory[address];
			registers.x = value;
			registers.p &= ~(Flags::Sign | Flags::Zero);
			if((registers.x & 0x80u) != 0) registers.p |= Flags::Sign;
			if(registers.x == 0) registers.p |= Flags::Zero;
			break;

		case Instruction::LDY:
			if(lazy_value) value = memory[address];
			registers.y = value;
			registers.p &= ~(Flags::Sign | Flags::Zero);
			if((registers.y & 0x80u) != 0) registers.p |= Flags::Sign;
			if(registers.y == 0) registers.p |= Flags::Zero;
			break;

		case Instruction::LSR:
			if(lazy_value) value = memory[address];
			temp = value >> 1u;
			registers.p &= ~(Flags::Sign | Flags::Zero | Flags::Carry);
			if((value & 1u) != 0) registers.p |= (Flags::Carry);
			if((temp & 0xFFu) == 0) registers.p |= Flags::Zero;
			if(temp & 128u) registers.p |= Flags::Sign;
			//registers.a = temp & 0xFF;
			goto PutValue;
		
		case Instruction::DOP:
		case Instruction::NOP:
			if(lazy_value) value = memory[address];
			break;

		case Instruction::ORA:
			if(lazy_value) value = memory[address];
			registers.a |= value;
			registers.p &= ~(Flags::Sign | Flags::Zero);
			if((registers.a & 0x80u) != 0) registers.p |= Flags::Sign;
			if(registers.a == 0) registers.p |= Flags::Zero;
			break;

		case Instruction::TAX:
			registers.x = registers.a;
			registers.p &= ~(Flags::Sign | Flags::Zero);
			if((registers.a & 0x80u) != 0) registers.p |= Flags::Sign;
			if(registers.a == 0) registers.p |= Flags::Zero;
			break;

		case Instruction::TXA:
			registers.a = registers.x;
			registers.p &= ~(Flags::Sign | Flags::Zero);
			if((registers.a & 0x80u) != 0) registers.p |= Flags::Sign;
			if(registers.a == 0) registers.p |= Flags::Zero;
			break;

		case Instruction::TAY:
			registers.y = registers.a;
			registers.p &= ~(Flags::Sign | Flags::Zero);
			if((registers.a & 0x80u) != 0) registers.p |= Flags::Sign;
			if(registers.a == 0) registers.p |= Flags::Zero;
			break;

		case Instruction::TYA:
			registers.a = registers.y;
			registers.p &= ~(Flags::Sign | Flags::Zero);
			if((registers.a & 0x80u) != 0) registers.p |= Flags::Sign;
			if(registers.a == 0) registers.p |= Flags::Zero;
			break;

		case Instruction::INX:
			registers.x++;
			registers.p &= ~(Flags::Sign | Flags::Zero);
			if((registers.x & 0x80u) != 0) registers.p |= Flags::Sign;
			if(registers.x == 0) registers.p |= Flags::Zero;
			break;

		case Instruction::DEX:
			registers.x--;
			registers.p &= ~(Flags::Sign | Flags::Zero);
			if((registers.x & 0x80u) != 0) registers.p |= Flags::Sign;
			if(registers.x == 0) registers.p |= Flags::Zero;
			break;

		case Instruction::INY:
			registers.y++;
			registers.p &= ~(Flags::Sign | Flags::Zero);
			if((registers.y & 0x80u) != 0) registers.p |= Flags::Sign;
			if(registers.y == 0) registers.p |= Flags::Zero;
			break;

		case Instruction::DEY:
			registers.y--;
			registers.p &= ~(Flags::Sign | Flags::Zero);
			if((registers.y & 0x80u) != 0) registers.p |= Flags::Sign;
			if(registers.y == 0) registers.p |= Flags::Zero;
			break;

		case Instruction::ROL:
			if(lazy_value) value = memory[address];
			temp = (value << 1u) | (registers.p & Flags::Carry);
			registers.p &= ~(Flags::Sign | Flags::Zero | Flags::Carry);
			if(value & 128u) registers.p |= Flags::Carry;
			if((temp & 0xFFu) == 0) registers.p |= Flags::Zero;
			if((temp & 0x80u) != 0) registers.p |= Flags::Sign;
			//registers.a = temp & 0xFF;
			goto PutValue;

		case Instruction::ROR:
			if(lazy_value) value = memory[address];
			temp = (value >> 1u) | ((registers.p & Flags::Carry) << 7u);
			registers.p &= ~(Flags::Sign | Flags::Zero | Flags::Carry);
			if((value & 1u) != 0) registers.p |= Flags::Carry;
			if((temp & 0xFFu) == 0) registers.p |= Flags::Zero;
			if((temp & 0x80u) != 0) registers.p |= Flags::Sign;
			//registers.a = temp & 0xFF;
			
		PutValue:	// Used for ASL, LSR, ROL and ROR
					// The only instructions for which the result can be stored
					// either in memory or in the accumulator
			if(opCodes[opCode].addressing == Addressing::Accumulator) registers.a = temp & 0xFFu;
			else memory[address] = temp & 0xFFu;
			break;

		case Instruction::RTI:
			registers.p = memory[0x0100u | ((++registers.s) & 0xFFu)];
			registers.pc = memory[0x0100u | ((++registers.s) & 0xFFu)];
			registers.pc |= memory[0x0100u | ((++registers.s) & 0xFFu)] << 8u;
			break;

		case Instruction::RTS:
			registers.pc = memory[0x0100u | ((++registers.s) & 0xFFu)];
			registers.pc |= memory[0x0100u | ((++registers.s) & 0xFFu)] << 8u;
			registers.pc++;
			break;

		case Instruction::SBC:
			if(lazy_value) value = memory[address];
			temp = registers.a;
			temp -= value + (!(registers.p & Flags::Carry));
			registers.p &= ~(Flags::Carry | Flags::Zero | Flags::Overflow | Flags::Sign);
			if(!(temp > 0xFFu)) registers.p |= Flags::Carry;
			if((registers.a ^ temp) & (registers.a ^ value) & 0x80u) registers.p |= Flags::Overflow;
			if((temp & 0xFFu) == 0) registers.p |= Flags::Zero;
			if(temp & 128u) registers.p |= Flags::Sign;
			registers.a = temp & 0xFFu;
			break;

		case Instruction::STA:
			memory[address] = registers.a;
			break;

		case Instruction::STX:
			memory[address] = registers.x;
			break;

		case Instruction::STY:
			memory[address] = registers.y;
			break;

		case Instruction::TXS:
			registers.s = registers.x;
			break;

		case Instruction::TSX:
			registers.x = registers.s;
			registers.p &= ~(Flags::Sign | Flags::Zero);
			if((registers.x & 0x80u) != 0) registers.p |= Flags::Sign;
			if(registers.x == 0) registers.p |= Flags::Zero;
			break;

		case Instruction::PHA:
			memory[0x0100u | (registers.s--)] = registers.a;
			break;

		case Instruction::PLA:
			registers.a = memory[0x0100u | ((++registers.s) & 0xFFu)];
			registers.p &= ~(Flags::Zero | Flags::Sign);
			if((registers.a & 0x80u) != 0) registers.p |= Flags::Sign;
			if(registers.a == 0) registers.p |= Flags::Zero;
			break;

		case Instruction::PHP:
			memory[0x0100u | (registers.s--)] = registers.p | bit4 | bit5;
			break;

		case Instruction::PLP:
			registers.p = memory[0x0100u | ((++registers.s) & 0xFFu)];
			break;

		case Instruction::Unknown:
			fprintf(stderr, "%04X:\tUnknown instruction: %02X\n", registers.pc, opCode);
			abort();
			break;
		
		case Instruction::RLA:
		case Instruction::SRE:		
		case Instruction::RRA:		
		case Instruction::AAX:		
		case Instruction::LAX:
		case Instruction::DCP:		
		case Instruction::ISC:
		case Instruction::TOP:
		case Instruction::SYA:
		case Instruction::SXA:
			//I lost the patience to implement this crap
			break;
	}
	
	if(log_file){
		#if 0 
		while(strlen(log_line) < 61) strcat(log_line, " ");
		char str_flags[] = "nvubdizc";
	
		if(registers.p & Flags::Carry)			*(strchr(str_flags, 'c')) -= 32;
		if(registers.p & Flags::Zero)			*(strchr(str_flags, 'z')) -= 32;
		if(registers.p & Flags::Interrupt)		*(strchr(str_flags, 'i')) -= 32;
		if(registers.p & Flags::Decimal)		*(strchr(str_flags, 'd')) -= 32;
		if(registers.p & Flags::Break)			*(strchr(str_flags, 'b')) -= 32;
		//if(registers.p & Flags::
		if(registers.p & Flags::Overflow)		*(strchr(str_flags, 'v')) -= 32;
		if(registers.p & Flags::Sign)			*(strchr(str_flags, 'n')) -= 32;
	
		sprintf(log_line + strlen(log_line), "A:%02X X:%02X Y:%02X S:%02X P:%s ",
			registers.a,
			registers.x,
			registers.y,
			registers.s,
			str_flags
		);
		#else
		//Match VICE log format
		while(strlen(log_line) < 36) strcat(log_line, " ");
		
		sprintf(log_line + strlen(log_line), "- A:%02X X:%02X Y:%02X SP:%02x ",
			oldRegisters.a,
			oldRegisters.x,
			oldRegisters.y,
			oldRegisters.s
		);
		
		strcat(log_line, (oldRegisters.p & Flags::Sign)     ? "N" : ".");
		strcat(log_line, (oldRegisters.p & Flags::Overflow) ? "V" : ".");
		strcat(log_line,                                      "-"      );
		strcat(log_line, (oldRegisters.p & Flags::Break)    ? "B" : ".");
		strcat(log_line, (oldRegisters.p & Flags::Decimal)  ? "D" : ".");
		strcat(log_line, (oldRegisters.p & Flags::Interrupt)? "I" : "."); 
		strcat(log_line, (oldRegisters.p & Flags::Zero)     ? "Z" : ".");
		strcat(log_line, (oldRegisters.p & Flags::Carry)    ? "C" : ".");
		#endif
		
		#if 0
		uint16_t addr2 = 0x100;
		sprintf(log_line + strlen(log_line), " %04X:", addr2);
		for(int i = 0; i < 0x100; i++, addr2++){
			sprintf(log_line + strlen(log_line), " %02X", memory[addr2].inspect());
		}
		#endif
		
		strcat(log_line, "\n");
		fputs(log_line, log_file);
	}
	
	return cycles;
}

void Machine::Interrupt(uint16_t vec_addr, bool push, bool setB)
{
	if(push){
		memory[0x0100u | (registers.s--)] = (registers.pc & 0xFF00u) >> 8u;
		memory[0x0100u | (registers.s--)] = registers.pc & 0xFFu;
		memory[0x0100u | (registers.s--)] = registers.p | (setB ? (Flags::Break | 0x20) : 0);
	}
	uint16_t hi = memory[vec_addr + 1u];
	uint16_t lo = memory[vec_addr];
	registers.pc =  lo | (hi << 8u);
}

void Machine::NMI()
{
	Interrupt(0xFFFA, true, false);
	registers.p |= Flags::Interrupt;
}

void Machine::Reset()
{
	Interrupt(0xFFFC, false, false);
	registers.s -= 0x3u;
}

void Machine::IRQ()
{
	if(registers.p & Flags::Interrupt) return;
	Interrupt(0xFFFE, true, false);
	registers.p |= Flags::Interrupt;
}

void Machine::BRK()
{
	Interrupt(0xFFFE, true, true);
	registers.p |= Flags::Interrupt;
}

void Machine::Savestate(FILE *f)
{
	fputc(registers.a, f);
	fputc(registers.x, f);
	fputc(registers.y, f);
	fputc(registers.p, f);
	fputc(registers.s, f);
	fwrite(&registers.pc, 2, 1, f);
}

void Machine::Loadstate(FILE *f)
{
	registers.a = fgetc(f);
	registers.x = fgetc(f);
	registers.y = fgetc(f);
	registers.p = fgetc(f);
	registers.s = fgetc(f);
	fread(&registers.pc, 2, 1, f);
}
