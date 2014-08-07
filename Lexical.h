#ifndef _YANJUN_LEXICAL_H_
#define _YANJUN_LEXICAL_H_

#include "DFA.h"

#include <map>
#include <string>
#include <vector>
using namespace std;

struct TokenLexDef
{
	TokenLexDef(){re[0]=lexAction[0]=0;}
	char re[REGEXPR_MAX_LENGTH];
	char lexAction[LEXACTION_MAX_LENGTH];
};

class Lexical
{
public:
	map<string,NFA*> marcoDefinitions;
	vector<TokenLexDef> tokenDefs;
	vector<string> startStates;

	DFA dfa;

	Lexical()
	{
		tokenDefs.reserve(TOKENDEF_MAX_COUNT);
		startStates.reserve(STARTSTATE_MAX_COUNT);
		startStates.push_back("INITIAL");

		// why???
		tokenDefs.push_back(TokenLexDef()); // todo: mark empty string
		//tokenDefs[0].re[0]=tokenDefs[0].lexAction[0]=NULL;
	}

	void addStartState(char* sp)
	{
		startStates.push_back(sp);
	}

	void addTokenDef(char* re,char* code)
	{
		tokenDefs.push_back(TokenLexDef());
		strcpy(tokenDefs.back().re,re);
		strcpy(tokenDefs.back().lexAction,code);
	}

	void addMacroDefinition(const char* name,const char* re)
	{
		marcoDefinitions[name]=new NFA(re,marcoDefinitions);
	}

	

	void buildTable()
	{
		dfa.build(this);
		//dfa.minimize();
	}

	void generate();
};



#endif