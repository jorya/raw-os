#include <raw_api.h>
#include <rsh.h>
#include <mm/raw_malloc.h>
#include <lib_string.h>
#include <mm/raw_page.h>
#include <device/device_interface.h>


#define boardAVAILABLE_DEVICES_LIST												\
{																				\
	{ ( const char * const ) "/UART2/", eUART_TYPE, ( void * ) 0 },	\
	{ ( const char * const ) "/SSP2/", eSSP_TYPE, ( void * ) 0 },		\
	{ ( const char * const ) "/IFC3/", eI2C_TYPE, ( void * ) 0 }		\
}



char boardFreeRTOS_PopulateFunctionPointers( const Peripheral_Types_t ePeripheralType, Peripheral_Control_t * const pxPeripheralControl )
{


	return 0;
}





/*-----------------------------------------------------------*/

/* Holds the list of peripherals that are available to the FreeRTOS+IO
interface.  boardAVAILABLE_DEVICED_LIST is defined in FreeRTOS_IO_BSP.h, and is
specific to a hardware platform. */
static const Available_Peripherals_t xAvailablePeripherals[] = boardAVAILABLE_DEVICES_LIST;

/*-----------------------------------------------------------*/

/* See the function prototype definition for documentation information. */
Peripheral_Descriptor_t raw_open(const char *pcPath, const unsigned int ulFlags)
{
	char xIndex, xInitialiseResult;
	const char xNumberOfPeripherals = sizeof( xAvailablePeripherals ) / sizeof( Available_Peripherals_t );
	char cPeripheralNumber;
	Peripheral_Control_t *pxPeripheralControl = 0;

	/* The flags exist to maintain a standard looking interface, but are not
	(yet) used. */
	( void ) ulFlags;

	/* Search for the peripheral in the list of peripherals for the board being
	used. */
	for( xIndex = 0; xIndex < xNumberOfPeripherals; xIndex++ )
	{
		if( raw_strcmp( ( const char * const ) pcPath, ( const char * const ) xAvailablePeripherals[ xIndex ].pcPath ) == 0 )
		{
			/* pcPath is a valid path, search no further. */
			break;
		}
	}

	if( xIndex < xNumberOfPeripherals )
	{
		/* pcPath was a valid path.  Extract the peripheral number. */
		while( ( *pcPath < '0' ) || ( *pcPath > '9' ) )
		{
			pcPath++;
		}

		/* Convert the number from its ASCII representation. */
		cPeripheralNumber = *pcPath - '0';

		pxPeripheralControl = raw_malloc( sizeof( Peripheral_Control_t ) );
		
		if (pxPeripheralControl) {
		
			/* Initialise the common parts of the control structure. */
			pxPeripheralControl->pxTxControl = 0;
			pxPeripheralControl->pxRxControl = 0;
			pxPeripheralControl->pxDevice = &( xAvailablePeripherals[ xIndex ] );
			pxPeripheralControl->cPeripheralNumber = cPeripheralNumber;

			/* Initialise the peripheral specific parts of the control
			structure, and call the peripheral specific open function. */
			xInitialiseResult = boardFreeRTOS_PopulateFunctionPointers( xAvailablePeripherals[ xIndex ].xPeripheralType, pxPeripheralControl );

			if( xInitialiseResult != 0 )
			{
				/* Something went wrong.  Free up resources and return NULL. */
				raw_free( pxPeripheralControl );
				pxPeripheralControl = 0;
			}
		}
	}

	return ( Peripheral_Descriptor_t ) pxPeripheralControl;
}



