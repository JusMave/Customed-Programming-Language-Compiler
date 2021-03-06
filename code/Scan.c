
#include "Globals.h"
#include "Scan.h"
#include "Util.h"



#define BUFFERLENGTH 256

char lineText[BUFFERLENGTH];      
char tokenString[MAXTOKENLEN+1];  
int  lineIndex = 0;               
int  lineSize = 0;               

typedef enum
{
    START, INNUM, INID, INDIV, INCOMMENT, INCOMMENT2, INNE, INLT, INGT,
	INEQ, ERRORSTATE, DONE
} LexerState;

char reservedWords[MAXRESERVED][7] =
  { "int", "void", "if", "else", "return", "while" };

TokenType reservedWordsTokens[MAXRESERVED] = 
  { INT, VOID, IF, ELSE, RETURN, WHILE };

struct fastReservedWords { char *name; TokenType tok; };

#define TOTAL_KEYWORDS 6
#define MIN_WORD_LENGTH 2
#define MAX_WORD_LENGTH 6
#define MIN_HASH_VALUE 2
#define MAX_HASH_VALUE 9

#ifdef __GNUC__
__inline
#endif
static unsigned int
hash (str, len)
     register const char *str;
     register unsigned int len;
{
  static unsigned char asso_values[] =
    {
     10,10,10,10,10,10,10,10,10,10,
     10,10,10,10,10,10,10,10,10,10,
     10,10,10,10,10,10,10,10,10,10,
     10,10,10,10,10,10,10,10,10,10,
     10,10,10,10,10,10,10,10,10,10,
     10,10,10,10,10,10,10,10,10,10,
     10,10,10,10,10,10,10,10,10,10,
     10,10,10,10,10,10,10,10,10,10,
     10,10,10,10,10,10,10,10,10,10,
     10,10,10,10,10,10,10,10,10,10,
      5, 0, 0,10,10, 0,10,10,10,10,
      0,10,10,10, 0,10, 0,10, 0, 0,
     10,10,10,10,10,10,10,10,10,10,
     10,10,10,10,10,10,10,10,10,10,
     10,10,10,10,10,10,10,10,10,10,
     10,10,10,10,10,10,10,10,10,10,
     10,10,10,10,10,10,10,10,10,10,
     10,10,10,10,10,10,10,10,10,10,
     10,10,10,10,10,10,10,10,10,10,
     10,10,10,10,10,10,10,10,10,10,
     10,10,10,10,10,10,10,10,10,10,
     10,10,10,10,10,10,10,10,10,10,
     10,10,10,10,10,10,10,10,10,10,
     10,10,10,10,10,10,10,10,10,10,
     10,10,10,10,10,10,10,10,10,10,
     10,10,10,10,10,10
    };
  return len + asso_values[(unsigned char)str[len - 1]] + asso_values[(unsigned char)str[0]];
}

#ifdef __GNUC__
__inline
#endif
struct fastReservedWords *
in_word_set (str, len)
     register const char *str;
     register unsigned int len;
{
  static struct fastReservedWords wordlist[] =
    {
      {"", 0}, {"", 0},
      {"if", IF},
      {"int", INT},
      {"else", ELSE},
      {"while", WHILE},
      {"return", RETURN},
      {"", 0}, {"", 0},
      {"void", VOID}
    };

  if (len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH)
    {
      register int key = hash (str, len);

      if (key <= MAX_HASH_VALUE && key >= 0)
        {
          register const char *s = wordlist[key].name;

          if (*str == *s && !strcmp (str + 1, s + 1))
            return &wordlist[key];
        }
    }
  return 0;
}

TokenType LookupReservedWord(char *lexeme)
{   
#ifdef FAST_RESERVED_WORDS

    struct fastReservedWords *rWord;

    rWord = in_word_set(lexeme, strlen(lexeme));

    if (rWord)
        return (rWord->tok);
    else
        return ID;

#else

    int i;

    for (i=0; i<MAXRESERVED; ++i)
    {
        if (!strcmp(lexeme, reservedWords[i]))
            return reservedWordsTokens[i];
    }

    return ID;
#endif
}


static char getNextChar()
{
    if (lineIndex >= lineSize)
    {
	++lineno;

	if (fgets(lineText, BUFFERLENGTH-1, source))
	{
	    lineSize = strlen(lineText);
	    lineIndex = 0;
	    
	    if (EchoSource)
		fprintf(listing, "SOURCE: %5d: %s", lineno, lineText);

	    return lineText[lineIndex++];
	}
	else
	    return EOF;
    }

    return lineText[lineIndex++];
}


