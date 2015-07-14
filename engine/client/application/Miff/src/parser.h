/* A Bison parser, made by GNU Bison 2.5.  */

/* Bison interface for Yacc-like parsers in C
   
      Copyright (C) 1984, 1989-1990, 2000-2011 Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.
   
   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     CHAR_LIT = 258,
     STR_LIT = 259,
     IDENTIFIER = 260,
     LIT = 261,
     FLOAT_LIT = 262,
     INT32 = 263,
     INT16 = 264,
     INT8 = 265,
     UINT32 = 266,
     UINT16 = 267,
     UINT8 = 268,
     FLOAT = 269,
     DOUBLE = 270,
     STRING = 271,
     WSTRING = 272,
     LABELHASH = 273,
     FORM = 274,
     CHUNK = 275,
     PRAGMA = 276,
     PRAGMA_DRIVE = 277,
     PRAGMA_DIR = 278,
     PRAGMA_FNAME = 279,
     PRAGMA_EXT = 280,
     ENUMSTRUCT = 281,
     INCLUDESOURCE = 282,
     INCLUDEBIN = 283,
     INCLUDEIFF = 284,
     SIN = 285,
     COS = 286,
     TAN = 287,
     ACOS = 288,
     ASIN = 289,
     ATAN = 290,
     POUND = 291,
     SHIFTRIGHT = 292,
     SHIFTLEFT = 293,
     RAISEDPOWER = 294
   };
#endif



#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 2068 of yacc.c  */
#line 137 "/swg/whitengold/src/engine/client/application/Miff/src/linux/parser.yac"

	long			ltype;
	double			dtype;
	char			*stype;
	char			chtype;

	int				tokentype;



/* Line 2068 of yacc.c  */
#line 100 "/swg/whitengold/src/engine/client/application/Miff/src/parser.h"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif

extern YYSTYPE yylval;


