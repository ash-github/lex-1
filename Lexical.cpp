#include "lexical.h"

#include <fstream>
#include <string>
#include <set>
using namespace std;

struct AcceptCase
{
	AcceptCase(int a,int s):statenum(s),accindex(a){}
	int statenum,accindex;
};
struct setAcceptCaseCMP
{
	bool operator()(const AcceptCase& first, const AcceptCase& second) const 
	{   
		return (first.accindex<second.accindex)||(first.accindex==second.accindex&&first.statenum<second.statenum);
	} 
};

typedef set<AcceptCase,setAcceptCaseCMP>::iterator AcceptCaseIterator;
void Lexical::generate()
{
	ofstream outf("_lex.h");
	ifstream inf("LEX_H_TEMPLATE");
	string buffer;
	if(inf.is_open())
	{
		while(!inf.eof())
		{
			getline(inf,buffer);
			outf<<buffer<<endl;
		}		
	}
	else puts("missing file: LEX_H_TEMPLATE");

	inf.close();inf.clear();	
	outf.close();

	outf.open("_lex.cpp");

	outf<<"#include \"_lex.h\""<<endl;
	outf<<"#include \"_tokens.h\""<<endl;
	outf<<"#include <stack>"<<endl;
	outf<<"using namespace std;"<<endl;
	outf<<endl;
	outf<<"#define ECHO puts(yytext)"<<endl;

	outf<<"#define BEGINSTATE startState="<<endl;
	for(int i=0;i<startStateCnt;i++)
		outf<<"#define "<<startStates[i]<<"\t\t"<<i+1<<endl;
	outf<<endl;

	inf.open("_lblock1");
	if(inf.is_open())
	{
		while(!inf.eof())
		{
			getline(inf,buffer);
			outf<<buffer<<endl;
		}
		inf.close();inf.clear();
		remove("_lblock1");
	}	
	else{inf.close();inf.clear();}

	outf<<"int lexTable[][SYMBOL_CNT]={"<<endl;
	for(int i=0;i<=dfa.stateCnt;i++)
	{
		outf<<"\t";
		for(int j=0;j<SYMBOL_CNT;j++)
		{
			outf<<dfa.dfaTable[i][j]<<',';
		}
		outf<<endl;
	}
	outf<<"};"<<endl;

	outf<<"bool lexAccept[]={"<<endl;
	outf<<"\t";
	for(int i=0;i<=dfa.stateCnt;i++)
	{
		if(dfa.dfaAccept>0) outf<<"true,";
		else outf<<"false,";
	}
	outf<<endl<<"};"<<endl;	

	outf<<"int Lex::retType(int st)"<<endl;
	outf<<"{"<<endl;
	outf<<"\tswitch(st)"<<endl;
	outf<<"\t{"<<endl;


	set<AcceptCase,setAcceptCaseCMP> accset;
	for(int i=1;i<=dfa.stateCnt;i++)
	{
		if(dfa.dfaAccept[i]>0)
			accset.insert(AcceptCase(dfa.dfaAccept[i],i));
	}
	AcceptCaseIterator acit=accset.begin();
	int lastacindex=acit->accindex;	
	for(;acit!=accset.end();++acit)
	{
		if(acit->accindex!=lastacindex)
		{
			outf<<"\t\t"<<tokenDefs[lastacindex].lexAction<<endl;
			lastacindex=acit->accindex;
		}

		outf<<"\tcase "<<acit->statenum<<':'<<endl;
	}
	if(dfa.stateCnt)
		outf<<"\t\t"<<tokenDefs[lastacindex].lexAction<<endl;

	// 	for(int i=1;i<=dfa.stateCnt;i++)
	// 	{
	// 		if(dfa.dfaAccept[i]>0)
	// 		{
	// 			outf<<"\tcase "<<i<<':'<<endl;
	// 			outf<<"\t\t"<<tokenDefs[dfa.dfaAccept[i]].lexAction<<endl;
	// 			//outf<<"\t\tbreak;"<<endl;
	// 		}
	// 	}
	outf<<"\tdefault:"<<endl;
	outf<<"\t\treturn 0;"<<endl;

	outf<<"\t}"<<endl;
	outf<<"\treturn 0;"<<endl;
	outf<<"}"<<endl;

	outf<<"int Lex::yylex()"<<endl;
	outf<<"{"<<endl;
	outf<<"Lrescan:"<<endl;

	inf.open("_lblock2");
	if(inf.is_open())
	{
		while(!inf.eof())
		{
			getline(inf,buffer);
			outf<<buffer<<endl;
		}
		inf.close();inf.clear();
		remove("_lblock2");
	}	
	else{inf.close();inf.clear();}

	inf.open("LEX_CPP_TEMPLATE");
	if(inf.is_open())
	{
		while(!inf.eof())
		{
			getline(inf,buffer);
			outf<<buffer<<endl;
		}		
	}
	else puts("missing file: LEX_CPP_TEMPLATE");
	inf.close();inf.clear();

	inf.open("_lblock3");
	if(inf.is_open())
	{
		while(!inf.eof())
		{
			getline(inf,buffer);
			outf<<buffer<<endl;
		}
		inf.close();inf.clear();
		remove("_lblock3");
	}	
	else{inf.close();inf.clear();}

	outf.close();
}