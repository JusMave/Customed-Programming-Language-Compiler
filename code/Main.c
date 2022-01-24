

#include "Globals.h"
#include "Util.h"
#include "Scan.h"
#include "Parse.h"
#include "Analyse.h"
#define BUILDTYPE "SCANNER/PARSER/ANALYSER ONLY"

int lineno = 0;

FILE* source;   
FILE* listing;   
FILE* code;      

#define MAXFILENAMESIZE  64

char sourceFileName[MAXFILENAMESIZE];

int EchoSource   = FALSE;
int TraceScan    = FALSE;
int TraceParse   = FALSE;
int TraceAnalyse = FALSE;
int TraceCode    = FALSE;


int Error = FALSE;

TreeNode *syntaxTree;

void usage(void)
{
    fprintf(stderr, USAGE);
}

int main(int argc, char **argv)
{
	char *sourceFileName = "sample.tiny";   
   if (strchr(sourceFileName, '.') == NULL)
	strcat(sourceFileName, ".tiny");
    source = fopen(sourceFileName, "r");
    if (source == NULL)
    {
	fprintf(stderr, ">Sorry, but the source file %s could not be found.\n",
		sourceFileName);
	exit(1);
    };
    listing = stdout;
    fprintf(listing, ">Welcome to the calculator\n>Parsing source program...\n");
    syntaxTree = Parse();
    if (TraceParse)
    {
        fprintf(listing, ">Dumping syntax tree\n");
    	printTree(syntaxTree);
    };

    if (!Error)
    {
	fprintf(listing, ">Building symbol table...\n");
	buildSymbolTable(syntaxTree);
	fprintf(listing, ">Performing type checking...\n");
	typeCheck(syntaxTree);
    }
    
    if (!Error)
        fprintf(listing,">Compilation complete: %d lines processed.\n", 
                lineno);
    else
        fprintf(listing,">Error: %d lines processed.\n", 
                lineno);
    
    return EXIT_SUCCESS;
}




