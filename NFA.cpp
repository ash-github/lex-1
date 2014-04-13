#include "NFA.h"
#include <cassert>
#include <stack>
#include <hash_map>
#include <map>
#include <string>
using namespace std;
using namespace stdext;


static int getOpPriority(char op)
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

static char handleTransChar(char after)
{
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

NFAPart::NFAPart()
{
	stateCnt=0;startState=endState=NULL;
	memset(allStates,0,sizeof(allStates));
}
NFAPart::~NFAPart()
{
	clear();
}

// static void elinkNFAState(NFAState* a,NFAState* b)
// {
// 	if(a==b) return;
// 	
// 	a->edge0out.insert(b);
// 	a->eBits.set(b->state);	
// 	a->edge0out.insert(b->edge0out.begin(),b->edge0out.end());
// 	a->eBits.or(b->eBits);
// 
// 	b->edge0in.insert(a);
// 
// 	EdgeInItType it=a->edge0in.begin();
// 	for(;it!=a->edge0in.end();it++)
// 	{
// 		(*it)->edge0out.insert(b);
// 		(*it)->eBits.set(b->state);	
// 		(*it)->edge0out.insert(b->edge0out.begin(),b->edge0out.end());
// 		(*it)->eBits.or(b->eBits);
// 
// 		b->edge0in.insert(*it);
// 	}
// }

static void recursiveLink(NFAState* a,NFAState* b)
{	
	a->edge0out.insert(b);
	a->eBits.set(b->state);	
	
	a->edge0out.insert(b->edge0out.begin(),b->edge0out.end());
	a->eBits.or(b->eBits);
	EdgeInItType it=a->edge0in.begin();
	for(;it!=a->edge0in.end();it++)
	{
		recursiveLink(*it,b);
	}
}

static void elinkNFAState(NFAState* a,NFAState* b)
{
	if(a==b) return;
	recursiveLink(a,b);
	b->edge0in.insert(a);
}

void NFAPart::pushOp(char op,stack<char>& opstack,stack<NFAState*>& stateStack,int& stateNum)
{
	if(opstack.empty()) opstack.push(op);
	else
	{
		while(!opstack.empty()&&getOpPriority(opstack.top())>=getOpPriority(op))
		{
			handleOp(opstack.top(),stateStack,stateNum);
			opstack.pop();
		}
		opstack.push(op);
	}
}

int countl=0,counth=0;
void NFAPart::handleOp(char op,stack<NFAState*>& stateStack,int& stateNum)
{
	switch(op)
	{
	case '.':
		{
			NFAState* startState=new NFAState(stateNum);allStates[stateNum++]=startState;
			NFAState* endState=new NFAState(stateNum);allStates[stateNum++]=endState;

			for(int symbol=SYMBOL_BASE;symbol<'\n';symbol++)
				startState->edges.insert(NFAEdge(symbol,endState));

			for(int symbol='\n'+1;symbol<=SYMBOL_LAST;symbol++)
				startState->edges.insert(NFAEdge(symbol,endState));

			stateStack.push(endState);stateStack.push(startState);
		}
		break;
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
			NFAState* startStateA=stateStack.top();stateStack.pop();
			NFAState* endStateA=stateStack.top();stateStack.pop();
			//thompsonClosure
			NFAState* startState=new NFAState(stateNum);allStates[stateNum++]=startState;
			NFAState* endState=new NFAState(stateNum);allStates[stateNum++]=endState;

			elinkNFAState(startState,startStateA);
			elinkNFAState(startState,endState);
			elinkNFAState(endStateA,endState);

			stateStack.push(endState);stateStack.push(startState);	
		}
		break;
	case '+':
		{
			NFAState* startStateA=stateStack.top();stateStack.pop();
			NFAState* endStateA=stateStack.top();stateStack.pop();
			//thompsonClosure
			NFAState* startState=new NFAState(stateNum);allStates[stateNum++]=startState;
			NFAState* endState=new NFAState(stateNum);allStates[stateNum++]=endState;

			elinkNFAState(startState,startStateA);
			elinkNFAState(endStateA,startStateA);
			elinkNFAState(endStateA,endState);

			stateStack.push(endState);stateStack.push(startState);	
		}
		break;		
	case '|':
		{
			NFAState* startStateA=stateStack.top();stateStack.pop();
			NFAState* endStateA=stateStack.top();stateStack.pop();
			NFAState* startStateB=stateStack.top();stateStack.pop();
			NFAState* endStateB=stateStack.top();stateStack.pop();
			//thompsonOr
			NFAState* startState=new NFAState(stateNum);allStates[stateNum++]=startState;
			NFAState* endState=new NFAState(stateNum);allStates[stateNum++]=endState;

			elinkNFAState(startState,startStateA);
			elinkNFAState(startState,startStateB);
			elinkNFAState(endStateA,endState);
			elinkNFAState(endStateB,endState);	

			stateStack.push(endState);stateStack.push(startState);
		}
		break;
	case '{':
		{
			NFAState* startStateA=stateStack.top();stateStack.pop();
			NFAState* endStateA=stateStack.top();stateStack.pop();

			//if(countl>counth){int temp=counth;counth=countl;countl=temp;}

			if(startStateA==endStateA)
			{				
				stateStack.push(endStateA);stateStack.push(startStateA);	
				break;
			}

			NFAFrag frag(startStateA,endStateA);
			NFAState* newStart,*newEnd,*lastEnd,*firstStart;			

			if(counth==0)//infinite
			{
				if(countl==0)
				{	
					elinkNFAState(startStateA,endStateA);
					elinkNFAState(endStateA,startStateA);
					stateStack.push(startStateA);stateStack.push(startStateA);
					break;
				}
				else if(countl==1)
				{					
					elinkNFAState(endStateA,startStateA);
					stateStack.push(endStateA);stateStack.push(startStateA);
					break;
				}
				else
				{
					frag.getCopy(allStates,stateNum,newStart,newEnd);
					lastEnd=newEnd;firstStart=newStart;

					for(int i=1;i<countl;i++)
					{
						frag.getCopy(allStates,stateNum,newStart,newEnd);
						elinkNFAState(lastEnd,newStart);	
						lastEnd=newEnd;
					}
					///////////check
					elinkNFAState(startStateA,endStateA);
					elinkNFAState(endStateA,startStateA);
					elinkNFAState(lastEnd,startStateA);

					stateStack.push(endStateA);stateStack.push(firstStart);
					break;
				}
				
			}
			else if(counth==1)
			{
				if(countl==0)
				{
					elinkNFAState(startStateA,endStateA);
					stateStack.push(endStateA);stateStack.push(startStateA);	
					break;
				}
				else if(countl==1)
				{
					stateStack.push(endStateA);stateStack.push(startStateA);	
					break;
				}
				//no else,countl<=counth
			}
			else//counth>1
			{
				int startindex=countl;
				if(countl==0)
				{
					frag.getCopy(allStates,stateNum,newStart,newEnd);
					elinkNFAState(newStart,newEnd);
					lastEnd=newEnd;
					startindex=1;
					firstStart=newStart;
				}
				else
				{
					frag.getCopy(allStates,stateNum,newStart,newEnd);
					lastEnd=newEnd;firstStart=newStart;
					startindex=countl;

					for(int i=1;i<countl;i++)
					{
						frag.getCopy(allStates,stateNum,newStart,newEnd);
						elinkNFAState(lastEnd,newStart);	
						lastEnd=newEnd;
					}
				}

				for(int i=startindex;i<counth-1;i++)
				{
					frag.getCopy(allStates,stateNum,newStart,newEnd);
					elinkNFAState(lastEnd,newStart);				
					elinkNFAState(lastEnd,newEnd);
					lastEnd=newEnd;
				}

				elinkNFAState(lastEnd,startStateA);
				elinkNFAState(lastEnd,endStateA);

				stateStack.push(endStateA);stateStack.push(firstStart);	
				break;
			}			
			
		}
		break;	
	case '&':		
		{
			NFAState* startStateA=stateStack.top();stateStack.pop();
			NFAState* endStateA=stateStack.top();stateStack.pop();
			NFAState* startStateB=stateStack.top();stateStack.pop();
			NFAState* endStateB=stateStack.top();stateStack.pop();

			elinkNFAState(endStateB,startStateA);	

			stateStack.push(endStateA);stateStack.push(startStateB);
		}
		break;
// 	case '*':		
// 		{
// 			NFAState* startStateA=stateStack.top();stateStack.pop();
// 			NFAState* endStateA=stateStack.top();stateStack.pop();
// 
// 			elinkNFAState(startStateA,endStateA);
// 			elinkNFAState(endStateA,startStateA);
// 
// 			stateStack.push(endStateA);stateStack.push(startStateA);			
// 		}
// 		break;
	case '*':		
		{
			NFAState* startStateA=stateStack.top();stateStack.pop();
			NFAState* endStateA=stateStack.top();stateStack.pop();
			//thompsonClosure
			NFAState* startState=new NFAState(stateNum);allStates[stateNum++]=startState;
			NFAState* endState=new NFAState(stateNum);allStates[stateNum++]=endState;

			elinkNFAState(startState,startStateA);	
			elinkNFAState(startState,endState);	
			elinkNFAState(endStateA,startStateA);	
			elinkNFAState(endStateA,endState);	

			stateStack.push(endState);stateStack.push(startState);			
		}
		break;
	default:
		{
			NFAState* startState=new NFAState(stateNum);allStates[stateNum++]=startState;
			NFAState* endState=new NFAState(stateNum);allStates[stateNum++]=endState;
			startState->edges.insert(NFAEdge(op,endState));
			stateStack.push(endState);stateStack.push(startState);
		}
		break;
	}
}

