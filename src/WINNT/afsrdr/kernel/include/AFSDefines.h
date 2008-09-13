#ifndef _AFS_DEFINES_H
#define _AFS_DEFINES_H
//
// File: AFSDefines.h
//

//
// Debug information
//

#ifdef DBG

#define AFSPrint        DbgPrint

#define AFS_DEBUG_LOG   1

static inline void AFSBreakPoint() {
#if !defined(KD_DEBUGGER_ENABLED)
#define KD_DEBUGGER_ENABLED DBG
#endif // KD_DEBUGGER_ENABLED   

#if (NTDDI_VERSION >= NTDDI_WS03)
    KdRefreshDebuggerNotPresent();
#endif

#if defined(KD_DEBUGGER_NOT_PRESENT)
    if (KD_DEBUGGER_NOT_PRESENT == FALSE) DbgBreakPoint();
#endif // KD_DEBUGGER_NOT_PRESENT

}

#else

#define AFSPrint   

#define AFSBreakPoint

#endif

//
// For 2K support
//

#ifndef FsRtlSetupAdvancedHeader

#define FSRTL_FLAG_ADVANCED_HEADER              (0x40)
#define FSRTL_FLAG2_SUPPORTS_FILTER_CONTEXTS    (0x02)

#define FsRtlSetupAdvancedHeader( _advhdr, _fmutx )                         \
{                                                                           \
    SetFlag( (_advhdr)->Flags, FSRTL_FLAG_ADVANCED_HEADER );                \
    SetFlag( (_advhdr)->Flags2, FSRTL_FLAG2_SUPPORTS_FILTER_CONTEXTS );     \
    InitializeListHead( &(_advhdr)->FilterContexts );                       \
    if ((_fmutx) != NULL) {                                                 \
        (_advhdr)->FastMutex = (_fmutx);                                    \
    }                                                                       \
}

#endif

//
// Allocation defines
//

#define AFS_GENERIC_MEMORY_TAG       'SFFA'
#define AFS_FCB_ALLOCATION_TAG       'CFFA'
#define AFS_FCB_NP_ALLOCATION_TAG    'NFFA'
#define AFS_VCB_ALLOCATION_TAG       'CVFA'
#define AFS_VCB_NP_ALLOCATION_TAG    'NVFA'
#define AFS_CCB_ALLOCATION_TAG       'CCFA'
#define AFS_WORKER_CB_TAG            'CWFA'
#define AFS_WORK_ITEM_TAG            'IWFA'
#define AFS_POOL_ENTRY_TAG           'EPFA'
#define AFS_PROCESS_CB_TAG           'CPFA'
#define AFS_DIR_BUFFER_TAG           'BDFA'
#define AFS_DIR_ENTRY_TAG            'EDFA'
#define AFS_NAME_BUFFER_TAG          'BNFA'
#define AFS_FILE_CREATE_BUFFER_TAG   'NFFA'
#define AFS_RENAME_REQUEST_TAG       'RFFA'
#define AFS_DIR_ENTRY_NP_TAG         'NDFA'
#define AFS_PROVIDER_CB              'PNFA'
#define AFS_EXTENT_TAG               'xSFA'
#define AFS_EXTENT_REQUEST_TAG       'XSFA'
#define AFS_EXTENT_RELEASE_TAG       'LSFA'
#define AFS_IO_RUN_TAG               'iSFA'
#define AFS_GATHER_TAG               'gSFA'
#define AFS_RENMAME_RESULT_TAG       'RRFA'
#define AFS_UPDATE_RESULT_TAG        'RUFA'
#define AFS_EXTENTS_RESULT_TAG       'XEFA'

#define __Enter

#define try_return(S) { S; goto try_exit; }

//
// Registry names
//

#define AFS_REG_DEBUG_FLAGS              L"DebugFlags"
#define AFS_REG_DEBUG_LEVEL              L"DebugLevel"
#define AFS_REG_SERVER_NAME              L"Server"
#define AFS_REG_MAX_DIRTY                L"MaxDirtyMb"
#define AFS_REG_MAX_IO                   L"MaxIOMb"

//
// Debug information
//

