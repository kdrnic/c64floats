#ifndef _6502_H
#define _6502_H

#include <stdint.h>
#include <iostream>
#include <cstddef>

// See http://www.emulator101.com.s3-website-us-east-1.amazonaws.com/6502-addressing-modes/
namespace Addressing
{
	enum Type
	{
		Accumulator,
		Immediate,
		Implicit,
		Relative,
		Absolute,
		ZeroPage,
		Indirect,
		AbsoluteX,
		AbsoluteY,
		ZeroPageX,
		ZeroPageY,
		IndirectX,
		IndirectY,
		Unknown
	};
	static const char *toString[] = {
		"A",        //Accumulator,
		"#$%02X",   //Immediate,
		"",         //Implicit,
		"$%04X",    //Relative,
		"$%04X",    //Absolute,
		"$%02X",    //ZeroPage,
		"($%04X)",  //Indirect,
		"$%04X,X",  //AbsoluteX,
		"$%04X,X",  //AbsoluteY,
		"$%02X,X",  //ZeroPageX,
		"$%02X,Y",  //ZeroPageY,
		"($%02X,X)",//IndirectX,
		"($%02X),Y",//IndirectY,
		"",         //Unknown
	};
};

namespace Instruction
{
	enum Type
	{
		ADC,		AND,		ASL,		BCC,
		BCS,		BEQ,		BIT,		BMI,
		BNE,		BPL,		BRK,		BVC,
		BVS,		CLC,		CLD,		CLI,
		CLV,		CMP,		CPX,		CPY,
		DEC,		DEX,		DEY,		EOR,
		INC,		INX,		INY,		JMP,
		JSR,		LDA,		LDX,		LDY,
		LSR,		NOP,		ORA,		PHA,
		PHP,		PLA,		PLP,		ROL,
		ROR,		RTI,		RTS,		SBC,
		SEC,		SED,		SEI,		STA,
		STX,		STY,		TAX,		TAY,
		TSX,		TXA,		TXS,		TYA,
		DOP,		AAC,		ASR,		ARR,
		ATX,		AXS,		SLO,		RLA,
		SRE,		RRA,		AAX,		LAX,
		DCP,		ISC,		TOP,		SYA,
		SXA,
		Unknown
	};
	static const char *toString[] = {
		"ADC",		"AND",		"ASL",		"BCC",
		"BCS",		"BEQ",		"BIT",		"BMI",
		"BNE",		"BPL",		"BRK",		"BVC",
		"BVS",		"CLC",		"CLD",		"CLI",
		"CLV",		"CMP",		"CPX",		"CPY",
		"DEC",		"DEX",		"DEY",		"EOR",
		"INC",		"INX",		"INY",		"JMP",
		"JSR",		"LDA",		"LDX",		"LDY",
		"LSR",		"NOP",		"ORA",		"PHA",
		"PHP",		"PLA",		"PLP",		"ROL",
		"ROR",		"RTI",		"RTS",		"SBC",
		"SEC",		"SED",		"SEI",		"STA",
		"STX",		"STY",		"TAX",		"TAY",
		"TSX",		"TXA",		"TXS",		"TYA",
		"DOP",		"AAC",		"ASR",		"ARR",
		"ATX",		"AXS",		"SLO",		"RLA",
		"SRE",		"RRA",		"AAX",		"LAX",
		"DCP",		"ISC",		"TOP",		"SYA",
		"SXA",
		"???"
	};
};

struct OpCode
{
	Addressing::Type addressing;
	Instruction::Type instruction;
	unsigned int cycles;			// Machine cycles recquired for execution
	bool pageBoundaryPenalty;		// If true, will recquire and extra cycle when crossing page boundaries
};