void NFAPart::fromRE(const char* re,AuxNFAPart& auxNFAPart)
{	
	//clear();

	char* p=(char*)re;

	stack<char> opstack;

	stack<NFAState*> stateStack;
	int stateNum=stateCnt;

	bool canFollowConcat=false;
	while(*p)
	{
		//handle concate
		if(*p=='(')
		{
			if(canFollowConcat) pushOp('&',opstack,stateStack,stateNum);
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
					pushOp('&',opstack,stateStack,stateNum);
				canFollowConcat=true;
			}						
		}

		switch(*p)
		{
		case '{':			
			{
				if(*(++p)=='}') break;

				int mode=0;//0 for subst, 1 for count

				int l=0,h=0;

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
					assert(counth>=countl);
					pushOp('{',opstack,stateStack,stateNum);
				}
				else
				{
					if(canFollowConcat) pushOp('&',opstack,stateStack,stateNum);

					NFAPart* auxPart=auxNFAPart.searchPart(buf);
					if(auxPart!=NULL)
					{
						//copy
						NFAState* startState=NULL;
						NFAState* endState=NULL;
						int cnt=0;
						auxPart->getCopy(allStates,stateNum,startState,endState,true);						

						stateStack.push(endState);stateStack.push(startState);
					}
				}

				canFollowConcat=true;
			}
			break;
			
		case '"':	
			{
				if(*(++p)=='"') break;
				
				NFAState* startState=NULL;
				NFAState* endState=NULL;
				NFAState* leftEnd,*rightEnd;

				leftEnd=new NFAState(stateNum);allStates[stateNum++]=leftEnd;
				rightEnd=new NFAState(stateNum);allStates[stateNum++]=rightEnd;
				if(*p=='\\') 
					leftEnd->edges.insert(NFAEdge(handleTransChar(*(++p)),rightEnd));
				else
					leftEnd->edges.insert(NFAEdge(*p,rightEnd));


				startState=leftEnd;

				while(*(++p)!='"')
				{
					leftEnd=rightEnd;
					rightEnd=new NFAState(stateNum);allStates[stateNum++]=rightEnd;
					if(*p=='\\')
						leftEnd->edges.insert(NFAEdge(handleTransChar(*(++p)),rightEnd));
					else
						leftEnd->edges.insert(NFAEdge(*p,rightEnd));
				}

				endState=rightEnd;

				stateStack.push(endState);stateStack.push(startState);					
			}
			break;

		case '[':
			{
				if(*(++p)==']') break;

				int mode=0;

				if(*p=='^') {mode=1;p++;}

				NFAState* startState=new NFAState(stateNum);allStates[stateNum++]=startState;
				NFAState* endState=new NFAState(stateNum);allStates[stateNum++]=endState;

				if(mode==1)
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
						else if(*p=='\\')	
							exclude[handleTransChar(*(++p))-SYMBOL_BASE]=true;
						else
							exclude[*p-SYMBOL_BASE]=true;

						lastC=*p;
						p++;
					}

					for(int symbol=0;symbol<SYMBOL_CNT;symbol++)
						if(!exclude[symbol]) 
						{
							startState->edges.insert(NFAEdge(symbol+SYMBOL_BASE,endState));
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
								startState->edges.insert(NFAEdge(lastC++,endState));
							}
							++p;
						}
						else if(*p=='\\')					
						{
							startState->edges.insert(NFAEdge(handleTransChar(*(++p)),endState));
						}
						else
						{
							startState->edges.insert(NFAEdge(*p,endState));
						}

						lastC=*p;
						p++;
					}
				}	

				stateStack.push(endState);stateStack.push(startState);
			}	
			break;
		
		case '\\':			
			{
				NFAState* startState=new NFAState(stateNum);allStates[stateNum++]=startState;
				NFAState* endState=new NFAState(stateNum);allStates[stateNum++]=endState;
				startState->edges.insert(NFAEdge(handleTransChar(*(++p)),endState));
				stateStack.push(endState);stateStack.push(startState);
			}
			break;

		case '(':
			opstack.push('(');
			break;
		case ')':
			while(!opstack.empty()&&opstack.top()!='(')
			{
				handleOp(opstack.top(),stateStack,stateNum);
				opstack.pop();
			}
			opstack.pop();
			break;

		case '|':
		case '?':
		case '+':
		case '*':
			pushOp(*p,opstack,stateStack,stateNum);
			break;

		default:
			//include .
			handleOp(*p,stateStack,stateNum);//normal
			break;
		}
		p++;
	}
	while(!opstack.empty())
	{
		handleOp(opstack.top(),stateStack,stateNum);
		opstack.pop();
	}

	stateCnt=stateNum;
	startState=stateStack.top();stateStack.pop();
	endState=stateStack.top();stateStack.pop();

	//stateStack should be empty
}

