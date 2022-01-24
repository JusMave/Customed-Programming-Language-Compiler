
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "Globals.h"
#include "SymTab.h"
#include "Util.h"


#define  MAXTABLESIZE 211

#define  HIGHWATERMARK "13_4_666_invalid"

static HashNodePtr hashtable[MAXTABLESIZE];  

static HashNodePtr secondList;

extern int TraceAnalyse;

int scopeDepth;


static HashNodePtr allocateSymbolNode(char *name,
				      TreeNode *declaration,
				      int lineDefined);

static int hashFunction(char *key);

static void flagError(char *message);

static char *formatSymbolType(TreeNode *node);

static void dumpCurrentScope2(HashNodePtr cursor);

void initSymbolTable(void)
{
    memset(hashtable, 0, sizeof(HashNodePtr) * MAXTABLESIZE);
    secondList = NULL;
}


void insertSymbol(char *name, TreeNode *symbolDefNode, int lineDefined)
{
    char errorString[80];  
    
    HashNodePtr newHashNode, temp;
    int hashBucket;

    if (symbolAlreadyDeclared(name))
    {
	sprintf(errorString, "duplicate identifier \"%s\"\n", name);
	flagError(errorString);
    }
    else
    {
	
	hashBucket = hashFunction(name);
	DEBUG_ONLY( fprintf(listing,
 		     ">insertSymbol(): bucket is %d\n", hashBucket); );


	newHashNode = allocateSymbolNode(name, symbolDefNode, lineDefined);
	if (newHashNode != NULL)
	{
	    temp = hashtable[hashBucket];
	    hashtable[hashBucket] = newHashNode;
	    newHashNode->next = temp;
	}

	newHashNode = allocateSymbolNode(name, symbolDefNode, lineDefined);
	if (newHashNode != NULL)
	{
	    temp = secondList;
	    secondList = newHashNode;
	    secondList->next = temp;
	}
    }
}

int symbolAlreadyDeclared(char *name)
{
    int         symbolFound = FALSE;  
    HashNodePtr cursor;
    
    cursor = secondList;

    while ((cursor != NULL) && (!symbolFound)
	   && ((strcmp(cursor->name, HIGHWATERMARK) != 0)))
    {
	if (strcmp(name, cursor->name) == 0)
	    symbolFound = TRUE;
	else
	    cursor = cursor->next;
    }

    return (symbolFound);
}


HashNodePtr lookupSymbol(char *name)
{
    HashNodePtr cursor;
    int         hashBucket;   
    int         found = FALSE;  
    hashBucket = hashFunction(name);
    cursor = hashtable[hashBucket];
    
    while (cursor != NULL)
    {
	if (strcmp(name, cursor->name) == 0)
	{
	    found = TRUE;
	    break;
	}
	    
	cursor = cursor->next;
    }

    if (found == TRUE)
	return cursor;
    else
	return NULL;
}


void dumpCurrentScope()
{
    HashNodePtr cursor;

    cursor = secondList;

  
    if ((cursor != NULL) && (strcmp(HIGHWATERMARK, cursor->name)))
	dumpCurrentScope2(cursor);
}

#define IDENT_LEN 12

static void dumpCurrentScope2(HashNodePtr cursor)
{   
    char paddedIdentifier[IDENT_LEN+1];
    char *typeInformation;   
    
    if ((cursor->next != NULL)
	&& (strcmp(cursor->next->name, HIGHWATERMARK) != 0))
	dumpCurrentScope2(cursor->next);

    memset(paddedIdentifier, ' ', IDENT_LEN);
    memmove(paddedIdentifier, cursor->name, strlen(cursor->name));
    paddedIdentifier[IDENT_LEN] = '\0';
    
    typeInformation = formatSymbolType(cursor->declaration);
    
    fprintf(listing, "%3d   %s   %7d     %c    %s\n",
	    scopeDepth,
	    paddedIdentifier,
	    cursor->lineFirstReferenced,
	    cursor->declaration->isParameter ? 'Y' : 'N', 
	    typeInformation);

    free(typeInformation);
}


void newScope()
{
    HashNodePtr newNode, temp;
    
    newNode = allocateSymbolNode(HIGHWATERMARK, NULL, 0);
    if (newNode != NULL)
    {
	temp = secondList;
	secondList = newNode;
	secondList->next = temp;
    }
}


void endScope()
{
  
    
    HashNodePtr hashPtr;
    HashNodePtr temp; 
    int         hashBucket;
    
    while ((secondList != NULL)
        && (strcmp(HIGHWATERMARK, secondList->name)) != 0)
    {
	
	hashBucket = hashFunction(secondList->name);
	hashPtr = hashtable[hashBucket];
	
	

	assert((secondList != NULL) && (hashtable[hashBucket] != NULL));
	assert(strcmp(secondList->name, hashPtr->name) == 0);

	
	temp = hashtable[hashBucket]->next;
	free(hashtable[hashBucket]);
	hashtable[hashBucket] = temp;


	temp = secondList->next;
	free(secondList);
	secondList = temp;
    }

   
    assert(strcmp(secondList->name, HIGHWATERMARK) == 0);
    temp = secondList->next;
    free(secondList);
    secondList = temp;
}


static HashNodePtr allocateSymbolNode(char *name,
				      TreeNode *declaration,
				      int lineDefined)
{
    HashNode *temp;

    temp = (HashNode*)malloc(sizeof(HashNode));
    if (temp == NULL)
    {
	Error = TRUE;
	fprintf(listing,
		">Out of memory allocating memory for symbol table\n");
    }
    else
    {
	temp->name = copyString(name);
	temp->declaration = declaration;
	temp->lineFirstReferenced = lineDefined;
	temp->next = NULL;
    }

    return temp;
}


#define SHIFT 4


static int hashFunction( char *key)
{
    int temp = 0;
    int i = 0;

    while (key[i] != '\0')
    {
	temp = ((temp << SHIFT) + key[i]) % MAXTABLESIZE;
	++i;
    }

    return temp;
}


static void flagError(char *message)
{
	fprintf(listing, ">Semantic error (symbol table): %s", message);
	Error = TRUE;
}

static char *formatSymbolType(TreeNode *node)
{
    char stringBuffer[100];

    if ((node == NULL) || (node->nodekind != DecK))
	strcpy(stringBuffer, "<<ERROR>>");
    else
    {

	switch (node->kind.dec)
	{
	case ScalarDecK:
	    sprintf(stringBuffer, "Scalar of type %s",
		    typeName(node->variableDataType));
	    break;
	case ArrayDecK:
	    sprintf(stringBuffer, "Array of type %s with %d elements",
		    typeName(node->variableDataType), node->val);
	    break;
	case FuncDecK:
	    sprintf(stringBuffer, "Function with return type %s",
		    typeName(node->functionReturnType));
	    break;
	default:
	    strcpy(stringBuffer, "<<UNKNOWN>>");
	    break;
	}
    }

    return copyString(stringBuffer);
}