// See http://www.atarimax.com/jindroush.atari.org/aopc.html#STA
const OpCode opCodes[] =
{
	{Addressing::Implicit,			Instruction::BRK,		7,	false	},	// $00
	{Addressing::IndirectX,			Instruction::ORA,		6,	false	},	// $01
	{Addressing::Unknown,			Instruction::Unknown,	0,	false	},	// $02
	{Addressing::IndirectX,			Instruction::SLO,		8,	false	},	// $03
	{Addressing::ZeroPage,			Instruction::DOP,		3,	false	},	// $04
	{Addressing::ZeroPage,			Instruction::ORA,		2,	false	},	// $05
	{Addressing::ZeroPage,			Instruction::ASL,		5,	false	},	// $06
	{Addressing::ZeroPage,			Instruction::SLO,		5,	false	},	// $07
	{Addressing::Implicit,			Instruction::PHP,		3,	false	},	// $08
	{Addressing::Immediate,			Instruction::ORA,		2,	false	},	// $09
	{Addressing::Accumulator,		Instruction::ASL,		2,	false	},	// $0A
	{Addressing::Immediate,			Instruction::AAC,		2,	false	},	// $0B
	{Addressing::Absolute,			Instruction::TOP,		4,	false	},	// $0C
	{Addressing::Absolute,			Instruction::ORA,		4,	false	},	// $0D
	{Addressing::Absolute,			Instruction::ASL,		6,	false	},	// $0E
	{Addressing::Absolute,			Instruction::SLO,		6,	false	},	// $0F
	{Addressing::Relative,			Instruction::BPL,		2,	true	},	// $10
	{Addressing::IndirectY,			Instruction::ORA,		5,	true	},	// $11
	{Addressing::Unknown,			Instruction::Unknown,	0,	false	},	// $12
	{Addressing::IndirectY,			Instruction::SLO,		8,	false	},	// $13
	{Addressing::ZeroPageX,			Instruction::DOP,		4,	false	},	// $14
	{Addressing::ZeroPageX,			Instruction::ORA,		3,	false	},	// $15
	{Addressing::ZeroPageX,			Instruction::ASL,		6,	false	},	// $16
	{Addressing::ZeroPageX,			Instruction::SLO,		6,	false	},	// $17
	{Addressing::Implicit,			Instruction::CLC,		2,	false	},	// $18
	{Addressing::AbsoluteY,			Instruction::ORA,		4,	true	},	// $19
	{Addressing::Implicit,			Instruction::NOP,		2,	false	},	// $1A
	{Addressing::AbsoluteY,			Instruction::SLO,		7,	false	},	// $1B
	{Addressing::AbsoluteX,			Instruction::TOP,		4,	true	},	// $1C
	{Addressing::AbsoluteX,			Instruction::ORA,		4,	true	},	// $1D
	{Addressing::AbsoluteX,			Instruction::ASL,		7,	false	},	// $1E
	{Addressing::AbsoluteX,			Instruction::SLO,		7,	false	},	// $1F
	{Addressing::Absolute,			Instruction::JSR,		6,	false	},	// $20
	{Addressing::IndirectX,			Instruction::AND,		6,	false	},	// $21
	{Addressing::Unknown,			Instruction::Unknown,	0,	false	},	// $22
	{Addressing::IndirectX,			Instruction::RLA,		8,	false	},	// $23
	{Addressing::ZeroPage,			Instruction::BIT,		3,	false	},	// $24
	{Addressing::ZeroPage,			Instruction::AND,		2,	false	},	// $25
	{Addressing::ZeroPage,			Instruction::ROL,		5,	false	},	// $26
	{Addressing::ZeroPage,			Instruction::RLA,		5,	false	},	// $27
	{Addressing::Implicit,			Instruction::PLP,		4,	false	},	// $28
	{Addressing::Immediate,			Instruction::AND,		2,	false	},	// $29
	{Addressing::Accumulator,		Instruction::ROL,		2,	false	},	// $2A
	{Addressing::Immediate,			Instruction::AAC,		2,	false	},	// $2B
	{Addressing::Absolute,			Instruction::BIT,		4,	false	},	// $2C
	{Addressing::Absolute,			Instruction::AND,		4,	false	},	// $2D
	{Addressing::Absolute,			Instruction::ROL,		6,	false	},	// $2E
	{Addressing::Absolute,			Instruction::RLA,		6,	false	},	// $2F
	{Addressing::Relative,			Instruction::BMI,		2,	true	},	// $30
	{Addressing::IndirectY,			Instruction::AND,		5,	true	},	// $31
	{Addressing::Unknown,			Instruction::Unknown,	0,	false	},	// $32
	{Addressing::IndirectY,			Instruction::RLA,		8,	false	},	// $33
	{Addressing::ZeroPageX,			Instruction::DOP,		4,	false	},	// $34
	{Addressing::ZeroPageX,			Instruction::AND,		3,	false	},	// $35
	{Addressing::ZeroPageX,			Instruction::ROL,		6,	false	},	// $36
	{Addressing::ZeroPageX,			Instruction::RLA,		6,	false	},	// $37
	{Addressing::Implicit,			Instruction::SEC,		2,	false	},	// $38
	{Addressing::AbsoluteY,			Instruction::AND,		4,	true	},	// $39
	{Addressing::Implicit,			Instruction::NOP,		2,	false	},	// $3A
	{Addressing::AbsoluteY,			Instruction::RLA,		7,	false	},	// $3B
	{Addressing::AbsoluteX,			Instruction::TOP,		4,	true	},	// $3C
	{Addressing::AbsoluteX,			Instruction::AND,		4,	true	},	// $3D
	{Addressing::AbsoluteX,			Instruction::ROL,		7,	false	},	// $3E
	{Addressing::AbsoluteX,			Instruction::RLA,		7,	false	},	// $3F
	{Addressing::Implicit,			Instruction::RTI,		6,	false	},	// $40
	{Addressing::IndirectX,			Instruction::EOR,		6,	false	},	// $41
	{Addressing::Unknown,			Instruction::Unknown,	0,	false	},	// $42
	{Addressing::IndirectX,			Instruction::SRE,		8,	false	},	// $43
	{Addressing::ZeroPage,			Instruction::DOP,		3,	false	},	// $44
	{Addressing::ZeroPage,			Instruction::EOR,		3,	false	},	// $45
	{Addressing::ZeroPage,			Instruction::LSR,		5,	false	},	// $46
	{Addressing::ZeroPage,			Instruction::SRE,		5,	false	},	// $47
	{Addressing::Implicit,			Instruction::PHA,		3,	false	},	// $48
	{Addressing::Immediate,			Instruction::EOR,		2,	false	},	// $49
	{Addressing::Accumulator,		Instruction::LSR,		2,	false	},	// $4A
	{Addressing::Immediate,			Instruction::ASR,		2,	false	},	// $4B
	{Addressing::Absolute,			Instruction::JMP,		3,	false	},	// $4C
	{Addressing::Absolute,			Instruction::EOR,		4,	false	},	// $4D
	{Addressing::Absolute,			Instruction::LSR,		6,	false	},	// $4E
	{Addressing::Absolute,			Instruction::SRE,		6,	false	},	// $4F
	{Addressing::Relative,			Instruction::BVC,		2,	true	},	// $50
	{Addressing::IndirectY,			Instruction::EOR,		5,	true	},	// $51
	{Addressing::Unknown,			Instruction::Unknown,	0,	false	},	// $52
	{Addressing::IndirectY,			Instruction::SRE,		8,	false	},	// $53
	{Addressing::ZeroPageX,			Instruction::DOP,		4,	false	},	// $54
	{Addressing::ZeroPageX,			Instruction::EOR,		4,	false	},	// $55
	{Addressing::ZeroPageX,			Instruction::LSR,		6,	false	},	// $56
	{Addressing::ZeroPageX,			Instruction::SRE,		6,	false	},	// $57
	{Addressing::Implicit,			Instruction::CLI,		2,	false	},	// $58
	{Addressing::AbsoluteY,			Instruction::EOR,		4,	true	},	// $59
	{Addressing::Implicit,			Instruction::NOP,		2,	false	},	// $5A
	{Addressing::AbsoluteY,			Instruction::SRE,		7,	false	},	// $5B
	{Addressing::AbsoluteX,			Instruction::TOP,		4,	true	},	// $5C
	{Addressing::AbsoluteX,			Instruction::EOR,		4,	true	},	// $5D
	{Addressing::AbsoluteX,			Instruction::LSR,		7,	false	},	// $5E
	{Addressing::AbsoluteX,			Instruction::SRE,		7,	false	},	// $5F
	{Addressing::Implicit,			Instruction::RTS,		6,	false	},	// $60
	{Addressing::IndirectX,			Instruction::ADC,		6,	false	},	// $61
	{Addressing::Unknown,			Instruction::Unknown,	0,	false	},	// $62
	{Addressing::IndirectX,			Instruction::RRA,		8,	false	},	// $63
	{Addressing::ZeroPage,			Instruction::DOP,		3,	false	},	// $64
	{Addressing::ZeroPage,			Instruction::ADC,		3,	false	},	// $65
	{Addressing::ZeroPage,			Instruction::ROR,		5,	false	},	// $66
	{Addressing::ZeroPage,			Instruction::RRA,		5,	false	},	// $67
	{Addressing::Implicit,			Instruction::PLA,		4,	false	},	// $68
	{Addressing::Immediate,			Instruction::ADC,		2,	false	},	// $69
	{Addressing::Accumulator,		Instruction::ROR,		2,	false	},	// $6A
	{Addressing::Immediate,			Instruction::ARR,		2,	false	},	// $6B
	{Addressing::Indirect,			Instruction::JMP,		5,	false	},	// $6C
	{Addressing::Absolute,			Instruction::ADC,		4,	false	},	// $6D
	{Addressing::Absolute,			Instruction::ROR,		6,	false	},	// $6E
	{Addressing::Absolute,			Instruction::RRA,		6,	false	},	// $6F
	{Addressing::Relative,			Instruction::BVS,		2,	true	},	// $70
	{Addressing::IndirectY,			Instruction::ADC,		5,	true	},	// $71
	{Addressing::Unknown,			Instruction::Unknown,	0,	false	},	// $72
	{Addressing::IndirectY,			Instruction::RRA,		8,	false	},	// $73
	{Addressing::ZeroPageX,			Instruction::DOP,		4,	false	},	// $74
	{Addressing::ZeroPageX,			Instruction::ADC,		4,	false	},	// $75
	{Addressing::ZeroPageX,			Instruction::ROR,		6,	false	},	// $76
	{Addressing::ZeroPageX,			Instruction::RRA,		6,	false	},	// $77
	{Addressing::Implicit,			Instruction::SEI,		2,	false	},	// $78
	{Addressing::AbsoluteY,			Instruction::ADC,		4,	true	},	// $79
	{Addressing::Implicit,			Instruction::NOP,		2,	false	},	// $7A
	{Addressing::AbsoluteY,			Instruction::RRA,		7,	false	},	// $7B
	{Addressing::AbsoluteX,			Instruction::TOP,		4,	true	},	// $7C
	{Addressing::AbsoluteX,			Instruction::ADC,		4,	true	},	// $7D
	{Addressing::AbsoluteX,			Instruction::ROR,		7,	false	},	// $7E
	{Addressing::AbsoluteX,			Instruction::RRA,		7,	false	},	// $7F
	{Addressing::Immediate,			Instruction::DOP,		2,	false	},	// $80
	{Addressing::IndirectX,			Instruction::STA,		6,	false	},	// $81
	{Addressing::Immediate,			Instruction::DOP,		2,	false	},	// $82
	{Addressing::IndirectX,			Instruction::AAX,		6,	false	},	// $83
	{Addressing::ZeroPage,			Instruction::STY,		3,	false	},	// $84
	{Addressing::ZeroPage,			Instruction::STA,		3,	false	},	// $85
	{Addressing::ZeroPage,			Instruction::STX,		3,	false	},	// $86
	{Addressing::ZeroPage,			Instruction::AAX,		3,	false	},	// $87
	{Addressing::Implicit,			Instruction::DEY,		2,	false	},	// $88
	{Addressing::Immediate,			Instruction::DOP,		2,	false	},	// $89
	{Addressing::Implicit,			Instruction::TXA,		2,	false	},	// $8A
	{Addressing::Unknown,			Instruction::Unknown,	0,	false	},	// $8B
	{Addressing::Absolute,			Instruction::STY,		4,	false	},	// $8C
	{Addressing::Absolute,			Instruction::STA,		4,	false	},	// $8D
	{Addressing::Absolute,			Instruction::STX,		4,	false	},	// $8E
	{Addressing::Absolute,			Instruction::AAX,		4,	false	},	// $8F
	{Addressing::Relative,			Instruction::BCC,		2,	true	},	// $90
	{Addressing::IndirectY,			Instruction::STA,		6,	false	},	// $91
	{Addressing::Unknown,			Instruction::Unknown,	0,	false	},	// $92
	{Addressing::Unknown,			Instruction::Unknown,	0,	false	},	// $93
	{Addressing::ZeroPageX,			Instruction::STY,		4,	false	},	// $94
	{Addressing::ZeroPageX,			Instruction::STA,		4,	false	},	// $95
	{Addressing::ZeroPageY,			Instruction::STX,		4,	false	},	// $96
	{Addressing::ZeroPageY,			Instruction::AAX,		4,	false	},	// $97
	{Addressing::Implicit,			Instruction::TYA,		2,	false	},	// $98
	{Addressing::AbsoluteY,			Instruction::STA,		5,	false	},	// $99
	{Addressing::Implicit,			Instruction::TXS,		2,	false	},	// $9A
	{Addressing::Unknown,			Instruction::Unknown,	0,	false	},	// $9B
	{Addressing::AbsoluteX,			Instruction::SYA,		5,	false	},	// $9C
	{Addressing::AbsoluteX,			Instruction::STA,		5,	false	},	// $9D
	{Addressing::AbsoluteY,			Instruction::SXA,		5,	false	},	// $9E
	{Addressing::Unknown,			Instruction::Unknown,	0,	false	},	// $9F
	{Addressing::Immediate,			Instruction::LDY,		2,	false	},	// $A0
	{Addressing::IndirectX,			Instruction::LDA,		6,	false	},	// $A1
	{Addressing::Immediate,			Instruction::LDX,		2,	false	},	// $A2
	{Addressing::IndirectX,			Instruction::LAX,		6,	false	},	// $A3
	{Addressing::ZeroPage,			Instruction::LDY,		3,	false	},	// $A4
	{Addressing::ZeroPage,			Instruction::LDA,		3,	false	},	// $A5
	{Addressing::ZeroPage,			Instruction::LDX,		3,	false	},	// $A6
	{Addressing::ZeroPage,			Instruction::LAX,		3,	false	},	// $A7
	{Addressing::Implicit,			Instruction::TAY,		2,	false	},	// $A8
	{Addressing::Immediate,			Instruction::LDA,		2,	false	},	// $A9
	{Addressing::Implicit,			Instruction::TAX,		2,	false	},	// $AA
	{Addressing::Immediate,			Instruction::ATX,		2,	false	},	// $AB
	{Addressing::Absolute,			Instruction::LDY,		4,	false	},	// $AC
	{Addressing::Absolute,			Instruction::LDA,		4,	false	},	// $AD
	{Addressing::Absolute,			Instruction::LDX,		4,	false	},	// $AE
	{Addressing::Absolute,			Instruction::LAX,		4,	false	},	// $AF
	{Addressing::Relative,			Instruction::BCS,		2,	true	},	// $B0
	{Addressing::IndirectY,			Instruction::LDA,		5,	true	},	// $B1
	{Addressing::Unknown,			Instruction::Unknown,	0,	false	},	// $B2
	{Addressing::IndirectY,			Instruction::LAX,		5,	true	},	// $B3
	{Addressing::ZeroPageX,			Instruction::LDY,		4,	false	},	// $B4
	{Addressing::ZeroPageX,			Instruction::LDA,		4,	false	},	// $B5
	{Addressing::ZeroPageY,			Instruction::LDX,		4,	false	},	// $B6
	{Addressing::ZeroPageY,			Instruction::LAX,		4,	false	},	// $B7
	{Addressing::Implicit,			Instruction::CLV,		2,	false	},	// $B8
	{Addressing::AbsoluteY,			Instruction::LDA,		4,	true	},	// $B9
	{Addressing::Implicit,			Instruction::TSX,		2,	false	},	// $BA
	{Addressing::Unknown,			Instruction::Unknown,	0,	false	},	// $BB
	{Addressing::AbsoluteX,			Instruction::LDY,		4,	true	},	// $BC
	{Addressing::AbsoluteX,			Instruction::LDA,		4,	true	},	// $BD
	{Addressing::AbsoluteY,			Instruction::LDX,		4,	true	},	// $BE
	{Addressing::AbsoluteY,			Instruction::LAX,		4,	true	},	// $BF
	{Addressing::Immediate,			Instruction::CPY,		2,	false	},	// $C0
	{Addressing::IndirectX,			Instruction::CMP,		6,	false	},	// $C1
	{Addressing::Immediate,			Instruction::DOP,		2,	false	},	// $C2
	{Addressing::IndirectX,			Instruction::DCP,		8,	false	},	// $C3
	{Addressing::ZeroPage,			Instruction::CPY,		3,	false	},	// $C4
	{Addressing::ZeroPage,			Instruction::CMP,		3,	false	},	// $C5
	{Addressing::ZeroPage,			Instruction::DEC,		5,	false	},	// $C6
	{Addressing::ZeroPage,			Instruction::DCP,		5,	false	},	// $C7
	{Addressing::Implicit,			Instruction::INY,		2,	false	},	// $C8
	{Addressing::Immediate,			Instruction::CMP,		2,	false	},	// $C9
	{Addressing::Implicit,			Instruction::DEX,		2,	false	},	// $CA
	{Addressing::Immediate,			Instruction::AXS,		2,	false	},	// $CB
	{Addressing::Absolute,			Instruction::CPY,		4,	false	},	// $CC
	{Addressing::Absolute,			Instruction::CMP,		4,	false	},	// $CD
	{Addressing::Absolute,			Instruction::DEC,		6,	false	},	// $CE
	{Addressing::Absolute,			Instruction::DCP,		6,	false	},	// $CF
	{Addressing::Relative,			Instruction::BNE,		2,	true	},	// $D0
	{Addressing::IndirectY,			Instruction::CMP,		5,	true	},	// $D1
	{Addressing::Unknown,			Instruction::Unknown,	0,	false	},	// $D2
	{Addressing::IndirectY,			Instruction::DCP,		8,	false	},	// $D3
	{Addressing::ZeroPageX,			Instruction::DOP,		4,	false	},	// $D4
	{Addressing::ZeroPageX,			Instruction::CMP,		4,	false	},	// $D5
	{Addressing::ZeroPageX,			Instruction::DEC,		6,	false	},	// $D6
	{Addressing::ZeroPageX,			Instruction::DCP,		6,	false	},	// $D7
	{Addressing::Implicit,			Instruction::CLD,		2,	false	},	// $D8
	{Addressing::AbsoluteY,			Instruction::CMP,		4,	true	},	// $D9
	{Addressing::Implicit,			Instruction::NOP,		2,	false	},	// $DA
	{Addressing::AbsoluteY,			Instruction::DCP,		7,	false	},	// $DB
	{Addressing::AbsoluteX,			Instruction::TOP,		4,	true	},	// $DC
	{Addressing::AbsoluteX,			Instruction::CMP,		4,	true	},	// $DD
	{Addressing::AbsoluteX,			Instruction::DEC,		7,	false	},	// $DE
	{Addressing::AbsoluteX,			Instruction::DCP,		7,	false	},	// $DF
	{Addressing::Immediate,			Instruction::CPX,		2,	false	},	// $E0
	{Addressing::IndirectX,			Instruction::SBC,		6,	false	},	// $E1
	{Addressing::Immediate,			Instruction::DOP,		2,	false	},	// $E2
	{Addressing::IndirectX,			Instruction::ISC,		8,	false	},	// $E3
	{Addressing::ZeroPage,			Instruction::CPX,		3,	false	},	// $E4
	{Addressing::ZeroPage,			Instruction::SBC,		3,	false	},	// $E5
	{Addressing::ZeroPage,			Instruction::INC,		5,	false	},	// $E6
	{Addressing::ZeroPage,			Instruction::ISC,		5,	false	},	// $E7
	{Addressing::Implicit,			Instruction::INX,		2,	false	},	// $E8
	{Addressing::Immediate,			Instruction::SBC,		2,	false	},	// $E9
	{Addressing::Implicit,			Instruction::NOP,		2,	false	},	// $EA
	{Addressing::Immediate,			Instruction::SBC,		2,	false	},	// $EB
	{Addressing::Absolute,			Instruction::CPX,		4,	false	},	// $EC
	{Addressing::Absolute,			Instruction::SBC,		4,	false	},	// $ED
	{Addressing::Absolute,			Instruction::INC,		6,	false	},	// $EE
	{Addressing::Absolute,			Instruction::ISC,		6,	false	},	// $EF
	{Addressing::Relative,			Instruction::BEQ,		2,	true	},	// $F0
	{Addressing::IndirectY,			Instruction::SBC,		5,	true	},	// $F1
	{Addressing::Unknown,			Instruction::Unknown,	0,	false	},	// $F2
	{Addressing::IndirectY,			Instruction::ISC,		8,	false	},	// $F3
	{Addressing::ZeroPageX,			Instruction::DOP,		4,	false	},	// $F4
	{Addressing::ZeroPageX,			Instruction::SBC,		4,	false	},	// $F5
	{Addressing::ZeroPageX,			Instruction::INC,		6,	false	},	// $F6
	{Addressing::ZeroPageX,			Instruction::ISC,		6,	false	},	// $F7
	{Addressing::Implicit,			Instruction::SED,		2,	false	},	// $F8
	{Addressing::AbsoluteY,			Instruction::SBC,		4,	true	},	// $F9
	{Addressing::Implicit,			Instruction::NOP,		2,	false	},	// $FA
	{Addressing::AbsoluteY,			Instruction::ISC,		7,	false	},	// $FB
	{Addressing::AbsoluteX,			Instruction::TOP,		4,	true	},	// $FC
	{Addressing::AbsoluteX,			Instruction::SBC,		4,	true	},	// $FD
	{Addressing::AbsoluteX,			Instruction::INC,		7,	false	},	// $FE
	{Addressing::AbsoluteX,			Instruction::ISC,		7,	false	},	// $FF
};
// Javascript code used to order the lines:
/*
	strs=aaa.value.split("\n"); newStrs = []; for(var i = 0; i < strs.length; i++){ str=strs[i];
	newStrs[parseInt(str.substr(str.indexOf("$")+1,2), 16)] = str; } for(var i = 0; i < 256; i++)
	{ if(typeof newStrs[i] == "undefined") newStrs[i] =
	"\t{Addressing::Unknown,			Instruction::Unknown,	0,	false	},	// $" +
	((i < 16) ? "0" : "") + (i.toString(16)).toUpperCase(); } aaa.value = newStrs.join("\n");
*/

