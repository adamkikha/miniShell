
/*
 * CS354: Shell project
 *
 * Template file.
 * You will need to add more code here to execute the command table.
 *
 * NOTE: You are responsible for fixing any bugs this code may have!
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#include <cstring>
#include <time.h>
#include "command.h"

SimpleCommand::SimpleCommand()
{
	// Creat available space for 5 arguments
	_numberOfAvailableArguments = 5;
	_numberOfArguments = 0;
	_arguments = (char **) malloc( _numberOfAvailableArguments * sizeof( char * ) );
}

void
SimpleCommand::insertArgument( char * argument )
{
	if ( _numberOfAvailableArguments == _numberOfArguments  + 1 ) {
		// Double the available space
		_numberOfAvailableArguments *= 2;
		_arguments = (char **) realloc( _arguments,
				  _numberOfAvailableArguments * sizeof( char * ) );
	}
	
	_arguments[ _numberOfArguments ] = argument;

	// Add NULL argument at the end
	_arguments[ _numberOfArguments + 1] = NULL;
	
	_numberOfArguments++;
}

Command::Command()
{
	// Create available space for one simple command
	_numberOfAvailableSimpleCommands = 1;
	_simpleCommands = (SimpleCommand **)
		malloc( _numberOfSimpleCommands * sizeof( SimpleCommand * ) );

	_numberOfSimpleCommands = 0;
	_outFile = 0;
	_errMode = 0;
	_outMode = 0;
	_inputFile = 0;
	_errFile = 0;
	_background = 0;
}

void
Command::insertSimpleCommand( SimpleCommand * simpleCommand )
{
	if ( _numberOfAvailableSimpleCommands == _numberOfSimpleCommands ) {
		_numberOfAvailableSimpleCommands *= 2;
		_simpleCommands = (SimpleCommand **) realloc( _simpleCommands,
			 _numberOfAvailableSimpleCommands * sizeof( SimpleCommand * ) );
	}
	
	_simpleCommands[ _numberOfSimpleCommands ] = simpleCommand;
	_numberOfSimpleCommands++;
}

void
Command:: clear()
{
	for ( int i = 0; i < _numberOfSimpleCommands; i++ ) {
		for ( int j = 0; j < _simpleCommands[ i ]->_numberOfArguments; j ++ ) {
			free ( _simpleCommands[ i ]->_arguments[ j ] );
		}
		
		free ( _simpleCommands[ i ]->_arguments );
		free ( _simpleCommands[ i ] );
	}

	if ( _outFile ) {
		free( _outFile );
	}

	if ( _inputFile ) {
		free( _inputFile );
	}

	if ( _errFile ) {
		free( _errFile );
	}

	_numberOfSimpleCommands = 0;
	_outFile = 0;
	_inputFile = 0;
	_errFile = 0;
	_outMode = 0;
	_errMode = 0;
	_background = 0;
}

void
Command::print()
{
	printf("\n\n");
	printf("              COMMAND TABLE                \n");
	printf("\n");
	printf("  #   Simple Commands\n");
	printf("  --- ----------------------------------------------------------\n");
	
	for ( int i = 0; i < _numberOfSimpleCommands; i++ ) {
		printf("\n  %-3d ", i );
		for ( int j = 0; j < _simpleCommands[i]->_numberOfArguments; j++ ) {
			printf("\"%s\" \t", _simpleCommands[i]->_arguments[ j ] );
		}
	}

	printf( "\n\n" );
	printf( "  Output       OutMode      Input        Error        ErrMode      Background\n" );
	printf( "  ------------ ------------ ------------ ------------ ------------ ------------\n" );
	printf( "  %-12s %-12s %-12s %-12s %-12s %-12s\n", _outFile?_outFile:"default",_outMode? (_outMode==1? "create":"append"):"None",
		_inputFile?_inputFile:"default", _errFile?_errFile:"default",_errMode? (_errMode==1? "create":"append"):"None",
		_background?"YES":"NO");
	printf( "\n\n" );
	
}

void
Command::execute()
{
	// Don't do anything if there are no simple commands
	if ( _numberOfSimpleCommands == 0 ) {
		prompt();
		return;
	}
	// Print contents of Command data structure
	print();
	signal(SIGCHLD,Command::log);
	Command command = Command::_currentCommand;

	if (command._numberOfSimpleCommands == 1){

		// handling for cd command
		if(!strcmp(command._simpleCommands[0]->_arguments[0],"cd")){
			if(command._simpleCommands[0]->_arguments[1]){
				chdir(command._simpleCommands[0]->_arguments[1]);
				printf("current directory: %s\n",get_current_dir_name());
			} else {
				chdir("/home");
				printf("current directory: /home\n");
			}
			clear();
			prompt();
			return;
		}
		int pid = fork();
		if(pid < 0){
			perror("couldn't fork");
			exit(2);
		}
		if(!pid){
			int in =  dup( 0 );
			int out = dup( 1 );
			int err = dup( 2 );
			if (command._outFile){
				out = open(command._outFile,O_RDWR | (command._outMode==1 ? O_TRUNC : O_APPEND) | O_CREAT,S_IRWXU | S_IRWXG | S_IRWXO);
			} 
			if (command._errFile){
				err = open(command._errFile,O_RDWR | (command._errMode==1 ? O_TRUNC : O_APPEND) | O_CREAT,S_IRWXG | S_IRWXO | S_IRWXU);
			} 
			if (command._inputFile){
				in = open(command._inputFile,O_RDONLY,S_IRWXG | S_IRWXO | S_IRWXU);
			}
			dup2(in,0);
			dup2(out,1);
			dup2(err,2);
			close(in);
			close(out);
			close(err);
			execvp(command._simpleCommands[0]->_arguments[0],command._simpleCommands[0]->_arguments);
			perror("couldn't execute command");
			exit(2);
		}
		if (!command._background){
			waitpid(pid,nullptr,0);
		}
	} else {
		int in =  dup( 0 );
		int err = dup( 2 );
		if (command._inputFile){
			in = open(command._inputFile,O_RDONLY,S_IRWXG | S_IRWXO | S_IRWXU);
		}
		int pip[2];
		int out;
		int pid;
		for (int i = 0; i < command._numberOfSimpleCommands; i++)
		{
			if(i == command._numberOfSimpleCommands - 1){
				out = (command._outFile) ? open(command._outFile,O_RDWR | (command._outMode==1 ? O_TRUNC : O_APPEND) | O_CREAT,S_IRWXU | S_IRWXG | S_IRWXO) : dup(1);
				err = (command._errFile) ? open(command._errFile,O_RDWR | (command._errMode==1 ? O_TRUNC : O_APPEND) | O_CREAT,S_IRWXG | S_IRWXO | S_IRWXU) : dup(2);
			} else {
				pipe(pip);
				out = pip[1];
			}
			pid = fork();
			if(pid < 0){
				perror("couldn't fork");
				exit(2);
			}
			if (!pid){
				dup2(in,0);
				dup2(out,1);
				dup2(err,2);
				close(in);
				close(out);
				close(err);
				execvp(command._simpleCommands[i]->_arguments[0],command._simpleCommands[i]->_arguments);
				perror("couldn't execute command");
				exit(2);
			}
			close(in);
			close(out);
			in = pip[0];
		}
		if (!command._background){
			waitpid(pid,nullptr,0);
		}
	}
	// Clear to prepare for next command
	clear();
	
	// Print new prompt
	prompt();
}

// Shell implementation

void
Command::prompt()
{
	printf("myshell>");
	fflush(stdout);
}

void
Command::log(int signum){
	FILE *file = fopen("log","a+");
	time_t t= time(NULL);
	fprintf(file, "Process terminated at %s\n",,ctime(&t));
	fclose(file);
}

Command Command::_currentCommand;
SimpleCommand * Command::_currentSimpleCommand;

int yyparse(void);

int 
main()
{
	Command::_currentCommand.prompt();
	signal(SIGINT,SIG_IGN);
	yyparse();
	return 0;
}
