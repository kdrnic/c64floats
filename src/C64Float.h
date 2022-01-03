#include <stdint.h>

class C64Float
{
	public:
	uint8_t val[5];
	
	static C64Float zero, unit;
	
	void fromString(const char *str);
	void toString(char *out);
	C64Float operator *(const C64Float other) const;
	C64Float operator +(const C64Float other) const;
	C64Float operator -(const C64Float other) const;
	C64Float operator /(const C64Float other) const;
	bool operator >(const C64Float other) const;
	bool operator ==(const C64Float other) const;
	operator int() const;
	C64Float operator -() const;
	
	C64Float sqrt();
	C64Float abs();
	C64Float atan();
	C64Float cos();
	C64Float exp();
	C64Float sin();
	C64Float tan();
	C64Float log();
	C64Float round();
	
	C64Float pow(C64Float other);
	
	C64Float operator *=(const C64Float other){ *this = *this * other; return *this; }
	C64Float operator +=(const C64Float other){ *this = *this + other; return *this; }
	C64Float operator /=(const C64Float other){ *this = *this / other; return *this; }
	C64Float operator -=(const C64Float other){ *this = *this - other; return *this; }
	C64Float operator ++(){ *this += unit; return *this; }
	C64Float operator --(){ *this -= unit; return *this; }
	bool operator <(C64Float other)  const{ return !((*this) == other) && !((*this) > other); }
	
	static unsigned long long GetCycles();
	
	double toDouble();
	
	C64Float()
	{
	}
	
	C64Float(const char *str)
	{
		fromString(str);
	}
	
	C64Float(int i);
	C64Float(double d);
};

static C64Float round(C64Float f){ return f.round(); }
static C64Float abs(C64Float f){ return f.abs(); }
static C64Float sqrt(C64Float f){ return f.sqrt(); }
static C64Float atan(C64Float f){ return f.atan(); }
static C64Float tan(C64Float f){ return f.tan(); }
static C64Float exp(C64Float f){ return f.exp(); }
static C64Float pow(C64Float f1, C64Float f2){ return f1.pow(f2); }
static C64Float log(C64Float f){ return f.log(); }
static C64Float sin(C64Float f){ return f.sin(); }
static C64Float cos(C64Float f){ return f.cos(); }
static C64Float log2(C64Float f){ return log(f) / log(C64Float("2")); }
static C64Float log10(C64Float f){ return log(f) / log(C64Float("10")); }

C64Float operator "" _C64F(const char *);
