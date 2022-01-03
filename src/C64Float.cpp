#include "C64Float.h"
#include "C64Memory.h"

#include <unordered_map>
#include <csignal>
#include <cstdio>
#include <cstring>

static unsigned long long cycles = 0;

C64Float C64Float::zero("0.0"), C64Float::unit("1.0");

class C64Prog
{
	public:
	C64Memory mem;
	Machine cpu;
	uint8_t *ram;
	uint8_t *prg;
	uint8_t *start;
	
	C64Prog getAddr(size_t &addr)
	{
		addr = prg - ram;
		return *this;
	}
	
	C64Prog pushBytes(uint8_t b1)
	{
		*(prg++) = b1;
		return *this;
	}
	
	C64Prog pushBytes(uint8_t b1, uint8_t b2)
	{
		*(prg++) = b1;
		*(prg++) = b2;
		return *this;
	}
	
	C64Prog pushBytes(uint8_t b1, uint8_t b2, uint8_t b3)
	{
		*(prg++) = b1;
		*(prg++) = b2;
		*(prg++) = b3;
		return *this;
	}
	
	C64Prog reserve(size_t n)
	{
		for(size_t i = 0; i < n; i++){
			pushBytes(0);
		}
		return *this;
	}
	
	C64Prog pushFloat(const C64Float f)
	{
		for(size_t i = 0; i < sizeof(f.val); i++){
			pushBytes(f.val[i]);
		}
		return *this;
	}
	
	C64Prog pushString(const char *str)
	{
		do{
			pushBytes(*str);
		}while(*(str++));
		return *this;
	}
	
