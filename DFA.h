#ifndef _YANJUN_DFA_H_
#define _YANJUN_DFA_H_

#include "define.h"
#include "NFA.h"

class DFA
{
public:
	~DFA();

	DFA(){stateCnt=0;}

	void build(const LexEnv*);

	void clear();

	//string recognize(TokenLexDef tokenDefs[],const char* str);

	//void minimize();

	DFAState* newState()
	{
		return new DFAState(stateCnt++);
	}

	int dfaTable[DFASTATE_MAX_COUNT][SYMBOL_CNT];
	int dfaAccept[DFASTATE_MAX_COUNT];//re index
	int stateCnt;
	int startState;

	NFA nfa;
};
#endif// _YANJUN_DFA_H_
