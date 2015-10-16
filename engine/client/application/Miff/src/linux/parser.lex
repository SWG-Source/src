%option full

%{
/*-----------------------------------------------------------------------------**
**  FILE: parser.lex                                                           **
**  (c) 1998 - Bootprint GTInteractive                                         **
**                                                                             **
**  DESCRIPTION: lexical analyzer for mIFF                                     **
**                                                                             **
**  AUTHOR: Hideki Ikeda                                                       **
**                                                                             **
**  HISTORY:                                                                   **
**                                                                             **
**  Notes: companion to parser.yac                                             **
**-----------------------------------------------------------------------------*/
/* Disable compiler warnings (we want warning level 4) for anything that flex spits out */
#pragma warning (disable: 4127)			/* conditional expression is constant - ie. while(1) */
#pragma warning (disable: 4131)			/* usage of old-style declarator */
#pragma warning (disable: 4098)			/* void function returning a vlue - this is because yyterminate() is defined as return() */
#pragma warning (disable: 4505)			/* unreferenced local function has been removed (to be direct: yyunput()) */

/* include files */
#include "parser.h"		/* NOTE: make sure this matches what Bison/yacc spits out */

#include <stdlib.h>
#include <string.h>

/*--------------------------------**
** exteranl prototype declaration **
**--------------------------------*/
void	MIFFMessage(char *message, int forceOutput);
void	MIFFSetError(void);
char    * MIFFallocString(int sizeOfString);
void    MIFFfreeString(char * pointer);

int		yyparse();

/* prototype declaration */
int		MIFFYYInput(char *buf,int max_size);
void	initParser(void);
void	count(void);
int	yyerror(char *err);
void	open_brace(void);
void	close_brace(void);
int		count_brace(void);

void printString(char *str);

/* global vars that has to be pre-declared because it's referenced by the lexical analyzer */
int		initialCompile = 0;
int		globalErrorFlag = 0;
char	inFileName[512];		/* keep track of source file name for error message */

#undef YY_INPUT
#define	YY_INPUT(buf,result,max_size) (result = MIFFYYInput(buf,max_size))

#define	SPACE_COUNT_FOR_TAB			(8)

%}

DIGIT		[0-9]
HEXDIGIT	[0-9a-fA-F]
LETTER		[A-z_]
FLOATSYM	(f|F|l|L)
INTSYM		(u|U|l|L)*
EXP			(e|E)(\+|-)?

%%
"//"[^\n]*\n	{
				/* don't do count(); */
			}

"#define"[^\n]*\n {
				/* don't you love regular expression?  [^\n]* everything but \n, and then end with \n */
				/* don't do count(); just like comments */
				/* return(DEFINE);  <-- note: #define's are ignored in parser for they are handled via preprocessors CCCP */
			}

\"([^\"]|(\\\"))*\"	{
				/* start with " then ( [^\"] | (\\\") )* which means either anything but " OR \" of multiple encounter, and then close with " */
				/* case for "string" literals */
				char *s;			// allocate space for string and pass the string pointer rather then yytext

				count();
				s = MIFFallocString(strlen(yytext) + 1);
				strcpy(s, yytext+1);		/* strip off the double quotes */
				s[strlen(yytext+1)-1] = 0;	/* strip off the ending double quotes */
				yylval.stype = s;
				return(STR_LIT);
			}

"form"		|
"FORM"		{
				count();
				return(FORM);
			}

"chunk"		|
"CHUNK"		{
				count();
				return(CHUNK);
			}

"int32"		{
				count();
				return(INT32);
			}
"int16"		{
				count();
				return(INT16);
			}
"int8"		{
				count();
				return(INT8);
			}
"uint32"	{
				count();
				return(UINT32);
			}
"uint16"	{
				count();
				return(UINT16);
			}
"uint8"		{
				count();
				return(UINT8);
			}
"float"		{
				count();
				return(FLOAT);
			}
"double"	{
				count();
				return(DOUBLE);
			}
"string"	|
"cstring"	|
"CString"	{
				count();
				return(STRING);
			}
"wstring"	|
"WString"	{
				count();
				return(WSTRING);
			}
"labelhash"	{
				count();
				return(LABELHASH);
			}

"sin"		{
				count();
				return(SIN);
			}