namespace Flags
{
	enum Type
	{
		Carry = 1u,
		Zero = 2u,
		Interrupt = 4u,
		Decimal = 8u,
		Break = 16u,
		// Bit 5 of the status register is not used and is always set to 1
		Overflow = 64u,
		Sign = 128u
	};
};

class MemoryByte;

class Memory
{
	public:
		virtual MemoryByte operator[](std::size_t address) = 0;
		virtual void Read(const MemoryByte&mb) = 0;
		virtual void Write(const MemoryByte&mb) = 0;
		void Load(std::istream &is);
		void Dump(std::ostream &os);
};

class MemoryByte
{
	public:
	const uint8_t *ptr;
	uint8_t *wptr;
	uint16_t addr;
	Memory *mem;
	
	//No side effects
	uint8_t inspect() const
	{
		uint8_t temp = *ptr;
		return temp;
	}
	
	operator uint8_t() const
	{
		uint8_t temp = *ptr;
		mem->Read(*this);
		return temp;
	};
	MemoryByte& operator=(const uint8_t &other)
	{
		//if(wptr) *wptr = other;
		//else *ptr = other;
		*wptr = other;
		mem->Write(*this);
		return *this;
	};
	
	MemoryByte(Memory *m) : ptr(0), wptr(0), addr(0), mem(m) {};
};

class Machine
{
	public:
		FILE *log_file;
		Memory &memory;
		struct Registers
		{
			uint8_t a, x, y, p, s;	// Accumulator, X index, Y index, Processor status, Stack pointer
			uint16_t pc;			// Program counter
		};
		Registers registers;
		
		void Dump(std::ostream &os);
		unsigned int DoStep();
		
		Machine(Memory &m) :
			log_file(0),
			memory(m),
			registers{}
		{
		}
		
		void Interrupt(uint16_t vec_addr,  bool push, bool setB);
		void BRK();
		void NMI();
		void Reset();
		void IRQ();
		
		void Savestate(FILE *f);
		void Loadstate(FILE *f);
};

#endif
