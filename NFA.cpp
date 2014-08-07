#include "NFA.h"
#include "Lexical.h"

#include <cassert>
#include <stack>
#include <map>
#include <string>
using namespace std;


static __forceinline int getOpPriority(char op)
{	
	switch(op)
	{
	case '(':
		return 0;
	case '|':
		return 1;
	case '&':
		return 2;
	case '?':
	case '+':
	case '*':
	case '{':
		return 3;
	default:
		return -1;
	}
}

static __forceinline char escapeChar(char*& p)
{
	if(*p!='\\') return *p;
	char after=*(++p);
	switch(after)
	{
	case 'b':
		return '\b';
	case 'f':
		return '\f';
	case 'n':
		return '\n';
	case 'r':
		return '\r';
	case 't':
		return '\t';
	default:
		return after;
	}
}

NFA::NFA()
{
	stateCnt=0;start=out=NULL;
	memset(allStates,0,sizeof(allStates));
}
NFA::NFA(const char* re,const map<string,NFA*>& marcoDefs)
{
	stateCnt=0;start=out=NULL;
	memset(allStates,0,sizeof(allStates));
	NFAFrag frag=fromRE(re,marcoDefs);
	start=frag.start;out=frag.out;
}
NFA::~NFA()
{
	clear();
}

void patch(NFAState* a,NFAState* b)
{
	if(!a->eBits[b->stateId])
	{
		a->eedges.push_back(b);
		a->eBits.set(b->stateId);
	}
}


void NFA::pushOp(char op,stack<char>& opstack,stack<NFAFrag>& stateStack,void* extra_data)
{
	if(opstack.empty()) opstack.push(op);
	else
	{
		while(!opstack.empty()&&getOpPriority(opstack.top())>=getOpPriority(op))
		{
			handleOp(opstack.top(),opstack,stateStack);
			opstack.pop();
		}

		if(op=='{')
		{
			int* counts=(int*)extra_data; // [0]:l, [1]: h
			opstack.push(counts[0]);
			opstack.push(counts[1]);
		}
		opstack.push(op);
	}
}

