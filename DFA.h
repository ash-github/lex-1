#ifndef _YANJUN_DFA_H_
#define _YANJUN_DFA_H_

#include "Limits.h"
#include "NFA.h"
struct LexicalDesc;
class DFA
{
public:
	~DFA();

	DFA(){stateCnt=0;}

	void build(LexicalDesc& lexdesc);

	void clear();

	string recognize(TokenLexDef tokenDefs[],const char* str);

	void minimize();

	DFAState* eclosure(NFAState* nfastate);	
	
	DFAState* quickEclosure(DFAState* dfastate);
	void completeEclosure(DFAState* dfastate,DFAState* res);

	int dfaTable[DFASTATE_MAX_COUNT][SYMBOL_CNT];
	int dfaAccept[DFASTATE_MAX_COUNT];//re index
	int stateCnt;
	int startState;

	NFA nfa;
};
#endif// _YANJUN_DFA_H_
