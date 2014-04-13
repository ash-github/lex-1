#ifndef _YANJUN_LEXICALDESC_H_
#define _YANJUN_LEXICALDESC_H_
#include "AuxNFAPart.h"
struct LexicalDesc
{
	LexicalDesc(AuxNFAPart& aux,TokenLexDef* td,int tdcnt,char sstates[][STARTSTATE_NAME_LENGTH],int sstatecnt)
		:auxNFAPart(aux),tokenDefs(td),tokenDefCnt(tdcnt),startStates(sstates),startStateCnt(sstatecnt)
	{}

	AuxNFAPart& auxNFAPart;
	TokenLexDef* tokenDefs;
	int tokenDefCnt;

	char (*startStates)[STARTSTATE_NAME_LENGTH];
	int startStateCnt;
};

#endif