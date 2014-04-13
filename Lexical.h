#ifndef _YANJUN_LEXICAL_H_
#define _YANJUN_LEXICAL_H_

#include "AuxNFAPart.h"
#include "DFA.h"

#include "lexicaldesc.h"

class Lexical
{
public:
	AuxNFAPart auxNFAPart;
	TokenLexDef tokenDefs[TOKENDEF_MAX_COUNT];
	int tokenDefCnt;

	char startStates[STARTSTATE_MAX_COUNT][STARTSTATE_NAME_LENGTH];
	int startStateCnt;

	DFA dfa;

	Lexical()
	{
		tokenDefCnt=1;tokenDefs[0].re[0]=tokenDefs[0].lexAction[0]=NULL;
		startStateCnt=1;
		strcpy(startStates[0],"INITIAL");
	}

	void addStartState(char* sp)
	{
		strcpy(startStates[startStateCnt++],sp);
	}

	void addTokenDef(char* re,char* code)
	{
		strcpy(tokenDefs[tokenDefCnt].re,re);
		strcpy(tokenDefs[tokenDefCnt].lexAction,code);
		tokenDefCnt++;
	}

	void buildTable()
	{
		dfa.build(LexicalDesc(auxNFAPart,tokenDefs,tokenDefCnt,startStates,startStateCnt));

		//dfa.minimize();
	}

	void generate();
};


#endif