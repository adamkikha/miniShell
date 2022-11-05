/*
 * CS-413 Spring 98
 * shell.y: parser for shell
 *
 * This parser compiles the following grammar:
 *
 *	cmd [arg]* [> filename]
 *
 * you must extend it to understand the complete shell grammar
 *
 */
%{
#define yylex yylex
#include <stdio.h>
#include "command.h"
extern "C" 
{
	int yylex();
	void yyerror (char const *s);
}
%}

%token	<string_val> WORD

%token 	NOTOKEN GREAT TWOGREAT TWOGREATGREAT GREATGREAT LESS AMPERSAND PIPE NEWLINE EXIT

%union	{
		char   *string_val;
	}
%%

goal:	
	commands
	| goal commands
	;

commands: 
	command AMPERSAND NEWLINE {
		printf("   Yacc: Execute command\n");
		Command::_currentCommand._background = 1;
		Command::_currentCommand.execute();
	}
	| command NEWLINE {
		printf("   Yacc: Execute command\n");
		Command::_currentCommand.execute();
	}
	;

command: 
	command iomodifier_opt
	| full_command
    ;

full_command:	
	EXIT {
		return 0;
	}
	| simple_command
	| NEWLINE 
	| error NEWLINE { yyerrok; }
	;

simple_command:
	simple_command PIPE command_and_args {
	}
	| command_and_args
	;	

command_and_args:
	command_word arg_list {
		Command::_currentCommand.
			insertSimpleCommand( Command::_currentSimpleCommand );
	}
	;

arg_list:
	arg_list argument
	| /* can be empty */
	;

argument:
	WORD {
               printf("   Yacc: insert argument \"%s\"\n", $1);
	       Command::_currentSimpleCommand->insertArgument( $1 );\
	}
	;

command_word:
	WORD {
               printf("   Yacc: insert command \"%s\"\n", $1);
	       Command::_currentSimpleCommand = new SimpleCommand();
	       Command::_currentSimpleCommand->insertArgument( $1 );
	}
	;

iomodifier_opt:
	TWOGREATGREAT WORD {
		printf("   Yacc: append error \"%s\"\n", $2);
		Command::_currentCommand._errFile = $2;
		Command::_currentCommand._outMode = 2;
	}
	| GREATGREAT WORD {
		printf("   Yacc: append output \"%s\"\n", $2);
		Command::_currentCommand._outFile = $2;
		Command::_currentCommand._outMode = 2;
	}
	| TWOGREAT WORD {
		printf("   Yacc: insert error \"%s\"\n", $2);
		Command::_currentCommand._errFile = $2;
		Command::_currentCommand._outMode = 1;
	}
	| GREAT WORD {
		printf("   Yacc: insert output \"%s\"\n", $2);
		Command::_currentCommand._outFile = $2;
		Command::_currentCommand._outMode = 1;
	}
	| LESS WORD {
		printf("   Yacc: insert input \"%s\"\n", $2);
		Command::_currentCommand._inputFile = $2;
	}
	;

%%

void
yyerror(const char * s)
{
	fprintf(stderr,"%s", s);
}

#if 0
int
main()
{
	yyparse();
}
#endif
