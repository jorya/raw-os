#include <raw_api.h>

static RAW_SEMAPHORE device_sem;
#define MaxRegDev 64

#define DEVICE_NAME_LENGTH	20

typedef RAW_S32		FN;		/* Function code */
typedef RAW_S32		RNO;		/* Rendezvous number */
typedef RAW_U32		ATR;		/* Object/handler attribute */
typedef void		(*FP)();	/* Function address general */


/*
 * Device registration information
 */
typedef struct t_ddev {

	void	*exinf;		/* Extended information */
	ATR	drvatr;		/* Driver attribute */
	ATR	devatr;		/* Device attribute */
	RAW_S32	nsub;		/* Number of subunits */
	RAW_S32	blksz;		/* Specific data block size (-1: Unknown) */
	FP	openfn;		/* Open function */
	FP	closefn;	/* Close function */
	FP	execfn;		/* Execute function */
	FP	waitfn;		/* Completion wait function */
	FP	abortfn;	/* Abort function */
	FP	eventfn;	/* Event function */

} T_DDEV;

/*
 * Device initial setting information
 */
typedef struct t_idev {
	ID	evtmbfid;	/* Message buffer ID for event notification */
} T_IDEV;


/*
 * Device registration information
 */
typedef struct DeviceControlBlock {
	LIST	q;
	RAW_U8	devnm[DEVICE_NAME_LENGTH + 1];	/* Device name */
	T_DDEV	ddev;			/* Registration information */
	LIST	openq;			/* Open device management queue */
	LIST	syncq;			/* Task Queue for Synchronization */
} DevCB;



DevCB		*DevCBtbl;	/* Device registration informationtable */
LIST		UsedDevCB;	/* In-use queue */
LIST		FreeDevCB;	/* Unused queue */
RAW_S32		MaxRegDev;  /* Maximum number of device registrations */

#define DID(devcb)		( ((devcb) - DevCBtbl + 1) << 8 )
#define DEVID(devcb, unitno)	( DID(devcb) + (unitno) )
#define DEVCB(devid)		( DevCBtbl + (((devid) >> 8) - 1) )
#define UNITNO(devid)		( (devid) & 0xff )


#define MAX_UNIT	255		/* Maximum number of subunits */

static int chklen(unsigned char *p, int max )
{
	int	len = 0;

	while ( *p++ != '\0' ) {
		len++;
		if ( --max == 0 ) {
			break;
		}
	}
	return len;
}

RAW_S32 ChkSpaceBstrR(RAW_U8 *str, INT max )
{
	return chklen(str, max);
}

RAW_S32 ChkSpaceBstrRW(RAW_U8 *str, INT max )
{
	return chklen(str, max);
}


/*
 * Search registration device
 */
DevCB* searchDevCB(RAW_U8 *devnm )
{
	LIST	*q;
	DevCB	*devcb;

	for ( q = UsedDevCB.next; q != &UsedDevCB; q = q->next ) {
		devcb = (DevCB*)q;

		if (raw_strcmp((char*)devcb->devnm, (char*)devnm) == 0 ) {
			return devcb; /* Found */
		}
	}

	return 0;
}


LIST *list_remove_next(LIST *que )
{
	LIST	*entry;

	if ( que->next == que ) {
		return NULL;
	}

	entry = que->next;
	que->next = (struct LIST*)entry->next;
	entry->next->prev = que;

	return entry;
}

/*
 * Get DevCB for new registration
 */
static DevCB* newDevCB(RAW_U8 *devnm )
{
	DevCB	*devcb;

	devcb = (DevCB*)list_remove_next(&FreeDevCB);
	if ( devcb == NULL ) {
		return NULL; /* No space */
	}

	raw_strncpy((char*)devcb->devnm, (char*)devnm, DEVICE_NAME_LENGTH + 1);
	list_init(&devcb->openq);
	list_init(&devcb->syncq);

	list_insert(&devcb->q, &UsedDevCB);

	return devcb;
}

/*
 * Free DevCB
 */
