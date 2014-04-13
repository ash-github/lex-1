#include "State.h"

#include <stack>
using namespace std;

bool setNFACMP::operator()(const NFAState* first, const NFAState* second) const 
{        
	return first->state<second->state;  
} 

NFAState::NFAState(int num)
{
	state=num;acceptStrIndex=0;eBits.reset();
	eBits.set(num);
}
NFAState::~NFAState()
{
	// 	set<NFAEdge*,setNFAEdgeCMP>::iterator it=edges.begin();
	// 	for(;it!=edges.end();it++)
	// 		delete *it;
	edges.clear();
}

DFAState::DFAState()
{
	stateNum=0;acceptStrIndex=0;
	nfaBits.reset();
}
DFAState::~DFAState()
{
	clear();
}
void DFAState::clear()
{
	nfaStates.clear();
	nfaBits.reset();
	//transitSymbols.clear();
	acceptStrIndex=0;
}

void DFAState::updateAcString()
{
	acceptStrIndex=0;
	set<NFAState*>::iterator it=nfaStates.begin();
	int minIndex=TOKENDEF_MAX_COUNT+100;
	for(;it!=nfaStates.end();it++)
	{
		if((*it)->acceptStrIndex>0&&(*it)->acceptStrIndex<minIndex)
			minIndex=(*it)->acceptStrIndex;
	}
	if(minIndex<TOKENDEF_MAX_COUNT+100)
	{
		acceptStrIndex=minIndex;
	}
}