"cos"		{
				count();
				return(COS);
			}
"tan"		{
				count();
				return(TAN);
			}
"asin"		{
				count();
				return(ASIN);
			}
"acos"		{
				count();
				return(ACOS);
			}
"atan"		{
				count();
				return(ATAN);
			}

"enum"		{
				count();
				return(ENUMSTRUCT);
			}

"includeIFF" |
"includeiff" {
				count();
				return(INCLUDEIFF);
			}
"include"	{
				count();
				return(INCLUDEBIN);
			}
"#include"	{
				count();
				return(INCLUDESOURCE);
			}
"#pragma"	{
				count();
				return(PRAGMA);
			}
"drive"		{
				count();
				return(PRAGMA_DRIVE);
			}
"directory"	{
				count();
				return(PRAGMA_DIR);
			}
"filename"	{
				count();
				return(PRAGMA_FNAME);
			}
"extension"	{
				count();
				return(PRAGMA_EXT);
			}

{LETTER}({LETTER}|{DIGIT})*	{
								/* label identifiers */
								char *s;			// allocate space for string and pass the string pointer rather then yytext

								count();
								s = MIFFallocString(strlen(yytext) + 1);
								strcpy(s, yytext);
								yylval.stype = s;
								return(IDENTIFIER);
							}

{DIGIT}*"."{DIGIT}+{FLOATSYM}?	{
									/* handle numericals (floats) */
									/*
									 *	{DIGIT}*"."{DIGIT}+{FLOATSYM}?	means zero or more digits . one or more digit and with/without f at the end
									 */
									count();
									/* make sure to store it to dtype, and use strtod to convert to double */
									yylval.dtype = strtod((char *) yytext, (char **) 0);
									return(FLOAT_LIT);
								}

0[xX]{HEXDIGIT}+{INTSYM}?		|
0{DIGIT}+{INTSYM}?				|
{DIGIT}+{INTSYM}?				{
									/* handle numericals ( hex, ints) */
									/*
									 *	0[xX]{HEXDIGIT}+{INTSYM}?		means start with 0, then X one or more digit and you can put int symbol if you want
									 *	0{DIGIT}+{INTSYM}?				means start with 0, one ore more digit and w/or w/o int symbol
									 *	{DIGIT}+{INTSYM}?				means one or more digit and w/or w/o int symbol
									 */
									count();
									/* make sure to store it to ltype (long), and use strtod to convert to unsigned long */
									yylval.ltype = strtoul((char *) yytext, (char **) 0, 0);
									return(LIT);
								}

'(\\.|[^\\'])+'	{
					/* handle 'x' - single character */
					count();
					yylval.chtype = yytext[1];
					return(CHAR_LIT);
				}

"#"			{
				/* #'s are used for informing the parser which file and line number it is processing (debug purpose) */
				count();
				return(POUND);
			}

">>"		{
				count();
				return(SHIFTRIGHT);
			}
"<<"		{
				count();
				return(SHIFTLEFT);
			}
"^^"		{
				count();
				return(RAISEDPOWER);
			}

"["			|
"]"			|
"^"			|
";"			|
","			|
":"			|
"="			|
"("			|
")"			|
"."			|
"&"			|
"!"			|
"~"			|
"-"			|
"+"			|
"*"			|
"/"			|
"%"			|
"<"			|
">"			|
"|"			|
"?"			{
				/* valid operators */
				count();
				yylval.stype = yytext;
				return(* yylval.stype);
			}

"{"			{
				count();
				open_brace();
				yylval.stype = yytext;
				return(* yylval.stype);
			}
"}"			{
				count();
				close_brace();
				yylval.stype = yytext;
				return(* yylval.stype);
			}

[ \t\n\r]+	{
				/* white spaces and newlines are ignored */
				count();
			}

<<EOF>>		{
				/* do a count on bracket matching... */
				if (0 == count_brace())
				{
					if (!initialCompile && !globalErrorFlag)
						MIFFMessage("mIFF successfully compiled!\n", 0);
				}

				yyterminate();				/* tell yyparse() it's time to quit! DO NOT comment or delete this line! */
			}

.			{
				/* anything that's not a rule from above goes here */
				count();
				yyerror((char *) yytext);
			}
%%

/*--------------------**
** C supporting codes **
**--------------------*/

