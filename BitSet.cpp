#include "BitSet.h"

bool operator<(const BitSet& a,const BitSet& b)
{
	int i=a.getByteCnt();
	while(i>=0&&a.bits[i]==b.bits[i]) i--;

	if(i<0) return false;
	return a.bits[i]<b.bits[i];
}

bool operator==(const BitSet& a,const BitSet& b)
{
	int i=a.getByteCnt();
	while(i>=0&&a.bits[i]==b.bits[i]) i--;

	return i<0;
}
