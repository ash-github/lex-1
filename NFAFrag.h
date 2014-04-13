#ifndef _YANJUN_NFAFRAG_H_
#define _YANJUN_NFAFRAG_H_

#include "State.h"
#include <stack>
using namespace std;

class NFAFrag
{
public:
	NFAFrag(){startState=endState=NULL;}
	NFAFrag(NFAState* start,NFAState* end){startState=start;endState=end;}
	virtual void getCopy(NFAState** dstStates,int& startpos,NFAState*& sState,NFAState*& eStatet,bool startfrom0=false);

	NFAState* startState;
	NFAState* endState;
};

class AuxNFAPart;
class NFAPart:public NFAFrag
{
public:
	NFAPart();
	//NFAPart(const char* re);
	~NFAPart();

	void fromRE(const char* re,AuxNFAPart& auxNFAPart);
	void clear();


	//void getCopy(NFAState** dstStates,int& startpos,NFAState*& sState,NFAState*& eState);

	NFAPart& operator=(const NFAPart& anpart);

	int stateCnt;
	NFAState* allStates[NFASTATE_MAX_COUNT];

private:
	void pushOp(char op,stack<char>& opstack,stack<NFAState*>& stateStack,int& stateNum);
	void handleOp(char op,stack<NFAState*>& stateStack,int& stateNum);
};

#endif