void NFAPart::clear()
{
	for(int i=0;i<stateCnt;i++) delete allStates[i];
	memset(allStates,0,sizeof(allStates));
	stateCnt=0;startState=endState=NULL;
}

// void NFAPart::getCopy(NFAState** dstStates,int& startpos,NFAState*& sState,NFAState*& eState)
// {
// 	for(int i=0;i<stateCnt;i++)
// 	{
// 		if(allStates[i])
// 		{
// 		dstStates[startpos+i]=new NFAState(startpos+i);	
// 		dstStates[startpos+i]->eBits=(allStates[i]->eBits)<<startpos;
// 		}
// 		else dstStates[startpos+i]=NULL;
// 	}
// 
// 	//copy edge
// 	NFAEdge* newEdge=NULL;
// 	for(int i=0;i<stateCnt;i++)
// 	{
// 		if(!allStates[i]) continue;
// 		for(EdgeItType ite=allStates[i]->edges.begin();ite!=allStates[i]->edges.end();ite++)
// 			dstStates[startpos+i]->edges.insert(NFAEdge((*ite).onSymbol,dstStates[(*ite).toState->state+startpos]));
// 
// 		for(EdgeInItType ite=allStates[i]->edgesin.begin();ite!=allStates[i]->edgesin.end();ite++)
// 			dstStates[startpos+i]->edgesin.insert(dstStates[(*ite)->state+startpos]);
// 		
// 		for(Edge0ItType it0=allStates[i]->edge0out.begin();it0!=allStates[i]->edge0out.end();it0++)
// 			dstStates[startpos+i]->edge0out.insert(dstStates[(*it0)->state+startpos]);
// 		
// 		for(EdgeInItType it0=allStates[i]->edge0in.begin();it0!=allStates[i]->edge0in.end();it0++)
// 			dstStates[startpos+i]->edge0in.insert(dstStates[(*it0)->state+startpos]);
// 	}
// 
// 	sState=dstStates[startState->state+startpos];
// 	eState=dstStates[endState->state+startpos];
// 	//stCnt=stateCnt;
// 	startpos+=stateCnt;
// }
// 
// 


