#ifndef _YANJUN_STATE_H_
#define _YANJUN_STATE_H_

#include <list>
#include <set>
#include <vector>
using namespace std;

#include "Limits.h"
#include "BitSet.h"

class NFAState;
class DFAState;

struct NFAEdge
{
	NFAEdge(int sb, NFAState* next){onSymbol=sb;toState=next;}
	int onSymbol;
	NFAState* toState;
};

struct setNFAEdgeCMP
{
	bool operator()(const NFAEdge& first, const NFAEdge& second) const 
	{        
		return first.toState<second.toState||(first.toState==second.toState&&first.onSymbol<second.onSymbol);  
	} 
};

// typedef set<NFAState*>::iterator EdgeInItType;
// typedef set<NFAEdge,setNFAEdgeCMP>::iterator EdgeItType;
// typedef set<NFAState*>::iterator Edge0ItType;

struct setNFACMP
{
	bool operator()(const NFAState* first, const NFAState* second) const;
};

typedef set<NFAState*,setNFACMP>::iterator EdgeInItType;
typedef set<NFAEdge,setNFAEdgeCMP>::iterator EdgeItType;
typedef set<NFAState*,setNFACMP>::iterator Edge0ItType;

class NFAState
{
public:
	NFAState(int num);
	~NFAState();

	int state;
	
	set<NFAEdge,setNFAEdgeCMP> edges;

	set<NFAState*,setNFACMP> edge0in;//another state link to this
	set<NFAState*,setNFACMP> edge0out;//link to another state
	
	BitSet eBits;
	int acceptStrIndex;
};



//this class is used as set of nfastates
class DFAState
{
public:
	DFAState();
	~DFAState();

	void clear();
	bool addNFAState(NFAState* nfastate);
	void updateAcString();

	int stateNum;
	int acceptStrIndex;
	
	set<NFAState*> nfaStates;

//	set<int> nfaStateNums;
	BitSet nfaBits;

	//set<int> transitSymbols;
};

inline bool DFAState::addNFAState(NFAState* nfastate)
{
	if(nfaBits[nfastate->state]) return false;

	nfaStates.insert(nfastate);
	nfaBits.set(nfastate->state);
	return true;
}


#endif// _YANJUN_STATE_H_