	C64Prog pushLDA(uint8_t val){    return pushBytes(0xA9, val); }                                                      //LDA immediate
	C64Prog pushLDY(uint8_t val){    return pushBytes(0xA0, val); }                                                      //LDY immediate
	C64Prog pushLDX(uint8_t val){    return pushBytes(0xA2, val); }                                                      //LDX immediate
	C64Prog pushJSR(size_t addr){    return pushBytes(0x20, addr & 0xFFu, addr >> 8u); }                                 //JSR addr
	C64Prog pushAddrAY(size_t addr){ return pushLDA(addr & 0xFFu).pushLDY(addr >> 8u); }                                 //LDA #<addr LDY #>addr
	C64Prog pushAddrXY(size_t addr){ return pushLDX(addr & 0xFFu).pushLDY(addr >> 8u); }                                 //LDX #<addr LDY #>addr
	C64Prog pushSTA(size_t addr){                                                                                        //
		if(addr >= 0x100)                                                                                                //
			                         return pushBytes(0x8D, addr & 0xFFu, addr >> 8u);                                   //STA $addr
		else                                                                                                             //
			                         return pushBytes(0x85, addr);                                                       //STA $addr (zero-page)
	}                                                                                                                    //
	C64Prog pushSTY(size_t addr){                                                                                        //
		if(addr >= 0x100)                                                                                                //
			                         return pushBytes(0x8C, addr & 0xFFu, addr >> 8u);                                   //STY $addr
		else                                                                                                             //
			                         return pushBytes(0x84, addr);                                                       //STY $addr (zero-page)
	}                                                                                                                    //
	C64Prog pushMOVFM(size_t addr){  return pushAddrAY(addr).pushJSR(0xBBA2); }                                          //Fetch a number from a RAM location to FAC (A=Addr.LB, Y=Addr.HB) 
	C64Prog pushCONUPK(size_t addr){ return pushAddrAY(addr).pushJSR(0xBA8C); }                                          //Fetch a number from a RAM location to ARG (A=Addr.LB, Y=Addr.HB) 
	C64Prog pushMOVMF(size_t addr){  return pushAddrXY(addr).pushJSR(0xBBD4); }                                          //
	                                                                                                                     //Store the number currently in FAC, to a RAM location. Uses X and Y rather than A and Y to point to RAM. (X=Addr.LB, Y=Addr.HB)
	C64Prog pushMOVEF(){             return pushJSR(0xBBFC); }                                                           //Copy a number currently in ARG, over into FAC 
	C64Prog pushMOVFA(size_t addr){  return pushJSR(0xBC0F); }                                                           //Copy a number currently in FAC, over into ARG 
	C64Prog pushCHRGET(){            return pushJSR(0x0079); }                                                           //CHRGET routine: fetches next character of BASIC program text 
	C64Prog pushFADD(size_t addr){   return pushAddrAY(addr).pushJSR(0xB867); }                                          //Adds the number in FAC with one stored in RAM (A=Addr.LB, Y=Addr.HB) 
	C64Prog pushFSUB(size_t addr){   return pushAddrAY(addr).pushJSR(0xB850); }                                          //Subtracts the number in FAC from one stored in RAM (A=Addr.LB, Y=Addr.HB) 
	C64Prog pushFDIV(size_t addr){   return pushAddrAY(addr).pushJSR(0xBB0F); }                                          //Divides a number stored in RAM by the number in FAC (A=Addr.LB, Y=Addr.HB) 
	C64Prog pushFMUL(size_t addr){   return pushAddrAY(addr).pushJSR(0xBA28); }                                          //Multiplication with memory contents pointed to by A/Y (low/high).
	C64Prog pushSQR(){               return pushJSR(0xBF71); }                                                           //Performs the SQR function on the number in FAC 
	C64Prog pushABS(){               return pushJSR(0xBC58); }                                                           //Performs the ABS function on the number in FAC 
	C64Prog pushFIN(size_t addr){    return pushAddrAY(addr).pushSTA(0x7A).pushSTY(0x7B).pushCHRGET().pushJSR(0xBCF3); } //
	                                                                                                                     //Convert number expressed as a zero-terminated PETSCII string, to floating point number in FAC. Expects string-address in $7a/$7b, and to make it work either call CHRGOT ($0079) beforehand or load the accumulator with the first char of the string and clear the carry-flag manually. 
	C64Prog pushFOUT(){              return pushJSR(0xBDDD); }                                                           //
	                                                                                                                     //Convert number in FAC to a zero-terminated PETSCII string (starting at $0100, address in A, Y too). Direct output of FAC also via $AABC/43708 possible. 
	C64Prog pushFCOMP(size_t addr){  return pushAddrAY(addr).pushJSR(0xBC5B); }                                          //
	                                                                                                                     //Compares the number in FAC against one stored in RAM (A=Addr.LB, Y=Addr.HB). The result of the comparison is stored in A: Zero (0) indicates the values were equal. One (1) indicates FAC was greater than RAM and negative one (-1 or $FF) indicates FAC was less than RAM. Also sets processor flags (N,Z) depending on whether the number in FAC is zero, positive or negative
	C64Prog pushATN(){               return pushJSR(0xE30E); }                                                           //Performs the ATN function on the number in FAC 
	C64Prog pushCOS(){               return pushJSR(0xE264); }                                                           //Performs the COS function on the number in FAC 
	C64Prog pushEXP(){               return pushJSR(0xBFED); }                                                           //Performs the EXP function on the number in FAC 
	C64Prog pushPWR(size_t addr){    return pushAddrAY(addr).pushJSR(0xBF78); }                                          //Raises a number stored ín RAM to the power in FAC (A=Addr.LB, Y=Addr.HB)
	C64Prog pushPWR_(){              return pushJSR(0xBF7B); }                                                           //FAC2 raised to the power of FAC1 (FAC2^FAC1). 
																														 //This routine uses the formula exp(x*log(y)) to calculate yx, so it calculates two series (log and exp). It is slow and not entirely accurate. For whole number powers, it is often quicker and more accurate to use a series of multiplies. 
	C64Prog pushLOG(){               return pushJSR(0xB9EA); }                                                           //Performs the LOG function on the number in FAC 
	C64Prog pushSIN(){               return pushJSR(0xE26B); }                                                           //Performs the SIN function on the number in FAC 
	C64Prog pushTAN(){               return pushJSR(0xE2B4); }                                                           //Performs the TAN function on the number in FAC 
	C64Prog pushINT(){               return pushJSR(0xBCCC); }                                                           //Performs the INT function on the number in FAC 
	C64Prog pushQINT(){              return pushJSR(0xBC9B); }                                                           //Convert number in FAC to 32-bit signed integer ($62-$65, big-endian order).
	
