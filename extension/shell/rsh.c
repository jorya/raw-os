/*
     raw os - Copyright (C)  Lingjun Chen(jorya_txj).

    This file is part of raw os.

    raw os is free software; you can redistribute it it under the terms of the 
    GNU General Public License as published by the Free Software Foundation; 
    either version 3 of the License, or  (at your option) any later version.

    raw os is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; 
    without even the implied warranty of  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  
    See the GNU General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program. if not, write email to jorya.txj@gmail.com
                                      ---

    A special exception to the LGPL can be applied should you wish to distribute
    a combined work that includes raw os, without being obliged to provide
    the source code for any proprietary components. See the file exception.txt
    for full details of how and when the exception can be applied.
*/


/* 	2012-8  Created by jorya_txj
  *	xxxxxx   please added here
  */

#include <raw_api.h>
#include <lib_string.h>
#include <rsh.h>

/*
 * The callback function that is executed when "help" is entered.  This is the
 * only default command that is always present.
 */
static RAW_S32 rsh_help_command(RAW_S8 *pcWriteBuffer, size_t xWriteBufferLen, const RAW_S8 *pcCommandString );

/*
 * Return the number of parameters that follow the command name.
 */
static RAW_S8 rsh_get_parameters_numbers( const RAW_S8 * pcCommandString );

/* The definition of the "help" command.  This command is always at the front
of the list of registered commands. */
static const xCommandLineInput xHelpCommand = 
{
	"help",
	"help: Lists all the registered commands\r\n",
	rsh_help_command,
	0
};

/* The definition of the list of commands.  Commands that are registered are
added to this list. */
static xCommandLineInputListItem xRegisteredCommands =
{	
	&xHelpCommand,	/* The first command in the list is always the help command, defined in this file. */
	0			/* The next pointer is initialised to NULL, as there are no other registered commands yet. */
};


static xCommandLineInputListItem *current_command_list_point = &xRegisteredCommands;


static RAW_U8 command_length_get(const RAW_S8 *command)
{

	const RAW_S8 *sc;

	for (sc = command; (*sc != '\0') && (*sc != ' '); ++sc)
		/* nothing */;
	return sc - command;

}


/*-----------------------------------------------------------*/

RAW_VOID rsh_register_command(const xCommandLineInput * const pxCommandToRegister, xCommandLineInputListItem *pxNewListItem)
{

	/* Check the parameter is not NULL. */
	if (pxCommandToRegister == 0) {

		RAW_ASSERT(0);
	}

	
	if (pxNewListItem == 0) {
		
		RAW_ASSERT(0);
	}

	
	/* Reference the command being registered from the newly created 
	list item. */
	pxNewListItem->pxCommandLineDefinition = pxCommandToRegister;

	/* The new list item will get added to the end of the list, so 
	pxNext has nowhere to point. */
	pxNewListItem->pxNext = 0;

	/* Add the newly created list item to the end of the already existing
	list. */
	current_command_list_point->pxNext = pxNewListItem;

	/* Set the end of list marker to the new list item. */
	current_command_list_point = pxNewListItem;
	
}


static const xCommandLineInputListItem *current_process_command_point;


RAW_S32 rsh_process_command( const RAW_S8 * const pcCommandInput, RAW_S8 *pcWriteBuffer, size_t xWriteBufferLen  )
{
	RAW_S32 xReturn = 0;
	const RAW_S8 *pcRegisteredCommandString;
	RAW_U8 s_input_length;
	RAW_U8 s_command_length;
	RAW_U8 s_commpare_length;

	/* Check the parameter is not NULL. */
	if (pcCommandInput == 0) {

		RAW_ASSERT(0);
	}


	if (pcWriteBuffer == 0) {

		RAW_ASSERT(0);
	}


	if (xWriteBufferLen == 0) {

		RAW_ASSERT(0);

	}
	
	/* Note:  This function is not re-entrant.  It must not be called from more
	thank one task. */

	if (current_process_command_point == 0) {

		/* Search for the command string in the list of registered commands. */
		for (current_process_command_point = &xRegisteredCommands; current_process_command_point != 0; current_process_command_point = current_process_command_point->pxNext) {
		
			pcRegisteredCommandString = current_process_command_point->pxCommandLineDefinition->pcCommand;
			s_command_length = command_length_get((const RAW_S8 *)pcRegisteredCommandString);
			s_input_length = command_length_get((const RAW_S8 *)pcCommandInput);

			if (s_input_length > s_command_length) {

				s_commpare_length = s_input_length;
			}

			else {

				s_commpare_length = s_command_length;
			}
			
			if (raw_strncmp((const char *)pcCommandInput, (const char *)pcRegisteredCommandString, s_commpare_length) == 0 ) {
			
				/* The command has been found.  Check it has the expected
				number of parameters.  If cExpectedNumberOfParameters is -1,
				then there could be a variable number of parameters and no
				check is made. */
				if (current_process_command_point->pxCommandLineDefinition->cExpectedNumberOfParameters >= 0) {
				
					if (rsh_get_parameters_numbers( pcCommandInput ) != current_process_command_point->pxCommandLineDefinition->cExpectedNumberOfParameters) {
					
						xReturn = 1;
					}
				}

				break;
			}
		}
	}

	if (current_process_command_point && (xReturn == 1)) {
	
		/* The command was found, but the number of parameters with the command
		was incorrect. */
		raw_strncpy( ( char * ) pcWriteBuffer, "\rIncorrect command parameter(s).  Enter \"help\" to view a list of available commands.\r\n\r\n", xWriteBufferLen );
		current_process_command_point = 0;
	}
	
	else if (current_process_command_point != 0) {
	
		raw_memset(pcWriteBuffer, 0, xWriteBufferLen);
		/* Call the callback function that is registered to this command. */
		xReturn = current_process_command_point->pxCommandLineDefinition->pxCommandInterpreter(pcWriteBuffer, xWriteBufferLen, pcCommandInput);

		/* If xReturn is 1, then no further strings will be returned
		after this one, and	pxCommand can be reset to NULL ready to search 
		for the next entered command. */
		if (xReturn == 1) {
			
			current_process_command_point = 0;
		}
	}
	
	else {
	
		/* pxCommand was NULL, the command was not found. */
		raw_strncpy( ( char * ) pcWriteBuffer, (const char *) "\rCommand not recognised.  Enter \"help\" to view a list of available commands.\r\n\r\n", xWriteBufferLen );
		xReturn = 1;
	}

	return xReturn;
}


