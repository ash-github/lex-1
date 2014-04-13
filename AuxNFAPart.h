#ifndef _YANJUN_AUXNFAPART_H_
#define _YANJUN_AUXNFAPART_H_

#include "NFAFrag.h"

struct TokenLexDef
{
	char re[REGEXPR_MAX_LENGTH];
	char lexAction[LEXACTION_MAX_LENGTH];
};

//class NFAPart;
struct _AuxNFAPart
{
	_AuxNFAPart(const char* n,const char* r)
	{
		strcpy(name,n);
		strcpy(re,r);
	}
	char name[256];
	char re[REGEXPR_MAX_LENGTH];
	NFAPart nfapart;
};

class AuxNFAPart
{
public:
	~AuxNFAPart()
	{
		clear();
	}

	void addPart(const char* n,const char* r);
	NFAPart* searchPart(const char* n);
	void clear();

private:
	list<_AuxNFAPart*> parts;
};

#endif