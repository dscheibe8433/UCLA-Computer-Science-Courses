// UCLA CS 111 Lab 1 command execution

#include "command.h"
#include "command-internals.h"

#include <error.h>

/* FIXME: You may need to add #include directives, macro definitions,
   static function definitions, etc.  */

#include <unistd.h>
#include <sys/wait.h>
#include <fcnt1.h>
int
command_status (command_t c)
{
  return c->status;
}

void
execute_command (command_t c, bool time_travel)
{
	//Need to write implentation for simple command execution first
	//We can then use a recursive call to execute_command to execute other commands.
	//Should execute the command and then set the process's status so that it can return
	//the correct value.
        pid_t child;
        int child_status; 
	
	switch( c->type ){
	case SIMPLE_COMMAND:
		
		// create child to execute command
		child = fork();
		
		if (child == 0) // child process
		{

			//If there is an input redirection
			if( c->input != 0 )
			{
				int fd = open( c->input, O_RDONLY, 0444 );
				if ( fd < 0 )
				{
				  error(1, 0, "File does not exit\n");
					//Is trying to read from a file that doesn't exist
					return;
				}

				if(dup2(fd, 0) < 0 ) //Changes the standard input to read from this file.
				{
				  error(1, 0, "Error changing standard input for reading from file\n");
					return;
				}

				close(fd);
			}
			//If there is an output redirection
			if( c->output != 0 )
			{
				int fd = open( c->output, O_CREAT|O_TRUNC|O_WRONLY, 0644 );
				if ( fd < 0 ) 
					return;
				if(dup2(fd, 1) < 0) //redirects the output to the file specified
				{
				  error(1, 0, "Error redirecting output to file\n");
					return;
				}
				close(fd);
			}
			
			execvp( c->u.word[0], c->u.word );
			error(1, 0, "Error executing c->u.word[0]\n");
		}
		else if (child > 0) // parent process
		{
		  waitpid(child, &child_status, 0);
		  c->status = child_status;
			//PARENT PROCESS STUFF
		}
		else
		{
		  error(1, 0, "Error creating child through fork()\n");
			//NO CHILD PROCESS CREATED
		}
		break;

	case AND_COMMAND: //return status of cmd[0] if false, else cmd[1] status if cmd[0] is true
	        execute_command(c->u.command[0], time_travel);
		c->status = c->u.command[0]->status;
		if ( c->status == 0 )
	        {
		  execute_command(c->u.command[1], time_travel);
		  c->status = c->u.command[1]->status;
		}
		break;
	case OR_COMMAND: //return status of cmd[0] if true, else return cmd[1] status
	        execute_command(c->u.command[0], time_travel);
		c->status = c->u.command[0]->status;
		if ( c->status != 0 )
		{
		  execute_command(c->u.command[1], time_travel);
		  c->status = c->u.command[1]->status;
		}
		break;
	case SEQUENCE_COMMAND:
		execute_command(c->u.command[0], time_travel);
		c->status = c->u.command[0]->status;
		if (c->u.command[1] != 0)
	        {
		    execute_command(c->u.command[1], time_travel);
		    c->status = c->u.command[1]->status;
        	}
              	break;
	case SUBSHELL_COMMAND:
		execute_command(c->u.subshell_command, false);
		c->status = c->u.subshell_command->status;
		break;
	case PIPE_COMMAND:
		break;
	default:
		//NOT A VALID COMMAND TYPE
	}

  error (1, 0, "command execution not yet implemented");
}