/*------------------**
** static variables **
**------------------*/
static char	*MIFFInputStream;
int			column = 0;
int			line_num = 1;
int			line_num2 = 1;
char		error_line_buffer[256];
long		brace_counter = 0;


/*---------------------------------------------------------------------**
** Initialize all the static variables before all calls to MIFFCompile **
**---------------------------------------------------------------------*/
void initParser(void)
{
	line_num = 1;
	column = 0;

	brace_counter = 0;
	error_line_buffer[0] = 0;

	globalErrorFlag = 0;

	memset(inFileName, 0, 512);	/* make sure to change this size if the char array gets bigger... */
}

/*-------------------------------------------------**
** generate a dialog box to MFC to report an error **
**-------------------------------------------------*/
int yyerror(char *err)		/* called by yyparse() */
{
	char myString[256];

	if (!initialCompile)
	{
		/* spit it out in MSDev error format */
		sprintf(myString, "\n%s(%d) : yyERROR : %s\n>>%s<<", inFileName, line_num, err, error_line_buffer);
		MIFFMessage(myString, 1);
		globalErrorFlag = 1;
		MIFFSetError();		/* set global error flag for shell as well */
		yyterminate();
	}
	return 0;
}

/*-------------------------**
** our version of YY_INPUT **
**-------------------------*/
int MIFFYYInput(char *buf,int max_size)
{
	int len = strlen(MIFFInputStream);
	int n = max_size < len ? max_size : len;
	if (n > 0)
	{
		memcpy(buf,MIFFInputStream,n);
		MIFFInputStream += n;
	}
	return(n);
}

/*------------------------------------------------------------**
** line and column counter for error searching during compile **
**------------------------------------------------------------*/
void count()
{
	int			i;
	static char	*elb = error_line_buffer;
	for (i = 0; yytext[i] != '\0'; i++)
	{
		if (yytext[i] == '\n')
		{
			column = 0;
			line_num++;
			elb = error_line_buffer;
		}
		else
		{
			*elb++ = yytext[i];
			if (yytext[i] == '\t')
				column += SPACE_COUNT_FOR_TAB - (column & (SPACE_COUNT_FOR_TAB - 1));
			else
				column++;
		}
		*elb = 0;
	}
}

/*--------------------------------------------------------------**
** sets up current line number and filename the error came from **
**--------------------------------------------------------------*/
void setCurrentLineNumber(int lineNum, char * fileName, int mysteryNum)
{
	line_num = lineNum;
	strcpy(inFileName, fileName);
	line_num2 = mysteryNum;
}

/*----------------------------------------------**
** MIFFCompile called by CMIFFView::OnCompile() **
**----------------------------------------------*/
void MIFFCompile(char *inputStream, char *inputFileName)
{
	MIFFInputStream = inputStream;
	yyrestart(0);
	initParser();
	initialCompile = 0;
	strcpy(inFileName, inputFileName);
	yyparse();
}

void MIFFCompileInit(char *inputStream, char *inputFileName)
{
	MIFFInputStream = inputStream;
	yyrestart(0);
	initParser();
	initialCompile = 1;
	strcpy(inFileName, inputFileName);
	yyparse();
}

/*---------------------------------------**
** matching of open/close brace checking **
**---------------------------------------*/
void open_brace(void)
{
	brace_counter++;
}

void close_brace(void)
{
	brace_counter--;
}

/*
 *	 what: count_brace():
 * return: 0 == all braces matched
 */
int count_brace(void)
{
	if (0 == brace_counter)		/* things are fine... */
		return(0);

	/* if this is called, we should have 0 brace counter if not, we have a mis-match*/
	if (brace_counter > 0)
	{
		/* a mismatch */
		yyerror("There are more OPEN brackets then closed");
	}
	else if (brace_counter < 0)
	{
		yyerror("There are more CLOSED brackets then open");
	}

	return(-1);
}

/*-----------------------------------------------------------------------**
** FLEX.SLK requires this prototype function so I'm forced to do this... **
**-----------------------------------------------------------------------*/
int yywrap()
{
	return(1);
}

void printString(char *str)
{
	char ts[256];
	sprintf(ts, "%s - %s", str, yytext);
	MIFFMessage(ts, 0);
}