	C64Prog popFloat(size_t addr, C64Float &f)
	{
		for(size_t i = 0; i < sizeof(f.val); i++){
			f.val[i] = ram[addr + i];
		}
		return *this;
	}
	
	C64Float retFloat(size_t addr)
	{
		C64Float f;
		popFloat(addr, f);
		return f;
	}
	
	int retIntBigEndian(size_t addr)
	{
		int64_t result = ram[addr];
		result = (result << 8) | ram[addr + 1];
		result = (result << 8) | ram[addr + 2];
		result = (result << 8) | ram[addr + 3];
		/*
		if(result >= 0x80000000u){
			result -= 0x80000000u;
			result -= 0x80000000u;
		}
		*/
		
		return result;
	}
	
	int retA()
	{
		int a = cpu.registers.a;
		if(a >= 0x80) a -= 0x100;
		return a;
	}
	
	C64Prog popString(size_t addr, char *out)
	{
		uint8_t *str = ram + addr;
		do{
			*(out++) = (char) *str;
		}while(*(str++));
		return *this;
	}
	
	C64Prog begin()
	{
		start = prg;
		return *this;
	}
	
	C64Prog execute()
	{
		//Avoid return addresses being overwritten by string
		cpu.registers.s = 0xFF;
		
		cpu.registers.pc = start - ram;
		size_t end = prg - ram;
		
		while(
			cpu.registers.pc != end &&
			cpu.registers.pc != 0xFF48
		){
			cycles += cpu.DoStep();
		}
		
		if(cpu.registers.pc == 0xFF48){
			std::raise(SIGFPE);
		}
		
		return *this;
	}
	
	C64Prog() :
		mem(),
		cpu(mem),
		ram(mem.ram),
		prg(ram + 0xC000)
	{
	}
	
	~C64Prog()
	{
		if(cpu.log_file){
			std::fflush(cpu.log_file);
		}
	}
};

unsigned long long C64Float::GetCycles()
{
	return cycles;
}

#if 0
C64Prog NewProg(const char *func)
#define NewProg() NewProg(__FUNCTION__)
#else
C64Prog NewProg()
#endif
{
	C64Prog p;
	
	#ifdef NewProg
	static std::unordered_map<std::string, int> count;
	int d = count[func]++;
	FILE *f;
	char fn[256];
	std::strcpy(fn, "logs/");
	std::strcat(fn, func);
	if(std::strstr(fn, ">=")) std::strcpy(std::strstr(fn, ">="), "_geq");
	if(std::strstr(fn, "<=")) std::strcpy(std::strstr(fn, "<="), "_leq");
	if(std::strstr(fn, ">") ) std::strcpy(std::strstr(fn, ">") , "_gt");
	if(std::strstr(fn, "<") ) std::strcpy(std::strstr(fn, "<") , "_lt");
	std::sprintf(fn + strlen(fn), "_%d", d);
	std::strcat(fn, ".log");
	f = std::fopen(fn, "w");
	p.cpu.log_file = f;
	#endif
	
	return p;
}

void C64Float::fromString(const char *str)
{
	size_t addrStr, addrFAC, addrARG;
	NewProg()
		.getAddr(addrStr)
		.pushString(str)
		.getAddr(addrFAC)
		.reserve(5)
		.begin()
		.pushFIN(addrStr)
		.pushMOVMF(addrFAC)
		.execute()
		.popFloat(addrFAC, *this);
}

