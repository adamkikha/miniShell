
/*
 *
 * CS-413 Spring 98
 * shell.l: lexical analyzer for shell
 * You have to extend it.
 *
 */

%{

#include <string.h>
#include "y.tab.h"

%}

%%

\n 	{
		return NEWLINE;
	}

[ \t] 	{
		/* Discard spaces and tabs */
     	}

"exit"  {
		return EXIT;
	}

"2>>" 	{
		return TWOGREATGREAT;
	}

">>" 	{
		return GREATGREAT;
	}

"2>" 	{
		return TWOGREAT;
	}

">" 	{
		return GREAT;
	}

"<" 	{
		return LESS;
	}

"&" 	{
		return AMPERSAND;
	}

"|" 	{
		return PIPE;
	}

[^ \t\n><&|]+  {
		/* Assume that file names have only alpha chars */
		yylval.string_val = strdup(yytext);
		return WORD;
	}

.	{
		/* Invalid character in input */
		return NOTOKEN;
	}
%%