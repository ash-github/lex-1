#ifndef _YANJUN_BITSET_H_
#define _YANJUN_BITSET_H_

#include "define.h"
#include "string.h"

#define CALC_UNITS(x) (((x/sizeof(unsigned long))>>3)+1)
#define UNIT_IN_BITS (sizeof(unsigned long)<<3)
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
		int lp=pos/UNIT_IN_BITS,rp=pos%UNIT_IN_BITS;
		bits[lp]|=1<<rp;		
	}	
	void clear(int pos)
	{
		int lp=pos/UNIT_IN_BITS,rp=pos%UNIT_IN_BITS;
		bits[lp]&=~(1<<rp);	
	}
	void or(const BitSet& b)
	{		
		for(int i=0;i<totalBytes;i++)
			bits[i]|=b.bits[i];
	}

	bool get(int pos)
	{
		int lp=pos/UNIT_IN_BITS,rp=pos%UNIT_IN_BITS;

		return ((bits[lp]&(1<<rp))!=0);
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

	bool operator<(const BitSet& b)const
	{
		int i=getByteCnt();
		while(i>=0&&bits[i]==b.bits[i]) i--;

		if(i<0) return false;
		return bits[i]<b.bits[i];
	}
	bool operator==(const BitSet& b)const
	{
		int i=getByteCnt();
		while(i>=0&&bits[i]==b.bits[i]) i--;

		return i<0;
	}

private:
	unsigned long bits[CALC_UNITS(NFASTATE_MAX_COUNT)];
	int totalBytes;
};


#endif