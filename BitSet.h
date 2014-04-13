#ifndef _YANJUN_BITSET_H_
#define _YANJUN_BITSET_H_

#include "Limits.h"
#include "string.h"

#define CALC_UNITS(x) (((x/sizeof(unsigned long))>>3)+1)

class BitSet
{
public:
	~BitSet()
	{
	}
	BitSet(const BitSet& b)
	{
		*this=b;
	}
	BitSet()
	{
		totalBytes=CALC_UNITS(NFASTATE_MAX_COUNT);
		memset(bits,0,sizeof(bits));
	}
	void reset()
	{
		memset(bits,0,sizeof(bits));
	}
	void set(int pos)
	{
		int lp=pos/32,rp=pos%32;
		bits[lp]|=1<<rp;		
	}	
	void clear(int pos)
	{
		int lp=pos/32,rp=pos%32;
		bits[lp]&=~(1<<rp);	
	}
	void or(const BitSet& b)
	{		
		for(int i=0;i<totalBytes;i++)
			bits[i]|=b.bits[i];
	}

	bool get(int pos)
	{
		int lp=pos/32,rp=pos%32;

		return bits[lp]&(1<<rp);
	}

	int getByteCnt() const{return totalBytes;}

	bool operator[](int pos){return get(pos);}

	BitSet operator<<(int p)
	{
		BitSet res;

		unsigned long savePBits=0ul;
		int mp=sizeof(unsigned long)*8-p;

		for(int i=0;i<totalBytes;i++)
		{			
			res.bits[i]=bits[i]<<p;
			res.bits[i]|=savePBits;
			savePBits=bits[i]>>mp;
		}
		return res;
	}

	friend bool operator<(const BitSet& a,const BitSet& b);
	friend bool operator==(const BitSet& a,const BitSet& b);

private:
	unsigned long bits[CALC_UNITS(NFASTATE_MAX_COUNT)];
	int totalBytes;
};

#endif