void NFA::handleOp(char op,stack<char>& opstack,stack<NFAFrag>& stateStack)
{
	switch(op)
	{
	case '.':
		{
			NFAState* start=newState();
			NFAState* end=newState();

			// todo: a special node
			for(int symbol=SYMBOL_BASE;symbol<'\n';symbol++)
				start->cedges[symbol]=end;
			for(int symbol='\n'+1;symbol<=SYMBOL_LAST;symbol++)
				start->cedges[symbol]=end;

			stateStack.push(NFAFrag(start,end));
		}
		break;
		// check?
// 	case '?':
// 		{
// 			NFAState* startStateA=stateStack.top();stateStack.pop();
// 			NFAState* endStateA=stateStack.top();stateStack.pop();
// 
// 			//a?*
// 			if(startStateA!=endStateA)
// 				elinkNFAState(startStateA,endStateA);
// 
// 			stateStack.push(endStateA);stateStack.push(startStateA);	
// 		}
// 		break;
// 	case '+':
// 		{
// 			NFAState* startStateA=stateStack.top();stateStack.pop();
// 			NFAState* endStateA=stateStack.top();stateStack.pop();
// 
// 			//a+*
// 			if(startStateA!=endStateA)
// 				elinkNFAState(endStateA,startStateA);
// 
// 			stateStack.push(endStateA);stateStack.push(startStateA);	
// 		}
// 		break;	
	case '?':
		{
// 			NFAState* startStateA=stateStack.top();stateStack.pop();
// 			NFAState* endStateA=stateStack.top();stateStack.pop();

			NFAFrag operand=stateStack.top();stateStack.pop();

			//thompsonClosure
			NFAState* startState=newState();
			NFAState* endState=newState();

			patch(startState,operand.start);
			patch(operand.out,endState);
			patch(startState,endState);

			stateStack.push(NFAFrag(startState,endState));

// 			elinkNFAState(startState,startStateA);
// 			elinkNFAState(startState,endState);
// 			elinkNFAState(endStateA,endState);
// 
// 			stateStack.push(endState);stateStack.push(startState);	
		}
		break;
	case '+':
		{
			NFAFrag operand=stateStack.top();stateStack.pop();

			patch(operand.out,operand.start);

			stateStack.push(operand);
		}
		break;		
	case '|':
		{
			NFAFrag operand1=stateStack.top();stateStack.pop();
			NFAFrag operand2=stateStack.top();stateStack.pop();

			//NFAState* start=newState();
			NFAState* end=newState();

			//patch(start,operand1.start);
			//patch(start,operand2.start);
			patch(operand1.start,operand2.start);
			patch(operand1.out,end);
			patch(operand2.out,end);	

			stateStack.push(NFAFrag(operand1.start,end));
		}
		break;
	case '{':
		{
			NFAFrag operand=stateStack.top();stateStack.pop();

			//if(countl>counth){int temp=counth;counth=countl;countl=temp;}

// 			if(startStateA==endStateA)
// 			{				
// 				stateStack.push(endStateA);stateStack.push(startStateA);	
// 				break;
// 			}

			int counth=opstack.top();opstack.pop();
			int countl=opstack.top();opstack.pop();

			if(counth==0)//infinite
			{
				if(countl==0) //*
				{
					patch(operand.start,operand.out);
					patch(operand.out,operand.start);
					stateStack.push(operand);
					break;
				}
				else if(countl==1) // +
				{					
					patch(operand.out,operand.start);
					stateStack.push(operand);
					break;
				}
				else
				{
					NFAState * start=operand.start,
						*end=operand.out;

					for(int i=1;i<countl;i++)
					{
						NFAFrag new_frag=cloneFrag(operand);

						patch(end,new_frag.start);
						end=new_frag.out;
					}

					///////////check
					patch(end,start);
					stateStack.push(NFAFrag(start,end));
					break;
				}
			}
			else if(counth==1)
			{
				if(countl==0) // ?
				{
					patch(operand.start,operand.out);
					stateStack.push(operand);
					break;
				}
				else if(countl==1)
				{
					stateStack.push(operand);
					break;
				}
				//no else,countl<=counth
			}
			else//counth>1
			{
				int startindex=countl;

				NFAState* lend; // countl end
				NFAState* lastend=operand.out; // for concate

				if(countl==0)
				{
 					lend=operand.start;
					patch(lend,lastend);
				}
				else
				{
					for(int i=1;i<countl;i++)
					{
						NFAFrag new_frag=cloneFrag(operand);
						patch(lastend,new_frag.start);
						lastend=new_frag.out;
					}
					lend=lastend;
				}

				for(int i=max(1,countl);i<counth;i++)
				{
					NFAFrag new_frag=cloneFrag(operand);
					patch(lastend,new_frag.start);
					lastend=new_frag.out;
					patch(lend,lastend);
				}
				
				operand.out=lastend;
				stateStack.push(operand);	
				break;
			}			
			
		}
		break;	
	case '&':		
		{
			NFAFrag operand1=stateStack.top();stateStack.pop();
			NFAFrag operand2=stateStack.top();stateStack.pop();
			patch(operand2.out,operand1.start);

			stateStack.push(NFAFrag(operand2.start,operand1.out));
		}
		break;
	case '*':		
		{
			NFAFrag operand=stateStack.top();stateStack.pop();

			//NFAState* end=newState();
			patch(operand.start,operand.out);
			patch(operand.out,operand.start);
			//patch(operand.out,end);

			//stateStack.push(operand.start,end));
			stateStack.push(operand);
		}
		break;
	default:
		{
			NFAState* start=newState();
			NFAState* end=newState();
			start->cedges[op]=end;
			stateStack.push(NFAFrag(start,end));
		}
		break;
	}
}

// little wrap
struct MacroDefMap
{
	MacroDefMap(const map<string,NFA*>* _mm):_themap(_mm){}
	NFA* get(const string& s)
	{
		map<string,NFA*>::const_iterator it=_themap->find(s);
		return (it==_themap->end()?0:it->second);
	}
	const map<string,NFA*>* _themap;
};