void C64Float::toString(char *out)
{
	size_t addrStr, addrFAC, addrARG;
	char tmp[256];
	NewProg()
		.getAddr(addrFAC)
		.pushFloat(*this)
		.begin()
		.pushMOVFM(addrFAC)
		.pushFOUT()
		.execute()
		.popString(0x100, tmp);
	//Fix lack of zero in string
	char *c = tmp;
	*out = 0;
	while(1){
		if(*c == '-') c++;
		else if(*c == '+') c++;
		else if(*c == ' ') c++;
		else if(*c == '.'){
			*c = 0;
			strcpy(out, tmp);
			strcat(out, "0.");
			strcat(out, c + 1);
			return;
		}
		else if(*c >= '0' && *c <= '9') break;
	}
	strcpy(out, tmp);
}

C64Float C64Float::operator *(const C64Float other) const
{
	size_t addrStr, addrFAC, addrARG;
	return NewProg()
		.getAddr(addrFAC)
		.pushFloat(*this)
		.getAddr(addrARG)
		.pushFloat(other)
		.begin()
		.pushMOVFM(addrFAC)
		.pushFMUL(addrARG)
		.pushMOVMF(addrFAC)
		.execute()
		.retFloat(addrFAC);
}

C64Float C64Float::operator +(const C64Float other) const
{
	size_t addrStr, addrFAC, addrARG;
	return NewProg()
		.getAddr(addrFAC)
		.pushFloat(*this)
		.getAddr(addrARG)
		.pushFloat(other)
		.begin()
		.pushMOVFM(addrFAC)
		.pushFADD(addrARG)
		.pushMOVMF(addrFAC)
		.execute()
		.retFloat(addrFAC);
}

bool C64Float::operator >(const C64Float other) const
{
	size_t addrStr, addrFAC, addrARG;
	return NewProg()
		.getAddr(addrFAC)
		.pushFloat(*this)
		.getAddr(addrARG)
		.pushFloat(other)
		.begin()
		.pushMOVFM(addrFAC)
		.pushFCOMP(addrARG)
		.execute()
		.retA() > 0;
}

bool C64Float::operator ==(const C64Float other) const
{
	if(!val[0] && !other.val[0]) return 1;
	return !memcmp(val, other.val, sizeof(val));
}

C64Float C64Float::abs()
{
	size_t addrStr, addrFAC, addrARG;
	return NewProg()
		.getAddr(addrFAC)
		.pushFloat(*this)
		.begin()
		.pushMOVFM(addrFAC)
		.pushABS()
		.pushMOVMF(addrFAC)
		.execute()
		.retFloat(addrFAC);
}

C64Float C64Float::operator -(const C64Float other) const
{
	size_t addrStr, addrFAC, addrARG;
	return NewProg()
		.getAddr(addrFAC)
		.pushFloat(other)
		.getAddr(addrARG)
		.pushFloat(*this)
		.begin()
		.pushMOVFM(addrFAC)
		.pushFSUB(addrARG)
		.pushMOVMF(addrFAC)
		.execute()
		.retFloat(addrFAC);
}

C64Float C64Float::operator /(const C64Float other) const
{
	size_t addrStr, addrFAC, addrARG;
	return NewProg()
		.getAddr(addrFAC)
		.pushFloat(other)
		.getAddr(addrARG)
		.pushFloat(*this)
		.begin()
		.pushMOVFM(addrFAC)
		.pushFDIV(addrARG)
		.pushMOVMF(addrFAC)
		.execute()
		.retFloat(addrFAC);
}

C64Float C64Float::sqrt()
{
	size_t addrStr, addrFAC, addrARG;
	return NewProg()
		.getAddr(addrFAC)
		.pushFloat(*this)
		.begin()
		.pushMOVFM(addrFAC)
		.pushSQR()
		.pushMOVMF(addrFAC)
		.execute()
		.retFloat(addrFAC);
}

C64Float C64Float::atan()
{
	size_t addrStr, addrFAC, addrARG;
	return NewProg()
		.getAddr(addrFAC)
		.pushFloat(*this)
		.begin()
		.pushMOVFM(addrFAC)
		.pushATN()
		.pushMOVMF(addrFAC)
		.execute()
		.retFloat(addrFAC);
}

