
#include "Analyse.h"
#include "Globals.h"
#include "SymTab.h"
#include "Util.h"


static void drawRuler(FILE *output, char *string);

static void buildSymbolTable2(TreeNode *syntaxTree);

static void flagSemanticError(char *str);

static void traverse(TreeNode *syntaxTree,
		     void (*preProc)(TreeNode *),
		     void (*postProc)(TreeNode *) );

static void checkNode(TreeNode *syntaxTree);

static void nullProc(TreeNode *syntaxTree);

void markGlobals(TreeNode *tree);

static void declarePredefines(void);

static int checkFormalAgainstActualParms(TreeNode *formal, TreeNode *actual);

void buildSymbolTable(TreeNode *syntaxTree)
{

    if (TraceAnalyse)
    {
	drawRuler(listing, "");
	fprintf(listing,
		"Scope Identifier        Line   Is a   Symbol type\n");
	fprintf(listing,
		"depth                   Decl.  parm?\n");
    }

    declarePredefines();   
    buildSymbolTable2(syntaxTree);
    markGlobals(syntaxTree);
    if (TraceAnalyse)
    {
	drawRuler(listing, "GLOBALS");
	dumpCurrentScope();
	drawRuler(listing, "");
	fprintf(listing, "*** Symbol table dump complete\n");
    }
}


void typeCheck(TreeNode *syntaxTree)
{
    traverse(syntaxTree, nullProc, checkNode);
}

static void declarePredefines(void)
{
    TreeNode *input;
    TreeNode *output;
    TreeNode *temp;

    input = newDecNode(FuncDecK);
    input->name = copyString("input");
    input->functionReturnType = Integer;
    input->expressionType = Function;
    
    temp = newDecNode(ScalarDecK); 
    temp->name = copyString("arg"); 
    temp->variableDataType = Integer;
    temp->expressionType = Integer; 
    
    output = newDecNode(FuncDecK); 
    output->name = copyString("output"); 
    output->functionReturnType = Void; 
    output->expressionType = Function; 
    output->child[0] = temp;

    insertSymbol("input", input, 0);
    insertSymbol("output", output, 0);
}


static void buildSymbolTable2(TreeNode *syntaxTree)
{
    int         i;         
    HashNodePtr luSymbol;  
    char        errorMessage[80];  

    static TreeNode *enclosingFunction = NULL;
    
    while (syntaxTree != NULL)
    {

	if (syntaxTree->nodekind == DecK)
	    insertSymbol(syntaxTree->name, syntaxTree, syntaxTree->lineno);
	
	
	if ((syntaxTree->nodekind == DecK)
	    && (syntaxTree->kind.dec == FuncDecK))
	{
	    enclosingFunction = syntaxTree;	    
	    if (TraceAnalyse)	drawRuler(listing, syntaxTree->name);  
	    newScope();
	    ++scopeDepth;
	}
	if ((syntaxTree->nodekind == StmtK)
	    && (syntaxTree->kind.stmt == CompoundK))
	{
	    newScope();
	    ++scopeDepth;
	}

	if (((syntaxTree->nodekind == ExpK)  
	     && (syntaxTree->kind.exp == IdK))
	    || ((syntaxTree->nodekind == StmtK)  
		&& (syntaxTree->kind.stmt == CallK)))
	{
	    DEBUG_ONLY(
		fprintf(listing,
			">Annotating identifier \"%s\" on line %d\n",
			syntaxTree->name, syntaxTree->lineno); );

	    luSymbol = lookupSymbol(syntaxTree->name);
	    if (luSymbol == NULL)
	    {
		
		sprintf(errorMessage,
			"identifier \"%s\" unknown or out of scope\n",
			syntaxTree->name);
		flagSemanticError(errorMessage);
	    }
	    else
	    {
	
		syntaxTree->declaration = luSymbol->declaration;
	    }		 
	}

	if ((syntaxTree->nodekind == StmtK) &&
	    (syntaxTree->kind.stmt == ReturnK))
	{
	    syntaxTree->declaration = enclosingFunction;

	    DEBUG_ONLY( fprintf(listing,
	       ">Marking return statement on line %d with pointer to "
	       "%s() declaration\n",
               syntaxTree->lineno,
	       syntaxTree->declaration->name) );
	}
	
	for (i=0; i < MAXCHILDREN; ++i)
	    buildSymbolTable2(syntaxTree->child[i]);
	
	if (((syntaxTree->nodekind == DecK)
	    && (syntaxTree->kind.dec == FuncDecK))
	    || ((syntaxTree->nodekind == StmtK)
	    && (syntaxTree->kind.stmt == CompoundK)))
	{
	    if (TraceAnalyse)
		dumpCurrentScope();
	    --scopeDepth;
	    endScope();	
	}
	
	syntaxTree = syntaxTree->sibling;
    }
}