NFAFrag NFA::fromRE(const char* re,const map<string,NFA*>& _marcoDefs)
{	
	// no clear, call many times

	MacroDefMap marcoDefs(&_marcoDefs);

	char* p=(char*)re;

	stack<char> opstack;

	stack<NFAFrag> stateStack;
	int stateId=stateCnt;

	bool canFollowConcat=false;
	while(*p)
	{
		//handle concatenate
		if(*p=='(')
		{
			if(canFollowConcat) pushOp('&',opstack,stateStack);
			canFollowConcat=false;
		}
		else if(*p==')'||
			*p=='?'||
			*p=='+'||
			*p=='*')
		{
			canFollowConcat=true;
		}
		else if(*p=='|')
		{
			canFollowConcat=false;
		}
		else
		{
			//special case for {n,m}
			if(*p!='{')
			{					
				if(canFollowConcat)			
					pushOp('&',opstack,stateStack);
				canFollowConcat=true;
			}						
		}

		switch(*p)
		{
		case '{':			
			{
				if(*(++p)=='}') break;

				int mode=0;//0 for subst, 1 for count

				int countl=0,counth=0;

				char buf[256];char* q=buf;
				while(*p!='}')
				{
					if(*p==',')
					{
						*q='\0';
						countl=atoi(buf);
						mode=1;
						q=buf;
						p++;
					}

					*(q++)=*(p++);
				}
				*q='\0';

				if(mode==1)
				{
					counth=atoi(buf);

					bool check=(counth>=countl&&countl>=0&&counth<=255);
					assert(check);
					if(!check)
					{
						printf("Wrong params for {m,n}\n");
						exit(-1);
					}
					
					int counts[]={countl,counth};

					pushOp('{',opstack,stateStack,counts);
				}
				else
				{
					if(canFollowConcat) pushOp('&',opstack,stateStack);

					NFA* macrodef=marcoDefs.get(buf);
					if(macrodef!=NULL)
					{
						stateStack.push(cloneFrag(*macrodef));
					}
				}

				canFollowConcat=true;
			}
			break;
			
		case '"':	
			{
				if(*(++p)=='"') break;
				
				NFAState* start=newState();
				NFAState* last=start;

				do
				{
					NFAState* newst=newState();
					last->cedges[escapeChar(p)]=newst;
					last=newst;
				}while(*(++p)!='"'); // escaped " already consumed;
				
				stateStack.push(NFAFrag(start,last));					
			}
			break;

		case '[':
			{
				if(*(++p)==']') break;

				int mode=0;

				if(*p=='^') {mode=1;p++;}

				NFAState* startState=newState();
				NFAState* endState=newState();

				if(mode==1) // exclude
				{
					char lastC=*p;
					bool exclude[SYMBOL_CNT]={false};

					while(*p!=']')
					{
						if(*p=='-'&&*(p-1)!='['&&*(p+1)!=']')
						{
							lastC++;
							while(lastC<=*(p+1))
								exclude[lastC++-SYMBOL_BASE]=true;
							++p;
						}
						else 
							exclude[escapeChar(p)-SYMBOL_BASE]=true;

						lastC=*p;
						p++;
					}

					// todo: a special node
					for(int symbol=0;symbol<SYMBOL_CNT;symbol++)
						if(!exclude[symbol]) 
						{
							startState->cedges[symbol+SYMBOL_BASE]=endState;
						}
				}
				else
				{
					char lastC=*p;

					while(*p!=']')
					{
						if(*p=='-'&&*(p-1)!='['&&*(p+1)!=']')
						{
							lastC++;
							while(lastC<=*(p+1))
							{
								startState->cedges[lastC++]=endState;
							}
							++p;
						}
						else
						{
							startState->cedges[escapeChar(p)]=endState;
						}

						lastC=*p;
						p++;
					}
				}

				stateStack.push(NFAFrag(startState,endState));
			}
			break;
		
		case '\\':			
			{
				NFAState* startState=newState();
				NFAState* endState=newState();
				startState->cedges[escapeChar(p)]=endState;
				stateStack.push(NFAFrag(startState,endState));
			}
			break;

		case '(':
			opstack.push('(');
			break;
		case ')':
			// todo: error handling
			while(!opstack.empty()&&opstack.top()!='(')
			{
				handleOp(opstack.top(),opstack,stateStack);
				opstack.pop();
			}
			opstack.pop();
			break;

		case '|':
		case '?':
		case '+':
		case '*':
			pushOp(*p,opstack,stateStack);
			break;

		default:
			//include .
			handleOp(*p,opstack,stateStack);//normal
			break;
		}
		p++;
	}
	while(!opstack.empty())
	{
		handleOp(opstack.top(),opstack,stateStack);
		opstack.pop();
	}

	
	NFAFrag frag=stateStack.top();stateStack.pop();
	//start=frag.start;
	//out=frag.out;
	return frag;

	// todo: check state
	//stateStack should be empty
}