const RAW_S8 *rsh_get_parameter( const RAW_S8 *pcCommandString, RAW_S32 uxWantedParameter, RAW_S32 *pxParameterStringLength )
{
	RAW_S32 uxParametersFound = 0;
	const RAW_S8 *pcReturn = 0;

	*pxParameterStringLength = 0;

	while( uxParametersFound < uxWantedParameter )
	{
		/* Index the character pointer past the current word.  If this is the start
		of the command string then the first word is the command itself. */
		while( ( ( *pcCommandString ) != 0x00 ) && ( ( *pcCommandString ) != ' ' ) )
		{
			pcCommandString++;
		}

		/* Find the start of the next string. */
		while( ( ( *pcCommandString ) != 0x00 ) && ( ( *pcCommandString ) == ' ' ) )
		{
			pcCommandString++;
		}

		/* Was a string found? */
		if( *pcCommandString != 0x00 )
		{
			/* Is this the start of the required parameter? */
			uxParametersFound++;

			if( uxParametersFound == uxWantedParameter )
			{
				/* How long is the parameter? */
				pcReturn = pcCommandString;
				while( ( ( *pcCommandString ) != 0x00 ) && ( ( *pcCommandString ) != ' ' ) )
				{
					( *pxParameterStringLength )++;
					pcCommandString++;
				}

				break;
			}
		}
		else
		{
			break;
		}
	}

	return pcReturn;
}
/*-----------------------------------------------------------*/


static const xCommandLineInputListItem *current_help_command_point;

static RAW_S32 rsh_help_command( RAW_S8 *pcWriteBuffer, size_t xWriteBufferLen, const RAW_S8 *pcCommandString )
{
	
	RAW_S32 xReturn;

	( void ) pcCommandString;

	if (current_help_command_point == 0) {
	
		/* Reset the pxCommand pointer back to the start of the list. */
		current_help_command_point = &xRegisteredCommands;
	}

	/* Return the next command help string, before moving the pointer on to
	the next command in the list. */
	raw_strncpy( ( char * ) pcWriteBuffer, ( const char * )current_help_command_point->pxCommandLineDefinition->pcHelpString, xWriteBufferLen );
	current_help_command_point = current_help_command_point->pxNext;

	if (current_help_command_point == 0) {

		/* There are no more commands in the list, so there will be no more
		strings to return after this one and 1 should be returned. */
		xReturn = 1;
	}
	else {

		xReturn = 0;
	}

	return xReturn;
}


static RAW_S8 rsh_get_parameters_numbers( const RAW_S8 * pcCommandString )
{
	RAW_S8 cParameters = 0;
	RAW_S32 xLastCharacterWasSpace = 1;

	/* Count the number of space delimited words in pcCommandString. */
	while( *pcCommandString != 0x00 )
	{
		if( ( *pcCommandString ) == ' ' )
		{
			if( xLastCharacterWasSpace != 0 )
			{
				cParameters++;
				xLastCharacterWasSpace = 0;
			}
		}
		else
		{
			xLastCharacterWasSpace = 1;
		}

		pcCommandString++;
	}

	/* The value returned is one less than the number of space delimited words,
	as the first word should be the command itself. */
	return cParameters;
}