static void drawRuler(FILE *output, char *string)
{
    int length;
    int numTrailingDashes;
    int i;

    /* empty string? */
    if (strcmp(string, "") == 0)
	length = 0;
    else
	length = strlen(string) + 2;
    
    fprintf(output, "---");
    if (length > 0) fprintf(output, " %s ", string);
    numTrailingDashes = 45 - length;

    for (i=0; i<numTrailingDashes; ++i)
	fprintf(output, "-");
    fprintf(output, "\n");
}


static void flagSemanticError(char *str)
{
    fprintf(listing, ">Semantic error (type checker): %s", str);
    Error = TRUE;
}

static void traverse(TreeNode *syntaxTree,
		     void (*preProc)(TreeNode *),
		     void (*postProc)(TreeNode *) )
{
    int i;
    
    while (syntaxTree != NULL)
    {
	preProc(syntaxTree);

	for (i=0; i < MAXCHILDREN; ++i)
	    traverse(syntaxTree->child[i], preProc, postProc);

	postProc(syntaxTree);

	syntaxTree = syntaxTree->sibling;
    }
}

static int checkFormalAgainstActualParms(TreeNode *formal, TreeNode *actual)
{
    TreeNode *firstList;
    TreeNode *secondList;
    
    firstList = formal->child[0];
    secondList = actual->child[0];

    while ((firstList != NULL) && (secondList != NULL))
    {
	if (firstList->expressionType != secondList->expressionType)
	    return FALSE;

	if (firstList) firstList = firstList->sibling;
	if (secondList) secondList = secondList->sibling;
    }

    if (((firstList == NULL) && (secondList != NULL))
	|| ((firstList != NULL) && (secondList == NULL)))
	return FALSE;
    
    return TRUE;
}


