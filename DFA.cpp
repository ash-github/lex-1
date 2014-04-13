#include "DFA.h"
#include <stack>
#include <hash_map>
using namespace std;
using namespace stdext;


#include <time.h>
clock_t timeWHILE=0,timeINSERT=0,timeMOVE=0,timeCLEAR=0;
int totalClose=0;

DFA::~DFA()
{
}

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

DFAState* DFA::eclosure(NFAState* nfastate)
{
	DFAState* res=new DFAState;

	res->nfaStates.insert(nfastate);
	res->nfaStates.insert(nfastate->edge0out.begin(),nfastate->edge0out.end());
	res->nfaBits.or(nfastate->eBits);

	return res;
}

// void DFA::eclosure(NFAState* nfastate,DFAState* res)
// {
// 	res->nfaStates.insert(nfastate);
// 	res->nfaStates.insert(nfastate->edge0out.begin(),nfastate->edge0out.end());
// 	res->nfaBits.or(nfastate->eBits);
// }

void DFA::completeEclosure(DFAState* dfastate,DFAState* res)
{
	set<NFAState*>::iterator it=dfastate->nfaStates.begin();
	for(;it!=dfastate->nfaStates.end();it++)
	{
		res->nfaStates.insert(*it);
		res->nfaStates.insert((*it)->edge0out.begin(),(*it)->edge0out.end());
	}
}

DFAState* DFA::quickEclosure(DFAState* dfastate)
{
	DFAState* res=new DFAState;
	res->nfaBits=dfastate->nfaBits;
	set<NFAState*>::iterator it=dfastate->nfaStates.begin();
	for(;it!=dfastate->nfaStates.end();it++)
		res->nfaBits.or((*it)->eBits);

	return res;
}


struct setDFAStateCMP
{
	bool operator()(const DFAState* first, const DFAState* second) const 
	{			
		return first->nfaBits<second->nfaBits;
	}
};
void DFA::build(LexicalDesc& lexdesc)
{
	clear();

	set<DFAState*,setDFAStateCMP> Dstates;	
	list<DFAState*> workList;	

	nfa.build(lexdesc);

	//0 for dead state
	stateCnt=1;

	for(int i=0;i<lexdesc.startStateCnt;i++)
	{
		DFAState* initialState=eclosure(nfa.nfaStartStates[i]);
		initialState->stateNum=stateCnt;
		initialState->updateAcString();

		dfaAccept[stateCnt++]=initialState->acceptStrIndex;

		Dstates.insert(initialState);
		workList.push_back(initialState);
	}

	DFAState* T,*U;
	DFAState tmpMoveState;

	int nextstatenum=0;

//	DFAState moveon[SYMBOL_ALL_CNT];
	set<NFAState*>::iterator itnfa;
	EdgeItType itnfae;

	while(!workList.empty())
	{			
		T=workList.front();workList.pop_front();

		DFAState moveon[SYMBOL_ALL_CNT];
 		
		clock_t time1=clock();
		for(itnfa=T->nfaStates.begin();itnfa!=T->nfaStates.end();itnfa++)
		{
			for(itnfae=(*itnfa)->edges.begin();itnfae!=(*itnfa)->edges.end();itnfae++)
				moveon[(*itnfae).onSymbol].addNFAState((*itnfae).toState);
		}
 timeMOVE+=clock()-time1;		
		for(int symbol=SYMBOL_BASE;symbol<=SYMBOL_LAST;symbol++)	
		{
			clock_t time1=clock();
			if(moveon[symbol].nfaStates.empty()) continue;
			 timeCLEAR+=clock()-time1;	
			
 time1=clock();
			U=quickEclosure(&moveon[symbol]);

			totalClose+=moveon[symbol].nfaStates.size();
			timeWHILE+=clock()-time1;
		
			time1=clock();
			pair<set<DFAState*,setDFAStateCMP>::iterator,bool> insRes=Dstates.insert(U);
			timeINSERT+=clock()-time1;
			time1=clock();

			if(insRes.second==true)
			{
				clock_t time1=clock();
				completeEclosure(&moveon[symbol],U);
				timeWHILE+=clock()-time1;		

				U->stateNum=stateCnt++;	
				U->updateAcString();

				nextstatenum=U->stateNum;	
				dfaAccept[nextstatenum]=U->acceptStrIndex;
				
				workList.push_back(U);

				
			}
			else
			{
				nextstatenum=(*(insRes.first))->stateNum;					

				delete U;U=NULL;
			}

			dfaTable[T->stateNum][symbol-SYMBOL_BASE]=nextstatenum;			
			//moveon[symbol].clear();
			timeCLEAR+=clock()-time1;
		}
	}

	startState=1;

	//clear
	set<DFAState*,setDFAStateCMP>::iterator itd=Dstates.begin();
	for(;itd!=Dstates.end();itd++)
		delete *itd;
	Dstates.clear();
}



void DFA::clear()
{
	memset(dfaTable,0,sizeof(dfaTable));
	memset(dfaAccept,0,sizeof(dfaAccept));
	stateCnt=0;
}

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
		groups.insert(new setGroupItem(dfaAccept[stt]+1,stt));
		belongto[stt]=dfaAccept[stt]+1;
		if(dfaAccept[stt]+1>maxgroup) maxgroup=dfaAccept[stt]+1;
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