void NFA::clear()
{
	for(int i=0;i<stateCnt;i++) delete allStates[i];
	memset(allStates,0,sizeof(allStates));
	stateCnt=0;start=out=NULL;
}

NFAFrag NFA::cloneFrag(const NFAFrag& frag)
{
	list<NFAState*> workList;
	BitSet visited;

	workList.push_back(frag.start);
	visited.set(frag.start->stateId);	

	map<NFAState*,NFAState*> old2new;
	typedef map<NFAState*,NFAState*>::iterator Old2NewIter;

	while(!workList.empty())
	{
		NFAState* cur_state=workList.front();workList.pop_front();

		for(map<int,NFAState*>::iterator it=cur_state->cedges.begin();it!=cur_state->cedges.end();++it)
		{
			NFAState* nextstate=0;
			
			Old2NewIter findit;
			if((findit=old2new.find(it->second))==old2new.end())
			{
				old2new[it->second]=nextstate=newState();
			}
			else
				nextstate=findit->second;

			cur_state->cedges[it->first]=nextstate;

			if(!visited[nextstate->stateId])
				workList.push_back(nextstate);
		}

		for(NFAStateArrIter it=cur_state->eedges.begin();it!=cur_state->eedges.end();++it)
		{
			NFAState* nextstate=0;

			Old2NewIter findit;
			if((findit=old2new.find(*it))==old2new.end())
			{
				old2new[*it]=nextstate=newState();
			}
			else
				nextstate=findit->second;

			patch(cur_state,nextstate);

			if(!visited[nextstate->stateId])
				workList.push_back(nextstate);
		}
	}

	NFAFrag ret(old2new[frag.start],old2new[frag.out]);
	return ret;
}


/////////////////////////////////////////////
void NFA::build(const LexEnv* lexEnv)
{
	clear();

	map<string,int> mapStartStates;
	typedef map<string,int>::iterator String2IntIter;

	for(int i=0,iLen=lexEnv->startStates.size();i<iLen;i++)
	{
		allStates[i]=nfaStartStates[i]=newState();
		mapStartStates[lexEnv->startStates[i]]=i;
	}

// 	mapStartStates.insert(pair<char*,int>("COMMENT",1));
// 	mapStartStates.insert(pair<char*,int>("QUOTE",2));	

// 	if(lexDesc.tokenDefCnt<=1) return;
// 	if(lexDesc.tokenDefCnt==2)
// 	{
// 		fromRE(lexDesc.tokenDefs[1].re,lexDesc.auxNFAPart);
// 		endState->acceptStrIndex=1;	
// 		return;
// 	}
	
	int tokenDefCnt=lexEnv->tokenDefs.size();
	for(int i=1;i<tokenDefCnt;i++) // skip dummy 0
	{
		int startindex=0;
		const char* p=lexEnv->tokenDefs[i].re;
		if(lexEnv->tokenDefs[i].re[0]=='<')
		{
			const char* q=++p;
			while(*p&&*p!='>') ++p;

			string sstate_name(q,p-q);

			String2IntIter it=mapStartStates.find(sstate_name);
			if(it!=mapStartStates.end())
				startindex=it->second;
			//else
			//report it if no such start-state, and force it to 0 to continue

			if(*p=='>') ++p;
		}

		NFAFrag frag=fromRE(p,lexEnv->marcoDefinitions);
		frag.out->acceptStrIndex=i; // >0
		patch(nfaStartStates[startindex],frag.start);
	}

	start=nfaStartStates[0];
	// out doesn't matter
	out=0;//?

	printf("# NFA states: %d\n",stateCnt);
}