static void ungetNextChar()
{
    --lineIndex;
}


TokenType getToken(void)
{
    int        tokenIndex = 0;   
    TokenType  currentToken;     
    LexerState state = START;    
    int        save;           

    int        c;          
    

    while (state != DONE)
    {
	c = getNextChar();
	save = TRUE;      

	switch(state)
	{
	case START:
	    if (isdigit(c))
		state = INNUM;
	    else if (isalpha(c))
		state = INID;
	    else if (c == '/')
		state = INDIV;
	    else if (c == '!')
		state = INNE;
	    else if (c == '<')
		state = INLT;
	    else if (c == '>')
		state = INGT;
	    else if (c == '=')
		state = INEQ;
	    else if ((c == '\n') || (c == '\t') || (c == ' '))
		save = FALSE;
	    else
	    {
		state = DONE;   
		switch (c)
		{
		case EOF:
		    save = FALSE;
		    currentToken = ENDOFFILE;
		    break;
		case '+':
		    currentToken = PLUS;
		    break;
		case '-':
		    currentToken = MINUS;
		    break;
		case '*':
		    currentToken = TIMES;
		    break;
		case ';':
		    currentToken = SEMI;
		    break;
		case ',':
		    currentToken = COMMA;
		    break;
		case '(':
		    currentToken = LPAREN;
		    break;
		case ')':
		    currentToken = RPAREN;
		    break;
		
		default:
		    currentToken = ERROR;
		    break;
		}  
	    } 
	    break;

	case INNE:
	    state = DONE;
	    
	    if (c == '=')
		currentToken = NE;
	    else
	    {
		ungetNextChar();
		save = FALSE;
		currentToken = ERROR;
	    }
	    break;
	    
	case INLT:
	    state = DONE;

	    if (c == '=')
		currentToken = LTE;
	    else
	    {
	
		ungetNextChar();
		save = FALSE;
		currentToken = LT;
	    }
	    break;

	case INGT:
	    state = DONE;
	    
	    if (c == '=')
		currentToken = GTE;
	    else
	    {

		ungetNextChar();
		save = FALSE;
		currentToken = GT;
	    }
	    break;
	    
	case INEQ:
	    state = DONE;
	    
	    if (c == '=')
		currentToken = EQ;
	    else
	    {

		ungetNextChar();
		save = FALSE;
		currentToken = ASSIGN;
	    }
	    break;

	case INDIV:
	    if (c == '*')
	    {
		save = FALSE;
		state = INCOMMENT;
		tokenIndex -=1;    
	    }
	    else
	    {
		ungetNextChar();
		save = FALSE;
		state = DONE;
		currentToken = DIVIDE;
	    }
	    break;

	case INCOMMENT:
	    save = FALSE;
	    
	    if (c == '*')
		state = INCOMMENT2;
	    
	    else if (c == EOF)
		state = ERROR;
	    
	    break;

	case INCOMMENT2:
	    save = FALSE;

	    if (c == '/')
		state = START;

            else if (c == '*')
                state = INCOMMENT2;  
	    else
		state = INCOMMENT;
	    break;

	case INID:
	    if (!isalpha(c))
	    {
		ungetNextChar();
		save = FALSE;
		state = DONE;
		currentToken = ID;
	    }

	    break;

	case INNUM:
	    if (!isdigit(c))
	    {
		ungetNextChar();
		save = FALSE;
		state = DONE;
		currentToken = NUM;
	    }
	    break;

	case DONE:
	default:
	    fprintf(listing, ">SCANNER ERROR: state = %d\n", state);
	    state = DONE;
	    currentToken = ERROR;
	    break;
	}  
	if ((save) && (tokenIndex <= MAXTOKENLEN))
	    tokenString[tokenIndex++] = c;
	
	if (state == DONE)
	{  
	    tokenString[tokenIndex] = '\0';
	    
            if (currentToken == ID)
                currentToken = LookupReservedWord(tokenString);
	}
	
    }  
    if (TraceScan)
    {
	fprintf(listing, "SCAN: %5d: ", lineno);
	printToken(currentToken, tokenString);
        fprintf(listing, "\n");
    }
    
    return currentToken;
}


