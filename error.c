/*
 * Error code functions
 */
#include "error.h"
#include "types.h"



static uint8 error_ptr = 0;

uint8 Error[SIZE_ERROR_BUFFER]; // hold the last 4 errors


void ClearErrorLog( void )
{
	uint8 i;
	
	for (i = 0; i < SIZE_ERROR_BUFFER; i++)
	{

		Error[i] = NO_ERROR;    
		
	}
	error_ptr = 0;
}	

void AddErrorCode(error_codes error)
{ // Sets the "Status.errorflag" when an error is thrown, 
  //and stores an error code in "Error" dict entry
//	unsigned int err  = (unsigned int)error;
//	__LOG(0, err);
	if (!DuplicateError(error))
	{
		if (error_ptr >= SIZE_ERROR_BUFFER)
			error_ptr = 0;
		Error[error_ptr++] = (uint8)error;
		
	}
}

boolean DuplicateError(error_codes error)
{ // If there is more than one error already present in the
  // slave "error" dictionary, do not store it
	uint8 i;
	
	for (i = 0; i < SIZE_ERROR_BUFFER; i++)
	{	
		if (Error[i] == (uint8) error)
			return TRUE;		
	}
	return FALSE;
}


void RemoveErrorCode(error_codes error)
{
	uint8 i;
	uint8 ErrorCopy[SIZE_ERROR_BUFFER]; // hold the last 4 errors
	
	// first copy the old errors	
	for (i = 0; i < SIZE_ERROR_BUFFER; i++)
	{	
		ErrorCopy[i] = Error[i];
	}
   
    // now clear old error buffer
	ClearErrorLog();

	// now place back the errors if they don't match the one we want to remove.
	for (i=0;i<SIZE_ERROR_BUFFER;i++)
	{
		if (ErrorCopy[i] != error )
		{
			AddErrorCode( ErrorCopy[i] );	
		}
	}	

	
}

