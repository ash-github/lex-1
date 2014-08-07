#ifndef _YANJUN_STATE_H_
#define _YANJUN_STATE_H_

#include <map>
#include <vector>
using namespace std;

#include "define.h"
#include "BitSet.h"


class NFAState
{
public:
	NFAState(int id)
	{
		stateId=id;acceptStrIndex=0;eBits.reset();
		eBits.set(id);
	}
	~NFAState()
	{
		// 	set<NFAEdge*,setNFAEdgeCMP>::iterator it=edges.begin();
		// 	for(;it!=edges.end();it++)
		// 		delete *it;
		//edges.clear();
	}

	int stateId;

	// from the way we build it, one char one next
	map<int,NFAState*> cedges;
	vector<NFAState*> eedges; // epsilon
	BitSet eBits; // fast check if a state is in its eclosure

	int acceptStrIndex;
};


//this class is used as set of nfastates
class DFAState
{
public:
	DFAState()
	{
		stateId=-1;acceptStrIndex=0;
		nfaBits.reset();
	}
	DFAState(int id)
	{
		stateId=id;acceptStrIndex=0;
		nfaBits.reset();
	}
	~DFAState()
	{
		clear();
	}

	void clear()
	{
		nfaStates.clear();
		// todo: delete
		nfaBits.reset();
		//transitSymbols.clear();
		acceptStrIndex=0;
	}

	bool addNFAState(NFAState* nfastate)
	{
		if(nfaBits[nfastate->stateId]) return false;

		nfaStates.push_back(nfastate);
		nfaBits.set(nfastate->stateId);
		return true;
	}
	void updateAcString(NFAState* nfastate)
	{
		if(nfastate->acceptStrIndex>0&&nfastate->acceptStrIndex<acceptStrIndex)
			acceptStrIndex=nfastate->acceptStrIndex;
	}

	int stateId;
	int acceptStrIndex;
	
	vector<NFAState*> nfaStates;
	BitSet nfaBits;
};

typedef vector<NFAState*>::iterator NFAStateArrIter;


#endif// _YANJUN_STATE_H_