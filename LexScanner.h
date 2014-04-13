#ifndef _YANJUN_LEX_H_
#define _YANJUN_LEX_H_

#include <fstream>
using namespace std;
#include "YYSTYPE.h"
#include "Limits.h"

class LexScanner
{
private:
	char* code;
	int startState;	
	char yytext[YYTEXT_MAX_LENGTH];

	YYSTYPE& yylval;

	char* litstart,*litend;
	bool indef;
	int partcnt;
	bool writeToMain;

	int retType(int st);

public:
	LexScanner(YYSTYPE& y,bool wmain):yylval(y),writeToMain(wmain){code=0;startState=1;yytext[0]=0;indef=false;partcnt=0;}

	//	generally, the yylval comes from the class Parser,
	//	if there is no need to use yylval, 
	//	comment out the include line of YYSTYPE, the declaration of yylval and the corresponding constructor 
	//	and de-comment the following constructor
	//
	// 	Lex(){startState=1;}

	const char* getText(){return yytext;}
	void scanNewCode(char* c){code=c;startState=1;yytext[0]=0;indef=false;partcnt=0;}
	int setStartState(int s){startState=s;}

	int yylex();

	/* add other methods here */
	bool handleAction();
	bool skipLitBlock();
	bool skipComment();
	void writeLitBlock(ofstream& outf);
	void writeLastBlock();
};

#endif
