#include "DFA.h"
#include "lexical.h"

#include <stack>
//#include <hash_map>
#include <set>
using namespace std;
//using namespace stdext;


DFA::~DFA()
{
}

/*
string DFA::recognize(TokenLexDef tokenDefs[],const char* str)
{
	//return "hh";	
	int s=startState;
	char* c=(char*)str;

	stack<int> traceStack;

	while(*c&&dfaTable[s][*c-SYMBOL_BASE])
	{
		traceStack.push(s);
		s=dfaTable[s][*c-SYMBOL_BASE];
		c++;
	}

// 	if(*c=='\0')
// 	{
		if(dfaAccept[s]) return string(tokenDefs[dfaAccept[s]].re);
		else return string("noaccept");
// 	}
// 	else
// 	{
// 		int backtract=0;
// 		while(!traceStack.empty())
// 		{
// 			backtract=traceStack.top();traceStack.pop();
// 			if(dfaAccept[backtract]>0) return string(g_TokenDefs[dfaAccept[backtract]].re);
// 		}
// 		return string("noaccept");			
// 	}

}
*/

// pop the nfaStates array
void eclosure(NFAState* nfastate,DFAState* res)
{
	if(res->nfaBits[nfastate->stateId]) return;
	
	res->nfaBits.set(nfastate->stateId);
	res->nfaStates.push_back(nfastate);
	res->updateAcString(nfastate);

	for(NFAStateArrIter it=nfastate->eedges.begin();it!=nfastate->eedges.end();++it)
	{
		eclosure(*it,res);
	}
}
void eclosure(DFAState* start,DFAState* res)
{
	res->nfaStates.clear();
	res->nfaBits.reset();
	for(NFAStateArrIter it=start->nfaStates.begin();it!=start->nfaStates.end();it++)
	{
		eclosure(*it,res);
	}
}

// only set epsilon bits
void quickEclosure(NFAState* start,DFAState* res,BitSet& visited)
{
	if(visited[start->stateId]) return;

	visited.set(start->stateId);
	res->nfaBits.or(start->eBits);
	for(NFAStateArrIter it=start->eedges.begin();it!=start->eedges.end();++it)
	{
		quickEclosure(*it,res,visited);
	}
}
void quickEclosure(DFAState* start,DFAState* res)
{
	res->nfaBits=start->nfaBits;
	BitSet visited;
	for(NFAStateArrIter it=start->nfaStates.begin();it!=start->nfaStates.end();it++)
	{
		quickEclosure(*it,res,visited);
	}
}


struct setDFAStateCMP
{
	bool operator()(const DFAState* first, const DFAState* second) const 
	{			
		return first->nfaBits<second->nfaBits;
	}
};
void DFA::build(const LexEnv* lexEnv)
{
	clear();

	set<DFAState*,setDFAStateCMP> Dstates;
	typedef set<DFAState*,setDFAStateCMP>::iterator DFASetIter;

	list<DFAState*> workList;	

	printf("building nfa...\n");
	nfa.build(lexEnv);

	printf("converting to dfa...\n");

	//0 for dead state
	stateCnt=1;

	for(int i=0,iLen=lexEnv->startStates.size();i<iLen;i++)
	{
		DFAState* initialState=newState();
		eclosure(nfa.nfaStartStates[i],initialState);
		//initialState->stateId=stateCnt;
		//initialState->updateAcString();

		dfaAccept[initialState->stateId]=initialState->acceptStrIndex;

		Dstates.insert(initialState);
		workList.push_back(initialState);
	}

	DFAState* U=newState();

	while(!workList.empty())
	{			
		DFAState* T=workList.front();workList.pop_front();

		DFAState moveon[SYMBOL_ALL_CNT];// temporary, stateId doesn't matter

		for(NFAStateArrIter itnfa=T->nfaStates.begin();itnfa!=T->nfaStates.end();itnfa++)
		{
			for(map<int,NFAState*>::iterator edge_it=(*itnfa)->cedges.begin();edge_it!=(*itnfa)->cedges.end();edge_it++)
				moveon[edge_it->first].addNFAState(edge_it->second);
		}
	
		for(int symbol=SYMBOL_BASE;symbol<=SYMBOL_LAST;symbol++)	
		{
			if(moveon[symbol].nfaStates.empty()) continue;

			quickEclosure(&moveon[symbol],U);

			//totalClose+=moveon[symbol].nfaStates.size();
			int transit_id=U->stateId;

			pair<DFASetIter,bool> insRes=Dstates.insert(U); // unique DFAState
			if(insRes.second==true)
			{
				eclosure(&moveon[symbol],U);

				//U->stateId=stateCnt++;	
				//U->updateAcString();

				transit_id=U->stateId;	
				dfaAccept[transit_id]=U->acceptStrIndex;
				
				workList.push_back(U);

				U=newState();
			}
			else
			{
				transit_id=(*(insRes.first))->stateId;
			}

			dfaTable[T->stateId][symbol-SYMBOL_BASE]=transit_id;			
			//moveon[symbol].clear();
		}
	}

	startState=1;

	//clear
	set<DFAState*,setDFAStateCMP>::iterator itd=Dstates.begin();
	for(;itd!=Dstates.end();itd++)
		delete *itd;
	if(U) delete U;

	printf("#\n");
}