#define AFS_DBG_FLAG_BREAK_ON_ENTRY     0x00000001

//
// Validation info
//
#define AFS_VALIDATE_EXTENTS            0

//
// Control Device name
//

#define AFS_CONTROL_DEVICE_NAME     L"\\Device\\AFSControlDevice"
#define AFS_SYMLINK_NAME            L"\\??\\AFSRedirector"

//
// Device flags
//

#define AFS_DEVICE_FLAGS_REGISTERED      0x00000001

//
// Worker thread count
//

#define AFS_WORKER_COUNT        5

//
// Worker thread states
//

#define AFS_WORKER_INITIALIZED                  0x0001
#define AFS_WORKER_PROCESS_REQUESTS             0x0002

//
// Worker Thread codes
//

#define AFS_WORK_REQUEST_RELEASE                0x0001
#define AFS_WORK_FLUSH_FCB                      0x0002
#define AFS_ASYNCH_READ                         0x0003
#define AFS_ASYNCH_WRITE                        0x0004

//
// Pool state
//

#define POOL_UNKNOWN            0
#define POOL_INACTIVE           1
#define POOL_ACTIVE             2

//
// Worker requests
//

#define AFS_WORK_REQUEST_REMOVE_VOLUME           0x00000001

//
// Worker request flags
//

#define AFS_SYNCHRONOUS_REQUEST                 0x00000001

//
// Object types allocated
//

#define AFS_FILE_FCB                            0x0001
#define AFS_DIRECTORY_FCB                       0x0002
#define AFS_NON_PAGED_FCB                       0x0003
#define AFS_CCB                                 0x0004
#define AFS_ROOT_FCB                            0x0006
#define AFS_VCB                                 0x0007
#define AFS_NON_PAGED_VCB                       0x0008
#define AFS_ROOT_ALL                            0x0009
#define AFS_IOCTL_FCB                           0x000A
#define AFS_MOUNT_POINT_FCB                     0x000B
#define AFS_SYMBOLIC_LINK_FCB                   0x000C

#define AFS_INVALID_FCB                         0x00FF

//
// Fcb flags
//

#define AFS_FCB_INVALID                                      0x00000001
#define AFS_FCB_DIRECTORY_ENUMERATED                         0x00000002
#define AFS_FCB_PENDING_DELETE                               0x00000004
#define AFS_FILE_MODIFIED                                    0x00000008
#define AFS_FCB_DELETED                                      0x00000010
#define AFS_UPDATE_WRITE_TIME                                0x00000020
#define AFS_FCB_INSERTED_ID_TREE                             0x00000040
#define AFS_FCB_STANDALONE_NODE                              0x00000080
#define AFS_FCB_VOLUME_OFFLINE                               0x00000100
#define AFS_FCB_DELETE_FCB_ON_CLOSE                          0x00000200

//
// Fcb lifetime in seconds
//

#define AFS_FCB_LIFETIME             100000000

//
// How big to make the runs
//
#define AFS_MAX_STACK_IO_RUNS              5

#ifndef FlagOn
#define FlagOn(_F,_SF)        ((_F) & (_SF))
#endif

#ifndef BooleanFlagOn
#define BooleanFlagOn(F,SF)   ((BOOLEAN)(((F) & (SF)) != 0))
#endif

#ifndef SetFlag
#define SetFlag(_F,_SF)       ((_F) |= (_SF))
#endif

#ifndef ClearFlag
#define ClearFlag(_F,_SF)     ((_F) &= ~(_SF))
#endif

#define QuadAlign(Ptr) (                \
    ((((ULONG)(Ptr)) + 7) & 0xfffffff8) \
    )

#define CRC32_POLYNOMIAL     0xEDB88320L;

//
// Define one second in terms of 100 nS units
//

#define AFS_ONE_SECOND          10000000

#define AFS_SERVER_FLUSH_DELAY  30
#define AFS_SERVER_PURGE_DELAY  3000
//
// PURGE_SLEEP is the number of PURGE_DELAYS we wait before we will unilaterally 
// give back extents.
//
// If the Server asks us, we will start at PURGE_SLEEP of delays and then work back
//
#define AFS_SERVER_PURGE_SLEEP  6

