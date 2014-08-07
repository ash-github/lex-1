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
	printf("generating scanner...\n");
	{
		ofstream outf("_lex.h");
		ifstream inf("LEX_TEMPLATE.H");
		if(inf.is_open())
		{
			outf<<inf.rdbuf();
		}
		else puts("missing file: LEX_TEMPLATE.H");

		inf.close();
		outf.close();
	}	

	ofstream outf("_lex.cpp");
	ifstream inf;

	outf<<	"#include \"_lex.h\"\n"
			"#include \"_tokens.h\"\n"
			"#include <stack>\n"
			"using namespace std;\n\n"
			"#define ECHO puts(yytext)\n"
			"#define BEGINSTATE startState=\n";
	for(int i=0,iLen=startStates.size();i<iLen;i++)
		outf<<"#define "<<startStates[i]<<"\t\t"<<i+1<<endl;
	outf<<endl;

	inf.open("_lblock1");
	if(inf.is_open())
	{
// 		while(!inf.eof())
// 		{
// 			getline(inf,buffer);
// 			outf<<buffer<<endl;
// 		}
		outf<<inf.rdbuf();
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
	outf<<	"\tdefault:\n"
			"\t\treturn 0;\n"
			"\t}\n"
			"\treturn 0;\n"
			"}\n"
			"int Lex::yylex()\n{\n"
			"Lrescan:\n";

	inf.open("_lblock2");
	if(inf.is_open())
	{
// 		while(!inf.eof())
// 		{
// 			getline(inf,buffer);
// 			outf<<buffer<<endl;
// 		}
		outf<<inf.rdbuf();
		inf.close();inf.clear();
		remove("_lblock2");
	}	
	else{inf.close();inf.clear();}

	inf.open("LEX_TEMPLATE.CPP");
	if(inf.is_open())
	{
// 		while(!inf.eof())
// 		{
// 			getline(inf,buffer);
// 			outf<<buffer<<endl;
// 		}
		outf<<inf.rdbuf();
	}
	else puts("missing file: LEX_TEMPLATE.CPP");
	inf.close();inf.clear();

	inf.open("_lblock3");
	if(inf.is_open())
	{
// 		while(!inf.eof())
// 		{
// 			getline(inf,buffer);
// 			outf<<buffer<<endl;
// 		}
		outf<<inf.rdbuf();
		inf.close();inf.clear();
		remove("_lblock3");
	}	
	else{inf.close();inf.clear();}

	outf.close();
}