//bool marked[NFASTATE_MAX_COUNT];
//BitSet<NFASTATE_MAX_COUNT> marked;
void NFAFrag::getCopy(NFAState** dstStates,int& startpos,NFAState*& sState,NFAState*& eState,bool startfrom0)
{
	int orgstart=startfrom0?0:startState->state;

	list<NFAState*> workList;

	int theindex,nextindex,maxindex=-1;

	NFAState* thestate;

	workList.push_back(startState);
	while(!workList.empty())
	{
		thestate=workList.front();workList.pop_front();

		theindex=startpos+thestate->state-orgstart;
		if(theindex>maxindex) maxindex=theindex;

		if(dstStates[theindex]==NULL)
		{
			dstStates[theindex]=new NFAState(theindex);
			dstStates[theindex]->eBits=((thestate->eBits)<<startpos);
		}

		for(EdgeInItType ite=thestate->edge0in.begin();ite!=thestate->edge0in.end();ite++)
		{
			nextindex=startpos+(*ite)->state-orgstart;
			if(dstStates[nextindex]==NULL)
			{
				if(nextindex>maxindex) maxindex=nextindex;
				dstStates[nextindex]=new NFAState(nextindex);
				dstStates[nextindex]->eBits=(((*ite)->eBits)<<startpos);

				workList.push_back(*ite);
			}		
			dstStates[theindex]->edge0in.insert(dstStates[nextindex]);
		}

		for(Edge0ItType ite=thestate->edge0out.begin();ite!=thestate->edge0out.end();ite++)
		{
			nextindex=startpos+(*ite)->state-orgstart;
			if(dstStates[nextindex]==NULL)
			{
				if(nextindex>maxindex) maxindex=nextindex;
				dstStates[nextindex]=new NFAState(nextindex);
				dstStates[nextindex]->eBits=(((*ite)->eBits)<<startpos);

				workList.push_back(*ite);
			}	
			dstStates[theindex]->edge0out.insert(dstStates[nextindex]);
		}

		for(EdgeItType ite=thestate->edges.begin();ite!=thestate->edges.end();ite++)
		{
			nextindex=startpos+(*ite).toState->state-orgstart;
			if(dstStates[nextindex]==NULL)
			{
				if(nextindex>maxindex) maxindex=nextindex;
				dstStates[nextindex]=new NFAState(nextindex);
				dstStates[nextindex]->eBits=(((*ite).toState->eBits)<<startpos);

				workList.push_back((*ite).toState);
			}
			dstStates[theindex]->edges.insert(NFAEdge((*ite).onSymbol,dstStates[nextindex]));
		}
	}


	sState=dstStates[startState->state-orgstart+startpos];
	eState=dstStates[endState->state-orgstart+startpos];
	//stCnt=maxindex-startpos+1;
	startpos=maxindex+1;
}



