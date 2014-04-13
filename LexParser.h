#ifndef _YANJUN_PARSER_H_
#define _YANJUN_PARSER_H_

#include <cstdio>
using namespace std;

#include "YYSTYPE.h"
#include "Lexical.h"
#include "LexScanner.h"

struct StackEntry
{
	StackEntry(){state=-1;memset(&val,0,sizeof(YYSTYPE));}
	StackEntry(int s):state(s){memset(&val,0,sizeof(YYSTYPE));}
	StackEntry(int s,YYSTYPE v):state(s),val(v){}
	int state;
	YYSTYPE val;
};

class Stack
{
public:
	Stack();
	~Stack();
	bool isEmpty();
	bool isFull();
	void init();
	StackEntry& getAt(int n);
	int top();//top position, not value
	void pop();
	bool push(const StackEntry& x);
	const StackEntry& peek();
private:
	StackEntry element[PARSESTACK_SIZE];
	int _top;
};

struct DefinitionExpr
{
	DefinitionExpr(){exprcnt=0;}
	void add(const char* str){strcpy(exprs[exprcnt++],str);}

	int exprcnt;
	char exprs[100][REGEXPR_MAX_LENGTH];
};
class LexParser
{
private:
	Stack sstack;
	YYSTYPE curval;
	Lexical& lex;
	LexScanner scanner;
	DefinitionExpr defExprs;
	

	void handleSematicAction(int rnum);

public:	
	YYSTYPE yylval;

	LexParser(Lexical& l,bool wmain=false):lex(l),scanner(yylval,wmain){}
	void parseerror(char*);
	bool yyparse(char* symbols);
};

#endif