C64Float C64Float::cos()
{
	size_t addrStr, addrFAC, addrARG;
	return NewProg()
		.getAddr(addrFAC)
		.pushFloat(*this)
		.begin()
		.pushMOVFM(addrFAC)
		.pushCOS()
		.pushMOVMF(addrFAC)
		.execute()
		.retFloat(addrFAC);
}

C64Float C64Float::exp()
{
	size_t addrStr, addrFAC, addrARG;
	return NewProg()
		.getAddr(addrFAC)
		.pushFloat(*this)
		.begin()
		.pushMOVFM(addrFAC)
		.pushEXP()
		.pushMOVMF(addrFAC)
		.execute()
		.retFloat(addrFAC);
}

C64Float C64Float::pow(const C64Float other)
{
	size_t addrStr, addrFAC, addrARG;
	return NewProg()
		.getAddr(addrFAC)
		.pushFloat(other)
		.getAddr(addrARG)
		.pushFloat(*this)
		.begin()
		.pushMOVFM(addrFAC)
		.pushCONUPK(addrARG)
		.pushPWR_()
		.pushMOVMF(addrFAC)
		.execute()
		.retFloat(addrFAC);
}

C64Float C64Float::sin()
{
	size_t addrStr, addrFAC, addrARG;
	return NewProg()
		.getAddr(addrFAC)
		.pushFloat(*this)
		.begin()
		.pushMOVFM(addrFAC)
		.pushSIN()
		.pushMOVMF(addrFAC)
		.execute()
		.retFloat(addrFAC);
}

C64Float C64Float::tan()
{
	size_t addrStr, addrFAC, addrARG;
	return NewProg()
		.getAddr(addrFAC)
		.pushFloat(*this)
		.begin()
		.pushMOVFM(addrFAC)
		.pushTAN()
		.pushMOVMF(addrFAC)
		.execute()
		.retFloat(addrFAC);
}

C64Float C64Float::log()
{
	size_t addrStr, addrFAC, addrARG;
	return NewProg()
		.getAddr(addrFAC)
		.pushFloat(*this)
		.begin()
		.pushMOVFM(addrFAC)
		.pushLOG()
		.pushMOVMF(addrFAC)
		.execute()
		.retFloat(addrFAC);
}

C64Float C64Float::round()
{
	size_t addrStr, addrFAC, addrARG;
	return NewProg()
		.getAddr(addrFAC)
		.pushFloat((*this) + C64Float("0.5"))
		.begin()
		.pushMOVFM(addrFAC)
		.pushINT()
		.pushMOVMF(addrFAC)
		.execute()
		.retFloat(addrFAC);
}

C64Float::operator int() const
{
	size_t addrStr, addrFAC, addrARG;
	return NewProg()
		.getAddr(addrFAC)
		.pushFloat(*this)
		.begin()
		.pushMOVFM(addrFAC)
		.pushQINT()
		.execute()
		.retIntBigEndian(0x62);
}

C64Float operator "" _C64F(const char *str)
{
	return C64Float(str);
}

#include <cmath>

double C64Float::toDouble()
{
	if(!val[0]) return 0.0;
	unsigned int mantissa =
		(((val[1] & 0x7F) | 0x80u) << 24) +
		(val[2] << 16) +
		(val[3] << 8) +
		(val[4] << 0);
	int exp = val[0];
	exp -= 0x81;
	return std::pow(2.0, exp) * double(mantissa) / double(0x80u << 24);
}

C64Float C64Float::operator -() const
{
	C64Float res = *this;
	res.val[1] ^= (1u << 7);
	return res;
}

C64Float::C64Float(int i)
{
	char str[256] = {};
	std::sprintf(str, "%d", i);
	fromString(str);
	//std::printf("C64Float(%d)\n", i);
}

C64Float::C64Float(double d)
{
	char str[256] = {};
	std::sprintf(str, "%f", d);
	fromString(str);
	//std::printf("C64Float(%f)\n", d);
}