static void checkNode(TreeNode *syntaxTree)
{
    char errorMessage[100];
    
    
    switch (syntaxTree->nodekind)
    {
    case DecK:

	switch (syntaxTree->kind.dec)
	{
	case ScalarDecK:
	    syntaxTree->expressionType = syntaxTree->variableDataType;
	    break;

	case ArrayDecK:
	    syntaxTree->expressionType = Array;
	    break;

	case FuncDecK:
	    syntaxTree->expressionType = Function;
	    break;
	}
	
	break;  /* case DecK */
	
    case StmtK:

	switch (syntaxTree->kind.stmt)
	{
	case IfK:
	    
	    if (syntaxTree->child[0]->expressionType != Integer)
	    {
		sprintf(errorMessage,
			"IF-expression must be integer (line %d)\n",
			syntaxTree->lineno);
		flagSemanticError(errorMessage);
	    }
	    
	    break;
	    
	case WhileK:

	    if (syntaxTree->child[0]->expressionType != Integer)
	    {
		sprintf(errorMessage,
			"WHILE-expression must be integer (line %d)\n",
			syntaxTree->lineno);
		flagSemanticError(errorMessage);
	    }
	    
	    break;
	    
	case CallK:

	    if (!checkFormalAgainstActualParms(syntaxTree->declaration,
					       syntaxTree))
	    {
		sprintf(errorMessage, "formal and actual parameters to "
			"function don\'t match (line %d)\n",
			syntaxTree->lineno);
		flagSemanticError(errorMessage);
	    }
	
	    syntaxTree->expressionType
		= syntaxTree->declaration->functionReturnType;;

	    break;
	    
	case ReturnK:

	    if (syntaxTree->declaration->functionReturnType == Integer)
	    {
		if ((syntaxTree->child[0] == NULL)
		    || (syntaxTree->child[0]->expressionType != Integer))
		{
		    sprintf(errorMessage, "RETURN-expression is either "
			    "missing or not integer (line %d)\n",
			    syntaxTree->lineno);
		    flagSemanticError(errorMessage);
		}
	    }
	    else if (syntaxTree->declaration->functionReturnType == Void)
	    {
		if (syntaxTree->child[0] != NULL)
		{
		    sprintf(errorMessage, "RETURN-expression must be"
			    "void (line %d)\n", syntaxTree->lineno);
		}
	    }
	    
	    break;

	case CompoundK:
	    
	    syntaxTree->expressionType = Void;
	    
	    break;
	}
	
	break; 
	
    case ExpK:

	switch (syntaxTree->kind.exp)
	{
	case OpK:

	    if ((syntaxTree->op == PLUS) || (syntaxTree->op == MINUS) ||
		(syntaxTree->op == TIMES) || (syntaxTree->op == DIVIDE))
	    {
		if ((syntaxTree->child[0]->expressionType == Integer) &&
		    (syntaxTree->child[1]->expressionType == Integer))
		    syntaxTree->expressionType = Integer;
		else
		{
		    sprintf(errorMessage, "arithmetic operators must have "
			 "integer operands (line %d)\n", syntaxTree->lineno);
		    flagSemanticError(errorMessage);
		}
	    }
	    
	    else if ((syntaxTree->op == GT) || (syntaxTree->op == LT) ||
		     (syntaxTree->op == LTE) || (syntaxTree->op == GTE) ||
		     (syntaxTree->op == EQ) || (syntaxTree->op == NE))
	    {
		if ((syntaxTree->child[0]->expressionType == Integer) &&
		    (syntaxTree->child[1]->expressionType == Integer))
		    syntaxTree->expressionType = Integer;
		else
		{
		    sprintf(errorMessage, "relational operators must have "
			  "integer operands (line %d)\n", syntaxTree->lineno);
		    flagSemanticError(errorMessage);
		}
	    }
	    else
	    {
		sprintf(errorMessage, "error in type checker: unknown operator"
			" (line %d)\n", syntaxTree->lineno);
		flagSemanticError(errorMessage);
	    }
	    
	    break;
	    
	case IdK:	   
	    
	    if (syntaxTree->declaration->expressionType == Integer)
	    {
		if (syntaxTree->child[0] == NULL)
		    syntaxTree->expressionType = Integer;
		else
		{
		 
		    sprintf(errorMessage, "can't access an element in "
			    "somthing that isn\t an array (line %d)\n",
			    syntaxTree->lineno);
		    flagSemanticError(errorMessage);
		}
	    }
	    else if (syntaxTree->declaration->expressionType == Array)
	    {
		if (syntaxTree->child[0] == NULL)
		    syntaxTree->expressionType = Array;
		else
		{
		  
		    if (syntaxTree->child[0]->expressionType == Integer)
			syntaxTree->expressionType = Integer;
		    else
		    {
			sprintf(errorMessage, "array must be indexed by a "
				"scalar (line %d)\n", syntaxTree->lineno);
			flagSemanticError(errorMessage);
		    }
		}
	    }
	    else
	    {
		sprintf(errorMessage, "identifier is an illegal type "
			"(line %d)\n", syntaxTree->lineno);
		flagSemanticError(errorMessage);
	    }
	    
	    break;
	    
	case ConstK:
	    
	    syntaxTree->expressionType = Integer;

	    break;
	    
	case AssignK:
	    
	    if ((syntaxTree->child[0]->expressionType == Integer) &&
		(syntaxTree->child[1]->expressionType == Integer))
		syntaxTree->expressionType = Integer;
	    else
	    {
		sprintf(errorMessage, "both assigning and assigned expression"
			" must be integer (line %d)\n", syntaxTree->lineno);
		flagSemanticError(errorMessage);
	    }
	    
	    break;	    
	}
	
	break; 
	
    }    
}


void markGlobals(TreeNode *syntaxTree)
{
    TreeNode *cursor;

    DEBUG_ONLY( char scratch[80]; );
    
    cursor = syntaxTree;

    while (cursor != NULL)
    {
	if ((cursor->nodekind==DecK)&&
	    ((cursor->kind.dec==ScalarDecK)||
	     (cursor->kind.dec==ArrayDecK)))
	{

	    DEBUG_ONLY(
		sprintf(scratch, ">Marked %s as a global variable\n",
			cursor->name);
		fprintf(listing, scratch);
		);
	    
	    cursor->isGlobal = TRUE;
	}
	
	cursor = cursor->sibling;
    }
}

static void nullProc(TreeNode *syntaxTree)
{
    if (syntaxTree == NULL)
	return;
    else
	return;
}