static void delDevCB( DevCB *devcb )
{
	list_delete(&devcb->q);
	list_insert(&devcb->q, &FreeDevCB);
	devcb->devnm[0] = '\0';
}


/*
 * Device registration
 */
RAW_U32 _tk_def_dev( CONST UB *devnm, CONST T_DDEV *ddev, void *caller_gp )
{
	DevCB	*devcb;
	INT	len, evttyp;
	ER	ercd;


	len = ChkSpaceBstrR(devnm, 0);
	
	if ( len <= 0 || len > MaxRegDev ) {
		
		RAW_ASSERT(0);
	}


	/* Search whether 'devnm' device is registered */
	devcb = searchDevCB(devnm);
	if ( devcb == 0 ) {
		if ( ddev == 0 ) {
			ercd = E_NOEXS;
			goto err_ret2;
		}

		/* Get 'devcb' for new registration because it is not
		   registered */
		devcb = newDevCB(devnm);
		if ( devcb == NULL ) {
			ercd = E_LIMIT;
			goto err_ret2;
		}
	}

	if ( ddev != NULL ) {
		/* Set/update device registration information */
		devcb->ddev = *ddev;

		evttyp = TSEVT_DEVICE_REGIST;
	} else {
		if ( !isQueEmpty(&devcb->openq) ) {
			/* In use (open) */
			ercd = E_BUSY;
			goto err_ret2;
		}

		/* Device unregistration */
		delDevCB(devcb);
		evttyp = TSEVT_DEVICE_DELETE;
	}

	return DID(devcb);
}



/*
 * Initialization of device registration information table
 */
static void init_device( void )
{
	DevCB	*devcb;
	INT	num;
	ER	ercd;

	num = MaxRegDev;

	/* Generate device registration information table */
	DevCBtbl = raw_malloc(num * sizeof(DevCB));
	
	if (DevCBtbl == NULL) {
		
		RAW_ASSERT(0);
	}

	list_init(&UsedDevCB);
	list_init(&FreeDevCB);

	devcb = DevCBtbl;
	while ( num-- > 0 ) {
		list_insert(&FreeDevCB, &devcb->q);
		devcb->devnm[0] = '\0';
		devcb++;
	}

	return RAW_SUCCESS;
	
}


/*
 * Initialization of system management
 */
RAW_U16 init_device_management( void )
{
	
	RAW_U16 ret;

	ret = raw_semaphore_create(&device_sem, "device_sem", 1);

	if (ret != RAW_SUCCESS) {

		return ret;
	}

	/* Generate device registration information table */
	init_device();

	/* Initialization of device input/output-related */


	/* Initialization of device initial setting information */


	/* Subsystem registration */
	return RAW_SUCCESS;
	
}

/*
 * Finalization sequence of system management
 */
EXPORT ER finish_devmgr( void )
{
	ER	ercd;

	/* Unregister subsystem */
	ercd = tk_def_ssy(DEVICE_SVC, NULL);
#ifdef DEBUG
	if ( ercd < E_OK ) {
		extension_printf(("1. finish_devmgr -> tk_def_ssy ercd = %d\n", ercd));
	}
#endif

	/* Unregister device initial setting information */
	ercd = delIDev();
#ifdef DEBUG
	if ( ercd < E_OK ) {
		extension_printf(("2. finish_devmgr -> delIDev ercd = %d\n", ercd));
	}
#endif

	/* Finalization sequence of device input/output-related */
	ercd = finishDevIO();
#ifdef DEBUG
	if ( ercd < E_OK ) {
		extension_printf(("3. finish_devmgr -> finishDevIO ercd = %d\n", ercd));
	}
#endif

	/* Delete device registration information table */
	if ( DevCBtbl != NULL ) {
		Ifree(DevCBtbl);
		DevCBtbl = NULL;
	}

	/* Delete semaphore for device management synchronous control */
	if ( DevMgrSync > 0 ) {
		tk_del_sem(DevMgrSync);
		DevMgrSync = 0;
	}

	/* Delete lock for device management exclusive control */
	DeleteMLock(&DevMgrLock);

	return ercd;
}

