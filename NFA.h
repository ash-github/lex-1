#ifndef _YANJUN_NFA_H_
#define _YANJUN_NFA_H_

#include <cstdlib>
#include <list>
#include <stack>
#include <string>
using namespace std;

#include "define.h"
#include "State.h"

class NFAFrag
{
public:
	NFAFrag():start(0),out(0){}
	NFAFrag(NFAState* st,NFAState* ed):start(st),out(ed){}

	//virtual void getCopy(NFAState** dstStates,int& startpos,NFAState*& sState,NFAState*& eStatet,bool startfrom0=false);

	NFAState* start;
	NFAState* out;
};


class NFA:public NFAFrag
{
public:
	NFA();
	NFA(const char* re,const map<string,NFA*>& marcoDefs);
	~NFA();

	NFAFrag fromRE(const char* re,const map<string,NFA*>& marcoDefs);
	void clear();


	//void getCopy(NFAState** dstStates,int& startpos,NFAState*& sState,NFAState*& eState);

	NFA& operator=(const NFA& anpart);

	int stateCnt;
	NFAState* allStates[NFASTATE_MAX_COUNT];

	NFAState* newState(/*int& id*/)
	{
		if(stateCnt>=NFASTATE_MAX_COUNT)
		{
			printf("too many nfa states");
			exit(-1);
			return 0;
		}
		NFAState* st=new NFAState(stateCnt);
		allStates[stateCnt++]=st;
		return st;
	}

	NFAFrag cloneFrag(const NFAFrag& frag);


	void build(const LexEnv* lexEnv);

	NFAState* nfaStartStates[STARTSTATE_MAX_COUNT];

private:
	void pushOp(char op,stack<char>& opstack,stack<NFAFrag>& stateStack,void* extra=0);
	void handleOp(char op,stack<char>& opstack,stack<NFAFrag>& stateStack);
};


#endif//_YANJUN_NFA_H_