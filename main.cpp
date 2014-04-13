#include "Lexical.h"
#include "LexParser.h"

Lexical lex;

//#define MAXBUFFER 8200

int main(int argc, char* argv[])
{
	bool write2main=false;
	if(argc<2)
	{
		printf("usage: Lex filename [-m]\n");
		return -1;
	}
	if(argc==3&&argv[2][1]=='m') write2main=true;

	LexParser parser(lex,write2main);

	char* buffer=NULL;//[MAXBUFFER+1];

	//FILE* inf=fopen("E:\\vproject\\lex&yacc\\Lex&Yacc\\testproj\\testproj\\ch3-03.l","r");
	//FILE* inf=fopen("E:\\vproject\\lex&yacc\\Lex&Yacc\\flic\\flic\\ansic.l","r");
	//FILE* inf=fopen("E:\\vproject\\lex&yacc\\Lex&Yacc\\cminus\\cminus\\cminus.l","r");
	FILE* inf=fopen(argv[1],"r");
	if(!inf)
	{
		printf("cannot open the file\n");
		return -1;
	}


	fseek(inf,0,SEEK_END);
	unsigned long filesize=ftell(inf);
	fseek(inf,0,SEEK_SET);

	unsigned long MAXBUFFER=filesize+20;
	buffer=new char[MAXBUFFER];


	int cnt=fread(buffer,sizeof(char),MAXBUFFER,inf);
	fclose(inf);
	buffer[cnt]='\0';

	parser.yyparse(buffer);

	lex.buildTable();
	lex.generate();

	delete [] buffer;

	return 0;
}