/////////////////////////////////////////////
NFA::NFA()
{
	stateCnt=0;startState=NULL;
	memset(allStates,0,sizeof(allStates));
}

void NFA::build(LexicalDesc& lexDesc)
{
	clear();

	map<string,int> mapStartStates;
	int i=0;
	for(;i<lexDesc.startStateCnt;i++)
	{
		allStates[i]=nfaStartStates[i]=new NFAState(i);
		mapStartStates.insert(pair<string,int>(lexDesc.startStates[i],i));
	}
	stateCnt=i;
// 	mapStartStates.insert(pair<char*,int>("COMMENT",1));
// 	mapStartStates.insert(pair<char*,int>("QUOTE",2));	

// 	if(lexDesc.tokenDefCnt<=1) return;
// 	if(lexDesc.tokenDefCnt==2)
// 	{
// 		fromRE(lexDesc.tokenDefs[1].re,lexDesc.auxNFAPart);
// 		endState->acceptStrIndex=1;	
// 		return;
// 	}
	
	char* p;

	for(i=1;i<lexDesc.tokenDefCnt;i++)
	{
		int startindex=0;
		p=lexDesc.tokenDefs[i].re;
		if(lexDesc.tokenDefs[i].re[0]=='<')
		{
			char buf[100];
			p++;
			char* q=buf;

			while(*p!='>') *(q++)=*(p++);
			*q='\0';p++;

			startindex=mapStartStates[buf];//if empty,0	
			//report it if no such startstate, and force it to 0 to continue
		}

		fromRE(p,lexDesc.auxNFAPart);
		endState->acceptStrIndex=i;
		
		elinkNFAState(nfaStartStates[startindex],startState);
	}

	startState=nfaStartStates[0];
}

NFA::~NFA()
{
	clear();
}