//
// Read ahead granularity
//

#define READ_AHEAD_GRANULARITY      0x10000     // 64KB

#define AFS_DIR_ENUM_BUFFER_LEN   (16 * 1024)

//
// IS_BYTE_OFFSET_WRITE_TO_EOF
// liOffset - should be from Irp.StackLocation.Parameters.Write.ByteOffset
// this macro checks to see if the Offset Large_Integer points to the 
// special constant value which denotes to start the write at EndOfFile
//
#define IS_BYTE_OFFSET_WRITE_TO_EOF(liOffset) \
    (((liOffset).LowPart == FILE_WRITE_TO_END_OF_FILE) \
      && ((liOffset).HighPart == 0xFFFFFFFF))

//
// Ccb Directory enum flags
//

#define CCB_FLAG_DIR_OF_DIRS_ONLY           0x00000001
#define CCB_FLAG_FULL_DIRECTORY_QUERY       0x00000002
#define CCB_FLAG_MASK_CONTAINS_WILD_CARDS   0x00000004
#define CCB_FLAG_FREE_FULL_PATHNAME         0x00000008

//
// DirEntry flags
//

#define AFS_DIR_RELEASE_NAME_BUFFER             0x00000001
#define AFS_DIR_ENTRY_NOT_EVALUATED             0x00000002
#define AFS_DIR_ENTRY_CASE_INSENSTIVE_LIST_HEAD 0x00000004
#define AFS_DIR_ENTRY_NOT_IN_PARENT_TREE        0x00000008
#define AFS_DIR_RELEASE_DIRECTORY_NODE          0x00000010

//
// Vcb flags
//

#define AFS_VCB_DIRTY_INFORMATION        0x00000001

//
// Network provider errors
//

#define WN_SUCCESS                              0L
#define WN_ALREADY_CONNECTED                    85L
#define WN_OUT_OF_MEMORY                        8L
#define WN_NOT_CONNECTED                        2250L
#define WN_BAD_NETNAME                          67L

//
// Method for determining the different control device open requests
//

#define AFS_CONTROL_INSTANCE            0x00000001
#define AFS_REDIRECTOR_INSTANCE         0x00000002

//
// Extent flags
//

#define AFS_EXTENT_DIRTY                0x00000001


// 
// Extent skip list sizes
// 
#define AFS_NUM_EXTENT_LISTS    3

//
// Extents skip lists
//
// We use constant sizes.
//
#define AFS_EXTENT_SIZE         (4*1024)
#define AFS_EXTENTS_LIST        0
//
// A max of 64 extents in ther first skip list
#define AFS_EXTENT_SKIP1_BITS   6

//
// Then 128 bits in the second skip list
#define AFS_EXTENT_SKIP2_BITS   7

//
// This means that the top list skips in steps of 2^25 (=12+6+7) which
// is 32 Mb.  It is to be expected that files which are massively
// larger that this will not be fully mapped.
//
#define AFS_EXTENT_SKIP1_SIZE (AFS_EXTENT_SIZE << AFS_EXTENT_SKIP1_BITS)
#define AFS_EXTENT_SKIP2_SIZE (AFS_EXTENT_SKIP1_SIZE << AFS_EXTENT_SKIP2_BITS)

#define AFS_EXTENTS_MASKS { (AFS_EXTENT_SIZE-1),       \
                            (AFS_EXTENT_SKIP1_SIZE-1), \
                            (AFS_EXTENT_SKIP2_SIZE-1) }

//
// Maximum count to release at a time
//

#define AFS_MAXIMUM_EXTENT_RELEASE_COUNT        100

// {41966169-3FD7-4392-AFE4-E6A9D0A92C72}  - generated using guidgen.exe
DEFINE_GUID (GUID_SD_AFS_REDIRECTOR_CONTROL_OBJECT,
        0x41966169, 0x3fd7, 0x4392, 0xaf, 0xe4, 0xe6, 0xa9, 0xd0, 0xa9, 0x2c, 0x72);

//
// Debug log length
// 

#define AFS_DBG_LOG_LENGTH              (256 * 1024)

#endif /* _AFS_DEFINES_H */