void DFA::clear()
{
	memset(dfaTable,0,sizeof(dfaTable));
	memset(dfaAccept,0,sizeof(dfaAccept));
	stateCnt=0;
}


// void DFA::parti(int c_start,vector<int>& group,int* old_belong_to,)
// {
// 	for(;c_start<=SYMBOL_LAST;++c_start)
// 	{
// 		for()
// 		dfaTable[group
// 	}
// }
// 
// 
// void new_partition(vector<int>& ,new_part,belong_to)
// {
// 	int map_same[i][j];
// 	for(int i=0;i<old_part.size()-1;++i)
// 		for(int j=i+1;j<old_part.size();++j)
// 		{
// 			if(old_part[i])
// 		}
// }


/*

struct setGroupItem
{
	setGroupItem(int g,int s)
	{
		group=g;state=s;
	}
	int group;
	int state;
};

struct setGroupCMP
{
	bool operator()(const setGroupItem* first, const setGroupItem* second) const 
	{        
		return first->group<second->group||(first->group==second->group&&first->state<second->state);  
	} 
};


void new_partition(vector<NFAState*> ,new_part,belong_to)
{
	int map_same[i][j];
	for(int i=0;i<old_part.size()-1;++i)
		for(int j=i+1;j<old_part.size();++j)
		{
			if(old_part[i])
		}
}

void DFA::minimize()
{
	set<setGroupItem*,setGroupCMP> groups;

	int* belongto=new int[DFASTATE_MAX_COUNT];
	int* represent=new int[DFASTATE_MAX_COUNT];
	memset(belongto,0,sizeof(belongto));
	memset(represent,0,sizeof(represent));

	int maxgroup=-1000;

	belongto[0]=0;
	for(int stt=1;stt<=stateCnt;stt++)//initial partion
	{
		int g=dfaAccept[stt]+1;
		belongto[stt]=g;
		groups.insert(new setGroupItem(g,stt));
		if(g>maxgroup) maxgroup=dfaAccept[stt]+1;
	}

	maxgroup++;

	int laststartpoint=maxgroup-1;
	int lastgroupcnt=maxgroup;
	hash_map<int,int> transitToGroup;
	typedef pair<int,int> hashitem;

	int curGroup=1;
	bool bOnlyOne=false;
	while(true)
	{
		set<setGroupItem*,setGroupCMP>::iterator gt,gtStart,gtEnd,tmpgt;

		setGroupItem curPos(curGroup,0);

		gtStart=groups.lower_bound(&curPos);
		if(gtStart==groups.end())
		{
			curGroup=1;
			curPos.group=curGroup;
			gtStart=groups.lower_bound(&curPos);
		}
		curPos.group=(*gtStart)->group;
		curPos.group++;
		gtEnd=groups.lower_bound(&curPos);

		tmpgt=gtStart;
		if(tmpgt==gtEnd||++tmpgt==gtEnd) 
		{
			if(curGroup==laststartpoint) break;
			curGroup++;continue;
		}

		for(int symbol=0;symbol<SYMBOL_CNT;symbol++)
		{
			bool needCalcEnd=false;
			transitToGroup.clear();
			gt=gtStart;
			int curState=(*gtStart)->state;
			int initGroup=belongto[dfaTable[(*gtStart)->state][symbol]];
			transitToGroup.insert(hashitem(initGroup,curGroup));
			gt++;
			while(gt!=gtEnd)
			{
				curState=(*gt)->state;
				int transitto=belongto[dfaTable[(*gt)->state][symbol]];
				if(transitToGroup.find(transitto)==transitToGroup.end())
					transitToGroup.insert(hashitem(transitto,maxgroup++));				
				
				int dstGroup=transitToGroup[transitto];
				if(dstGroup!=curGroup)
				{		
					tmpgt=gt;++tmpgt;
					if(tmpgt==groups.end())
						needCalcEnd=true;	

					int saveState=(*gt)->state;
					delete *gt;
					groups.erase(gt);
					groups.insert(new setGroupItem(dstGroup,saveState));

					belongto[saveState]=dstGroup;

					gt=tmpgt;

					if(needCalcEnd)
					{
						curPos.group=curGroup+1;curPos.state=0;
						gtEnd=groups.lower_bound(&curPos);
						needCalcEnd=false;						
					}

					tmpgt=gtStart;
					if(gtEnd==++tmpgt) bOnlyOne=true;
				}
				else
					gt++;
				
			}
			if(bOnlyOne)
			{
				bOnlyOne=false;
				break;
			}
		}		
		
		if(lastgroupcnt==maxgroup)
		{
			if(curGroup==laststartpoint) break;
		}
		else
		{
			lastgroupcnt=maxgroup;
			laststartpoint=curGroup;//-1;
			//if(laststartpoint==0) laststartpoint=maxgroup-1;
		}
		
		curGroup++;
	}

	memset(belongto,0,sizeof(belongto));
	int finalgroupcnt=1;
	int newStartState=-1;

	setGroupItem cmpKey(1,0);
	while(true)
	{		
		set<setGroupItem*,setGroupCMP>::iterator gt,gtStart,gtEnd;
		gtStart=groups.lower_bound(&cmpKey);
		if(gtStart==groups.end()) break;
		cmpKey.group=(*gtStart)->group;
		cmpKey.group++;
		gtEnd=groups.lower_bound(&cmpKey);

		represent[finalgroupcnt]=(*gtStart)->state;

		for(gt=gtStart;gt!=gtEnd;gt++)
		{			
			belongto[(*gt)->state]=finalgroupcnt;
			if((*gt)->state==startState) newStartState=finalgroupcnt;
		}
		finalgroupcnt++;
	}


	int newdfaTable[DFASTATE_MAX_COUNT][SYMBOL_CNT];
	int newdfaAccept[DFASTATE_MAX_COUNT];
	memset(newdfaTable,0,sizeof(newdfaTable));
	memset(newdfaAccept,0,sizeof(newdfaAccept));


	for(int g=1;g<finalgroupcnt;g++)
	{			
		for(int sb=0;sb<SYMBOL_CNT;sb++)
		{
			newdfaTable[g][sb]=belongto[dfaTable[represent[g]][sb]];
			newdfaAccept[g]=dfaAccept[represent[g]];
		}
	}

	memcpy(dfaTable,newdfaTable,sizeof(dfaTable));
	memcpy(dfaAccept,newdfaAccept,sizeof(dfaAccept));	
	stateCnt=finalgroupcnt-1;
	startState=newStartState;

	set<setGroupItem*,setGroupCMP>::iterator git=groups.begin();
	for(;git!=groups.end();git++)
		delete *git;

	groups.clear();	

	delete [] represent;
	delete [] belongto;
}
*/