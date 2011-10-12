/*
 * Copyright (c) 2008, 2009, 2010, 2011 Kernel Drivers, LLC.
 * Copyright (c) 2009, 2010, 2011 Your File System, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright
 *   notice,
 *   this list of conditions and the following disclaimer in the
 *   documentation
 *   and/or other materials provided with the distribution.
 * - Neither the names of Kernel Drivers, LLC and Your File System, Inc.
 *   nor the names of their contributors may be used to endorse or promote
 *   products derived from this software without specific prior written
 *   permission from Kernel Drivers, LLC and Your File System, Inc.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

//
// File: AFSGeneric.cpp
//

#include "AFSCommon.h"

//
// Function: AFSExceptionFilter
//
// Description:
//
//      This function is the exception handler
//
// Return:
//
//      A status is returned for the function
//

ULONG
AFSExceptionFilter( IN ULONG Code,
                    IN PEXCEPTION_POINTERS ExceptPtrs)
{

    PEXCEPTION_RECORD ExceptRec;
    PCONTEXT Context;

    __try
    {

        ExceptRec = ExceptPtrs->ExceptionRecord;

        Context = ExceptPtrs->ContextRecord;

        AFSDbgLogMsg( 0,
                      0,
                      "AFSExceptionFilter (Library) - EXR %p CXR %p Code %08lX Address %p Routine %p\n",
                      ExceptRec,
                      Context,
                      ExceptRec->ExceptionCode,
                      ExceptRec->ExceptionAddress,
                      (void *)AFSExceptionFilter);

        DbgPrint("**** Exception Caught in AFS Redirector Library ****\n");

        DbgPrint("\n\nPerform the following WnDbg Cmds:\n");
        DbgPrint("\n\t.exr %p ;  .cxr %p\n\n", ExceptRec, Context);

        DbgPrint("**** Exception Complete from AFS Redirector Library ****\n");

        if( BooleanFlagOn( AFSDebugFlags, AFS_DBG_BUGCHECK_EXCEPTION))
        {

            KeBugCheck( (ULONG)-2);
        }
        else
        {

            AFSBreakPoint();
        }
    }
    __except( EXCEPTION_EXECUTE_HANDLER)
    {

        NOTHING;
    }

    return EXCEPTION_EXECUTE_HANDLER;
}

//
// Function: AFSLibExAllocatePoolWithTag()
//
// Purpose: Allocate Pool Memory.  If BugCheck Exception flag
//          is configured on, then bugcheck the system if
//          a memory allocation fails.  The routine should be
//          used for all memory allocations that are to be freed
//          when the library is unloaded.  Memory allocations that
//          are to survive library unload and reload should be
//          performed using AFSExAllocatePoolWithTag() which is
//          provided by the AFS Framework.
//
// Parameters:
//                POOL_TYPE PoolType - Paged or NonPaged
//                SIZE_T NumberOfBytes - requested allocation size
//                ULONG  Tag - Pool Allocation Tag to be applied for tracking
//
// Return:
//                void * - the memory allocation
//

void *
AFSLibExAllocatePoolWithTag( IN POOL_TYPE  PoolType,
                             IN SIZE_T  NumberOfBytes,
                             IN ULONG  Tag)
{

    void *pBuffer = NULL;

    pBuffer = ExAllocatePoolWithTag( PoolType,
                                     NumberOfBytes,
                                     Tag);

    if( pBuffer == NULL)
    {

        if( BooleanFlagOn( AFSDebugFlags, AFS_DBG_BUGCHECK_EXCEPTION))
        {

            KeBugCheck( (ULONG)-2);
        }
        else
        {

            AFSDbgLogMsg( 0,
                          0,
                          "AFSLibExAllocatePoolWithTag failure Type %08lX Size %08lX Tag %08lX %08lX\n",
                          PoolType,
                          NumberOfBytes,
                          Tag,
                          PsGetCurrentThread());

            AFSBreakPoint();
        }
    }

    return pBuffer;
}

//
// Function: AFSAcquireExcl()
//
// Purpose: Called to acquire a resource exclusive with optional wait
//
// Parameters:
//                PERESOURCE Resource - Resource to acquire
//                BOOLEAN Wait - Whether to block
//
// Return:
//                BOOLEAN - Whether the mask was acquired
//

BOOLEAN
AFSAcquireExcl( IN PERESOURCE Resource,
                IN BOOLEAN wait)
{

    BOOLEAN bStatus = FALSE;

    //
    // Normal kernel APCs must be disabled before calling
    // ExAcquireResourceExclusiveLite. Otherwise a bugcheck occurs.
    //

    KeEnterCriticalRegion();

    bStatus = ExAcquireResourceExclusiveLite( Resource,
                                              wait);

    if( !bStatus)
    {

        KeLeaveCriticalRegion();
    }

    return bStatus;
}

BOOLEAN
AFSAcquireSharedStarveExclusive( IN PERESOURCE Resource,
                                 IN BOOLEAN Wait)
{

    BOOLEAN bStatus = FALSE;

    KeEnterCriticalRegion();

    bStatus = ExAcquireSharedStarveExclusive( Resource,
                                              Wait);

    if( !bStatus)
    {

        KeLeaveCriticalRegion();
    }

    return bStatus;
}

//
// Function: AFSAcquireShared()
//
// Purpose: Called to acquire a resource shared with optional wait
//
// Parameters:
//                PERESOURCE Resource - Resource to acquire
//                BOOLEAN Wait - Whether to block
//
// Return:
//                BOOLEAN - Whether the mask was acquired
//

BOOLEAN
AFSAcquireShared( IN PERESOURCE Resource,
                  IN BOOLEAN wait)
{

    BOOLEAN bStatus = FALSE;

    KeEnterCriticalRegion();

    bStatus = ExAcquireResourceSharedLite( Resource,
                                           wait);

    if( !bStatus)
    {

        KeLeaveCriticalRegion();
    }

    return bStatus;
}

//
// Function: AFSReleaseResource()
//
// Purpose: Called to release a resource
//
// Parameters:
//                PERESOURCE Resource - Resource to release
//
// Return:
//                None
//

void
AFSReleaseResource( IN PERESOURCE Resource)
{

    AFSDbgLogMsg( AFS_SUBSYSTEM_LOCK_PROCESSING,
                  AFS_TRACE_LEVEL_VERBOSE,
                  "AFSReleaseResource Releasing lock %08lX Thread %08lX\n",
                  Resource,
                  PsGetCurrentThread());

    ExReleaseResourceLite( Resource);

    KeLeaveCriticalRegion();

    return;
}

void
AFSConvertToShared( IN PERESOURCE Resource)
{

    AFSDbgLogMsg( AFS_SUBSYSTEM_LOCK_PROCESSING,
                  AFS_TRACE_LEVEL_VERBOSE,
                  "AFSConvertToShared Converting lock %08lX Thread %08lX\n",
                  Resource,
                  PsGetCurrentThread());

    ExConvertExclusiveToSharedLite( Resource);

    return;
}

//
// Function: AFSCompleteRequest
//
// Description:
//
//      This function completes irps
//
// Return:
//
//      A status is returned for the function
//

void
AFSCompleteRequest( IN PIRP Irp,
                    IN ULONG Status)
{

    Irp->IoStatus.Status = Status;

    IoCompleteRequest( Irp,
                       IO_NO_INCREMENT);

    return;
}

//
// Function: AFSBuildCRCTable
//
// Description:
//
//      This function builds the CRC table for mapping filenames to a CRC value.
//
// Return:
//
//      A status is returned for the function
//

void
AFSBuildCRCTable()
{
    ULONG crc;
    int i, j;

    for ( i = 0; i <= 255; i++)
    {
        crc = i;
        for ( j = 8; j > 0; j--)
        {
            if (crc & 1)
            {
                crc = ( crc >> 1 ) ^ CRC32_POLYNOMIAL;
            }
            else
            {
                crc >>= 1;
            }
        }

        AFSCRCTable[ i ] = crc;
    }
}

//
// Function: AFSGenerateCRC
//
// Description:
//
//      Given a device and filename this function generates a CRC
//
// Return:
//
//      A status is returned for the function
//

ULONG
AFSGenerateCRC( IN PUNICODE_STRING FileName,
                IN BOOLEAN UpperCaseName)
{

    ULONG crc;
    ULONG temp1, temp2;
    UNICODE_STRING UpcaseString;
    WCHAR *lpbuffer;
    USHORT size = 0;

    if( !AFSCRCTable[1])
    {
        AFSBuildCRCTable();
    }

    crc = 0xFFFFFFFFL;

    if( UpperCaseName)
    {

        RtlUpcaseUnicodeString( &UpcaseString,
                                FileName,
                                TRUE);

        lpbuffer = UpcaseString.Buffer;

        size = (UpcaseString.Length/sizeof( WCHAR));
    }
    else
    {

        lpbuffer = FileName->Buffer;

        size = (FileName->Length/sizeof( WCHAR));
    }

    while (size--)
    {
        temp1 = (crc >> 8) & 0x00FFFFFFL;
        temp2 = AFSCRCTable[((int)crc ^ *lpbuffer++) & 0xff];
        crc = temp1 ^ temp2;
    }

    if( UpperCaseName)
    {

        RtlFreeUnicodeString( &UpcaseString);
    }

    crc ^= 0xFFFFFFFFL;

    return crc;
}

void *
AFSLockSystemBuffer( IN PIRP Irp,
                     IN ULONG Length)
{

    NTSTATUS Status = STATUS_SUCCESS;
    void *pAddress = NULL;

    if( Irp->MdlAddress != NULL)
    {

        pAddress = MmGetSystemAddressForMdlSafe( Irp->MdlAddress,
                                                 NormalPagePriority);
    }
    else if( Irp->AssociatedIrp.SystemBuffer != NULL)
    {

        pAddress = Irp->AssociatedIrp.SystemBuffer;
    }
    else if( Irp->UserBuffer != NULL)
    {

        Irp->MdlAddress = IoAllocateMdl( Irp->UserBuffer,
                                         Length,
                                         FALSE,
                                         FALSE,
                                         Irp);

        if( Irp->MdlAddress != NULL)
        {

            //
            //  Lock the new Mdl in memory.
            //

            __try
            {
                PIO_STACK_LOCATION pIoStack;
                pIoStack = IoGetCurrentIrpStackLocation( Irp);


                MmProbeAndLockPages( Irp->MdlAddress, KernelMode,
                                     (pIoStack->MajorFunction == IRP_MJ_READ) ? IoWriteAccess : IoReadAccess);

                pAddress = MmGetSystemAddressForMdlSafe( Irp->MdlAddress, NormalPagePriority );

            }
            __except( AFSExceptionFilter( GetExceptionCode(), GetExceptionInformation()) )
            {

                IoFreeMdl( Irp->MdlAddress );
                Irp->MdlAddress = NULL;
                pAddress = NULL;
            }
        }
    }

    return pAddress;
}

void *
AFSLockUserBuffer( IN void *UserBuffer,
                   IN ULONG BufferLength,
				   OUT MDL ** Mdl)
{

    NTSTATUS ntStatus = STATUS_SUCCESS;
    void *pAddress = NULL;
	MDL *pMdl = NULL;

	__Enter
	{

        pMdl = IoAllocateMdl( UserBuffer,
                              BufferLength,
                              FALSE,
                              FALSE,
                              NULL);

		if( pMdl == NULL)
		{

			try_return( ntStatus = STATUS_INSUFFICIENT_RESOURCES);
		}

        //
        //  Lock the new Mdl in memory.
        //

        __try
        {

            MmProbeAndLockPages( pMdl,
								 KernelMode,
                                 IoWriteAccess);

            pAddress = MmGetSystemAddressForMdlSafe( pMdl,
													 NormalPagePriority);
        }
        __except( AFSExceptionFilter( GetExceptionCode(), GetExceptionInformation()) )
        {

            IoFreeMdl( pMdl);
            pMdl = NULL;
            pAddress = NULL;
        }

		if( pMdl != NULL)
		{

			*Mdl = pMdl;
		}

try_exit:

        NOTHING;
    }

    return pAddress;
}

void *
AFSMapToService( IN PIRP Irp,
                 IN ULONG ByteCount)
{

    NTSTATUS ntStatus = STATUS_SUCCESS;
    void *pMappedBuffer = NULL;
    AFSDeviceExt *pDevExt = (AFSDeviceExt *)AFSControlDeviceObject->DeviceExtension;
    KAPC stApcState;

    __Enter
    {

        if( pDevExt->Specific.Control.ServiceProcess == NULL)
        {

            try_return( ntStatus = STATUS_DEVICE_NOT_READY);
        }

        if( Irp->MdlAddress == NULL)
        {

            if( AFSLockSystemBuffer( Irp,
                                     ByteCount) == NULL)
            {

                try_return( ntStatus = STATUS_INSUFFICIENT_RESOURCES);
            }
        }

        //
        // Attach to the service process for mapping
        //

        KeStackAttachProcess( pDevExt->Specific.Control.ServiceProcess,
                              (PRKAPC_STATE)&stApcState);

        pMappedBuffer = MmMapLockedPagesSpecifyCache( Irp->MdlAddress,
                                                      UserMode,
                                                      MmCached,
                                                      NULL,
                                                      FALSE,
                                                      NormalPagePriority);

        KeUnstackDetachProcess( (PRKAPC_STATE)&stApcState);

try_exit:

        NOTHING;
    }

    return pMappedBuffer;
}

NTSTATUS
AFSUnmapServiceMappedBuffer( IN void *MappedBuffer,
                             IN PMDL Mdl)
{

    NTSTATUS ntStatus = STATUS_SUCCESS;
    void *pMappedBuffer = NULL;
    AFSDeviceExt *pDevExt = (AFSDeviceExt *)AFSControlDeviceObject->DeviceExtension;
    KAPC stApcState;

    __Enter
    {

        if( pDevExt->Specific.Control.ServiceProcess == NULL)
        {

            try_return( ntStatus = STATUS_DEVICE_NOT_READY);
        }

        if( Mdl != NULL)
        {

            //
            // Attach to the service process for mapping
            //

            KeStackAttachProcess( pDevExt->Specific.Control.ServiceProcess,
                                  (PRKAPC_STATE)&stApcState);

            MmUnmapLockedPages( MappedBuffer,
                                Mdl);

            KeUnstackDetachProcess( (PRKAPC_STATE)&stApcState);
        }

try_exit:

        NOTHING;
    }

    return ntStatus;
}

NTSTATUS
AFSInitializeLibraryDevice()
{

    NTSTATUS ntStatus = STATUS_SUCCESS;
    AFSDeviceExt *pDeviceExt = NULL;

    __Enter
    {

        pDeviceExt = (AFSDeviceExt *)AFSLibraryDeviceObject->DeviceExtension;

        //
        // The PIOCtl file name
        //

        RtlInitUnicodeString( &AFSPIOCtlName,
                              AFS_PIOCTL_FILE_INTERFACE_NAME);

        //
        // And the global root share name
        //

        RtlInitUnicodeString( &AFSGlobalRootName,
                              AFS_GLOBAL_ROOT_SHARE_NAME);

    }

    return ntStatus;
}

NTSTATUS
AFSRemoveLibraryDevice()
{

    NTSTATUS ntStatus = STATUS_SUCCESS;

    __Enter
    {

    }

    return ntStatus;
}

NTSTATUS
AFSDefaultDispatch( IN PDEVICE_OBJECT DeviceObject,
                    IN PIRP Irp)
{

    NTSTATUS            ntStatus = STATUS_INVALID_DEVICE_REQUEST;
    PIO_STACK_LOCATION  pIrpSp = IoGetCurrentIrpStackLocation( Irp);

    AFSCompleteRequest( Irp,
                        ntStatus);

    return ntStatus;
}

NTSTATUS
AFSInitializeGlobalDirectoryEntries()
{

    NTSTATUS ntStatus = STATUS_SUCCESS;
    AFSDirectoryCB *pDirNode = NULL;
    ULONG ulEntryLength = 0;
    AFSDeviceExt *pDeviceExt = (AFSDeviceExt *)AFSRDRDeviceObject->DeviceExtension;
    AFSObjectInfoCB *pObjectInfoCB = NULL;
    AFSNonPagedDirectoryCB *pNonPagedDirEntry = NULL;

    __Enter
    {

        //
        // Initialize the global . entry
        //

        pObjectInfoCB = AFSAllocateObjectInfo( &AFSGlobalRoot->ObjectInformation,
                                               0);

        if( pObjectInfoCB == NULL)
        {

            AFSDbgLogMsg( AFS_SUBSYSTEM_LOAD_LIBRARY | AFS_SUBSYSTEM_INIT_PROCESSING,
                          AFS_TRACE_LEVEL_ERROR,
                          "AFSInitializeGlobalDirectory AFSAllocateObjectInfo failure %08lX\n",
                          ntStatus);

            try_return( ntStatus = STATUS_INSUFFICIENT_RESOURCES);
        }

        InterlockedIncrement( &pObjectInfoCB->ObjectReferenceCount);

        AFSDbgLogMsg( AFS_SUBSYSTEM_OBJECT_REF_COUNTING,
                      AFS_TRACE_LEVEL_VERBOSE,
                      "AFSInitializeGlobalDirectoryEntries Increment count on object %08lX Cnt %d\n",
                      pObjectInfoCB,
                      pObjectInfoCB->ObjectReferenceCount);

        ntStatus = STATUS_SUCCESS;

        ulEntryLength = sizeof( AFSDirectoryCB) +
                                     sizeof( WCHAR);

        pDirNode = (AFSDirectoryCB *)AFSLibExAllocatePoolWithTag( PagedPool,
                                                                  ulEntryLength,
                                                                  AFS_DIR_ENTRY_TAG);

        if( pDirNode == NULL)
        {

            AFSDeleteObjectInfo( pObjectInfoCB);

            AFSDbgLogMsg( AFS_SUBSYSTEM_LOAD_LIBRARY | AFS_SUBSYSTEM_INIT_PROCESSING,
                          AFS_TRACE_LEVEL_ERROR,
                          "AFSInitializeGlobalDirectory AFS_DIR_ENTRY_TAG allocation failure\n");

            try_return( ntStatus = STATUS_INSUFFICIENT_RESOURCES);
        }

        pNonPagedDirEntry = (AFSNonPagedDirectoryCB *)AFSLibExAllocatePoolWithTag( NonPagedPool,
                                                                                   sizeof( AFSNonPagedDirectoryCB),
                                                                                   AFS_DIR_ENTRY_NP_TAG);

        if( pNonPagedDirEntry == NULL)
        {

            ExFreePool( pDirNode);

            AFSDeleteObjectInfo( pObjectInfoCB);

            AFSDbgLogMsg( AFS_SUBSYSTEM_LOAD_LIBRARY | AFS_SUBSYSTEM_INIT_PROCESSING,
                          AFS_TRACE_LEVEL_ERROR,
                          "AFSInitializeGlobalDirectory AFS_DIR_ENTRY_NP_TAG allocation failure\n");

            try_return( ntStatus = STATUS_INSUFFICIENT_RESOURCES);
        }

        RtlZeroMemory( pDirNode,
                       ulEntryLength);

        RtlZeroMemory( pNonPagedDirEntry,
                       sizeof( AFSNonPagedDirectoryCB));

        ExInitializeResourceLite( &pNonPagedDirEntry->Lock);

        pDirNode->NonPaged = pNonPagedDirEntry;

        pDirNode->ObjectInformation = pObjectInfoCB;

        //
        // Set valid entry
        //

        SetFlag( pDirNode->Flags, AFS_DIR_ENTRY_NOT_IN_PARENT_TREE | AFS_DIR_ENTRY_FAKE | AFS_DIR_ENTRY_VALID);

        pDirNode->FileIndex = (ULONG)AFS_DIR_ENTRY_DOT_INDEX;

        //
        // Setup the names in the entry
        //

        pDirNode->NameInformation.FileName.Length = sizeof( WCHAR);

        pDirNode->NameInformation.FileName.MaximumLength = sizeof( WCHAR);

        pDirNode->NameInformation.FileName.Buffer = (WCHAR *)((char *)pDirNode + sizeof( AFSDirectoryCB));

        pDirNode->NameInformation.FileName.Buffer[ 0] = L'.';

        //
        // Populate the rest of the data
        //

        pObjectInfoCB->FileType = AFS_FILE_TYPE_DIRECTORY;

        pObjectInfoCB->FileAttributes = FILE_ATTRIBUTE_DIRECTORY;

        AFSGlobalDotDirEntry = pDirNode;

        //
        // Now the .. entry
        //

        pObjectInfoCB = AFSAllocateObjectInfo( &AFSGlobalRoot->ObjectInformation,
                                               0);

        if( pObjectInfoCB == NULL)
        {

            AFSDbgLogMsg( AFS_SUBSYSTEM_LOAD_LIBRARY | AFS_SUBSYSTEM_INIT_PROCESSING,
                          AFS_TRACE_LEVEL_ERROR,
                          "AFSInitializeGlobalDirectory AFSAllocateObjectInfo (2) failure %08lX\n",
                          ntStatus);

            try_return( ntStatus = STATUS_INSUFFICIENT_RESOURCES);
        }

        InterlockedIncrement( &pObjectInfoCB->ObjectReferenceCount);

        AFSDbgLogMsg( AFS_SUBSYSTEM_OBJECT_REF_COUNTING,
                      AFS_TRACE_LEVEL_VERBOSE,
                      "AFSInitializeGlobalDirectoryEntries Increment count on object %08lX Cnt %d\n",
                      pObjectInfoCB,
                      pObjectInfoCB->ObjectReferenceCount);

        ntStatus = STATUS_SUCCESS;

        ulEntryLength = sizeof( AFSDirectoryCB) +
                                     ( 2 * sizeof( WCHAR));

        pDirNode = (AFSDirectoryCB *)AFSLibExAllocatePoolWithTag( PagedPool,
                                                                  ulEntryLength,
                                                                  AFS_DIR_ENTRY_TAG);

        if( pDirNode == NULL)
        {

            AFSDeleteObjectInfo( pObjectInfoCB);

            try_return( ntStatus = STATUS_INSUFFICIENT_RESOURCES);
        }

        pNonPagedDirEntry = (AFSNonPagedDirectoryCB *)AFSLibExAllocatePoolWithTag( NonPagedPool,
                                                                                   sizeof( AFSNonPagedDirectoryCB),
                                                                                   AFS_DIR_ENTRY_NP_TAG);

        if( pNonPagedDirEntry == NULL)
        {

            ExFreePool( pDirNode);

            AFSDeleteObjectInfo( pObjectInfoCB);

            try_return( ntStatus = STATUS_INSUFFICIENT_RESOURCES);
        }

        RtlZeroMemory( pDirNode,
                       ulEntryLength);

        RtlZeroMemory( pNonPagedDirEntry,
                       sizeof( AFSNonPagedDirectoryCB));

        ExInitializeResourceLite( &pNonPagedDirEntry->Lock);

        pDirNode->NonPaged = pNonPagedDirEntry;

        pDirNode->ObjectInformation = pObjectInfoCB;

        //
        // Set valid entry
        //

        SetFlag( pDirNode->Flags, AFS_DIR_ENTRY_NOT_IN_PARENT_TREE | AFS_DIR_ENTRY_FAKE | AFS_DIR_ENTRY_VALID);

        pDirNode->FileIndex = (ULONG)AFS_DIR_ENTRY_DOT_DOT_INDEX;

        //
        // Setup the names in the entry
        //

        pDirNode->NameInformation.FileName.Length = 2 * sizeof( WCHAR);

        pDirNode->NameInformation.FileName.MaximumLength = 2 * sizeof( WCHAR);

        pDirNode->NameInformation.FileName.Buffer = (WCHAR *)((char *)pDirNode + sizeof( AFSDirectoryCB));

        pDirNode->NameInformation.FileName.Buffer[ 0] = L'.';

        pDirNode->NameInformation.FileName.Buffer[ 1] = L'.';

        //
        // Populate the rest of the data
        //

        pObjectInfoCB->FileType = AFS_FILE_TYPE_DIRECTORY;

        pObjectInfoCB->FileAttributes = FILE_ATTRIBUTE_DIRECTORY;

        AFSGlobalDotDotDirEntry = pDirNode;

try_exit:

        if( !NT_SUCCESS( ntStatus))
        {

            if( AFSGlobalDotDirEntry != NULL)
            {

                AFSDeleteObjectInfo( AFSGlobalDotDirEntry->ObjectInformation);

                ExDeleteResourceLite( &AFSGlobalDotDirEntry->NonPaged->Lock);

                ExFreePool( AFSGlobalDotDirEntry->NonPaged);

                ExFreePool( AFSGlobalDotDirEntry);

                AFSGlobalDotDirEntry = NULL;
            }

            if( AFSGlobalDotDotDirEntry != NULL)
            {

                AFSDeleteObjectInfo( AFSGlobalDotDotDirEntry->ObjectInformation);

                ExDeleteResourceLite( &AFSGlobalDotDotDirEntry->NonPaged->Lock);

                ExFreePool( AFSGlobalDotDotDirEntry->NonPaged);

                ExFreePool( AFSGlobalDotDotDirEntry);

                AFSGlobalDotDotDirEntry = NULL;
            }
        }
    }

    return ntStatus;
}

AFSDirectoryCB *
AFSInitDirEntry( IN AFSObjectInfoCB *ParentObjectInfo,
                 IN PUNICODE_STRING FileName,
                 IN PUNICODE_STRING TargetName,
                 IN AFSDirEnumEntry *DirEnumEntry,
                 IN ULONG FileIndex)
{

    AFSDirectoryCB *pDirNode = NULL;
    NTSTATUS ntStatus = STATUS_SUCCESS;
    ULONG ulEntryLength = 0;
    AFSDirEnumEntry *pDirEnumCB = NULL;
    AFSFileID stTargetFileID;
    AFSFcb *pVcb = NULL;
    AFSDeviceExt *pDeviceExt = (AFSDeviceExt *)AFSRDRDeviceObject->DeviceExtension;
    AFSObjectInfoCB *pObjectInfoCB = NULL;
    BOOLEAN bAllocatedObjectCB = FALSE;
    ULONGLONG ullIndex = 0;
    AFSNonPagedDirectoryCB *pNonPagedDirEntry = NULL;

    __Enter
    {

        AFSDbgLogMsg( AFS_SUBSYSTEM_FILE_PROCESSING,
                      AFS_TRACE_LEVEL_VERBOSE,
                      "AFSInitDirEntry Initializing entry %wZ parent FID %08lX-%08lX-%08lX-%08lX\n",
                      FileName,
                      ParentObjectInfo->FileId.Cell,
                      ParentObjectInfo->FileId.Volume,
                      ParentObjectInfo->FileId.Vnode,
                      ParentObjectInfo->FileId.Unique);

        //
        // First thing is to locate/create our object information block
        // for this entry
        //

        AFSAcquireExcl( ParentObjectInfo->VolumeCB->ObjectInfoTree.TreeLock,
                        TRUE);

        ullIndex = AFSCreateLowIndex( &DirEnumEntry->FileId);

        ntStatus = AFSLocateHashEntry( ParentObjectInfo->VolumeCB->ObjectInfoTree.TreeHead,
                                       ullIndex,
                                       (AFSBTreeEntry **)&pObjectInfoCB);

        if( !NT_SUCCESS( ntStatus) ||
            pObjectInfoCB == NULL)
        {

            //
            // Allocate our object info cb
            //

            pObjectInfoCB = AFSAllocateObjectInfo( ParentObjectInfo,
                                                   ullIndex);

            if( pObjectInfoCB == NULL)
            {

                AFSReleaseResource( ParentObjectInfo->VolumeCB->ObjectInfoTree.TreeLock);

                try_return( ntStatus = STATUS_INSUFFICIENT_RESOURCES);
            }

            bAllocatedObjectCB = TRUE;

            AFSDbgLogMsg( AFS_SUBSYSTEM_CLEANUP_PROCESSING,
                          AFS_TRACE_LEVEL_VERBOSE,
                          "AFSInitDirEntry initialized object %08lX Parent Object %08lX for %wZ\n",
                          pObjectInfoCB,
                          ParentObjectInfo,
                          FileName);
        }

        InterlockedIncrement( &pObjectInfoCB->ObjectReferenceCount);

        AFSDbgLogMsg( AFS_SUBSYSTEM_OBJECT_REF_COUNTING,
                      AFS_TRACE_LEVEL_VERBOSE,
                      "AFSInitDirEntry Increment count on object %08lX Cnt %d\n",
                      pObjectInfoCB,
                      pObjectInfoCB->ObjectReferenceCount);

        AFSReleaseResource( ParentObjectInfo->VolumeCB->ObjectInfoTree.TreeLock);

        ntStatus = STATUS_SUCCESS;

        ulEntryLength = sizeof( AFSDirectoryCB) +
                                     FileName->Length;

        if( TargetName != NULL)
        {

            ulEntryLength += TargetName->Length;
        }

        pDirNode = (AFSDirectoryCB *)AFSExAllocatePoolWithTag( PagedPool,
                                                               ulEntryLength,
                                                               AFS_DIR_ENTRY_TAG);

        if( pDirNode == NULL)
        {

            try_return( ntStatus = STATUS_INSUFFICIENT_RESOURCES);
        }

        pNonPagedDirEntry = (AFSNonPagedDirectoryCB *)AFSExAllocatePoolWithTag( NonPagedPool,
                                                                                sizeof( AFSNonPagedDirectoryCB),
                                                                                AFS_DIR_ENTRY_NP_TAG);

        if( pNonPagedDirEntry == NULL)
        {

            try_return( ntStatus = STATUS_INSUFFICIENT_RESOURCES);
        }

        RtlZeroMemory( pDirNode,
                       ulEntryLength);

        RtlZeroMemory( pNonPagedDirEntry,
                       sizeof( AFSNonPagedDirectoryCB));

        ExInitializeResourceLite( &pNonPagedDirEntry->Lock);

        pDirNode->NonPaged = pNonPagedDirEntry;

        pDirNode->ObjectInformation = pObjectInfoCB;

        //
        // Set valid entry
        //

        SetFlag( pDirNode->Flags, AFS_DIR_ENTRY_VALID);

        pDirNode->FileIndex = FileIndex;

        //
        // Setup the names in the entry
        //

        if( FileName->Length > 0)
        {

            pDirNode->NameInformation.FileName.Length = FileName->Length;

            pDirNode->NameInformation.FileName.MaximumLength = FileName->Length;

            pDirNode->NameInformation.FileName.Buffer = (WCHAR *)((char *)pDirNode + sizeof( AFSDirectoryCB));

            RtlCopyMemory( pDirNode->NameInformation.FileName.Buffer,
                           FileName->Buffer,
                           pDirNode->NameInformation.FileName.Length);

            //
            // Create a CRC for the file
            //

            pDirNode->CaseSensitiveTreeEntry.HashIndex = AFSGenerateCRC( &pDirNode->NameInformation.FileName,
                                                                         FALSE);

            pDirNode->CaseInsensitiveTreeEntry.HashIndex = AFSGenerateCRC( &pDirNode->NameInformation.FileName,
                                                                           TRUE);
        }

        if( TargetName != NULL &&
            TargetName->Length > 0)
        {

            pDirNode->NameInformation.TargetName.Length = TargetName->Length;

            pDirNode->NameInformation.TargetName.MaximumLength = pDirNode->NameInformation.TargetName.Length;

            pDirNode->NameInformation.TargetName.Buffer = (WCHAR *)((char *)pDirNode +
                                                                            sizeof( AFSDirectoryCB) +
                                                                            pDirNode->NameInformation.FileName.Length);

            RtlCopyMemory( pDirNode->NameInformation.TargetName.Buffer,
                           TargetName->Buffer,
                           pDirNode->NameInformation.TargetName.Length);
        }

        //
        // If we allocated the object information cb then update the information
        //

        if( bAllocatedObjectCB)
        {

            //
            // Populate the rest of the data
            //

            pObjectInfoCB->FileId = DirEnumEntry->FileId;

            pObjectInfoCB->TargetFileId = DirEnumEntry->TargetFileId;

            pObjectInfoCB->FileType = DirEnumEntry->FileType;

            pObjectInfoCB->CreationTime = DirEnumEntry->CreationTime;

            pObjectInfoCB->LastAccessTime = DirEnumEntry->LastAccessTime;

            pObjectInfoCB->LastWriteTime = DirEnumEntry->LastWriteTime;

            pObjectInfoCB->ChangeTime = DirEnumEntry->ChangeTime;

            pObjectInfoCB->EndOfFile = DirEnumEntry->EndOfFile;

            pObjectInfoCB->AllocationSize = DirEnumEntry->AllocationSize;

            pObjectInfoCB->FileAttributes = DirEnumEntry->FileAttributes;

            if( pObjectInfoCB->FileType == AFS_FILE_TYPE_MOUNTPOINT ||
                pObjectInfoCB->FileType == AFS_FILE_TYPE_SYMLINK ||
                pObjectInfoCB->FileType == AFS_FILE_TYPE_DFSLINK)
            {

                pObjectInfoCB->FileAttributes |= FILE_ATTRIBUTE_REPARSE_POINT;
            }

            pObjectInfoCB->EaSize = DirEnumEntry->EaSize;

            //
            // Object specific information
            //

            pObjectInfoCB->Links = DirEnumEntry->Links;

            pObjectInfoCB->Expiration = DirEnumEntry->Expiration;

            pObjectInfoCB->DataVersion = DirEnumEntry->DataVersion;

            //
            // Check for the case where we have a filetype of SymLink but both the TargetFid and the
            // TargetName are empty. In this case set the filetype to zero so we evaluate it later in
            // the code
            //

            if( pObjectInfoCB->FileType == AFS_FILE_TYPE_SYMLINK &&
                pObjectInfoCB->TargetFileId.Vnode == 0 &&
                pObjectInfoCB->TargetFileId.Unique == 0 &&
                pDirNode->NameInformation.TargetName.Length == 0)
            {

                //
                // This will ensure we perform a validation on the node
                //

                pObjectInfoCB->FileType = AFS_FILE_TYPE_UNKNOWN;
            }

            if( pObjectInfoCB->FileType == AFS_FILE_TYPE_UNKNOWN)
            {

                SetFlag( pObjectInfoCB->Flags, AFS_OBJECT_FLAGS_NOT_EVALUATED);
            }
        }

try_exit:

        if( !NT_SUCCESS( ntStatus))
        {

            if( pNonPagedDirEntry != NULL)
            {

                ExDeleteResourceLite( &pNonPagedDirEntry->Lock);

                AFSExFreePool( pNonPagedDirEntry);
            }

            if( pDirNode != NULL)
            {

                AFSExFreePool( pDirNode);

                pDirNode = NULL;
            }

            //
            // Dereference our object info block if we have one
            //

            if( pObjectInfoCB != NULL)
            {

                InterlockedDecrement( &pObjectInfoCB->ObjectReferenceCount);

                AFSDbgLogMsg( AFS_SUBSYSTEM_OBJECT_REF_COUNTING,
                              AFS_TRACE_LEVEL_VERBOSE,
                              "AFSInitDirEntry Decrement count on object %08lX Cnt %d\n",
                              pObjectInfoCB,
                              pObjectInfoCB->ObjectReferenceCount);

                if( bAllocatedObjectCB)
                {

                    ASSERT( pObjectInfoCB->ObjectReferenceCount == 0);

                    AFSDeleteObjectInfo( pObjectInfoCB);
                }
            }
        }
    }

    return pDirNode;
}

BOOLEAN
AFSCheckForReadOnlyAccess( IN ACCESS_MASK DesiredAccess,
                           IN BOOLEAN DirectoryEntry)
{

    BOOLEAN bReturn = TRUE;
    ACCESS_MASK stAccessMask = 0;

    //
    // Get rid of anything we don't know about
    //

    DesiredAccess = (DesiredAccess   &
                          ( DELETE |
                            READ_CONTROL |
                            WRITE_OWNER |
                            WRITE_DAC |
                            SYNCHRONIZE |
                            ACCESS_SYSTEM_SECURITY |
                            FILE_WRITE_DATA |
                            FILE_READ_EA |
                            FILE_WRITE_EA |
                            FILE_READ_ATTRIBUTES |
                            FILE_WRITE_ATTRIBUTES |
                            FILE_LIST_DIRECTORY |
                            FILE_TRAVERSE |
                            FILE_DELETE_CHILD |
                            FILE_APPEND_DATA));

    //
    // Our 'read only' access mask. These are the accesses we will
    // allow for a read only file
    //

    stAccessMask = DELETE |
                        READ_CONTROL |
                        WRITE_OWNER |
                        WRITE_DAC |
                        SYNCHRONIZE |
                        ACCESS_SYSTEM_SECURITY |
                        FILE_READ_DATA |
                        FILE_READ_EA |
                        FILE_WRITE_EA |
                        FILE_READ_ATTRIBUTES |
                        FILE_WRITE_ATTRIBUTES |
                        FILE_EXECUTE |
                        FILE_LIST_DIRECTORY |
                        FILE_TRAVERSE;

    //
    // For a directory, add in the directory specific accesses
    //

    if( DirectoryEntry)
    {

        stAccessMask |= FILE_ADD_SUBDIRECTORY |
                                FILE_ADD_FILE |
                                FILE_DELETE_CHILD;
    }

    if( FlagOn( DesiredAccess, ~stAccessMask))
    {

        //
        // A write access is set ...
        //

        bReturn = FALSE;
    }

    return bReturn;
}

NTSTATUS
AFSEvaluateNode( IN GUID *AuthGroup,
                 IN AFSDirectoryCB *DirEntry)
{

    AFSDeviceExt *pDeviceExt = (AFSDeviceExt *)AFSRDRDeviceObject->DeviceExtension;
    NTSTATUS ntStatus = STATUS_SUCCESS;
    AFSDirEnumEntry *pDirEntry = NULL;
    UNICODE_STRING uniTargetName;

    __Enter
    {

        ntStatus = AFSEvaluateTargetByID( DirEntry->ObjectInformation,
                                          AuthGroup,
                                          FALSE,
                                          &pDirEntry);

        if( !NT_SUCCESS( ntStatus))
        {

            try_return( ntStatus);
        }

        DirEntry->ObjectInformation->TargetFileId = pDirEntry->TargetFileId;

        DirEntry->ObjectInformation->Expiration = pDirEntry->Expiration;

        DirEntry->ObjectInformation->DataVersion = pDirEntry->DataVersion;

        DirEntry->ObjectInformation->FileType = pDirEntry->FileType;

        DirEntry->ObjectInformation->CreationTime = pDirEntry->CreationTime;

        DirEntry->ObjectInformation->LastAccessTime = pDirEntry->LastAccessTime;

        DirEntry->ObjectInformation->LastWriteTime = pDirEntry->LastWriteTime;

        DirEntry->ObjectInformation->ChangeTime = pDirEntry->ChangeTime;

        DirEntry->ObjectInformation->EndOfFile = pDirEntry->EndOfFile;

        DirEntry->ObjectInformation->AllocationSize = pDirEntry->AllocationSize;

        DirEntry->ObjectInformation->FileAttributes = pDirEntry->FileAttributes;

        if( pDirEntry->FileType == AFS_FILE_TYPE_MOUNTPOINT ||
            pDirEntry->FileType == AFS_FILE_TYPE_SYMLINK ||
            pDirEntry->FileType == AFS_FILE_TYPE_DFSLINK)
        {

            DirEntry->ObjectInformation->FileAttributes |= FILE_ATTRIBUTE_REPARSE_POINT;
        }

        DirEntry->ObjectInformation->EaSize = pDirEntry->EaSize;

        DirEntry->ObjectInformation->Links = pDirEntry->Links;

        //
        // If we have a target name then see if it needs updating ...
        //

        if( pDirEntry->TargetNameLength > 0)
        {

            //
            // Update the target name information if needed
            //

            uniTargetName.Length = (USHORT)pDirEntry->TargetNameLength;

            uniTargetName.MaximumLength = uniTargetName.Length;

            uniTargetName.Buffer = (WCHAR *)((char *)pDirEntry + pDirEntry->TargetNameOffset);

            AFSAcquireExcl( &DirEntry->NonPaged->Lock,
                            TRUE);

            if( DirEntry->NameInformation.TargetName.Length == 0 ||
                RtlCompareUnicodeString( &uniTargetName,
                                         &DirEntry->NameInformation.TargetName,
                                         TRUE) != 0)
            {

                //
                // Update the target name
                //

                ntStatus = AFSUpdateTargetName( &DirEntry->NameInformation.TargetName,
                                                &DirEntry->Flags,
                                                uniTargetName.Buffer,
                                                uniTargetName.Length);

                if( !NT_SUCCESS( ntStatus))
                {

                    AFSReleaseResource( &DirEntry->NonPaged->Lock);

                    try_return( ntStatus);
                }
            }

            AFSReleaseResource( &DirEntry->NonPaged->Lock);
        }

try_exit:

        if( pDirEntry != NULL)
        {

            AFSExFreePool( pDirEntry);
        }
    }

    return ntStatus;
}

NTSTATUS
AFSValidateSymLink( IN GUID *AuthGroup,
                    IN AFSDirectoryCB *DirEntry)
{

    AFSDeviceExt *pDeviceExt = (AFSDeviceExt *)AFSRDRDeviceObject->DeviceExtension;
    NTSTATUS ntStatus = STATUS_SUCCESS;
    AFSDirEnumEntry *pDirEntry = NULL;
    UNICODE_STRING uniTargetName;

    __Enter
    {

        ntStatus = AFSEvaluateTargetByID( DirEntry->ObjectInformation,
                                          AuthGroup,
                                          FALSE,
                                          &pDirEntry);

        if( !NT_SUCCESS( ntStatus))
        {

            try_return( ntStatus);
        }

        if( pDirEntry->FileType == AFS_FILE_TYPE_UNKNOWN ||
            pDirEntry->FileType == AFS_FILE_TYPE_INVALID)
        {

            try_return( ntStatus = STATUS_OBJECT_NAME_NOT_FOUND);
        }

        DirEntry->ObjectInformation->TargetFileId = pDirEntry->TargetFileId;

        DirEntry->ObjectInformation->Expiration = pDirEntry->Expiration;

        DirEntry->ObjectInformation->DataVersion = pDirEntry->DataVersion;

        //
        // Update the target name information if needed
        //

        uniTargetName.Length = (USHORT)pDirEntry->TargetNameLength;

        uniTargetName.MaximumLength = uniTargetName.Length;

        uniTargetName.Buffer = (WCHAR *)((char *)pDirEntry + pDirEntry->TargetNameOffset);

        if( uniTargetName.Length > 0)
        {

            AFSAcquireExcl( &DirEntry->NonPaged->Lock,
                            TRUE);

            if( DirEntry->NameInformation.TargetName.Length == 0 ||
                RtlCompareUnicodeString( &uniTargetName,
                                         &DirEntry->NameInformation.TargetName,
                                         TRUE) != 0)
            {

                //
                // Update the target name
                //

                ntStatus = AFSUpdateTargetName( &DirEntry->NameInformation.TargetName,
                                                &DirEntry->Flags,
                                                uniTargetName.Buffer,
                                                uniTargetName.Length);

                if( !NT_SUCCESS( ntStatus))
                {

                    AFSReleaseResource( &DirEntry->NonPaged->Lock);

                    try_return( ntStatus = STATUS_INSUFFICIENT_RESOURCES);
                }
            }

            AFSReleaseResource( &DirEntry->NonPaged->Lock);
        }

        //
        // If the FileType is the same then nothing to do since it IS
        // a SymLink
        //

        if( pDirEntry->FileType == DirEntry->ObjectInformation->FileType)
        {

            ASSERT( pDirEntry->FileType == AFS_FILE_TYPE_SYMLINK);

            try_return( ntStatus = STATUS_SUCCESS);
        }

        DirEntry->ObjectInformation->FileType = pDirEntry->FileType;

        DirEntry->ObjectInformation->CreationTime = pDirEntry->CreationTime;

        DirEntry->ObjectInformation->LastAccessTime = pDirEntry->LastAccessTime;

        DirEntry->ObjectInformation->LastWriteTime = pDirEntry->LastWriteTime;

        DirEntry->ObjectInformation->ChangeTime = pDirEntry->ChangeTime;

        DirEntry->ObjectInformation->EndOfFile = pDirEntry->EndOfFile;

        DirEntry->ObjectInformation->AllocationSize = pDirEntry->AllocationSize;

        DirEntry->ObjectInformation->FileAttributes = pDirEntry->FileAttributes;

        if( pDirEntry->FileType == AFS_FILE_TYPE_MOUNTPOINT ||
            pDirEntry->FileType == AFS_FILE_TYPE_SYMLINK ||
            pDirEntry->FileType == AFS_FILE_TYPE_DFSLINK)
        {

            DirEntry->ObjectInformation->FileAttributes |= FILE_ATTRIBUTE_REPARSE_POINT;
        }

        DirEntry->ObjectInformation->EaSize = pDirEntry->EaSize;

        DirEntry->ObjectInformation->Links = pDirEntry->Links;

try_exit:

        if( pDirEntry != NULL)
        {

            AFSExFreePool( pDirEntry);
        }
    }

    return ntStatus;
}

NTSTATUS
AFSInvalidateCache( IN AFSInvalidateCacheCB *InvalidateCB)
{

    NTSTATUS ntStatus = STATUS_SUCCESS;
    AFSFcb      *pDcb = NULL, *pFcb = NULL, *pNextFcb = NULL;
    AFSVolumeCB *pVolumeCB = NULL;
    AFSFcb      *pTargetDcb = NULL;
    AFSDeviceExt *pDevExt = (AFSDeviceExt *) AFSRDRDeviceObject->DeviceExtension;
    AFSDirectoryCB *pCurrentDirEntry = NULL;
    BOOLEAN     bIsChild = FALSE;
    ULONGLONG   ullIndex = 0;
    AFSObjectInfoCB *pObjectInfo = NULL;
    IO_STATUS_BLOCK stIoStatus;
    ULONG ulFilter = 0;

    __Enter
    {

        //
        // Need to locate the Fcb for the directory to purge
        //

        AFSDbgLogMsg( AFS_SUBSYSTEM_LOCK_PROCESSING,
                      AFS_TRACE_LEVEL_VERBOSE,
                      "AFSInvalidateCache Acquiring RDR VolumeTreeLock lock %08lX SHARED %08lX\n",
                      &pDevExt->Specific.RDR.VolumeTreeLock,
                      PsGetCurrentThread());

        //
        // Starve any exclusive waiters on this paticular call
        //

        AFSAcquireSharedStarveExclusive( &pDevExt->Specific.RDR.VolumeTreeLock, TRUE);

        //
        // Locate the volume node
        //

        ullIndex = AFSCreateHighIndex( &InvalidateCB->FileID);

        ntStatus = AFSLocateHashEntry( pDevExt->Specific.RDR.VolumeTree.TreeHead,
                                       ullIndex,
                                       (AFSBTreeEntry **)&pVolumeCB);

        if( pVolumeCB != NULL)
        {

            InterlockedIncrement( &pVolumeCB->VolumeReferenceCount);

            AFSDbgLogMsg( AFS_SUBSYSTEM_VOLUME_REF_COUNTING,
                          AFS_TRACE_LEVEL_VERBOSE,
                          "AFSInvalidateCache Increment count on volume %08lX Cnt %d\n",
                          pVolumeCB,
                          pVolumeCB->VolumeReferenceCount);
        }

        AFSReleaseResource( &pDevExt->Specific.RDR.VolumeTreeLock);

        if( !NT_SUCCESS( ntStatus) ||
            pVolumeCB == NULL)
        {
            try_return( ntStatus = STATUS_SUCCESS);
        }

        //
        // If this is a whole volume invalidation then go do it now
        //

        if( InvalidateCB->WholeVolume ||
            AFSIsVolumeFID( &InvalidateCB->FileID))
        {

            ntStatus = AFSInvalidateVolume( pVolumeCB,
                                            InvalidateCB->Reason);

            AFSFsRtlNotifyFullReportChange( &pVolumeCB->ObjectInformation,
                                            NULL,
                                            FILE_NOTIFY_CHANGE_FILE_NAME |
                                                FILE_NOTIFY_CHANGE_DIR_NAME |
                                                FILE_NOTIFY_CHANGE_NAME |
                                                FILE_NOTIFY_CHANGE_ATTRIBUTES |
                                                FILE_NOTIFY_CHANGE_SIZE,
                                            FILE_ACTION_MODIFIED);

            InterlockedDecrement( &pVolumeCB->VolumeReferenceCount);

            try_return( ntStatus);
        }

        AFSAcquireShared( pVolumeCB->ObjectInfoTree.TreeLock,
                          TRUE);

        InterlockedDecrement( &pVolumeCB->VolumeReferenceCount);

        AFSDbgLogMsg( AFS_SUBSYSTEM_VOLUME_REF_COUNTING,
                      AFS_TRACE_LEVEL_VERBOSE,
                      "AFSInvalidateCache Decrement count on volume %08lX Cnt %d\n",
                      pVolumeCB,
                      pVolumeCB->VolumeReferenceCount);

        ullIndex = AFSCreateLowIndex( &InvalidateCB->FileID);

        ntStatus = AFSLocateHashEntry( pVolumeCB->ObjectInfoTree.TreeHead,
                                       ullIndex,
                                       (AFSBTreeEntry **)&pObjectInfo);

        if( pObjectInfo != NULL)
        {

            //
            // Reference the node so it won't be torn down
            //

            InterlockedIncrement( &pObjectInfo->ObjectReferenceCount);

            AFSDbgLogMsg( AFS_SUBSYSTEM_OBJECT_REF_COUNTING,
                          AFS_TRACE_LEVEL_VERBOSE,
                          "AFSInvalidateCache Increment count on object %08lX Cnt %d\n",
                          pObjectInfo,
                          pObjectInfo->ObjectReferenceCount);
        }

        AFSReleaseResource( pVolumeCB->ObjectInfoTree.TreeLock);

        if( !NT_SUCCESS( ntStatus) ||
            pObjectInfo == NULL)
        {
            try_return( ntStatus = STATUS_SUCCESS);
        }

        if( pObjectInfo->FileType == AFS_FILE_TYPE_SYMLINK ||
            pObjectInfo->FileType == AFS_FILE_TYPE_DFSLINK ||
            pObjectInfo->FileType == AFS_FILE_TYPE_MOUNTPOINT)
        {

            AFSDbgLogMsg( AFS_SUBSYSTEM_FILE_PROCESSING,
                          AFS_TRACE_LEVEL_VERBOSE,
                          "AFSInvalidateCache Invalidation on node type %d for fid %08lX-%08lX-%08lX-%08lX Reason %d\n",
                          pObjectInfo->FileType,
                          pObjectInfo->FileId.Cell,
                          pObjectInfo->FileId.Volume,
                          pObjectInfo->FileId.Vnode,
                          pObjectInfo->FileId.Unique,
                          InvalidateCB->Reason);

            //
            // We only act on the mount point itself, not the target. If the
            // node has been deleted then mark it as such otherwise indicate
            // it requires verification
            //

            if( InvalidateCB->Reason == AFS_INVALIDATE_DELETED)
            {
                SetFlag( pObjectInfo->Flags, AFS_OBJECT_FLAGS_OBJECT_INVALID);
            }
            else
            {

                if( InvalidateCB->Reason == AFS_INVALIDATE_FLUSHED ||
                    InvalidateCB->Reason == AFS_INVALIDATE_DATA_VERSION)
                {
                    pObjectInfo->DataVersion.QuadPart = (ULONGLONG)-1;
                }

                pObjectInfo->Expiration.QuadPart = 0;

                pObjectInfo->TargetFileId.Vnode = 0;

                pObjectInfo->TargetFileId.Unique = 0;

                AFSDbgLogMsg( AFS_SUBSYSTEM_FILE_PROCESSING,
                              AFS_TRACE_LEVEL_VERBOSE,
                              "AFSInvalidateCache Setting VERIFY flag on fid %08lX-%08lX-%08lX-%08lX\n",
                              pObjectInfo->FileId.Cell,
                              pObjectInfo->FileId.Volume,
                              pObjectInfo->FileId.Vnode,
                              pObjectInfo->FileId.Unique);

                SetFlag( pObjectInfo->Flags, AFS_OBJECT_FLAGS_VERIFY);
            }

            ulFilter = FILE_NOTIFY_CHANGE_FILE_NAME;

            if( InvalidateCB->Reason == AFS_INVALIDATE_CREDS)
            {
                ulFilter |= FILE_NOTIFY_CHANGE_SECURITY;
            }

            if( InvalidateCB->Reason == AFS_INVALIDATE_DATA_VERSION)
            {
                ulFilter |= FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_LAST_WRITE;
            }
            else
            {
                ulFilter |= FILE_NOTIFY_CHANGE_ATTRIBUTES;
            }

            AFSFsRtlNotifyFullReportChange( pObjectInfo->ParentObjectInformation,
                                            NULL,
                                            FILE_NOTIFY_CHANGE_FILE_NAME |
                                            FILE_NOTIFY_CHANGE_ATTRIBUTES,
                                            FILE_ACTION_MODIFIED);

            try_return( ntStatus);
        }

        //
        // Depending on the reason for invalidation then perform work on the node
        //

        switch( InvalidateCB->Reason)
        {

            case AFS_INVALIDATE_DELETED:
            {

                //
                // Mark this node as invalid
                //

                SetFlag( pObjectInfo->Flags, AFS_OBJECT_FLAGS_DELETED);

                AFSDbgLogMsg( AFS_SUBSYSTEM_FILE_PROCESSING,
                              AFS_TRACE_LEVEL_VERBOSE,
                              "AFSInvalidateCache Set DELETE flag on fid %08lX-%08lX-%08lX-%08lX\n",
                              pObjectInfo->FileId.Cell,
                              pObjectInfo->FileId.Volume,
                              pObjectInfo->FileId.Vnode,
                              pObjectInfo->FileId.Unique);

                if( pObjectInfo->ParentObjectInformation != NULL)
                {

                    AFSDbgLogMsg( AFS_SUBSYSTEM_FILE_PROCESSING,
                                  AFS_TRACE_LEVEL_VERBOSE,
                                  "AFSInvalidateCache Set VERIFY flag on parent fid %08lX-%08lX-%08lX-%08lX\n",
                                  pObjectInfo->ParentObjectInformation->FileId.Cell,
                                  pObjectInfo->ParentObjectInformation->FileId.Volume,
                                  pObjectInfo->ParentObjectInformation->FileId.Vnode,
                                  pObjectInfo->ParentObjectInformation->FileId.Unique);

                    SetFlag( pObjectInfo->ParentObjectInformation->Flags, AFS_OBJECT_FLAGS_VERIFY);
                    pObjectInfo->ParentObjectInformation->DataVersion.QuadPart = (ULONGLONG)-1;
                    pObjectInfo->ParentObjectInformation->Expiration.QuadPart = 0;
                }

                if( pObjectInfo->FileType == AFS_FILE_TYPE_DIRECTORY)
                {
                    ulFilter = FILE_NOTIFY_CHANGE_DIR_NAME;
                }
                else
                {
                    ulFilter = FILE_NOTIFY_CHANGE_FILE_NAME;
                }

                AFSFsRtlNotifyFullReportChange( pObjectInfo->ParentObjectInformation,
                                                NULL,
                                                ulFilter,
                                                FILE_ACTION_REMOVED);

                break;
            }

            case AFS_INVALIDATE_FLUSHED:
            {

                if( pObjectInfo->FileType == AFS_FILE_TYPE_FILE &&
                    pObjectInfo->Fcb != NULL)
                {

                    AFSDbgLogMsg( AFS_SUBSYSTEM_FILE_PROCESSING,
                                  AFS_TRACE_LEVEL_VERBOSE,
                                  "AFSInvalidateCache Flush/purge file fid %08lX-%08lX-%08lX-%08lX\n",
                                  pObjectInfo->FileId.Cell,
                                  pObjectInfo->FileId.Volume,
                                  pObjectInfo->FileId.Vnode,
                                  pObjectInfo->FileId.Unique);

                    AFSAcquireExcl( &pObjectInfo->Fcb->NPFcb->Resource,
                                    TRUE);

                    __try
                    {

                        CcFlushCache( &pObjectInfo->Fcb->NPFcb->SectionObjectPointers,
                                      NULL,
                                      0,
                                      &stIoStatus);

                        if( !NT_SUCCESS( stIoStatus.Status))
                        {

                            AFSDbgLogMsg( AFS_SUBSYSTEM_IO_PROCESSING,
                                          AFS_TRACE_LEVEL_ERROR,
                                          "AFSInvalidateCache CcFlushCache failure FID %08lX-%08lX-%08lX-%08lX Status 0x%08lX Bytes 0x%08lX\n",
                                          pObjectInfo->FileId.Cell,
                                          pObjectInfo->FileId.Volume,
                                          pObjectInfo->FileId.Vnode,
                                          pObjectInfo->FileId.Unique,
                                          stIoStatus.Status,
                                          stIoStatus.Information);

                            ntStatus = stIoStatus.Status;
                        }

                        CcPurgeCacheSection( &pObjectInfo->Fcb->NPFcb->SectionObjectPointers,
                                             NULL,
                                             0,
                                             FALSE);
                    }
                    __except( EXCEPTION_EXECUTE_HANDLER)
                    {

                        ntStatus = GetExceptionCode();
                    }

                    AFSReleaseResource( &pObjectInfo->Fcb->NPFcb->Resource);

                    //
                    // Clear out the extents
                    // Get rid of them (note this involves waiting
                    // for any writes or reads to the cache to complete)
                    //

                    (VOID) AFSTearDownFcbExtents( pObjectInfo->Fcb);
                }

                // Fall through to the default processing
            }

            default:
            {

                if( pObjectInfo->FileType == AFS_FILE_TYPE_DIRECTORY)
                {
                    ulFilter = FILE_NOTIFY_CHANGE_DIR_NAME;
                }
                else
                {
                    ulFilter = FILE_NOTIFY_CHANGE_FILE_NAME;
                }

                if( InvalidateCB->Reason == AFS_INVALIDATE_CREDS)
                {
                    ulFilter |= FILE_NOTIFY_CHANGE_SECURITY;
                }

                if( InvalidateCB->Reason == AFS_INVALIDATE_DATA_VERSION)
                {
                    ulFilter |= FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_LAST_WRITE;
                }
                else
                {
                    ulFilter |= FILE_NOTIFY_CHANGE_ATTRIBUTES;
                }

                AFSFsRtlNotifyFullReportChange( pObjectInfo->ParentObjectInformation,
                                                NULL,
                                                ulFilter,
                                                FILE_ACTION_MODIFIED);

                //
                // Indicate this node requires re-evaluation for the remaining reasons
                //

                pObjectInfo->Expiration.QuadPart = 0;

                AFSDbgLogMsg( AFS_SUBSYSTEM_FILE_PROCESSING,
                              AFS_TRACE_LEVEL_VERBOSE,
                              "AFSInvalidateCache Setting VERIFY flag on fid %08lX-%08lX-%08lX-%08lX\n",
                              pObjectInfo->FileId.Cell,
                              pObjectInfo->FileId.Volume,
                              pObjectInfo->FileId.Vnode,
                              pObjectInfo->FileId.Unique);

                SetFlag( pObjectInfo->Flags, AFS_OBJECT_FLAGS_VERIFY);

                if( InvalidateCB->Reason == AFS_INVALIDATE_FLUSHED ||
                    InvalidateCB->Reason == AFS_INVALIDATE_DATA_VERSION)
                {
                    pObjectInfo->DataVersion.QuadPart = (ULONGLONG)-1;

                    if( pObjectInfo->FileType == AFS_FILE_TYPE_FILE)
                    {

                        AFSDbgLogMsg( AFS_SUBSYSTEM_FILE_PROCESSING,
                                      AFS_TRACE_LEVEL_VERBOSE,
                                      "AFSInvalidateCache Setting VERIFY_DATA flag on fid %08lX-%08lX-%08lX-%08lX\n",
                                      pObjectInfo->FileId.Cell,
                                      pObjectInfo->FileId.Volume,
                                      pObjectInfo->FileId.Vnode,
                                      pObjectInfo->FileId.Unique);

                        SetFlag( pObjectInfo->Flags, AFS_OBJECT_FLAGS_VERIFY_DATA);
                    }
                }

                break;
            }
        }

try_exit:

        if( pObjectInfo != NULL)
        {

            InterlockedDecrement( &pObjectInfo->ObjectReferenceCount);

            AFSDbgLogMsg( AFS_SUBSYSTEM_OBJECT_REF_COUNTING,
                          AFS_TRACE_LEVEL_VERBOSE,
                          "AFSInvalidateCache Decrement count on object %08lX Cnt %d\n",
                          pObjectInfo,
                          pObjectInfo->ObjectReferenceCount);
        }
    }

    return ntStatus;
}

BOOLEAN
AFSIsChildOfParent( IN AFSFcb *Dcb,
                    IN AFSFcb *Fcb)
{

    BOOLEAN bIsChild = FALSE;
    AFSFcb *pCurrentFcb = Fcb;

    while( pCurrentFcb != NULL)
    {

        if( pCurrentFcb->ObjectInformation->ParentObjectInformation == Dcb->ObjectInformation)
        {

            bIsChild = TRUE;

            break;
        }

        pCurrentFcb = pCurrentFcb->ObjectInformation->ParentObjectInformation->Fcb;
    }

    return bIsChild;
}

inline
ULONGLONG
AFSCreateHighIndex( IN AFSFileID *FileID)
{

    ULONGLONG ullIndex = 0;

    ullIndex = (((ULONGLONG)FileID->Cell << 32) | FileID->Volume);

    return ullIndex;
}

inline
ULONGLONG
AFSCreateLowIndex( IN AFSFileID *FileID)
{

    ULONGLONG ullIndex = 0;

    ullIndex = (((ULONGLONG)FileID->Vnode << 32) | FileID->Unique);

    return ullIndex;
}

BOOLEAN
AFSCheckAccess( IN ACCESS_MASK DesiredAccess,
                IN ACCESS_MASK GrantedAccess,
                IN BOOLEAN DirectoryEntry)
{

    BOOLEAN bAccessGranted = TRUE;

    //
    // Check if we are asking for read/write and granted only read only
    // NOTE: There will be more checks here
    //

    if( !AFSCheckForReadOnlyAccess( DesiredAccess,
                                    DirectoryEntry) &&
        AFSCheckForReadOnlyAccess( GrantedAccess,
                                   DirectoryEntry))
    {

        bAccessGranted = FALSE;
    }

    return bAccessGranted;
}

NTSTATUS
AFSGetDriverStatus( IN AFSDriverStatusRespCB *DriverStatus)
{

    NTSTATUS         ntStatus = STATUS_SUCCESS;
    AFSDeviceExt    *pControlDevExt = (AFSDeviceExt *)AFSControlDeviceObject->DeviceExtension;

    //
    // Start with read
    //

    DriverStatus->Status = AFS_DRIVER_STATUS_READY;

    if( AFSGlobalRoot == NULL)
    {

        //
        // We are not ready
        //

        DriverStatus->Status = AFS_DRIVER_STATUS_NOT_READY;
    }

    if( pControlDevExt->Specific.Control.CommServiceCB.IrpPoolControlFlag != POOL_ACTIVE)
    {

        //
        // No service yet
        //

        DriverStatus->Status = AFS_DRIVER_STATUS_NO_SERVICE;
    }

    return ntStatus;
}

NTSTATUS
AFSSubstituteSysName( IN UNICODE_STRING *ComponentName,
                      IN UNICODE_STRING *SubstituteName,
                      IN ULONG StringIndex)
{

    NTSTATUS ntStatus = STATUS_SUCCESS;
    AFSDeviceExt    *pControlDevExt = (AFSDeviceExt *)AFSControlDeviceObject->DeviceExtension;
    AFSSysNameCB    *pSysName = NULL;
    ERESOURCE       *pSysNameLock = NULL;
    ULONG            ulIndex = 1;
    USHORT           usIndex = 0;
    UNICODE_STRING   uniSysName;

    __Enter
    {

#if defined(_WIN64)

        if( IoIs32bitProcess( NULL))
        {

            pSysNameLock = &pControlDevExt->Specific.Control.SysName32ListLock;

            pSysName = pControlDevExt->Specific.Control.SysName32ListHead;
        }
        else
        {

            pSysNameLock = &pControlDevExt->Specific.Control.SysName64ListLock;

            pSysName = pControlDevExt->Specific.Control.SysName64ListHead;
        }
#else

        pSysNameLock = &pControlDevExt->Specific.Control.SysName32ListLock;

        pSysName = pControlDevExt->Specific.Control.SysName32ListHead;

#endif

        AFSDbgLogMsg( AFS_SUBSYSTEM_LOCK_PROCESSING,
                      AFS_TRACE_LEVEL_VERBOSE,
                      "AFSSubstituteSysName Acquiring SysName lock %08lX SHARED %08lX\n",
                      pSysNameLock,
                      PsGetCurrentThread());

        AFSAcquireShared( pSysNameLock,
                          TRUE);

        //
        // Find where we are in the list
        //

        while( pSysName != NULL &&
            ulIndex < StringIndex)
        {

            pSysName = pSysName->fLink;

            ulIndex++;
        }

        if( pSysName == NULL)
        {

            try_return( ntStatus = STATUS_OBJECT_NAME_NOT_FOUND);
        }

        RtlInitUnicodeString( &uniSysName,
                              L"@SYS");
        //
        // If it is a full component of @SYS then just substitue the
        // name in
        //

        if( RtlCompareUnicodeString( &uniSysName,
                                     ComponentName,
                                     TRUE) == 0)
        {

            SubstituteName->Length = pSysName->SysName.Length;
            SubstituteName->MaximumLength = SubstituteName->Length;

            SubstituteName->Buffer = (WCHAR *)AFSExAllocatePoolWithTag( PagedPool,
                                                                        SubstituteName->Length,
                                                                        AFS_SUBST_BUFFER_TAG);

            if( SubstituteName->Buffer == NULL)
            {

                try_return( ntStatus = STATUS_INSUFFICIENT_RESOURCES);
            }

            RtlCopyMemory( SubstituteName->Buffer,
                           pSysName->SysName.Buffer,
                           pSysName->SysName.Length);
        }
        else
        {

            usIndex = 0;

            while( ComponentName->Buffer[ usIndex] != L'@')
            {

                usIndex++;
            }

            SubstituteName->Length = (usIndex * sizeof( WCHAR)) + pSysName->SysName.Length;
            SubstituteName->MaximumLength = SubstituteName->Length;

            SubstituteName->Buffer = (WCHAR *)AFSExAllocatePoolWithTag( PagedPool,
                                                                        SubstituteName->Length,
                                                                        AFS_SUBST_BUFFER_TAG);

            if( SubstituteName->Buffer == NULL)
            {

                try_return( ntStatus = STATUS_INSUFFICIENT_RESOURCES);
            }

            RtlCopyMemory( SubstituteName->Buffer,
                           ComponentName->Buffer,
                           usIndex * sizeof( WCHAR));

            RtlCopyMemory( &SubstituteName->Buffer[ usIndex],
                           pSysName->SysName.Buffer,
                           pSysName->SysName.Length);
        }

try_exit:

        AFSReleaseResource( pSysNameLock);
    }

    return ntStatus;
}

NTSTATUS
AFSSubstituteNameInPath( IN OUT UNICODE_STRING *FullPathName,
                         IN OUT UNICODE_STRING *ComponentName,
                         IN UNICODE_STRING *SubstituteName,
                         IN OUT UNICODE_STRING *RemainingPath,
                         IN BOOLEAN FreePathName)
{

    NTSTATUS ntStatus = STATUS_SUCCESS;
    UNICODE_STRING uniPathName;
    USHORT usPrefixNameLen = 0;
    SHORT  sNameLenDelta = 0;

    __Enter
    {

        //
        // If the passed in name can handle the additional length
        // then just moves things around
        //

        sNameLenDelta = SubstituteName->Length - ComponentName->Length;

        usPrefixNameLen = (USHORT)(ComponentName->Buffer - FullPathName->Buffer);

        if( FullPathName->MaximumLength > FullPathName->Length + sNameLenDelta)
        {

            if( FullPathName->Length > usPrefixNameLen + ComponentName->Length)
            {

                RtlMoveMemory( &FullPathName->Buffer[ ((usPrefixNameLen*sizeof( WCHAR) + SubstituteName->Length)/sizeof( WCHAR))],
                               &FullPathName->Buffer[ ((usPrefixNameLen*sizeof( WCHAR) + ComponentName->Length)/sizeof( WCHAR))],
                               FullPathName->Length - usPrefixNameLen*sizeof( WCHAR) - ComponentName->Length);
            }

            RtlCopyMemory( &FullPathName->Buffer[ usPrefixNameLen],
                           SubstituteName->Buffer,
                           SubstituteName->Length);

            FullPathName->Length += sNameLenDelta;

            ComponentName->Length += sNameLenDelta;

            ComponentName->MaximumLength = ComponentName->Length;

            if ( RemainingPath->Buffer)
            {

                RemainingPath->Buffer += sNameLenDelta/sizeof( WCHAR);
            }

            try_return( ntStatus);
        }

        //
        // Need to re-allocate the buffer
        //

        uniPathName.Length = FullPathName->Length -
                                         ComponentName->Length +
                                         SubstituteName->Length;

        uniPathName.MaximumLength = FullPathName->MaximumLength + PAGE_SIZE;

        uniPathName.Buffer = (WCHAR *)AFSExAllocatePoolWithTag( PagedPool,
                                                                uniPathName.MaximumLength,
                                                                AFS_NAME_BUFFER_FOUR_TAG);

        if( uniPathName.Buffer == NULL)
        {

            try_return( ntStatus = STATUS_INSUFFICIENT_RESOURCES);
        }

        usPrefixNameLen = (USHORT)(ComponentName->Buffer - FullPathName->Buffer);

        usPrefixNameLen *= sizeof( WCHAR);

        RtlZeroMemory( uniPathName.Buffer,
                       uniPathName.MaximumLength);

        RtlCopyMemory( uniPathName.Buffer,
                       FullPathName->Buffer,
                       usPrefixNameLen);

        RtlCopyMemory( &uniPathName.Buffer[ (usPrefixNameLen/sizeof( WCHAR))],
                       SubstituteName->Buffer,
                       SubstituteName->Length);

        if( FullPathName->Length > usPrefixNameLen + ComponentName->Length)
        {

            RtlCopyMemory( &uniPathName.Buffer[ (usPrefixNameLen + SubstituteName->Length)/sizeof( WCHAR)],
                           &FullPathName->Buffer[ (usPrefixNameLen + ComponentName->Length)/sizeof( WCHAR)],
                           FullPathName->Length - usPrefixNameLen - ComponentName->Length);
        }

        ComponentName->Buffer = uniPathName.Buffer + (ComponentName->Buffer - FullPathName->Buffer);

        ComponentName->Length += sNameLenDelta;

        ComponentName->MaximumLength = ComponentName->Length;

        if ( RemainingPath->Buffer)
        {

            RemainingPath->Buffer = uniPathName.Buffer
                + (RemainingPath->Buffer - FullPathName->Buffer)
                + sNameLenDelta/sizeof( WCHAR);
        }

        if( FreePathName)
        {
            AFSExFreePool( FullPathName->Buffer);
        }

        *FullPathName = uniPathName;

try_exit:

        NOTHING;
    }

    return ntStatus;
}

NTSTATUS
AFSInvalidateVolume( IN AFSVolumeCB *VolumeCB,
                     IN ULONG Reason)
{

    NTSTATUS ntStatus = STATUS_SUCCESS;
    AFSFcb *pFcb = NULL;
    AFSObjectInfoCB *pCurrentObject = NULL;
    ULONG ulFilter = 0;

    __Enter
    {

        AFSDbgLogMsg( AFS_SUBSYSTEM_FILE_PROCESSING,
                      AFS_TRACE_LEVEL_VERBOSE,
                      "AFSInvalidateVolume Invalidate volume fid %08lX-%08lX-%08lX-%08lX Reason %08lX\n",
                      VolumeCB->ObjectInformation.FileId.Cell,
                      VolumeCB->ObjectInformation.FileId.Volume,
                      VolumeCB->ObjectInformation.FileId.Vnode,
                      VolumeCB->ObjectInformation.FileId.Unique,
                      Reason);

        //
        // Depending on the reason for invalidation then perform work on the node
        //

        switch( Reason)
        {

            case AFS_INVALIDATE_DELETED:
            {

                //
                // Mark this volume as invalid
                //

                VolumeCB->ObjectInformation.Expiration.QuadPart = 0;

                SetFlag( VolumeCB->ObjectInformation.Flags, AFS_OBJECT_FLAGS_OBJECT_INVALID);

                SetFlag( VolumeCB->Flags, AFS_VOLUME_FLAGS_OFFLINE);

                AFSFsRtlNotifyFullReportChange( &VolumeCB->ObjectInformation,
                                                NULL,
                                                FILE_NOTIFY_CHANGE_DIR_NAME,
                                                FILE_ACTION_REMOVED);

                AFSAcquireShared( VolumeCB->ObjectInfoTree.TreeLock,
                                  TRUE);

                pCurrentObject = VolumeCB->ObjectInfoListHead;

                while( pCurrentObject != NULL)
                {

                    if( pCurrentObject->FileType == AFS_FILE_TYPE_DIRECTORY)
                    {
                        ulFilter = FILE_NOTIFY_CHANGE_DIR_NAME;
                    }
                    else
                    {
                        ulFilter = FILE_NOTIFY_CHANGE_FILE_NAME;
                    }

                    AFSFsRtlNotifyFullReportChange( pCurrentObject,
                                                    NULL,
                                                    ulFilter,
                                                    FILE_ACTION_REMOVED);

                    SetFlag( pCurrentObject->Flags, AFS_OBJECT_FLAGS_OBJECT_INVALID);

                    pFcb = pCurrentObject->Fcb;

                    if( pFcb != NULL &&
                        pFcb->Header.NodeTypeCode == AFS_FILE_FCB)
                    {


                        //
                        // Clear out the extents
                        // And get rid of them (note this involves waiting
                        // for any writes or reads to the cache to complete)
                        //

                        (VOID) AFSTearDownFcbExtents( pFcb);
                    }

                    pCurrentObject = (AFSObjectInfoCB *)pCurrentObject->ListEntry.fLink;
                }

                AFSReleaseResource( VolumeCB->ObjectInfoTree.TreeLock);

                break;
            }

            default:
            {

                //
                // Indicate this node requires re-evaluation for the remaining reasons
                //

                VolumeCB->ObjectInformation.Expiration.QuadPart = 0;

                AFSDbgLogMsg( AFS_SUBSYSTEM_FILE_PROCESSING,
                              AFS_TRACE_LEVEL_VERBOSE,
                              "AFSInvalidateVolume Setting VERIFY flag on fid %08lX-%08lX-%08lX-%08lX\n",
                              VolumeCB->ObjectInformation.FileId.Cell,
                              VolumeCB->ObjectInformation.FileId.Volume,
                              VolumeCB->ObjectInformation.FileId.Vnode,
                              VolumeCB->ObjectInformation.FileId.Unique);

                SetFlag( VolumeCB->ObjectInformation.Flags, AFS_OBJECT_FLAGS_VERIFY);

                if( Reason == AFS_INVALIDATE_FLUSHED ||
                    Reason == AFS_INVALIDATE_DATA_VERSION)
                {

                    VolumeCB->ObjectInformation.DataVersion.QuadPart = (ULONGLONG)-1;
                }

                //
                // Notify anyone that cares
                //

                ulFilter = FILE_NOTIFY_CHANGE_DIR_NAME;

                if( Reason == AFS_INVALIDATE_CREDS)
                {
                    ulFilter |= FILE_NOTIFY_CHANGE_SECURITY;
                }

                if( Reason == AFS_INVALIDATE_DATA_VERSION)
                {
                    ulFilter |= FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_LAST_WRITE;
                }
                else
                {
                    ulFilter |= FILE_NOTIFY_CHANGE_ATTRIBUTES;
                }

                AFSFsRtlNotifyFullReportChange( &VolumeCB->ObjectInformation,
                                                NULL,
                                                ulFilter,
                                                FILE_ACTION_MODIFIED);

                //
                // Volume invalidations require all objects in the volume be re-verified
                //

                AFSAcquireShared( VolumeCB->ObjectInfoTree.TreeLock,
                                  TRUE);

                pCurrentObject = VolumeCB->ObjectInfoListHead;

                while( pCurrentObject != NULL)
                {

                    pCurrentObject->Expiration.QuadPart = 0;

                    pCurrentObject->TargetFileId.Vnode = 0;

                    pCurrentObject->TargetFileId.Unique = 0;

                    AFSDbgLogMsg( AFS_SUBSYSTEM_FILE_PROCESSING,
                                  AFS_TRACE_LEVEL_VERBOSE,
                                  "AFSInvalidateVolume Setting VERIFY flag on fid %08lX-%08lX-%08lX-%08lX\n",
                                  pCurrentObject->FileId.Cell,
                                  pCurrentObject->FileId.Volume,
                                  pCurrentObject->FileId.Vnode,
                                  pCurrentObject->FileId.Unique);

                    SetFlag( pCurrentObject->Flags, AFS_OBJECT_FLAGS_VERIFY);

                    if( Reason == AFS_INVALIDATE_FLUSHED ||
                        Reason == AFS_INVALIDATE_DATA_VERSION)
                    {

                        pCurrentObject->DataVersion.QuadPart = (ULONGLONG)-1;

                        if( pCurrentObject->FileType == AFS_FILE_TYPE_FILE)
                        {

                            AFSDbgLogMsg( AFS_SUBSYSTEM_FILE_PROCESSING,
                                          AFS_TRACE_LEVEL_VERBOSE,
                                          "AFSInvalidateVolume Setting VERIFY_DATA flag on fid %08lX-%08lX-%08lX-%08lX\n",
                                          pCurrentObject->FileId.Cell,
                                          pCurrentObject->FileId.Volume,
                                          pCurrentObject->FileId.Vnode,
                                          pCurrentObject->FileId.Unique);

                            SetFlag( pCurrentObject->Flags, AFS_OBJECT_FLAGS_VERIFY_DATA);
                        }
                    }

                    if( pCurrentObject->FileType == AFS_FILE_TYPE_DIRECTORY)
                    {
                        ulFilter = FILE_NOTIFY_CHANGE_DIR_NAME;
                    }
                    else
                    {
                        ulFilter = FILE_NOTIFY_CHANGE_FILE_NAME;
                    }

                    if( Reason == AFS_INVALIDATE_CREDS)
                    {
                        ulFilter |= FILE_NOTIFY_CHANGE_SECURITY;
                    }

                    if( Reason == AFS_INVALIDATE_DATA_VERSION)
                    {
                        ulFilter |= FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_LAST_WRITE;
                    }
                    else
                    {
                        ulFilter |= FILE_NOTIFY_CHANGE_ATTRIBUTES;
                    }

                    AFSFsRtlNotifyFullReportChange( pCurrentObject,
                                                    NULL,
                                                    ulFilter,
                                                    FILE_ACTION_MODIFIED);

                    pCurrentObject = (AFSObjectInfoCB *)pCurrentObject->ListEntry.fLink;
                }

                AFSReleaseResource( VolumeCB->ObjectInfoTree.TreeLock);

                break;
            }
        }
    }

    return ntStatus;
}

NTSTATUS
AFSVerifyEntry( IN GUID *AuthGroup,
                IN AFSDirectoryCB *DirEntry)
{

    NTSTATUS ntStatus = STATUS_SUCCESS;
    AFSDirEnumEntry *pDirEnumEntry = NULL;
    AFSObjectInfoCB *pObjectInfo = DirEntry->ObjectInformation;
    IO_STATUS_BLOCK stIoStatus;

    __Enter
    {

        AFSDbgLogMsg( AFS_SUBSYSTEM_FILE_PROCESSING,
                      AFS_TRACE_LEVEL_VERBOSE_2,
                      "AFSVerifyEntry Verifying entry %wZ FID %08lX-%08lX-%08lX-%08lX\n",
                      &DirEntry->NameInformation.FileName,
                      pObjectInfo->FileId.Cell,
                      pObjectInfo->FileId.Volume,
                      pObjectInfo->FileId.Vnode,
                      pObjectInfo->FileId.Unique);

        ntStatus = AFSEvaluateTargetByID( pObjectInfo,
                                          AuthGroup,
                                          FALSE,
                                          &pDirEnumEntry);

        if( !NT_SUCCESS( ntStatus))
        {

            AFSDbgLogMsg( AFS_SUBSYSTEM_FILE_PROCESSING,
                          AFS_TRACE_LEVEL_ERROR,
                          "AFSValidateEntry Evaluate Target failed %wZ FID %08lX-%08lX-%08lX-%08lX Status 0x%08lX\n",
                          &DirEntry->NameInformation.FileName,
                          pObjectInfo->FileId.Cell,
                          pObjectInfo->FileId.Volume,
                          pObjectInfo->FileId.Vnode,
                          pObjectInfo->FileId.Unique,
                          ntStatus);

            try_return( ntStatus);
        }

        //
        // Check the data version of the file
        //

        if( pObjectInfo->DataVersion.QuadPart == pDirEnumEntry->DataVersion.QuadPart &&
            !BooleanFlagOn( pObjectInfo->Flags, AFS_OBJECT_FLAGS_VERIFY_DATA))
        {

            AFSDbgLogMsg( AFS_SUBSYSTEM_FILE_PROCESSING,
                          AFS_TRACE_LEVEL_VERBOSE,
                          "AFSVerifyEntry No DV change %I64X for Fcb %wZ FID %08lX-%08lX-%08lX-%08lX\n",
                          pObjectInfo->DataVersion.QuadPart,
                          &DirEntry->NameInformation.FileName,
                          pObjectInfo->FileId.Cell,
                          pObjectInfo->FileId.Volume,
                          pObjectInfo->FileId.Vnode,
                          pObjectInfo->FileId.Unique);

            //
            // We are ok, just get out
            //

            ClearFlag( pObjectInfo->Flags, AFS_OBJECT_FLAGS_VERIFY);

            try_return( ntStatus = STATUS_SUCCESS);
        }

        //
        // New data version so we will need to process the node based on the type
        //

        switch( pDirEnumEntry->FileType)
        {

            case AFS_FILE_TYPE_MOUNTPOINT:
            {

                //
                // For a mount point we need to ensure the target is the same
                //

                if( !AFSIsEqualFID( &pObjectInfo->TargetFileId,
                                    &pDirEnumEntry->TargetFileId))
                {

                }

                //
                // Update the metadata for the entry
                //

                ntStatus = AFSUpdateMetaData( DirEntry,
                                              pDirEnumEntry);

                if( NT_SUCCESS( ntStatus))
                {

                    ClearFlag( pObjectInfo->Flags, AFS_OBJECT_FLAGS_VERIFY);
                }

                break;
            }

            case AFS_FILE_TYPE_SYMLINK:
            {

                ASSERT( pDirEnumEntry->TargetNameLength > 0);

                //
                // Update the metadata for the entry
                //

                ntStatus = AFSUpdateMetaData( DirEntry,
                                              pDirEnumEntry);

                if( NT_SUCCESS( ntStatus))
                {

                    ClearFlag( pObjectInfo->Flags, AFS_OBJECT_FLAGS_VERIFY);
                }

                break;
            }

            case AFS_FILE_TYPE_FILE:
            {
                FILE_OBJECT * pCCFileObject = NULL;
                BOOLEAN bPurgeExtents = FALSE;

                if ( BooleanFlagOn( pObjectInfo->Flags, AFS_OBJECT_FLAGS_VERIFY_DATA))
                {
                    bPurgeExtents = TRUE;

                    AFSDbgLogMsg( AFS_SUBSYSTEM_FILE_PROCESSING,
                                  AFS_TRACE_LEVEL_VERBOSE,
                                  "AFSVerifyEntry Clearing VERIFY_DATA flag %wZ FID %08lX-%08lX-%08lX-%08lX\n",
                                  &DirEntry->NameInformation.FileName,
                                  pObjectInfo->FileId.Cell,
                                  pObjectInfo->FileId.Volume,
                                  pObjectInfo->FileId.Vnode,
                                  pObjectInfo->FileId.Unique);

                    ClearFlag( pObjectInfo->Flags, AFS_OBJECT_FLAGS_VERIFY_DATA);
                }

                //
                // Update the metadata for the entry
                //

                ntStatus = AFSUpdateMetaData( DirEntry,
                                              pDirEnumEntry);

                if( !NT_SUCCESS( ntStatus))
                {

                    AFSDbgLogMsg( AFS_SUBSYSTEM_FILE_PROCESSING,
                                  AFS_TRACE_LEVEL_ERROR,
                                  "AFSInvalidateCache Meta Data Update failed %wZ FID %08lX-%08lX-%08lX-%08lX ntStatus %08lX\n",
                                  &DirEntry->NameInformation.FileName,
                                  pObjectInfo->FileId.Cell,
                                  pObjectInfo->FileId.Volume,
                                  pObjectInfo->FileId.Vnode,
                                  pObjectInfo->FileId.Unique,
                                  ntStatus);

                    break;
                }

                if( pObjectInfo->Fcb != NULL)
                {

                    AFSDbgLogMsg( AFS_SUBSYSTEM_FILE_PROCESSING,
                                  AFS_TRACE_LEVEL_VERBOSE,
                                  "AFSVerifyEntry Flush/purge entry %wZ FID %08lX-%08lX-%08lX-%08lX\n",
                                  &DirEntry->NameInformation.FileName,
                                  pObjectInfo->FileId.Cell,
                                  pObjectInfo->FileId.Volume,
                                  pObjectInfo->FileId.Vnode,
                                  pObjectInfo->FileId.Unique);

                    AFSAcquireExcl( &pObjectInfo->Fcb->NPFcb->Resource,
                                    TRUE);

                    __try
                    {

                        CcFlushCache( &pObjectInfo->Fcb->NPFcb->SectionObjectPointers,
                                      NULL,
                                      0,
                                      &stIoStatus);

                        if( !NT_SUCCESS( stIoStatus.Status))
                        {

                            AFSDbgLogMsg( AFS_SUBSYSTEM_IO_PROCESSING,
                                          AFS_TRACE_LEVEL_ERROR,
                                          "AFSVerifyEntry CcFlushCache failure %wZ FID %08lX-%08lX-%08lX-%08lX Status 0x%08lX Bytes 0x%08lX\n",
                                          &DirEntry->NameInformation.FileName,
                                          pObjectInfo->FileId.Cell,
                                          pObjectInfo->FileId.Volume,
                                          pObjectInfo->FileId.Vnode,
                                          pObjectInfo->FileId.Unique,
                                          stIoStatus.Status,
                                          stIoStatus.Information);

                            ntStatus = stIoStatus.Status;
                        }

                        if ( bPurgeExtents)
                        {

                            CcPurgeCacheSection( &pObjectInfo->Fcb->NPFcb->SectionObjectPointers,
                                                 NULL,
                                                 0,
                                                 FALSE);
                        }
                    }
                    __except( EXCEPTION_EXECUTE_HANDLER)
                    {
                        ntStatus = GetExceptionCode();

                        AFSDbgLogMsg( AFS_SUBSYSTEM_IO_PROCESSING,
                                      AFS_TRACE_LEVEL_ERROR,
                                      "AFSVerifyEntry CcFlushCache or CcPurgeCacheSection Exception %wZ FID %08lX-%08lX-%08lX-%08lX Status 0x%08lX\n",
                                      &DirEntry->NameInformation.FileName,
                                      pObjectInfo->FileId.Cell,
                                      pObjectInfo->FileId.Volume,
                                      pObjectInfo->FileId.Vnode,
                                      pObjectInfo->FileId.Unique,
                                      ntStatus);
                    }

                    AFSReleaseResource( &pObjectInfo->Fcb->NPFcb->Resource);

                    if ( bPurgeExtents)
                    {
                        AFSFlushExtents( pObjectInfo->Fcb);
                    }

                    //
                    // Reacquire the Fcb to purge the cache
                    //

                    AFSDbgLogMsg( AFS_SUBSYSTEM_LOCK_PROCESSING,
                                  AFS_TRACE_LEVEL_VERBOSE,
                                  "AFSVerifyEntry Acquiring Fcb lock %08lX EXCL %08lX\n",
                                  &pObjectInfo->Fcb->NPFcb->Resource,
                                  PsGetCurrentThread());

                    AFSAcquireExcl( &pObjectInfo->Fcb->NPFcb->Resource,
                                    TRUE);

                    //
                    // Update file sizes
                    //

                    pObjectInfo->Fcb->Header.AllocationSize.QuadPart  = pObjectInfo->AllocationSize.QuadPart;
                    pObjectInfo->Fcb->Header.FileSize.QuadPart        = pObjectInfo->EndOfFile.QuadPart;
                    pObjectInfo->Fcb->Header.ValidDataLength.QuadPart = pObjectInfo->EndOfFile.QuadPart;

                    pCCFileObject = CcGetFileObjectFromSectionPtrs( &pObjectInfo->Fcb->NPFcb->SectionObjectPointers);

                    if ( pCCFileObject != NULL)
                    {
                        CcSetFileSizes( pCCFileObject,
                                        (PCC_FILE_SIZES)&pObjectInfo->Fcb->Header.AllocationSize);
                    }

                    AFSReleaseResource( &pObjectInfo->Fcb->NPFcb->Resource);
                }
                else
                {
                    AFSDbgLogMsg( AFS_SUBSYSTEM_FILE_PROCESSING,
                                  AFS_TRACE_LEVEL_WARNING,
                                  "AFSValidateEntry Fcb NULL %wZ FID %08lX-%08lX-%08lX-%08lX\n",
                                  &DirEntry->NameInformation.FileName,
                                  pObjectInfo->FileId.Cell,
                                  pObjectInfo->FileId.Volume,
                                  pObjectInfo->FileId.Vnode,
                                  pObjectInfo->FileId.Unique);
                }

                ClearFlag( pObjectInfo->Flags, AFS_OBJECT_FLAGS_VERIFY);

                break;
            }

            case AFS_FILE_TYPE_DIRECTORY:
            {

                AFSFcb *pCurrentFcb = NULL;
                AFSDirectoryCB *pCurrentDirEntry = NULL;

                //
                // For a directory or root entry flush the content of
                // the directory enumeration.
                //

                if( BooleanFlagOn( pObjectInfo->Flags, AFS_OBJECT_FLAGS_DIRECTORY_ENUMERATED))
                {

                    AFSDbgLogMsg( AFS_SUBSYSTEM_FILE_PROCESSING,
                                  AFS_TRACE_LEVEL_VERBOSE_2,
                                  "AFSVerifyEntry Validating directory content for entry %wZ FID %08lX-%08lX-%08lX-%08lX\n",
                                  &DirEntry->NameInformation.FileName,
                                  pObjectInfo->FileId.Cell,
                                  pObjectInfo->FileId.Volume,
                                  pObjectInfo->FileId.Vnode,
                                  pObjectInfo->FileId.Unique);

                    AFSAcquireExcl( pObjectInfo->Specific.Directory.DirectoryNodeHdr.TreeLock,
                                    TRUE);

                    AFSValidateDirectoryCache( pObjectInfo,
                                               AuthGroup);

                    AFSReleaseResource( pObjectInfo->Specific.Directory.DirectoryNodeHdr.TreeLock);
                }

                //
                // Update the metadata for the entry
                //

                ntStatus = AFSUpdateMetaData( DirEntry,
                                              pDirEnumEntry);

                if( NT_SUCCESS( ntStatus))
                {

                    ClearFlag( pObjectInfo->Flags, AFS_OBJECT_FLAGS_VERIFY);
                }

                break;
            }

            case AFS_FILE_TYPE_DFSLINK:
            {

                UNICODE_STRING uniTargetName;

                //
                // For a DFS link need to check the target name has not changed
                //

                uniTargetName.Length = (USHORT)pDirEnumEntry->TargetNameLength;

                uniTargetName.MaximumLength = uniTargetName.Length;

                uniTargetName.Buffer = (WCHAR *)((char *)pDirEnumEntry + pDirEnumEntry->TargetNameOffset);

                AFSAcquireExcl( &DirEntry->NonPaged->Lock,
                                TRUE);

                if( DirEntry->NameInformation.TargetName.Length == 0 ||
                    RtlCompareUnicodeString( &uniTargetName,
                                             &DirEntry->NameInformation.TargetName,
                                             TRUE) != 0)
                {

                    //
                    // Update the target name
                    //

                    ntStatus = AFSUpdateTargetName( &DirEntry->NameInformation.TargetName,
                                                    &DirEntry->Flags,
                                                    uniTargetName.Buffer,
                                                    uniTargetName.Length);

                    if( !NT_SUCCESS( ntStatus))
                    {

                        AFSReleaseResource( &DirEntry->NonPaged->Lock);

                        break;
                    }
                }

                AFSReleaseResource( &DirEntry->NonPaged->Lock);

                //
                // Update the metadata for the entry
                //

                ntStatus = AFSUpdateMetaData( DirEntry,
                                              pDirEnumEntry);

                if( NT_SUCCESS( ntStatus))
                {

                    ClearFlag( pObjectInfo->Flags, AFS_OBJECT_FLAGS_VERIFY);
                }

                break;
            }

            default:

                AFSDbgLogMsg( AFS_SUBSYSTEM_FILE_PROCESSING,
                              AFS_TRACE_LEVEL_WARNING,
                              "AFSVerifyEntry Attempt to verify node of type %d\n",
                              pObjectInfo->FileType);

                break;
        }

 try_exit:

        if( pDirEnumEntry != NULL)
        {

            AFSExFreePool( pDirEnumEntry);
        }
    }

    return ntStatus;
}

NTSTATUS
AFSSetVolumeState( IN AFSVolumeStatusCB *VolumeStatus)
{

    NTSTATUS ntStatus = STATUS_SUCCESS;
    AFSDeviceExt *pDevExt = (AFSDeviceExt *) AFSRDRDeviceObject->DeviceExtension;
    ULONGLONG   ullIndex = 0;
    AFSVolumeCB *pVolumeCB = NULL;
    AFSFcb *pFcb = NULL;
    AFSObjectInfoCB *pCurrentObject = NULL;

    __Enter
    {

        AFSDbgLogMsg( AFS_SUBSYSTEM_FILE_PROCESSING,
                      AFS_TRACE_LEVEL_VERBOSE,
                      "AFSSetVolumeState Marking volume state %d Volume Cell %08lX Volume %08lX\n",
                      VolumeStatus->Online,
                      VolumeStatus->FileID.Cell,
                      VolumeStatus->FileID.Volume);

        //
        // Need to locate the Fcb for the directory to purge
        //

        AFSDbgLogMsg( AFS_SUBSYSTEM_LOCK_PROCESSING,
                      AFS_TRACE_LEVEL_VERBOSE,
                      "AFSSetVolumeState Acquiring RDR VolumeTreeLock lock %08lX SHARED %08lX\n",
                      &pDevExt->Specific.RDR.VolumeTreeLock,
                      PsGetCurrentThread());

        AFSAcquireShared( &pDevExt->Specific.RDR.VolumeTreeLock, TRUE);

        //
        // Locate the volume node
        //

        ullIndex = AFSCreateHighIndex( &VolumeStatus->FileID);

        ntStatus = AFSLocateHashEntry( pDevExt->Specific.RDR.VolumeTree.TreeHead,
                                       ullIndex,
                                       (AFSBTreeEntry **)&pVolumeCB);

        if( pVolumeCB != NULL)
        {

            InterlockedIncrement( &pVolumeCB->VolumeReferenceCount);

            AFSReleaseResource( &pDevExt->Specific.RDR.VolumeTreeLock);

            //
            // Set the volume state accordingly
            //

            if( VolumeStatus->Online)
            {

                InterlockedAnd( (LONG *)&(pVolumeCB->Flags), ~AFS_VOLUME_FLAGS_OFFLINE);
            }
            else
            {

                InterlockedOr( (LONG *)&(pVolumeCB->Flags), AFS_VOLUME_FLAGS_OFFLINE);
            }

            AFSAcquireShared( pVolumeCB->ObjectInfoTree.TreeLock,
                              TRUE);

            pCurrentObject = pVolumeCB->ObjectInfoListHead;;

            while( pCurrentObject != NULL)
            {

                if( VolumeStatus->Online)
                {

                    ClearFlag( pCurrentObject->Flags, AFS_OBJECT_FLAGS_OBJECT_INVALID);

                    SetFlag( pCurrentObject->Flags, AFS_OBJECT_FLAGS_VERIFY);

                    pCurrentObject->DataVersion.QuadPart = (ULONGLONG)-1;
                }
                else
                {

                    SetFlag( pCurrentObject->Flags, AFS_OBJECT_FLAGS_OBJECT_INVALID);
                }

                pFcb = pCurrentObject->Fcb;

                if( pFcb != NULL &&
                    !(VolumeStatus->Online) &&
                    pFcb->Header.NodeTypeCode == AFS_FILE_FCB)
                {

                    AFSDbgLogMsg( AFS_SUBSYSTEM_EXTENT_PROCESSING,
                                  AFS_TRACE_LEVEL_ERROR,
                                  "AFSSetVolumeState Marking volume offline and canceling extents Volume Cell %08lX Volume %08lX\n",
                                  VolumeStatus->FileID.Cell,
                                  VolumeStatus->FileID.Volume);

                    //
                    // Clear out the extents
                    //

                    AFSDbgLogMsg( AFS_SUBSYSTEM_LOCK_PROCESSING,
                                  AFS_TRACE_LEVEL_VERBOSE,
                                  "AFSSetVolumeState Acquiring Fcb extents lock %08lX EXCL %08lX\n",
                                  &pFcb->NPFcb->Specific.File.ExtentsResource,
                                  PsGetCurrentThread());

                    AFSAcquireExcl( &pFcb->NPFcb->Specific.File.ExtentsResource,
                                    TRUE);

                    pFcb->NPFcb->Specific.File.ExtentsRequestStatus = STATUS_CANCELLED;

                    KeSetEvent( &pFcb->NPFcb->Specific.File.ExtentsRequestComplete,
                                0,
                                FALSE);

                    AFSDbgLogMsg( AFS_SUBSYSTEM_LOCK_PROCESSING,
                                  AFS_TRACE_LEVEL_VERBOSE,
                                  "AFSSetVolumeState Releasing Fcb extents lock %08lX EXCL %08lX\n",
                                  &pFcb->NPFcb->Specific.File.ExtentsResource,
                                  PsGetCurrentThread());

                    AFSReleaseResource( &pFcb->NPFcb->Specific.File.ExtentsResource);

                    //
                    // And get rid of them (note this involves waiting
                    // for any writes or reads to the cache to complete)
                    //

                    (VOID) AFSTearDownFcbExtents( pFcb);
                }

                pCurrentObject = (AFSObjectInfoCB *)pCurrentObject->ListEntry.fLink;
            }

            AFSReleaseResource( pVolumeCB->ObjectInfoTree.TreeLock);

            InterlockedDecrement( &pVolumeCB->VolumeReferenceCount);
        }
        else
        {

            AFSReleaseResource( &pDevExt->Specific.RDR.VolumeTreeLock);
        }
    }

    return ntStatus;
}

NTSTATUS
AFSSetNetworkState( IN AFSNetworkStatusCB *NetworkStatus)
{

    NTSTATUS ntStatus = STATUS_SUCCESS;

    __Enter
    {

        if( AFSGlobalRoot == NULL)
        {

            try_return( ntStatus);
        }

        AFSAcquireExcl( AFSGlobalRoot->VolumeLock,
                        TRUE);

        //
        // Set the network state according to the information
        //

        if( NetworkStatus->Online)
        {

            ClearFlag( AFSGlobalRoot->Flags, AFS_VOLUME_FLAGS_OFFLINE);
        }
        else
        {

            SetFlag( AFSGlobalRoot->Flags, AFS_VOLUME_FLAGS_OFFLINE);
        }

        AFSReleaseResource( AFSGlobalRoot->VolumeLock);

try_exit:

        NOTHING;
    }

    return ntStatus;
}

NTSTATUS
AFSValidateDirectoryCache( IN AFSObjectInfoCB *ObjectInfo,
                           IN GUID *AuthGroup)
{

    NTSTATUS ntStatus = STATUS_SUCCESS;
    BOOLEAN  bAcquiredLock = FALSE;
    AFSDirectoryCB *pCurrentDirEntry = NULL, *pNextDirEntry = NULL;
    AFSFcb *pFcb = NULL;

    __Enter
    {

        AFSDbgLogMsg( AFS_SUBSYSTEM_FILE_PROCESSING,
                      AFS_TRACE_LEVEL_VERBOSE,
                      "AFSValidateDirectoryCache Validating content for FID %08lX-%08lX-%08lX-%08lX\n",
                      ObjectInfo->FileId.Cell,
                      ObjectInfo->FileId.Volume,
                      ObjectInfo->FileId.Vnode,
                      ObjectInfo->FileId.Unique);

        if( !ExIsResourceAcquiredLite( ObjectInfo->Specific.Directory.DirectoryNodeHdr.TreeLock))
        {

            AFSDbgLogMsg( AFS_SUBSYSTEM_LOCK_PROCESSING,
                          AFS_TRACE_LEVEL_VERBOSE,
                          "AFSValidateDirectoryCache Acquiring DirectoryNodeHdr.TreeLock lock %08lX EXCL %08lX\n",
                          ObjectInfo->Specific.Directory.DirectoryNodeHdr.TreeLock,
                          PsGetCurrentThread());

            AFSAcquireExcl( ObjectInfo->Specific.Directory.DirectoryNodeHdr.TreeLock,
                            TRUE);

            bAcquiredLock = TRUE;
        }

        //
        // Check for inconsistency between DirectoryNodeList and DirectoryNodeCount
        //

        if ( ObjectInfo->Specific.Directory.DirectoryNodeListHead == NULL &&
             ObjectInfo->Specific.Directory.DirectoryNodeCount > 0)
        {

            AFSDbgLogMsg( AFS_SUBSYSTEM_FILE_PROCESSING,
                          AFS_TRACE_LEVEL_ERROR,
                          "AFSValidateDirectoryCache Empty Node List but Non-Zero Node Count %08lX for dir FID %08lX-%08lX-%08lX-%08lX\n",
                          ObjectInfo->Specific.Directory.DirectoryNodeCount,
                          ObjectInfo->FileId.Cell,
                          ObjectInfo->FileId.Volume,
                          ObjectInfo->FileId.Vnode,
                          ObjectInfo->FileId.Unique);
        }

        //
        // Reset the directory list information by clearing all valid entries
        //

        pCurrentDirEntry = ObjectInfo->Specific.Directory.DirectoryNodeListHead;

        while( pCurrentDirEntry != NULL)
        {

            if( !BooleanFlagOn( pCurrentDirEntry->Flags, AFS_DIR_ENTRY_FAKE))
            {

                ClearFlag( pCurrentDirEntry->Flags, AFS_DIR_ENTRY_VALID);
            }

            pCurrentDirEntry = (AFSDirectoryCB *)pCurrentDirEntry->ListEntry.fLink;
        }

        //
        // Reget the directory contents
        //

        AFSVerifyDirectoryContent( ObjectInfo,
                                   AuthGroup);

        //
        // Now start again and tear down any entries not valid
        //

        pCurrentDirEntry = ObjectInfo->Specific.Directory.DirectoryNodeListHead;

        while( pCurrentDirEntry != NULL)
        {

            pNextDirEntry = (AFSDirectoryCB *)pCurrentDirEntry->ListEntry.fLink;

            if( BooleanFlagOn( pCurrentDirEntry->Flags, AFS_DIR_ENTRY_VALID))
            {

                pCurrentDirEntry = pNextDirEntry;

                continue;
            }

            if( pCurrentDirEntry->OpenReferenceCount == 0)
            {

                AFSDbgLogMsg( AFS_SUBSYSTEM_FILE_PROCESSING,
                              AFS_TRACE_LEVEL_VERBOSE,
                              "AFSValidateDirectoryCache Deleting dir entry %wZ from parent FID %08lX-%08lX-%08lX-%08lX\n",
                              &pCurrentDirEntry->NameInformation.FileName,
                              ObjectInfo->FileId.Cell,
                              ObjectInfo->FileId.Volume,
                              ObjectInfo->FileId.Vnode,
                              ObjectInfo->FileId.Unique);

                AFSDeleteDirEntry( ObjectInfo,
                                   pCurrentDirEntry);
            }
            else
            {

                AFSDbgLogMsg( AFS_SUBSYSTEM_FILE_PROCESSING,
                              AFS_TRACE_LEVEL_VERBOSE,
                              "AFSValidateDirectoryCache Setting dir entry %p Name %wZ DELETED in parent FID %08lX-%08lX-%08lX-%08lX\n",
                              pCurrentDirEntry,
                              &pCurrentDirEntry->NameInformation.FileName,
                              ObjectInfo->FileId.Cell,
                              ObjectInfo->FileId.Volume,
                              ObjectInfo->FileId.Vnode,
                              ObjectInfo->FileId.Unique);

                SetFlag( pCurrentDirEntry->Flags, AFS_DIR_ENTRY_DELETED);

                AFSRemoveNameEntry( ObjectInfo,
                                    pCurrentDirEntry);
            }

            pCurrentDirEntry = pNextDirEntry;
        }

#if DBG
        if( !AFSValidateDirList( ObjectInfo))
        {

            AFSPrint("AFSValidateDirectoryCache Invalid count ...\n");
        }
#endif

        if( bAcquiredLock)
        {

            AFSReleaseResource( ObjectInfo->Specific.Directory.DirectoryNodeHdr.TreeLock);
        }
    }

    return ntStatus;
}

BOOLEAN
AFSIsVolumeFID( IN AFSFileID *FileID)
{

    BOOLEAN bIsVolume = FALSE;

    if( FileID->Vnode == 1 &&
        FileID->Unique == 1)
    {

        bIsVolume = TRUE;
    }

    return bIsVolume;
}

BOOLEAN
AFSIsFinalNode( IN AFSFcb *Fcb)
{

    BOOLEAN bIsFinalNode = FALSE;

    if( Fcb->Header.NodeTypeCode == AFS_ROOT_FCB ||
        Fcb->Header.NodeTypeCode == AFS_DIRECTORY_FCB ||
        Fcb->Header.NodeTypeCode == AFS_FILE_FCB ||
        Fcb->Header.NodeTypeCode == AFS_DFS_LINK_FCB ||
        Fcb->Header.NodeTypeCode == AFS_INVALID_FCB )
    {

        bIsFinalNode = TRUE;
    }
    else
    {

        ASSERT( Fcb->Header.NodeTypeCode == AFS_MOUNT_POINT_FCB ||
                Fcb->Header.NodeTypeCode == AFS_SYMBOLIC_LINK_FCB);
    }

    return bIsFinalNode;
}

NTSTATUS
AFSUpdateMetaData( IN AFSDirectoryCB *DirEntry,
                   IN AFSDirEnumEntry *DirEnumEntry)
{

    NTSTATUS ntStatus = STATUS_SUCCESS;
    UNICODE_STRING uniTargetName;
    AFSObjectInfoCB *pObjectInfo = DirEntry->ObjectInformation;

    __Enter
    {

        pObjectInfo->TargetFileId = DirEnumEntry->TargetFileId;

        pObjectInfo->Expiration = DirEnumEntry->Expiration;

        pObjectInfo->DataVersion = DirEnumEntry->DataVersion;

        pObjectInfo->FileType = DirEnumEntry->FileType;

        pObjectInfo->CreationTime = DirEnumEntry->CreationTime;

        pObjectInfo->LastAccessTime = DirEnumEntry->LastAccessTime;

        pObjectInfo->LastWriteTime = DirEnumEntry->LastWriteTime;

        pObjectInfo->ChangeTime = DirEnumEntry->ChangeTime;

        pObjectInfo->EndOfFile = DirEnumEntry->EndOfFile;

        pObjectInfo->AllocationSize = DirEnumEntry->AllocationSize;

        pObjectInfo->FileAttributes = DirEnumEntry->FileAttributes;

        if( pObjectInfo->FileType == AFS_FILE_TYPE_MOUNTPOINT ||
            pObjectInfo->FileType == AFS_FILE_TYPE_SYMLINK ||
            pObjectInfo->FileType == AFS_FILE_TYPE_DFSLINK)
        {

            pObjectInfo->FileAttributes |= FILE_ATTRIBUTE_REPARSE_POINT;
        }

        pObjectInfo->EaSize = DirEnumEntry->EaSize;

        pObjectInfo->Links = DirEnumEntry->Links;

        if( DirEnumEntry->TargetNameLength > 0)
        {

            //
            // Update the target name information if needed
            //

            uniTargetName.Length = (USHORT)DirEnumEntry->TargetNameLength;

            uniTargetName.MaximumLength = uniTargetName.Length;

            uniTargetName.Buffer = (WCHAR *)((char *)DirEnumEntry + DirEnumEntry->TargetNameOffset);

            AFSAcquireExcl( &DirEntry->NonPaged->Lock,
                            TRUE);

            if( DirEntry->NameInformation.TargetName.Length == 0 ||
                RtlCompareUnicodeString( &uniTargetName,
                                         &DirEntry->NameInformation.TargetName,
                                         TRUE) != 0)
            {

                //
                // Update the target name
                //

                ntStatus = AFSUpdateTargetName( &DirEntry->NameInformation.TargetName,
                                                &DirEntry->Flags,
                                                uniTargetName.Buffer,
                                                uniTargetName.Length);

                if( !NT_SUCCESS( ntStatus))
                {

                    AFSReleaseResource( &DirEntry->NonPaged->Lock);

                    try_return( ntStatus);
                }
            }

            AFSReleaseResource( &DirEntry->NonPaged->Lock);
        }
        else if( DirEntry->NameInformation.TargetName.Length > 0)
        {

            AFSAcquireExcl( &DirEntry->NonPaged->Lock,
                            TRUE);

            if( BooleanFlagOn( DirEntry->Flags, AFS_DIR_RELEASE_TARGET_NAME_BUFFER) &&
                DirEntry->NameInformation.TargetName.Buffer != NULL)
            {
                AFSExFreePool( DirEntry->NameInformation.TargetName.Buffer);
            }

            ClearFlag( DirEntry->Flags, AFS_DIR_RELEASE_TARGET_NAME_BUFFER);

            DirEntry->NameInformation.TargetName.Length = 0;
            DirEntry->NameInformation.TargetName.MaximumLength = 0;
            DirEntry->NameInformation.TargetName.Buffer = NULL;

            AFSReleaseResource( &DirEntry->NonPaged->Lock);
        }

try_exit:

        NOTHING;
    }

    return ntStatus;
}

NTSTATUS
AFSValidateEntry( IN AFSDirectoryCB *DirEntry,
                  IN GUID *AuthGroup,
                  IN BOOLEAN PurgeContent,
                  IN BOOLEAN FastCall)
{

    NTSTATUS ntStatus = STATUS_SUCCESS;
    LARGE_INTEGER liSystemTime;
    AFSDirEnumEntry *pDirEnumEntry = NULL;
    AFSFcb *pCurrentFcb = NULL;
    BOOLEAN bReleaseFcb = FALSE;
    AFSObjectInfoCB *pObjectInfo = DirEntry->ObjectInformation;

    __Enter
    {

        //
        // If we have an Fcb hanging off the directory entry then be sure to acquire the locks in the
        // correct order
        //

        AFSDbgLogMsg( AFS_SUBSYSTEM_FILE_PROCESSING,
                      AFS_TRACE_LEVEL_VERBOSE_2,
                      "AFSValidateEntry Validating entry %wZ FID %08lX-%08lX-%08lX-%08lX\n",
                      &DirEntry->NameInformation.FileName,
                      pObjectInfo->FileId.Cell,
                      pObjectInfo->FileId.Volume,
                      pObjectInfo->FileId.Vnode,
                      pObjectInfo->FileId.Unique);

        //
        // If this is a fake node then bail since the service knows nothing about it
        //

        if( BooleanFlagOn( DirEntry->Flags, AFS_DIR_ENTRY_FAKE))
        {

            try_return( ntStatus);
        }

        if( PurgeContent &&
            pObjectInfo->Fcb != NULL)
        {

            pCurrentFcb = pObjectInfo->Fcb;

            if( !ExIsResourceAcquiredLite( &pCurrentFcb->NPFcb->Resource))
            {

                AFSDbgLogMsg( AFS_SUBSYSTEM_LOCK_PROCESSING,
                              AFS_TRACE_LEVEL_VERBOSE,
                              "AFSValidateEntry Acquiring Fcb lock %08lX EXCL %08lX\n",
                              &pCurrentFcb->NPFcb->Resource,
                              PsGetCurrentThread());

                AFSAcquireExcl( &pCurrentFcb->NPFcb->Resource,
                                TRUE);

                bReleaseFcb = TRUE;
            }
        }

        //
        // This routine ensures that the current entry is valid by:
        //
        //      1) Checking that the expiration time is non-zero and after where we
        //         currently are
        //

        KeQuerySystemTime( &liSystemTime);

        if( !BooleanFlagOn( pObjectInfo->Flags, AFS_OBJECT_FLAGS_NOT_EVALUATED) &&
            !BooleanFlagOn( pObjectInfo->Flags, AFS_OBJECT_FLAGS_VERIFY) &&
            !BooleanFlagOn( pObjectInfo->Flags, AFS_OBJECT_FLAGS_VERIFY_DATA) &&
            pObjectInfo->Expiration.QuadPart >= liSystemTime.QuadPart)
        {

            AFSDbgLogMsg( AFS_SUBSYSTEM_FILE_PROCESSING,
                          AFS_TRACE_LEVEL_VERBOSE_2,
                          "AFSValidateEntry Directory entry %wZ FID %08lX-%08lX-%08lX-%08lX VALID\n",
                          &DirEntry->NameInformation.FileName,
                          pObjectInfo->FileId.Cell,
                          pObjectInfo->FileId.Volume,
                          pObjectInfo->FileId.Vnode,
                          pObjectInfo->FileId.Unique);

            try_return( ntStatus);
        }

        //
        // This node requires updating
        //

        ntStatus = AFSEvaluateTargetByID( pObjectInfo,
                                          AuthGroup,
                                          FastCall,
                                          &pDirEnumEntry);

        if( !NT_SUCCESS( ntStatus))
        {

            AFSDbgLogMsg( AFS_SUBSYSTEM_FILE_PROCESSING,
                          AFS_TRACE_LEVEL_ERROR,
                          "AFSValidateEntry Failed to evaluate entry %wZ FID %08lX-%08lX-%08lX-%08lX Status %08lX\n",
                          &DirEntry->NameInformation.FileName,
                          pObjectInfo->FileId.Cell,
                          pObjectInfo->FileId.Volume,
                          pObjectInfo->FileId.Vnode,
                          pObjectInfo->FileId.Unique,
                          ntStatus);

            //
            // Failed validation of node so return access-denied
            //

            try_return( ntStatus);
        }

        AFSDbgLogMsg( AFS_SUBSYSTEM_FILE_PROCESSING,
                      AFS_TRACE_LEVEL_VERBOSE,
                      "AFSValidateEntry Validating entry %wZ FID %08lX-%08lX-%08lX-%08lX DV %I64X returned DV %I64X FT %d\n",
                      &DirEntry->NameInformation.FileName,
                      pObjectInfo->FileId.Cell,
                      pObjectInfo->FileId.Volume,
                      pObjectInfo->FileId.Vnode,
                      pObjectInfo->FileId.Unique,
                      pObjectInfo->DataVersion.QuadPart,
                      pDirEnumEntry->DataVersion.QuadPart,
                      pDirEnumEntry->FileType);


        //
        // Based on the file type, process the node
        //

        switch( pDirEnumEntry->FileType)
        {

            case AFS_FILE_TYPE_MOUNTPOINT:
            {

                //
                // Update the metadata for the entry
                //

                ntStatus = AFSUpdateMetaData( DirEntry,
                                              pDirEnumEntry);

                if( NT_SUCCESS( ntStatus))
                {

                    ClearFlag( pObjectInfo->Flags, AFS_OBJECT_FLAGS_VERIFY | AFS_OBJECT_FLAGS_NOT_EVALUATED);
                }

                break;
            }

            case AFS_FILE_TYPE_SYMLINK:
            case AFS_FILE_TYPE_DFSLINK:
            {

                //
                // Update the metadata for the entry
                //

                ntStatus = AFSUpdateMetaData( DirEntry,
                                              pDirEnumEntry);

                if( NT_SUCCESS( ntStatus))
                {

                    ClearFlag( pObjectInfo->Flags, AFS_OBJECT_FLAGS_VERIFY | AFS_OBJECT_FLAGS_NOT_EVALUATED);
                }

                break;
            }

            case AFS_FILE_TYPE_FILE:
            {

                //
                // For a file where the data version has become invalid we need to
                // fail any current extent requests and purge the cache for the file
                // Can't hold the Fcb resource while doing this
                //

                if( pCurrentFcb != NULL &&
                    (pObjectInfo->DataVersion.QuadPart != pDirEnumEntry->DataVersion.QuadPart ||
                    BooleanFlagOn( pObjectInfo->Flags, AFS_OBJECT_FLAGS_VERIFY_DATA)))
                {

                    IO_STATUS_BLOCK stIoStatus;
                    BOOLEAN bPurgeExtents = FALSE;

                    AFSDbgLogMsg( AFS_SUBSYSTEM_FILE_PROCESSING,
                                  AFS_TRACE_LEVEL_VERBOSE_2,
                                  "AFSValidateEntry Flush/purge entry %wZ FID %08lX-%08lX-%08lX-%08lX\n",
                                  &DirEntry->NameInformation.FileName,
                                  pObjectInfo->FileId.Cell,
                                  pObjectInfo->FileId.Volume,
                                  pObjectInfo->FileId.Vnode,
                                  pObjectInfo->FileId.Unique);

                    if ( BooleanFlagOn( pObjectInfo->Flags, AFS_OBJECT_FLAGS_VERIFY_DATA))
                    {
                        bPurgeExtents = TRUE;

                        AFSDbgLogMsg( AFS_SUBSYSTEM_FILE_PROCESSING,
                                      AFS_TRACE_LEVEL_VERBOSE,
                                      "AFSVerifyEntry Clearing VERIFY_DATA flag %wZ FID %08lX-%08lX-%08lX-%08lX\n",
                                      &DirEntry->NameInformation.FileName,
                                      pObjectInfo->FileId.Cell,
                                      pObjectInfo->FileId.Volume,
                                      pObjectInfo->FileId.Vnode,
                                      pObjectInfo->FileId.Unique);

                        ClearFlag( pObjectInfo->Flags, AFS_OBJECT_FLAGS_VERIFY_DATA);
                    }

                    __try
                    {

                        CcFlushCache( &pCurrentFcb->NPFcb->SectionObjectPointers,
                                      NULL,
                                      0,
                                      &stIoStatus);

                        if( !NT_SUCCESS( stIoStatus.Status))
                        {

                            AFSDbgLogMsg( AFS_SUBSYSTEM_IO_PROCESSING,
                                          AFS_TRACE_LEVEL_ERROR,
                                          "AFSValidateEntry CcFlushCache failure %wZ FID %08lX-%08lX-%08lX-%08lX Status 0x%08lX Bytes 0x%08lX\n",
                                          &DirEntry->NameInformation.FileName,
                                          pObjectInfo->FileId.Cell,
                                          pObjectInfo->FileId.Volume,
                                          pObjectInfo->FileId.Vnode,
                                          pObjectInfo->FileId.Unique,
                                          stIoStatus.Status,
                                          stIoStatus.Information);

                            ntStatus = stIoStatus.Status;
                        }

                        if ( bPurgeExtents)
                        {

                            CcPurgeCacheSection( &pObjectInfo->Fcb->NPFcb->SectionObjectPointers,
                                                 NULL,
                                                 0,
                                                 FALSE);
                        }
                    }
                    __except( EXCEPTION_EXECUTE_HANDLER)
                    {
                        ntStatus = GetExceptionCode();

                        AFSDbgLogMsg( AFS_SUBSYSTEM_IO_PROCESSING,
                                      AFS_TRACE_LEVEL_ERROR,
                                      "AFSValidateEntry CcFlushCache or CcPurgeCacheSection exception %wZ FID %08lX-%08lX-%08lX-%08lX Status 0x%08lX\n",
                                      &DirEntry->NameInformation.FileName,
                                      pObjectInfo->FileId.Cell,
                                      pObjectInfo->FileId.Volume,
                                      pObjectInfo->FileId.Vnode,
                                      pObjectInfo->FileId.Unique,
                                      ntStatus);

                    }

                    AFSReleaseResource( &pCurrentFcb->NPFcb->Resource);

                    if ( bPurgeExtents)
                    {
                        AFSFlushExtents( pCurrentFcb);
                    }

                    //
                    // Reacquire the Fcb to purge the cache
                    //

                    AFSDbgLogMsg( AFS_SUBSYSTEM_LOCK_PROCESSING,
                                  AFS_TRACE_LEVEL_VERBOSE,
                                  "AFSValidateEntry Acquiring Fcb lock %08lX EXCL %08lX\n",
                                  &pCurrentFcb->NPFcb->Resource,
                                  PsGetCurrentThread());

                    AFSAcquireExcl( &pCurrentFcb->NPFcb->Resource,
                                    TRUE);
                }

                //
                // Update the metadata for the entry
                //

                ntStatus = AFSUpdateMetaData( DirEntry,
                                              pDirEnumEntry);

                if( !NT_SUCCESS( ntStatus))
                {

                    AFSDbgLogMsg( AFS_SUBSYSTEM_FILE_PROCESSING,
                                  AFS_TRACE_LEVEL_ERROR,
                                  "AFSValidateEntry Meta Data Update failed %wZ FID %08lX-%08lX-%08lX-%08lX Status 0x%08lX\n",
                                  &DirEntry->NameInformation.FileName,
                                  pObjectInfo->FileId.Cell,
                                  pObjectInfo->FileId.Volume,
                                  pObjectInfo->FileId.Vnode,
                                  pObjectInfo->FileId.Unique,
                                  ntStatus);

                    break;
                }

                ClearFlag( pObjectInfo->Flags, AFS_OBJECT_FLAGS_VERIFY | AFS_OBJECT_FLAGS_NOT_EVALUATED);

                //
                // Update file sizes
                //

                if( pObjectInfo->Fcb != NULL)
                {
                    FILE_OBJECT *pCCFileObject = CcGetFileObjectFromSectionPtrs( &pObjectInfo->Fcb->NPFcb->SectionObjectPointers);

                    pObjectInfo->Fcb->Header.AllocationSize.QuadPart  = pObjectInfo->AllocationSize.QuadPart;
                    pObjectInfo->Fcb->Header.FileSize.QuadPart        = pObjectInfo->EndOfFile.QuadPart;
                    pObjectInfo->Fcb->Header.ValidDataLength.QuadPart = pObjectInfo->EndOfFile.QuadPart;

                    if ( pCCFileObject != NULL)
                    {
                        CcSetFileSizes( pCCFileObject,
                                        (PCC_FILE_SIZES)&pObjectInfo->Fcb->Header.AllocationSize);
                    }
                }

                break;
            }

            case AFS_FILE_TYPE_DIRECTORY:
            {

                AFSDirectoryCB *pCurrentDirEntry = NULL;

                if( pCurrentFcb != NULL &&
                    pObjectInfo->DataVersion.QuadPart != pDirEnumEntry->DataVersion.QuadPart)
                {

                    //
                    // For a directory or root entry flush the content of
                    // the directory enumeration.
                    //

                    AFSDbgLogMsg( AFS_SUBSYSTEM_LOCK_PROCESSING,
                                  AFS_TRACE_LEVEL_VERBOSE,
                                  "AFSValidateEntry Acquiring DirectoryNodeHdr.TreeLock lock %08lX EXCL %08lX\n",
                                  pObjectInfo->Specific.Directory.DirectoryNodeHdr.TreeLock,
                                  PsGetCurrentThread());

                    if( BooleanFlagOn( pObjectInfo->Flags, AFS_OBJECT_FLAGS_DIRECTORY_ENUMERATED))
                    {

                        AFSDbgLogMsg( AFS_SUBSYSTEM_FILE_PROCESSING,
                                      AFS_TRACE_LEVEL_VERBOSE_2,
                                      "AFSValidateEntry Validating directory content for %wZ FID %08lX-%08lX-%08lX-%08lX\n",
                                      &DirEntry->NameInformation.FileName,
                                      pObjectInfo->FileId.Cell,
                                      pObjectInfo->FileId.Volume,
                                      pObjectInfo->FileId.Vnode,
                                      pObjectInfo->FileId.Unique);

                        AFSAcquireExcl( pObjectInfo->Specific.Directory.DirectoryNodeHdr.TreeLock,
                                        TRUE);

                        AFSValidateDirectoryCache( pCurrentFcb->ObjectInformation,
                                                   AuthGroup);

                        AFSReleaseResource( pObjectInfo->Specific.Directory.DirectoryNodeHdr.TreeLock);
                    }

                    if( !NT_SUCCESS( ntStatus))
                    {

                        AFSDbgLogMsg( AFS_SUBSYSTEM_FILE_PROCESSING,
                                      AFS_TRACE_LEVEL_ERROR,
                                      "AFSValidateEntry Failed to re-enumerate %wZ FID %08lX-%08lX-%08lX-%08lX Status %08lX\n",
                                      &DirEntry->NameInformation.FileName,
                                      pObjectInfo->FileId.Cell,
                                      pObjectInfo->FileId.Volume,
                                      pObjectInfo->FileId.Vnode,
                                      pObjectInfo->FileId.Unique,
                                      ntStatus);

                        break;
                    }
                }

                //
                // Update the metadata for the entry
                //

                ntStatus = AFSUpdateMetaData( DirEntry,
                                              pDirEnumEntry);

                if( NT_SUCCESS( ntStatus))
                {

                    ClearFlag( pObjectInfo->Flags, AFS_OBJECT_FLAGS_VERIFY | AFS_OBJECT_FLAGS_NOT_EVALUATED);
                }

                break;
            }

            default:

                AFSDbgLogMsg( AFS_SUBSYSTEM_FILE_PROCESSING,
                              AFS_TRACE_LEVEL_WARNING,
                              "AFSValidateEntry Attempt to verify node of type %d\n",
                              pObjectInfo->FileType);

                break;
        }

 try_exit:

        if( bReleaseFcb)
        {

            AFSReleaseResource( &pCurrentFcb->NPFcb->Resource);
        }

        if( pDirEnumEntry != NULL)
        {

            AFSExFreePool( pDirEnumEntry);
        }
    }

    return ntStatus;
}

NTSTATUS
AFSInitializeSpecialShareNameList()
{

    NTSTATUS ntStatus = STATUS_SUCCESS;
    AFSDirectoryCB *pDirNode = NULL, *pLastDirNode = NULL;
    AFSObjectInfoCB *pObjectInfoCB = NULL;
    UNICODE_STRING uniShareName;
    ULONG ulEntryLength = 0;
    AFSNonPagedDirectoryCB *pNonPagedDirEntry = NULL;

    __Enter
    {

        RtlInitUnicodeString( &uniShareName,
                              L"PIPE\\srvsvc");

        pObjectInfoCB = AFSAllocateObjectInfo( &AFSGlobalRoot->ObjectInformation,
                                               0);

        if( pObjectInfoCB == NULL)
        {

            try_return( ntStatus = STATUS_INSUFFICIENT_RESOURCES);
        }

        AFSDbgLogMsg( AFS_SUBSYSTEM_OBJECT_REF_COUNTING,
                      AFS_TRACE_LEVEL_VERBOSE,
                      "AFSInitializeSpecialShareNameList (srvsvc) Initializing count (1) on object %08lX\n",
                                                    pObjectInfoCB);

        pObjectInfoCB->ObjectReferenceCount = 1;

        pObjectInfoCB->FileType = AFS_FILE_TYPE_SPECIAL_SHARE_NAME;

        ulEntryLength = sizeof( AFSDirectoryCB) +
                                     uniShareName.Length;

        pDirNode = (AFSDirectoryCB *)AFSLibExAllocatePoolWithTag( PagedPool,
                                                                  ulEntryLength,
                                                                  AFS_DIR_ENTRY_TAG);

        if( pDirNode == NULL)
        {

            AFSDeleteObjectInfo( pObjectInfoCB);

            try_return( ntStatus = STATUS_INSUFFICIENT_RESOURCES);
        }

        pNonPagedDirEntry = (AFSNonPagedDirectoryCB *)AFSLibExAllocatePoolWithTag( NonPagedPool,
                                                                                   sizeof( AFSNonPagedDirectoryCB),
                                                                                   AFS_DIR_ENTRY_NP_TAG);

        if( pNonPagedDirEntry == NULL)
        {

            ExFreePool( pDirNode);

            AFSDeleteObjectInfo( pObjectInfoCB);

            try_return( ntStatus = STATUS_INSUFFICIENT_RESOURCES);
        }

        RtlZeroMemory( pDirNode,
                       ulEntryLength);

        RtlZeroMemory( pNonPagedDirEntry,
                       sizeof( AFSNonPagedDirectoryCB));

        ExInitializeResourceLite( &pNonPagedDirEntry->Lock);

        pDirNode->NonPaged = pNonPagedDirEntry;

        pDirNode->ObjectInformation = pObjectInfoCB;

        //
        // Set valid entry
        //

        SetFlag( pDirNode->Flags, AFS_DIR_ENTRY_VALID | AFS_DIR_ENTRY_SERVER_SERVICE);

        pDirNode->NameInformation.FileName.Length = uniShareName.Length;

        pDirNode->NameInformation.FileName.MaximumLength = uniShareName.Length;

        pDirNode->NameInformation.FileName.Buffer = (WCHAR *)((char *)pDirNode + sizeof( AFSDirectoryCB));

        RtlCopyMemory( pDirNode->NameInformation.FileName.Buffer,
                       uniShareName.Buffer,
                       pDirNode->NameInformation.FileName.Length);

        pDirNode->CaseInsensitiveTreeEntry.HashIndex = AFSGenerateCRC( &pDirNode->NameInformation.FileName,
                                                                       TRUE);

        AFSSpecialShareNames = pDirNode;

        pLastDirNode = pDirNode;

        RtlInitUnicodeString( &uniShareName,
                              L"PIPE\\wkssvc");

        pObjectInfoCB = AFSAllocateObjectInfo( &AFSGlobalRoot->ObjectInformation,
                                               0);

        if( pObjectInfoCB == NULL)
        {

            try_return( ntStatus = STATUS_INSUFFICIENT_RESOURCES);
        }

        AFSDbgLogMsg( AFS_SUBSYSTEM_OBJECT_REF_COUNTING,
                      AFS_TRACE_LEVEL_VERBOSE,
                      "AFSInitializeSpecialShareNameList (wkssvc) Initializing count (1) on object %08lX\n",
                                                    pObjectInfoCB);

        pObjectInfoCB->ObjectReferenceCount = 1;

        pObjectInfoCB->FileType = AFS_FILE_TYPE_SPECIAL_SHARE_NAME;

        ulEntryLength = sizeof( AFSDirectoryCB) +
                                     uniShareName.Length;

        pDirNode = (AFSDirectoryCB *)AFSLibExAllocatePoolWithTag( PagedPool,
                                                                  ulEntryLength,
                                                                  AFS_DIR_ENTRY_TAG);

        if( pDirNode == NULL)
        {

            AFSDeleteObjectInfo( pObjectInfoCB);

            try_return( ntStatus = STATUS_INSUFFICIENT_RESOURCES);
        }

        pNonPagedDirEntry = (AFSNonPagedDirectoryCB *)AFSLibExAllocatePoolWithTag( NonPagedPool,
                                                                                   sizeof( AFSNonPagedDirectoryCB),
                                                                                   AFS_DIR_ENTRY_NP_TAG);

        if( pNonPagedDirEntry == NULL)
        {

            ExFreePool( pDirNode);

            AFSDeleteObjectInfo( pObjectInfoCB);

            try_return( ntStatus = STATUS_INSUFFICIENT_RESOURCES);
        }

        RtlZeroMemory( pDirNode,
                       ulEntryLength);

        RtlZeroMemory( pNonPagedDirEntry,
                       sizeof( AFSNonPagedDirectoryCB));

        ExInitializeResourceLite( &pNonPagedDirEntry->Lock);

        pDirNode->NonPaged = pNonPagedDirEntry;

        pDirNode->ObjectInformation = pObjectInfoCB;

        //
        // Set valid entry
        //

        SetFlag( pDirNode->Flags, AFS_DIR_ENTRY_VALID | AFS_DIR_ENTRY_WORKSTATION_SERVICE);

        pDirNode->NameInformation.FileName.Length = uniShareName.Length;

        pDirNode->NameInformation.FileName.MaximumLength = uniShareName.Length;

        pDirNode->NameInformation.FileName.Buffer = (WCHAR *)((char *)pDirNode + sizeof( AFSDirectoryCB));

        RtlCopyMemory( pDirNode->NameInformation.FileName.Buffer,
                       uniShareName.Buffer,
                       pDirNode->NameInformation.FileName.Length);

        pDirNode->CaseInsensitiveTreeEntry.HashIndex = AFSGenerateCRC( &pDirNode->NameInformation.FileName,
                                                                       TRUE);

        pLastDirNode->ListEntry.fLink = pDirNode;

        pDirNode->ListEntry.bLink = pLastDirNode;

        pLastDirNode = pDirNode;

        RtlInitUnicodeString( &uniShareName,
                              L"IPC$");

        pObjectInfoCB = AFSAllocateObjectInfo( &AFSGlobalRoot->ObjectInformation,
                                               0);

        if( pObjectInfoCB == NULL)
        {

            try_return( ntStatus = STATUS_INSUFFICIENT_RESOURCES);
        }

        AFSDbgLogMsg( AFS_SUBSYSTEM_OBJECT_REF_COUNTING,
                      AFS_TRACE_LEVEL_VERBOSE,
                      "AFSInitializeSpecialShareNameList (ipc$) Initializing count (1) on object %08lX\n",
                                                    pObjectInfoCB);

        pObjectInfoCB->ObjectReferenceCount = 1;

        pObjectInfoCB->FileType = AFS_FILE_TYPE_SPECIAL_SHARE_NAME;

        ulEntryLength = sizeof( AFSDirectoryCB) +
                                     uniShareName.Length;

        pDirNode = (AFSDirectoryCB *)AFSLibExAllocatePoolWithTag( PagedPool,
                                                                  ulEntryLength,
                                                                  AFS_DIR_ENTRY_TAG);

        if( pDirNode == NULL)
        {

            AFSDeleteObjectInfo( pObjectInfoCB);

            try_return( ntStatus = STATUS_INSUFFICIENT_RESOURCES);
        }

        pNonPagedDirEntry = (AFSNonPagedDirectoryCB *)AFSLibExAllocatePoolWithTag( NonPagedPool,
                                                                                   sizeof( AFSNonPagedDirectoryCB),
                                                                                   AFS_DIR_ENTRY_NP_TAG);

        if( pNonPagedDirEntry == NULL)
        {

            ExFreePool( pDirNode);

            AFSDeleteObjectInfo( pObjectInfoCB);

            try_return( ntStatus = STATUS_INSUFFICIENT_RESOURCES);
        }

        RtlZeroMemory( pDirNode,
                       ulEntryLength);

        RtlZeroMemory( pNonPagedDirEntry,
                       sizeof( AFSNonPagedDirectoryCB));

        ExInitializeResourceLite( &pNonPagedDirEntry->Lock);

        pDirNode->NonPaged = pNonPagedDirEntry;

        pDirNode->ObjectInformation = pObjectInfoCB;

        //
        // Set valid entry
        //

        SetFlag( pDirNode->Flags, AFS_DIR_ENTRY_VALID | AFS_DIR_ENTRY_IPC);

        pDirNode->NameInformation.FileName.Length = uniShareName.Length;

        pDirNode->NameInformation.FileName.MaximumLength = uniShareName.Length;

        pDirNode->NameInformation.FileName.Buffer = (WCHAR *)((char *)pDirNode + sizeof( AFSDirectoryCB));

        RtlCopyMemory( pDirNode->NameInformation.FileName.Buffer,
                       uniShareName.Buffer,
                       pDirNode->NameInformation.FileName.Length);

        pDirNode->CaseInsensitiveTreeEntry.HashIndex = AFSGenerateCRC( &pDirNode->NameInformation.FileName,
                                                                       TRUE);

        pLastDirNode->ListEntry.fLink = pDirNode;

        pDirNode->ListEntry.bLink = pLastDirNode;

try_exit:

        if( !NT_SUCCESS( ntStatus))
        {

            if( AFSSpecialShareNames != NULL)
            {

                pDirNode = AFSSpecialShareNames;

                while( pDirNode != NULL)
                {

                    pLastDirNode = (AFSDirectoryCB *)pDirNode->ListEntry.fLink;

                    AFSDeleteObjectInfo( pDirNode->ObjectInformation);

                    ExDeleteResourceLite( &pDirNode->NonPaged->Lock);

                    ExFreePool( pDirNode->NonPaged);

                    ExFreePool( pDirNode);

                    pDirNode = pLastDirNode;
                }

                AFSSpecialShareNames = NULL;
            }
        }
    }

    return ntStatus;
}

AFSDirectoryCB *
AFSGetSpecialShareNameEntry( IN UNICODE_STRING *ShareName,
                             IN UNICODE_STRING *SecondaryName)
{

    AFSDirectoryCB *pDirectoryCB = NULL;
    ULONGLONG ullHash = 0;
    UNICODE_STRING uniFullShareName;

    __Enter
    {

        //
        // Build up the entire name here. We are guaranteed that if there is a
        // secondary name, it is pointing to a portion of the share name buffer
        //

        if( SecondaryName->Length > 0 &&
            SecondaryName->Buffer != NULL)
        {

            uniFullShareName = *SecondaryName;

            //
            // The calling routine strips off the leading slash so add it back in
            //

            uniFullShareName.Buffer--;
            uniFullShareName.Length += sizeof( WCHAR);
            uniFullShareName.MaximumLength += sizeof( WCHAR);

            //
            // And the share name
            //

            uniFullShareName.Buffer -= (ShareName->Length/sizeof( WCHAR));
            uniFullShareName.Length += ShareName->Length;
            uniFullShareName.MaximumLength += ShareName->Length;
        }
        else
        {

            uniFullShareName = *ShareName;
        }

        //
        // Generate our hash value
        //

        ullHash = AFSGenerateCRC( &uniFullShareName,
                                  TRUE);

        //
        // Loop through our special share names to see if this is one of them
        //

        pDirectoryCB = AFSSpecialShareNames;

        while( pDirectoryCB != NULL)
        {

            if( ullHash == pDirectoryCB->CaseInsensitiveTreeEntry.HashIndex)
            {

                break;
            }

            pDirectoryCB = (AFSDirectoryCB *)pDirectoryCB->ListEntry.fLink;
        }
    }

    return pDirectoryCB;
}

void
AFSWaitOnQueuedFlushes( IN AFSFcb *Fcb)
{

    //
    // Block on the queue flush event
    //

    KeWaitForSingleObject( &Fcb->NPFcb->Specific.File.QueuedFlushEvent,
                           Executive,
                           KernelMode,
                           FALSE,
                           NULL);

    return;
}

void
AFSWaitOnQueuedReleases()
{

    AFSDeviceExt *pRDRDeviceExt = (AFSDeviceExt *)AFSRDRDeviceObject->DeviceExtension;

    //
    // Block on the queue flush event
    //

    KeWaitForSingleObject( &pRDRDeviceExt->Specific.RDR.QueuedReleaseExtentEvent,
                           Executive,
                           KernelMode,
                           FALSE,
                           NULL);

    return;
}

BOOLEAN
AFSIsEqualFID( IN AFSFileID *FileId1,
               IN AFSFileID *FileId2)
{

    BOOLEAN bIsEqual = FALSE;

    if( FileId1->Unique == FileId2->Unique &&
        FileId1->Vnode == FileId2->Vnode &&
        FileId1->Volume == FileId2->Volume &&
        FileId1->Cell == FileId2->Cell)
    {

        bIsEqual = TRUE;
    }

    return bIsEqual;
}

NTSTATUS
AFSResetDirectoryContent( IN AFSObjectInfoCB *ObjectInfoCB)
{

    NTSTATUS ntStatus = STATUS_SUCCESS;
    AFSDirectoryCB *pCurrentDirEntry = NULL, *pNextDirEntry = NULL;

    __Enter
    {

        ASSERT( ExIsResourceAcquiredExclusiveLite( ObjectInfoCB->Specific.Directory.DirectoryNodeHdr.TreeLock));

        //
        // Reset the directory list information
        //

        pCurrentDirEntry = ObjectInfoCB->Specific.Directory.DirectoryNodeListHead;

        while( pCurrentDirEntry != NULL)
        {

            pNextDirEntry = (AFSDirectoryCB *)pCurrentDirEntry->ListEntry.fLink;

            if( pCurrentDirEntry->OpenReferenceCount == 0)
            {

                AFSDbgLogMsg( AFS_SUBSYSTEM_CLEANUP_PROCESSING,
                              AFS_TRACE_LEVEL_VERBOSE,
                              "AFSResetDirectoryContent Deleting dir entry %p for %wZ\n",
                              pCurrentDirEntry,
                              &pCurrentDirEntry->NameInformation.FileName);

                AFSDeleteDirEntry( ObjectInfoCB,
                                   pCurrentDirEntry);
            }
            else
            {

                AFSDbgLogMsg( AFS_SUBSYSTEM_CLEANUP_PROCESSING,
                              AFS_TRACE_LEVEL_VERBOSE,
                              "AFSResetDirectoryContent Setting DELETE flag in dir entry %p for %wZ\n",
                              pCurrentDirEntry,
                              &pCurrentDirEntry->NameInformation.FileName);

                SetFlag( pCurrentDirEntry->Flags, AFS_DIR_ENTRY_DELETED);

                AFSRemoveNameEntry( ObjectInfoCB,
                                    pCurrentDirEntry);
            }

            pCurrentDirEntry = pNextDirEntry;
        }

        ObjectInfoCB->Specific.Directory.DirectoryNodeHdr.CaseSensitiveTreeHead = NULL;

        ObjectInfoCB->Specific.Directory.DirectoryNodeHdr.CaseInsensitiveTreeHead = NULL;

        ObjectInfoCB->Specific.Directory.ShortNameTree = NULL;

        ObjectInfoCB->Specific.Directory.DirectoryNodeListHead = NULL;

        ObjectInfoCB->Specific.Directory.DirectoryNodeListTail = NULL;

        ObjectInfoCB->Specific.Directory.DirectoryNodeCount = 0;

        AFSDbgLogMsg( AFS_SUBSYSTEM_DIR_NODE_COUNT,
                      AFS_TRACE_LEVEL_VERBOSE,
                      "AFSResetDirectoryContent Reset count to 0 on parent FID %08lX-%08lX-%08lX-%08lX\n",
                      ObjectInfoCB->FileId.Cell,
                      ObjectInfoCB->FileId.Volume,
                      ObjectInfoCB->FileId.Vnode,
                      ObjectInfoCB->FileId.Unique);
    }

    return ntStatus;
}

NTSTATUS
AFSEnumerateGlobalRoot( IN GUID *AuthGroup)
{

    NTSTATUS ntStatus = STATUS_SUCCESS;
    AFSDirectoryCB *pDirGlobalDirNode = NULL;
    UNICODE_STRING uniFullName;

    __Enter
    {

        AFSDbgLogMsg( AFS_SUBSYSTEM_LOCK_PROCESSING,
                      AFS_TRACE_LEVEL_VERBOSE,
                      "AFSEnumerateGlobalRoot Acquiring GlobalRoot DirectoryNodeHdr.TreeLock lock %08lX EXCL %08lX\n",
                      AFSGlobalRoot->ObjectInformation.Specific.Directory.DirectoryNodeHdr.TreeLock,
                      PsGetCurrentThread());

        AFSAcquireExcl( AFSGlobalRoot->ObjectInformation.Specific.Directory.DirectoryNodeHdr.TreeLock,
                        TRUE);

        if( BooleanFlagOn( AFSGlobalRoot->ObjectInformation.Flags, AFS_OBJECT_FLAGS_DIRECTORY_ENUMERATED))
        {

            try_return( ntStatus);
        }

        //
        // Initialize the root information
        //

        AFSGlobalRoot->ObjectInformation.Specific.Directory.DirectoryNodeHdr.ContentIndex = 1;

        //
        // Enumerate the shares in the volume
        //

        ntStatus = AFSEnumerateDirectory( AuthGroup,
                                          &AFSGlobalRoot->ObjectInformation,
                                          TRUE);

        if( !NT_SUCCESS( ntStatus))
        {

            try_return( ntStatus);
        }

        pDirGlobalDirNode = AFSGlobalRoot->ObjectInformation.Specific.Directory.DirectoryNodeListHead;

        //
        // Indicate the node is initialized
        //

        SetFlag( AFSGlobalRoot->ObjectInformation.Flags, AFS_OBJECT_FLAGS_DIRECTORY_ENUMERATED);

        uniFullName.MaximumLength = PAGE_SIZE;
        uniFullName.Length = 0;

        uniFullName.Buffer = (WCHAR *)AFSLibExAllocatePoolWithTag( PagedPool,
                                                                   uniFullName.MaximumLength,
                                                                   AFS_GENERIC_MEMORY_12_TAG);

        if( uniFullName.Buffer == NULL)
        {

            //
            // Reset the directory content
            //

            AFSResetDirectoryContent( &AFSGlobalRoot->ObjectInformation);

            ClearFlag( AFSGlobalRoot->ObjectInformation.Flags, AFS_OBJECT_FLAGS_DIRECTORY_ENUMERATED);

            try_return( ntStatus = STATUS_INSUFFICIENT_RESOURCES);
        }

        //
        // Populate our list of entries in the NP enumeration list
        //

        while( pDirGlobalDirNode != NULL)
        {

            uniFullName.Buffer[ 0] = L'\\';
            uniFullName.Buffer[ 1] = L'\\';

            uniFullName.Length = 2 * sizeof( WCHAR);

            RtlCopyMemory( &uniFullName.Buffer[ 2],
                           AFSServerName.Buffer,
                           AFSServerName.Length);

            uniFullName.Length += AFSServerName.Length;

            uniFullName.Buffer[ uniFullName.Length/sizeof( WCHAR)] = L'\\';

            uniFullName.Length += sizeof( WCHAR);

            RtlCopyMemory( &uniFullName.Buffer[ uniFullName.Length/sizeof( WCHAR)],
                           pDirGlobalDirNode->NameInformation.FileName.Buffer,
                           pDirGlobalDirNode->NameInformation.FileName.Length);

            uniFullName.Length += pDirGlobalDirNode->NameInformation.FileName.Length;

            AFSAddConnectionEx( &uniFullName,
                                RESOURCEDISPLAYTYPE_SHARE,
                                0);

            pDirGlobalDirNode = (AFSDirectoryCB *)pDirGlobalDirNode->ListEntry.fLink;
        }

        AFSExFreePool( uniFullName.Buffer);

try_exit:

        AFSReleaseResource( AFSGlobalRoot->ObjectInformation.Specific.Directory.DirectoryNodeHdr.TreeLock);
    }

    return ntStatus;
}

BOOLEAN
AFSIsRelativeName( IN UNICODE_STRING *Name)
{

    BOOLEAN bIsRelative = FALSE;

    if( Name->Buffer[ 0] != L'\\')
    {

        bIsRelative = TRUE;
    }

    return bIsRelative;
}

void
AFSUpdateName( IN UNICODE_STRING *Name)
{

    USHORT usIndex = 0;

    while( usIndex < Name->Length/sizeof( WCHAR))
    {

        if( Name->Buffer[ usIndex] == L'/')
        {

            Name->Buffer[ usIndex] = L'\\';
        }

        usIndex++;
    }

    return;
}

NTSTATUS
AFSUpdateTargetName( IN OUT UNICODE_STRING *TargetName,
                     IN OUT ULONG *Flags,
                     IN WCHAR *NameBuffer,
                     IN USHORT NameLength)
{

    NTSTATUS ntStatus = STATUS_SUCCESS;
    WCHAR *pTmpBuffer = NULL;

    __Enter
    {

        //
        // If we have enough space then just move in the name otherwise
        // allocate a new buffer
        //

        if( TargetName->Length < NameLength)
        {

            pTmpBuffer = (WCHAR *)AFSExAllocatePoolWithTag( PagedPool,
                                                            NameLength,
                                                            AFS_NAME_BUFFER_FIVE_TAG);

            if( pTmpBuffer == NULL)
            {

                try_return( ntStatus = STATUS_INSUFFICIENT_RESOURCES);
            }

            if( BooleanFlagOn( *Flags, AFS_DIR_RELEASE_TARGET_NAME_BUFFER))
            {

                AFSExFreePool( TargetName->Buffer);
            }

            TargetName->MaximumLength = NameLength;

            TargetName->Buffer = pTmpBuffer;

            SetFlag( *Flags, AFS_DIR_RELEASE_TARGET_NAME_BUFFER);
        }

        TargetName->Length = NameLength;

        RtlCopyMemory( TargetName->Buffer,
                       NameBuffer,
                       TargetName->Length);

        //
        // Update the name in the buffer
        //

        AFSUpdateName( TargetName);

try_exit:

        NOTHING;
    }

    return ntStatus;
}

AFSNameArrayHdr *
AFSInitNameArray( IN AFSDirectoryCB *DirectoryCB,
                  IN ULONG InitialElementCount)
{

    AFSNameArrayHdr *pNameArray = NULL;
    AFSDeviceExt *pDevExt = (AFSDeviceExt *) AFSRDRDeviceObject->DeviceExtension;

    __Enter
    {

        if( InitialElementCount == 0)
        {

            InitialElementCount = pDevExt->Specific.RDR.NameArrayLength;
        }

        pNameArray = (AFSNameArrayHdr *)AFSExAllocatePoolWithTag( PagedPool,
                                                                  sizeof( AFSNameArrayHdr) +
                                                                    (InitialElementCount * sizeof( AFSNameArrayCB)),
                                                                  AFS_NAME_ARRAY_TAG);

        if( pNameArray == NULL)
        {

            AFSDbgLogMsg( AFS_SUBSYSTEM_FILE_PROCESSING,
                          AFS_TRACE_LEVEL_ERROR,
                          "AFSInitNameArray Failed to allocate name array\n");

            try_return( pNameArray);
        }

        RtlZeroMemory( pNameArray,
                       sizeof( AFSNameArrayHdr) +
                          (InitialElementCount * sizeof( AFSNameArrayCB)));

        pNameArray->MaxElementCount = InitialElementCount;

        if( DirectoryCB != NULL)
        {

            pNameArray->CurrentEntry = &pNameArray->ElementArray[ 0];

            InterlockedIncrement( &pNameArray->Count);

            InterlockedIncrement( &DirectoryCB->OpenReferenceCount);

            AFSDbgLogMsg( AFS_SUBSYSTEM_DIRENTRY_REF_COUNTING,
                          AFS_TRACE_LEVEL_VERBOSE,
                          "AFSInitNameArray Increment count on %wZ DE %p Cnt %d\n",
                          &DirectoryCB->NameInformation.FileName,
                          DirectoryCB,
                          DirectoryCB->OpenReferenceCount);

            pNameArray->CurrentEntry->DirectoryCB = DirectoryCB;

            pNameArray->CurrentEntry->Component = DirectoryCB->NameInformation.FileName;

            pNameArray->CurrentEntry->FileId = DirectoryCB->ObjectInformation->FileId;
        }

try_exit:

        NOTHING;
    }

    return pNameArray;
}

NTSTATUS
AFSPopulateNameArray( IN AFSNameArrayHdr *NameArray,
                      IN UNICODE_STRING *Path,
                      IN AFSDirectoryCB *DirectoryCB)
{

    NTSTATUS ntStatus = STATUS_SUCCESS;
    AFSNameArrayCB *pCurrentElement = NULL;
    UNICODE_STRING uniComponentName, uniRemainingPath;
    AFSObjectInfoCB *pCurrentObject = NULL;
    ULONG  ulTotalCount = 0;
    ULONG ulIndex = 0;
    USHORT usLength = 0;

    __Enter
    {

        //
        // Init some info in the header
        //

        pCurrentElement = &NameArray->ElementArray[ 0];

        NameArray->CurrentEntry = pCurrentElement;

        //
        // The first entry points at the root
        //

        pCurrentElement->DirectoryCB = DirectoryCB->ObjectInformation->VolumeCB->DirectoryCB;

        InterlockedIncrement( &pCurrentElement->DirectoryCB->OpenReferenceCount);

        AFSDbgLogMsg( AFS_SUBSYSTEM_DIRENTRY_REF_COUNTING,
                      AFS_TRACE_LEVEL_VERBOSE,
                      "AFSPopulateNameArray Increment count on volume %wZ DE %p Cnt %d\n",
                      &pCurrentElement->DirectoryCB->NameInformation.FileName,
                      pCurrentElement->DirectoryCB,
                      pCurrentElement->DirectoryCB->OpenReferenceCount);

        pCurrentElement->Component = DirectoryCB->ObjectInformation->VolumeCB->DirectoryCB->NameInformation.FileName;

        pCurrentElement->FileId = DirectoryCB->ObjectInformation->VolumeCB->ObjectInformation.FileId;

        NameArray->Count = 1;

        NameArray->LinkCount = 0;

        //
        // If the root is the parent then we are done ...
        //

        if( &DirectoryCB->ObjectInformation->VolumeCB->ObjectInformation == DirectoryCB->ObjectInformation)
        {
            try_return( ntStatus);
        }

try_exit:

        NOTHING;
    }

    return ntStatus;
}

NTSTATUS
AFSPopulateNameArrayFromRelatedArray( IN AFSNameArrayHdr *NameArray,
                                      IN AFSNameArrayHdr *RelatedNameArray,
                                      IN AFSDirectoryCB *DirectoryCB)
{

    NTSTATUS ntStatus = STATUS_SUCCESS;
    AFSNameArrayCB *pCurrentElement = NULL, *pCurrentRelatedElement = NULL;
    UNICODE_STRING uniComponentName, uniRemainingPath;
    AFSObjectInfoCB *pObjectInfo = NULL;
    ULONG  ulTotalCount = 0;
    ULONG ulIndex = 0;
    USHORT usLength = 0;

    __Enter
    {

        //
        // Init some info in the header
        //

        pCurrentElement = &NameArray->ElementArray[ 0];

        pCurrentRelatedElement = &RelatedNameArray->ElementArray[ 0];

        NameArray->Count = 0;

        NameArray->LinkCount = RelatedNameArray->LinkCount;

        //
        // Populate the name array with the data from the related array
        //

        while( TRUE)
        {

            pCurrentElement->DirectoryCB = pCurrentRelatedElement->DirectoryCB;

            pCurrentElement->Component = pCurrentRelatedElement->DirectoryCB->NameInformation.FileName;

            pCurrentElement->FileId    = pCurrentElement->DirectoryCB->ObjectInformation->FileId;

            InterlockedIncrement( &pCurrentElement->DirectoryCB->OpenReferenceCount);

            AFSDbgLogMsg( AFS_SUBSYSTEM_DIRENTRY_REF_COUNTING,
                          AFS_TRACE_LEVEL_VERBOSE,
                          "AFSPopulateNameArrayFromRelatedArray Increment count on %wZ DE %p Cnt %d\n",
                          &pCurrentElement->DirectoryCB->NameInformation.FileName,
                          pCurrentElement->DirectoryCB,
                          pCurrentElement->DirectoryCB->OpenReferenceCount);

            InterlockedIncrement( &NameArray->Count);

            if( pCurrentElement->DirectoryCB == DirectoryCB ||
                NameArray->Count == RelatedNameArray->Count)
            {

                //
                // Done ...
                //

                break;
            }

            pCurrentElement++;

            pCurrentRelatedElement++;
        }

        if( NameArray->Count > 0)
        {
            NameArray->CurrentEntry = pCurrentElement;
        }
    }

    return ntStatus;
}

NTSTATUS
AFSFreeNameArray( IN AFSNameArrayHdr *NameArray)
{

    NTSTATUS ntStatus = STATUS_SUCCESS;
    AFSNameArrayCB *pCurrentElement = NULL;

    __Enter
    {

        pCurrentElement = &NameArray->ElementArray[ 0];

        while( TRUE)
        {

            if( pCurrentElement->DirectoryCB == NULL)
            {

                break;
            }

            InterlockedDecrement( &pCurrentElement->DirectoryCB->OpenReferenceCount);

            AFSDbgLogMsg( AFS_SUBSYSTEM_DIRENTRY_REF_COUNTING,
                          AFS_TRACE_LEVEL_VERBOSE,
                          "AFSFreeNameArray Decrement count on %wZ DE %p Cnt %d\n",
                          &pCurrentElement->DirectoryCB->NameInformation.FileName,
                          pCurrentElement->DirectoryCB,
                          pCurrentElement->DirectoryCB->OpenReferenceCount);

            pCurrentElement++;
        }

        AFSExFreePool( NameArray);
    }

    return ntStatus;
}

NTSTATUS
AFSInsertNextElement( IN AFSNameArrayHdr *NameArray,
                      IN AFSDirectoryCB *DirEntry)
{

    NTSTATUS ntStatus = STATUS_SUCCESS;
    AFSDeviceExt *pDevExt = (AFSDeviceExt *) AFSRDRDeviceObject->DeviceExtension;

    __Enter
    {

        if( NameArray->Count == NameArray->MaxElementCount)
        {

            try_return( ntStatus = STATUS_INSUFFICIENT_RESOURCES);
        }

        if( NameArray->CurrentEntry != NULL &&
            NameArray->CurrentEntry->DirectoryCB == DirEntry)
        {

            try_return( ntStatus);
        }

        if( NameArray->Count > 0)
        {

            NameArray->CurrentEntry++;
        }
        else
        {
            NameArray->CurrentEntry = &NameArray->ElementArray[ 0];
        }

        InterlockedIncrement( &NameArray->Count);

        InterlockedIncrement( &DirEntry->OpenReferenceCount);

        AFSDbgLogMsg( AFS_SUBSYSTEM_DIRENTRY_REF_COUNTING,
                      AFS_TRACE_LEVEL_VERBOSE,
                      "AFSInsertNextElement Increment count on %wZ DE %p Cnt %d\n",
                      &DirEntry->NameInformation.FileName,
                      DirEntry,
                      DirEntry->OpenReferenceCount);

        NameArray->CurrentEntry->DirectoryCB = DirEntry;

        NameArray->CurrentEntry->Component = DirEntry->NameInformation.FileName;

        NameArray->CurrentEntry->FileId = DirEntry->ObjectInformation->FileId;

try_exit:

        NOTHING;
    }

    return ntStatus;
}

void
AFSReplaceCurrentElement( IN AFSNameArrayHdr *NameArray,
                          IN AFSDirectoryCB *DirectoryCB)
{

    ASSERT( NameArray->CurrentEntry != NULL);

    InterlockedDecrement( &NameArray->CurrentEntry->DirectoryCB->OpenReferenceCount);

    AFSDbgLogMsg( AFS_SUBSYSTEM_DIRENTRY_REF_COUNTING,
                  AFS_TRACE_LEVEL_VERBOSE,
                  "AFSReplaceCurrentElement Decrement count on %wZ DE %p Cnt %d\n",
                  &NameArray->CurrentEntry->DirectoryCB->NameInformation.FileName,
                  NameArray->CurrentEntry->DirectoryCB,
                  NameArray->CurrentEntry->DirectoryCB->OpenReferenceCount);

    InterlockedIncrement( &DirectoryCB->OpenReferenceCount);

    AFSDbgLogMsg( AFS_SUBSYSTEM_DIRENTRY_REF_COUNTING,
                  AFS_TRACE_LEVEL_VERBOSE,
                  "AFSReplaceCurrentElement Increment count on %wZ DE %p Cnt %d\n",
                  &DirectoryCB->NameInformation.FileName,
                  DirectoryCB,
                  DirectoryCB->OpenReferenceCount);

    NameArray->CurrentEntry->DirectoryCB = DirectoryCB;

    NameArray->CurrentEntry->Component = DirectoryCB->NameInformation.FileName;

    NameArray->CurrentEntry->FileId = DirectoryCB->ObjectInformation->FileId;

    if( DirectoryCB->ObjectInformation->ParentObjectInformation == NULL)
    {

        SetFlag( NameArray->CurrentEntry->Flags, AFS_NAME_ARRAY_FLAG_ROOT_ELEMENT);
    }

    return;
}

AFSDirectoryCB *
AFSBackupEntry( IN AFSNameArrayHdr *NameArray)
{

    AFSDirectoryCB *pCurrentDirEntry = NULL;

    __Enter
    {

        if( NameArray->Count == 0)
        {
            try_return( pCurrentDirEntry);
        }

        InterlockedDecrement( &NameArray->CurrentEntry->DirectoryCB->OpenReferenceCount);

        AFSDbgLogMsg( AFS_SUBSYSTEM_DIRENTRY_REF_COUNTING,
                      AFS_TRACE_LEVEL_VERBOSE,
                      "AFSBackupEntry Decrement count on %wZ DE %p Cnt %d\n",
                      &NameArray->CurrentEntry->DirectoryCB->NameInformation.FileName,
                      NameArray->CurrentEntry->DirectoryCB,
                      NameArray->CurrentEntry->DirectoryCB->OpenReferenceCount);

        NameArray->CurrentEntry->DirectoryCB = NULL;

        if( InterlockedDecrement( &NameArray->Count) == 0)
        {
            NameArray->CurrentEntry = NULL;
        }
        else
        {
            NameArray->CurrentEntry--;
            pCurrentDirEntry = NameArray->CurrentEntry->DirectoryCB;
        }

try_exit:

        NOTHING;
    }

    return pCurrentDirEntry;
}

AFSDirectoryCB *
AFSGetParentEntry( IN AFSNameArrayHdr *NameArray)
{

    AFSDirectoryCB *pDirEntry = NULL;
    AFSNameArrayCB *pElement = NULL;

    __Enter
    {

        if( NameArray->Count == 0 ||
            NameArray->Count == 1)
        {

            try_return( pDirEntry = NULL);
        }

        pElement = &NameArray->ElementArray[ NameArray->Count - 2];

        pDirEntry = pElement->DirectoryCB;

try_exit:

        NOTHING;
    }

    return pDirEntry;
}

void
AFSResetNameArray( IN AFSNameArrayHdr *NameArray,
                   IN AFSDirectoryCB *DirEntry)
{

    AFSNameArrayCB *pCurrentElement = NULL;
    AFSDeviceExt *pDevExt = (AFSDeviceExt *) AFSRDRDeviceObject->DeviceExtension;

    __Enter
    {

        pCurrentElement = &NameArray->ElementArray[ 0];

        while( TRUE)
        {

            if( pCurrentElement->DirectoryCB == NULL)
            {

                break;
            }

            InterlockedDecrement( &pCurrentElement->DirectoryCB->OpenReferenceCount);

            AFSDbgLogMsg( AFS_SUBSYSTEM_DIRENTRY_REF_COUNTING,
                          AFS_TRACE_LEVEL_VERBOSE,
                          "AFSResetNameArray Decrement count on %wZ DE %p Cnt %d\n",
                          &pCurrentElement->DirectoryCB->NameInformation.FileName,
                          pCurrentElement->DirectoryCB,
                          pCurrentElement->DirectoryCB->OpenReferenceCount);

            pCurrentElement++;
        }

        RtlZeroMemory( NameArray,
                       sizeof( AFSNameArrayHdr) +
                          ((pDevExt->Specific.RDR.NameArrayLength - 1) * sizeof( AFSNameArrayCB)));

        NameArray->MaxElementCount = pDevExt->Specific.RDR.NameArrayLength;

        if( DirEntry != NULL)
        {

            NameArray->CurrentEntry = &NameArray->ElementArray[ 0];

            InterlockedIncrement( &NameArray->Count);

            InterlockedIncrement( &DirEntry->OpenReferenceCount);

            AFSDbgLogMsg( AFS_SUBSYSTEM_DIRENTRY_REF_COUNTING,
                          AFS_TRACE_LEVEL_VERBOSE,
                          "AFSResetNameArray Increment count on %wZ DE %p Cnt %d\n",
                          &DirEntry->NameInformation.FileName,
                          DirEntry,
                          DirEntry->OpenReferenceCount);

            NameArray->CurrentEntry->DirectoryCB = DirEntry;

            NameArray->CurrentEntry->Component = DirEntry->NameInformation.FileName;

            NameArray->CurrentEntry->FileId = DirEntry->ObjectInformation->FileId;
        }
    }

    return;
}

void
AFSDumpNameArray( IN AFSNameArrayHdr *NameArray)
{

    AFSNameArrayCB *pCurrentElement = NULL;

    pCurrentElement = &NameArray->ElementArray[ 0];

    AFSPrint("AFSDumpNameArray Start (%d)\n", NameArray->Count);

    while( pCurrentElement->DirectoryCB != NULL)
    {

        AFSPrint("FID %08lX-%08lX-%08lX-%08lX %wZ\n",
                  pCurrentElement->FileId.Cell,
                  pCurrentElement->FileId.Volume,
                  pCurrentElement->FileId.Vnode,
                  pCurrentElement->FileId.Unique,
                  &pCurrentElement->DirectoryCB->NameInformation.FileName);

        pCurrentElement++;
    }

    AFSPrint("AFSDumpNameArray End\n\n");

    return;
}

void
AFSSetEnumerationEvent( IN AFSFcb *Fcb)
{

    //
    // Depending on the type of node, set the event
    //

    switch( Fcb->Header.NodeTypeCode)
    {

        case AFS_DIRECTORY_FCB:
        {

            KeSetEvent( &Fcb->NPFcb->Specific.Directory.DirectoryEnumEvent,
                        0,
                        FALSE);

            InterlockedIncrement( &Fcb->NPFcb->Specific.Directory.DirectoryEnumCount);

            break;
        }

        case AFS_ROOT_FCB:
        case AFS_ROOT_ALL:
        {

            KeSetEvent( &Fcb->NPFcb->Specific.Directory.DirectoryEnumEvent,
                        0,
                        FALSE);

            InterlockedIncrement( &Fcb->NPFcb->Specific.Directory.DirectoryEnumCount);

            break;
        }
    }

    return;
}

void
AFSClearEnumerationEvent( IN AFSFcb *Fcb)
{

    //
    // Depending on the type of node, set the event
    //

    switch( Fcb->Header.NodeTypeCode)
    {

        case AFS_DIRECTORY_FCB:
        {

            ASSERT( Fcb->NPFcb->Specific.Directory.DirectoryEnumCount > 0);

            if( InterlockedDecrement( &Fcb->NPFcb->Specific.Directory.DirectoryEnumCount) == 0)
            {

                KeClearEvent( &Fcb->NPFcb->Specific.Directory.DirectoryEnumEvent);
            }

            break;
        }

        case AFS_ROOT_FCB:
        case AFS_ROOT_ALL:
        {

            ASSERT( Fcb->NPFcb->Specific.Directory.DirectoryEnumCount > 0);

            if( InterlockedDecrement( &Fcb->NPFcb->Specific.Directory.DirectoryEnumCount) == 0)
            {

                KeClearEvent( &Fcb->NPFcb->Specific.Directory.DirectoryEnumEvent);
            }

            break;
        }
    }

    return;
}

BOOLEAN
AFSIsEnumerationInProcess( IN AFSObjectInfoCB *ObjectInfo)
{

    BOOLEAN bIsInProcess = FALSE;

    __Enter
    {

        if( ObjectInfo->Fcb == NULL)
        {

            try_return( bIsInProcess);
        }

        //
        // Depending on the type of node, set the event
        //

        switch( ObjectInfo->Fcb->Header.NodeTypeCode)
        {

            case AFS_DIRECTORY_FCB:
            {

                if( KeReadStateEvent( &ObjectInfo->Fcb->NPFcb->Specific.Directory.DirectoryEnumEvent))
                {

                    bIsInProcess = TRUE;
                }

                break;
            }

            case AFS_ROOT_FCB:
            case AFS_ROOT_ALL:
            {

                if( KeReadStateEvent( &ObjectInfo->Fcb->NPFcb->Specific.Directory.DirectoryEnumEvent))
                {

                    bIsInProcess = TRUE;
                }

                break;
            }
        }

try_exit:

        NOTHING;
    }

    return bIsInProcess;
}

NTSTATUS
AFSVerifyVolume( IN ULONGLONG ProcessId,
                 IN AFSVolumeCB *VolumeCB)
{

    NTSTATUS ntStatus = STATUS_SUCCESS;


    return ntStatus;
}

NTSTATUS
AFSInitPIOCtlDirectoryCB( IN AFSObjectInfoCB *ObjectInfo)
{

    NTSTATUS ntStatus = STATUS_SUCCESS;
    AFSObjectInfoCB *pObjectInfoCB = NULL;
    AFSDirectoryCB *pDirNode = NULL;
    ULONG ulEntryLength = 0;
    AFSNonPagedDirectoryCB *pNonPagedDirEntry = NULL;

    __Enter
    {

        pObjectInfoCB = AFSAllocateObjectInfo( ObjectInfo,
                                               0);

        if( pObjectInfoCB == NULL)
        {

            try_return( ntStatus = STATUS_INSUFFICIENT_RESOURCES);
        }

        AFSDbgLogMsg( AFS_SUBSYSTEM_OBJECT_REF_COUNTING,
                      AFS_TRACE_LEVEL_VERBOSE,
                      "AFSInitPIOCtlDirectoryCB Initializing count (1) on object %08lX\n",
                      pObjectInfoCB);

        pObjectInfoCB->ObjectReferenceCount = 1;

        pObjectInfoCB->FileType = AFS_FILE_TYPE_PIOCTL;

        pObjectInfoCB->FileAttributes = FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM;

        ulEntryLength = sizeof( AFSDirectoryCB) +
                                     AFSPIOCtlName.Length;

        pDirNode = (AFSDirectoryCB *)AFSExAllocatePoolWithTag( PagedPool,
                                                               ulEntryLength,
                                                               AFS_DIR_ENTRY_TAG);

        if( pDirNode == NULL)
        {

            AFSDeleteObjectInfo( pObjectInfoCB);

            try_return( ntStatus = STATUS_INSUFFICIENT_RESOURCES);
        }

        pNonPagedDirEntry = (AFSNonPagedDirectoryCB *)AFSExAllocatePoolWithTag( NonPagedPool,
                                                                                sizeof( AFSNonPagedDirectoryCB),
                                                                                AFS_DIR_ENTRY_NP_TAG);

        if( pNonPagedDirEntry == NULL)
        {

            AFSExFreePool( pDirNode);

            AFSDeleteObjectInfo( pObjectInfoCB);

            try_return( ntStatus = STATUS_INSUFFICIENT_RESOURCES);
        }

        RtlZeroMemory( pDirNode,
                       ulEntryLength);

        RtlZeroMemory( pNonPagedDirEntry,
                       sizeof( AFSNonPagedDirectoryCB));

        ExInitializeResourceLite( &pNonPagedDirEntry->Lock);

        pDirNode->NonPaged = pNonPagedDirEntry;

        pDirNode->ObjectInformation = pObjectInfoCB;

        pDirNode->FileIndex = (ULONG)AFS_DIR_ENTRY_PIOCTL_INDEX;

        //
        // Set valid entry
        //

        SetFlag( pDirNode->Flags, AFS_DIR_ENTRY_VALID | AFS_DIR_ENTRY_FAKE);

        pDirNode->NameInformation.FileName.Length = AFSPIOCtlName.Length;

        pDirNode->NameInformation.FileName.MaximumLength = AFSPIOCtlName.Length;

        pDirNode->NameInformation.FileName.Buffer = (WCHAR *)((char *)pDirNode + sizeof( AFSDirectoryCB));

        RtlCopyMemory( pDirNode->NameInformation.FileName.Buffer,
                       AFSPIOCtlName.Buffer,
                       pDirNode->NameInformation.FileName.Length);

        pDirNode->CaseInsensitiveTreeEntry.HashIndex = AFSGenerateCRC( &pDirNode->NameInformation.FileName,
                                                                       TRUE);

        ObjectInfo->Specific.Directory.PIOCtlDirectoryCB = pDirNode;

try_exit:

        NOTHING;
    }

    return ntStatus;
}

NTSTATUS
AFSRetrieveFileAttributes( IN AFSDirectoryCB *ParentDirectoryCB,
                           IN AFSDirectoryCB *DirectoryCB,
                           IN UNICODE_STRING *ParentPathName,
                           IN AFSNameArrayHdr *RelatedNameArray,
                           OUT AFSFileInfoCB *FileInfo)
{

    NTSTATUS ntStatus = STATUS_SUCCESS;
    AFSDirEnumEntry *pDirEntry = NULL, *pLastDirEntry = NULL;
    UNICODE_STRING uniFullPathName;
    AFSNameArrayHdr    *pNameArray = NULL;
    AFSVolumeCB *pVolumeCB = NULL;
    AFSDirectoryCB *pDirectoryEntry = NULL, *pParentDirEntry = NULL;
    WCHAR *pwchBuffer = NULL;
    UNICODE_STRING uniComponentName, uniRemainingPath, uniParsedName;
    ULONG ulNameDifference = 0;
    GUID *pAuthGroup = NULL;

    __Enter
    {

        //
        // Retrieve a target name for the entry
        //

        AFSAcquireShared( &DirectoryCB->NonPaged->Lock,
                          TRUE);

        if( DirectoryCB->NameInformation.TargetName.Length == 0)
        {

            AFSReleaseResource( &DirectoryCB->NonPaged->Lock);

            if( ParentDirectoryCB->ObjectInformation->Fcb != NULL)
            {
                pAuthGroup = &ParentDirectoryCB->ObjectInformation->Fcb->AuthGroup;
            }
            else if( DirectoryCB->ObjectInformation->Fcb != NULL)
            {
                pAuthGroup = &DirectoryCB->ObjectInformation->Fcb->AuthGroup;
            }

            ntStatus = AFSEvaluateTargetByID( DirectoryCB->ObjectInformation,
                                              pAuthGroup,
                                              FALSE,
                                              &pDirEntry);

            if( !NT_SUCCESS( ntStatus) ||
                pDirEntry->TargetNameLength == 0)
            {

                if( pDirEntry != NULL)
                {

                    ntStatus = STATUS_ACCESS_DENIED;
                }

                try_return( ntStatus);
            }

            AFSAcquireExcl( &DirectoryCB->NonPaged->Lock,
                            TRUE);

            if( DirectoryCB->NameInformation.TargetName.Length == 0)
            {

                //
                // Update the target name
                //

                ntStatus = AFSUpdateTargetName( &DirectoryCB->NameInformation.TargetName,
                                                &DirectoryCB->Flags,
                                                (WCHAR *)((char *)pDirEntry + pDirEntry->TargetNameOffset),
                                                (USHORT)pDirEntry->TargetNameLength);

                if( !NT_SUCCESS( ntStatus))
                {

                    AFSReleaseResource( &DirectoryCB->NonPaged->Lock);

                    try_return( ntStatus);
                }
            }

            AFSConvertToShared( &DirectoryCB->NonPaged->Lock);
        }

        //
        // Need to pass the full path in for parsing.
        //

        if( AFSIsRelativeName( &DirectoryCB->NameInformation.TargetName))
        {

            uniFullPathName.Length = 0;
            uniFullPathName.MaximumLength = ParentPathName->Length +
                                                    sizeof( WCHAR) +
                                                    DirectoryCB->NameInformation.TargetName.Length;

            uniFullPathName.Buffer = (WCHAR *)AFSExAllocatePoolWithTag( PagedPool,
                                                                        uniFullPathName.MaximumLength,
                                                                        AFS_NAME_BUFFER_SIX_TAG);

            if( uniFullPathName.Buffer == NULL)
            {

                AFSReleaseResource( &DirectoryCB->NonPaged->Lock);

                try_return( ntStatus = STATUS_INSUFFICIENT_RESOURCES);
            }

            pwchBuffer = uniFullPathName.Buffer;

            RtlZeroMemory( uniFullPathName.Buffer,
                           uniFullPathName.MaximumLength);

            RtlCopyMemory( uniFullPathName.Buffer,
                           ParentPathName->Buffer,
                           ParentPathName->Length);

            uniFullPathName.Length = ParentPathName->Length;

            if( uniFullPathName.Buffer[ (uniFullPathName.Length/sizeof( WCHAR)) - 1] != L'\\' &&
                DirectoryCB->NameInformation.TargetName.Buffer[ 0] != L'\\')
            {

                uniFullPathName.Buffer[ uniFullPathName.Length/sizeof( WCHAR)] = L'\\';

                uniFullPathName.Length += sizeof( WCHAR);
            }

            RtlCopyMemory( &uniFullPathName.Buffer[ uniFullPathName.Length/sizeof( WCHAR)],
                           DirectoryCB->NameInformation.TargetName.Buffer,
                           DirectoryCB->NameInformation.TargetName.Length);

            uniFullPathName.Length += DirectoryCB->NameInformation.TargetName.Length;

            uniParsedName.Length = uniFullPathName.Length - ParentPathName->Length;
            uniParsedName.MaximumLength = uniParsedName.Length;

            uniParsedName.Buffer = &uniFullPathName.Buffer[ ParentPathName->Length/sizeof( WCHAR)];

            AFSReleaseResource( &DirectoryCB->NonPaged->Lock);

            //
            // We populate up to the current parent
            //

            if( RelatedNameArray != NULL)
            {

                pNameArray = AFSInitNameArray( NULL,
                                               RelatedNameArray->MaxElementCount);

                if( pNameArray == NULL)
                {

                    try_return( ntStatus = STATUS_INSUFFICIENT_RESOURCES);
                }

                ntStatus = AFSPopulateNameArrayFromRelatedArray( pNameArray,
                                                                 RelatedNameArray,
                                                                 ParentDirectoryCB);
            }
            else
            {

                pNameArray = AFSInitNameArray( NULL,
                                               0);

                if( pNameArray == NULL)
                {

                    try_return( ntStatus = STATUS_INSUFFICIENT_RESOURCES);
                }

                ntStatus = AFSPopulateNameArray( pNameArray,
                                                 NULL,
                                                 ParentDirectoryCB);
            }

            if( !NT_SUCCESS( ntStatus))
            {

                try_return( ntStatus);
            }

            pVolumeCB = ParentDirectoryCB->ObjectInformation->VolumeCB;

            AFSAcquireShared( pVolumeCB->VolumeLock,
                              TRUE);

            pParentDirEntry = ParentDirectoryCB;
        }
        else
        {

            uniFullPathName.Length = 0;
            uniFullPathName.MaximumLength = DirectoryCB->NameInformation.TargetName.Length;

            uniFullPathName.Buffer = (WCHAR *)AFSExAllocatePoolWithTag( PagedPool,
                                                                        uniFullPathName.MaximumLength,
                                                                        AFS_NAME_BUFFER_SEVEN_TAG);

            if( uniFullPathName.Buffer == NULL)
            {

                AFSReleaseResource( &DirectoryCB->NonPaged->Lock);

                try_return( ntStatus = STATUS_INSUFFICIENT_RESOURCES);
            }

            pwchBuffer = uniFullPathName.Buffer;

            RtlZeroMemory( uniFullPathName.Buffer,
                           uniFullPathName.MaximumLength);

            RtlCopyMemory( uniFullPathName.Buffer,
                           DirectoryCB->NameInformation.TargetName.Buffer,
                           DirectoryCB->NameInformation.TargetName.Length);

            uniFullPathName.Length = DirectoryCB->NameInformation.TargetName.Length;

            //
            // This name should begin with the \afs server so parse it off and check it
            //

            FsRtlDissectName( uniFullPathName,
                              &uniComponentName,
                              &uniRemainingPath);

            if( RtlCompareUnicodeString( &uniComponentName,
                                         &AFSServerName,
                                         TRUE) != 0)
            {

                AFSReleaseResource( &DirectoryCB->NonPaged->Lock);

                AFSDbgLogMsg( AFS_SUBSYSTEM_FILE_PROCESSING,
                              AFS_TRACE_LEVEL_ERROR,
                              "AFSRetrieveFileAttributes Name %wZ contains invalid server name\n",
                              &uniFullPathName);

                try_return( ntStatus = STATUS_OBJECT_PATH_INVALID);
            }

            uniFullPathName = uniRemainingPath;

            uniParsedName = uniFullPathName;

            ulNameDifference = (ULONG)(uniFullPathName.Length > 0 ? ((char *)uniFullPathName.Buffer - (char *)pwchBuffer) : 0);

            AFSReleaseResource( &DirectoryCB->NonPaged->Lock);

            //
            // Our name array
            //

            pNameArray = AFSInitNameArray( AFSGlobalRoot->DirectoryCB,
                                           0);

            if( pNameArray == NULL)
            {

                try_return( ntStatus = STATUS_INSUFFICIENT_RESOURCES);
            }

            pVolumeCB = AFSGlobalRoot;

            AFSAcquireShared( pVolumeCB->VolumeLock,
                              TRUE);

            pParentDirEntry = AFSGlobalRoot->DirectoryCB;
        }

        //
        // Increment the ref count on the volume and dir entry for correct processing below
        //

        InterlockedIncrement( &pVolumeCB->VolumeReferenceCount);

        AFSDbgLogMsg( AFS_SUBSYSTEM_VOLUME_REF_COUNTING,
                      AFS_TRACE_LEVEL_VERBOSE,
                      "AFSRetrieveFileAttributes Increment count on volume %08lX Cnt %d\n",
                      pVolumeCB,
                      pVolumeCB->VolumeReferenceCount);

        InterlockedIncrement( &pParentDirEntry->OpenReferenceCount);

        AFSDbgLogMsg( AFS_SUBSYSTEM_DIRENTRY_REF_COUNTING,
                      AFS_TRACE_LEVEL_VERBOSE,
                      "AFSRetrieveFileAttributes Increment count on %wZ DE %p Ccb %p Cnt %d\n",
                      &pParentDirEntry->NameInformation.FileName,
                      pParentDirEntry,
                      NULL,
                      pParentDirEntry->OpenReferenceCount);

        ntStatus = AFSLocateNameEntry( NULL,
                                       NULL,
                                       &uniFullPathName,
                                       &uniParsedName,
                                       pNameArray,
                                       AFS_LOCATE_FLAGS_NO_MP_TARGET_EVAL,
                                       &pVolumeCB,
                                       &pParentDirEntry,
                                       &pDirectoryEntry,
                                       NULL);

        if( !NT_SUCCESS( ntStatus))
        {

            //
            // The volume lock was released on failure above
            // Except for STATUS_OBJECT_NAME_NOT_FOUND
            //

            if( ntStatus == STATUS_OBJECT_NAME_NOT_FOUND)
            {

                if( pVolumeCB != NULL)
                {

                    InterlockedDecrement( &pVolumeCB->VolumeReferenceCount);

                    AFSDbgLogMsg( AFS_SUBSYSTEM_VOLUME_REF_COUNTING,
                                  AFS_TRACE_LEVEL_VERBOSE,
                                  "AFSRetrieveFileAttributes Decrement count on volume %08lX Cnt %d\n",
                                  pVolumeCB,
                                  pVolumeCB->VolumeReferenceCount);

                    AFSReleaseResource( pVolumeCB->VolumeLock);
                }

                if( pDirectoryEntry != NULL)
                {

                    InterlockedDecrement( &pDirectoryEntry->OpenReferenceCount);

                    AFSDbgLogMsg( AFS_SUBSYSTEM_DIRENTRY_REF_COUNTING,
                                  AFS_TRACE_LEVEL_VERBOSE,
                                  "AFSRetrieveFileAttributes Decrement1 count on %wZ DE %p Ccb %p Cnt %d\n",
                                  &pDirectoryEntry->NameInformation.FileName,
                                  pDirectoryEntry,
                                  NULL,
                                  pDirectoryEntry->OpenReferenceCount);
                }
                else
                {

                    InterlockedDecrement( &pParentDirEntry->OpenReferenceCount);

                    AFSDbgLogMsg( AFS_SUBSYSTEM_DIRENTRY_REF_COUNTING,
                                  AFS_TRACE_LEVEL_VERBOSE,
                                  "AFSRetrieveFileAttributes Decrement2 count on %wZ DE %p Ccb %p Cnt %d\n",
                                  &pParentDirEntry->NameInformation.FileName,
                                  pParentDirEntry,
                                  NULL,
                                  pParentDirEntry->OpenReferenceCount);
                }
            }

            pVolumeCB = NULL;

            try_return( ntStatus);
        }

        //
        // Store off the information
        //

        FileInfo->FileAttributes = pDirectoryEntry->ObjectInformation->FileAttributes;

        //
        // Check for the mount point being returned
        //

        if( pDirectoryEntry->ObjectInformation->FileType == AFS_FILE_TYPE_MOUNTPOINT)
        {

            FileInfo->FileAttributes |= (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_REPARSE_POINT);
        }
        else if( pDirectoryEntry->ObjectInformation->FileType == AFS_FILE_TYPE_SYMLINK ||
                 pDirectoryEntry->ObjectInformation->FileType == AFS_FILE_TYPE_DFSLINK)
        {

            FileInfo->FileAttributes |= FILE_ATTRIBUTE_REPARSE_POINT;
        }

        FileInfo->AllocationSize = pDirectoryEntry->ObjectInformation->AllocationSize;

        FileInfo->EndOfFile = pDirectoryEntry->ObjectInformation->EndOfFile;

        FileInfo->CreationTime = pDirectoryEntry->ObjectInformation->CreationTime;

        FileInfo->LastAccessTime = pDirectoryEntry->ObjectInformation->LastAccessTime;

        FileInfo->LastWriteTime = pDirectoryEntry->ObjectInformation->LastWriteTime;

        FileInfo->ChangeTime = pDirectoryEntry->ObjectInformation->ChangeTime;

        //
        // Remove the reference made above
        //

        InterlockedDecrement( &pDirectoryEntry->OpenReferenceCount);

        AFSDbgLogMsg( AFS_SUBSYSTEM_DIRENTRY_REF_COUNTING,
                      AFS_TRACE_LEVEL_VERBOSE,
                      "AFSRetrieveFileAttributes Decrement3 count on %wZ DE %p Ccb %p Cnt %d\n",
                      &pDirectoryEntry->NameInformation.FileName,
                      pDirectoryEntry,
                      NULL,
                      pDirectoryEntry->OpenReferenceCount);

try_exit:

        if( pDirEntry != NULL)
        {

            AFSExFreePool( pDirEntry);
        }

        if( pVolumeCB != NULL)
        {

            InterlockedDecrement( &pVolumeCB->VolumeReferenceCount);

            AFSDbgLogMsg( AFS_SUBSYSTEM_VOLUME_REF_COUNTING,
                          AFS_TRACE_LEVEL_VERBOSE,
                          "AFSRetrieveFileAttributes Decrement2 count on volume %08lX Cnt %d\n",
                          pVolumeCB,
                          pVolumeCB->VolumeReferenceCount);

            AFSReleaseResource( pVolumeCB->VolumeLock);
        }

        if( pNameArray != NULL)
        {

            AFSFreeNameArray( pNameArray);
        }

        if( pwchBuffer != NULL)
        {

            //
            // Always free the buffer that we allocated as AFSLocateNameEntry
            // will not free it.  If uniFullPathName.Buffer was allocated by
            // AFSLocateNameEntry, then we must free that as well.
            // Check that the uniFullPathName.Buffer in the string is not the same
            // offset by the length of the server name
            //

            AFSExFreePool( pwchBuffer);

            if( uniFullPathName.Length > 0 &&
                pwchBuffer != (WCHAR *)((char *)uniFullPathName.Buffer - ulNameDifference))
            {

                AFSExFreePool( uniFullPathName.Buffer);
            }
        }
    }

    return ntStatus;
}

AFSObjectInfoCB *
AFSAllocateObjectInfo( IN AFSObjectInfoCB *ParentObjectInfo,
                       IN ULONGLONG HashIndex)
{

    NTSTATUS ntStatus = STATUS_SUCCESS;
    AFSObjectInfoCB *pObjectInfo = NULL;

    __Enter
    {

        pObjectInfo = (AFSObjectInfoCB *)AFSExAllocatePoolWithTag( PagedPool,
                                                                   sizeof( AFSObjectInfoCB),
                                                                   AFS_OBJECT_INFO_TAG);

        if( pObjectInfo == NULL)
        {

            try_return( pObjectInfo);
        }

        RtlZeroMemory( pObjectInfo,
                       sizeof( AFSObjectInfoCB));

        pObjectInfo->NonPagedInfo = (AFSNonPagedObjectInfoCB *)AFSExAllocatePoolWithTag( NonPagedPool,
                                                                                         sizeof( AFSNonPagedObjectInfoCB),
                                                                                         AFS_NP_OBJECT_INFO_TAG);

        if( pObjectInfo->NonPagedInfo == NULL)
        {

            AFSExFreePool( pObjectInfo);

            try_return( pObjectInfo = NULL);
        }

        ExInitializeResourceLite( &pObjectInfo->NonPagedInfo->DirectoryNodeHdrLock);

        pObjectInfo->Specific.Directory.DirectoryNodeHdr.TreeLock = &pObjectInfo->NonPagedInfo->DirectoryNodeHdrLock;

        pObjectInfo->VolumeCB = ParentObjectInfo->VolumeCB;

        pObjectInfo->ParentObjectInformation = ParentObjectInfo;

        if( ParentObjectInfo != NULL)
        {
            InterlockedIncrement( &ParentObjectInfo->ObjectReferenceCount);
        }

        //
        // Initialize the access time
        //

        KeQueryTickCount( &pObjectInfo->LastAccessCount);

        if( HashIndex != 0)
        {

            //
            // Insert the entry into the object tree and list
            //

            pObjectInfo->TreeEntry.HashIndex = HashIndex;

            if( ParentObjectInfo->VolumeCB->ObjectInfoTree.TreeHead == NULL)
            {

                ParentObjectInfo->VolumeCB->ObjectInfoTree.TreeHead = &pObjectInfo->TreeEntry;
            }
            else
            {

                ntStatus = AFSInsertHashEntry( ParentObjectInfo->VolumeCB->ObjectInfoTree.TreeHead,
                                               &pObjectInfo->TreeEntry);

                ASSERT( NT_SUCCESS( ntStatus));
            }

            //
            // And the object list in the volume
            //

            if( ParentObjectInfo->VolumeCB->ObjectInfoListHead == NULL)
            {

                ParentObjectInfo->VolumeCB->ObjectInfoListHead = pObjectInfo;
            }
            else
            {

                ParentObjectInfo->VolumeCB->ObjectInfoListTail->ListEntry.fLink = (void *)pObjectInfo;

                pObjectInfo->ListEntry.bLink = (void *)ParentObjectInfo->VolumeCB->ObjectInfoListTail;
            }

            ParentObjectInfo->VolumeCB->ObjectInfoListTail = pObjectInfo;

            //
            // Indicate the object is in the hash tree and linked list in the volume
            //

            SetFlag( pObjectInfo->Flags, AFS_OBJECT_INSERTED_HASH_TREE | AFS_OBJECT_INSERTED_VOLUME_LIST);
        }

try_exit:

        NOTHING;
    }

    return pObjectInfo;
}

void
AFSDeleteObjectInfo( IN AFSObjectInfoCB *ObjectInfo)
{

    BOOLEAN bAcquiredTreeLock = FALSE;

    if( !ExIsResourceAcquiredExclusiveLite( ObjectInfo->VolumeCB->ObjectInfoTree.TreeLock))
    {

        ASSERT( !ExIsResourceAcquiredLite( ObjectInfo->VolumeCB->ObjectInfoTree.TreeLock));

        AFSAcquireExcl( ObjectInfo->VolumeCB->ObjectInfoTree.TreeLock,
                        TRUE);

        bAcquiredTreeLock = TRUE;
    }

    //
    // Remove it from the tree and list if it was inserted
    //

    if( BooleanFlagOn( ObjectInfo->Flags, AFS_OBJECT_INSERTED_HASH_TREE))
    {

        AFSRemoveHashEntry( &ObjectInfo->VolumeCB->ObjectInfoTree.TreeHead,
                            &ObjectInfo->TreeEntry);
    }

    if( BooleanFlagOn( ObjectInfo->Flags, AFS_OBJECT_INSERTED_VOLUME_LIST))
    {

        if( ObjectInfo->ListEntry.fLink == NULL)
        {

            ObjectInfo->VolumeCB->ObjectInfoListTail = (AFSObjectInfoCB *)ObjectInfo->ListEntry.bLink;

            if( ObjectInfo->VolumeCB->ObjectInfoListTail != NULL)
            {

                ObjectInfo->VolumeCB->ObjectInfoListTail->ListEntry.fLink = NULL;
            }
        }
        else
        {

            ((AFSObjectInfoCB *)(ObjectInfo->ListEntry.fLink))->ListEntry.bLink = ObjectInfo->ListEntry.bLink;
        }

        if( ObjectInfo->ListEntry.bLink == NULL)
        {

            ObjectInfo->VolumeCB->ObjectInfoListHead = (AFSObjectInfoCB *)ObjectInfo->ListEntry.fLink;

            if( ObjectInfo->VolumeCB->ObjectInfoListHead != NULL)
            {

                ObjectInfo->VolumeCB->ObjectInfoListHead->ListEntry.bLink = NULL;
            }
        }
        else
        {

            ((AFSObjectInfoCB *)(ObjectInfo->ListEntry.bLink))->ListEntry.fLink = ObjectInfo->ListEntry.fLink;
        }
    }

    if( ObjectInfo->ParentObjectInformation != NULL)
    {
        InterlockedDecrement( &ObjectInfo->ParentObjectInformation->ObjectReferenceCount);
    }

    if( bAcquiredTreeLock)
    {

        AFSReleaseResource( ObjectInfo->VolumeCB->ObjectInfoTree.TreeLock);
    }

    //
    // Release the fid in the service
    //

    if( BooleanFlagOn( ObjectInfo->Flags, AFS_OBJECT_HELD_IN_SERVICE))
    {

        AFSReleaseFid( &ObjectInfo->FileId);
    }

    ExDeleteResourceLite( &ObjectInfo->NonPagedInfo->DirectoryNodeHdrLock);

    AFSExFreePool( ObjectInfo->NonPagedInfo);

    AFSExFreePool( ObjectInfo);

    return;
}

NTSTATUS
AFSEvaluateRootEntry( IN AFSDirectoryCB *DirectoryCB,
                      OUT AFSDirectoryCB **TargetDirEntry)
{

    NTSTATUS ntStatus = STATUS_SUCCESS;
    AFSDirEnumEntry *pDirEntry = NULL, *pLastDirEntry = NULL;
    UNICODE_STRING uniFullPathName;
    AFSNameArrayHdr    *pNameArray = NULL;
    AFSVolumeCB *pVolumeCB = NULL;
    AFSDirectoryCB *pDirectoryEntry = NULL, *pParentDirEntry = NULL;
    WCHAR *pwchBuffer = NULL;
    UNICODE_STRING uniComponentName, uniRemainingPath, uniParsedName;
    ULONG ulNameDifference = 0;
    GUID *pAuthGroup = NULL;

    __Enter
    {

        //
        // Retrieve a target name for the entry
        //

        AFSAcquireShared( &DirectoryCB->NonPaged->Lock,
                          TRUE);

        if( DirectoryCB->NameInformation.TargetName.Length == 0)
        {

            AFSReleaseResource( &DirectoryCB->NonPaged->Lock);

            if( DirectoryCB->ObjectInformation->Fcb != NULL)
            {
                pAuthGroup = &DirectoryCB->ObjectInformation->Fcb->AuthGroup;
            }

            ntStatus = AFSEvaluateTargetByID( DirectoryCB->ObjectInformation,
                                              pAuthGroup,
                                              FALSE,
                                              &pDirEntry);

            if( !NT_SUCCESS( ntStatus) ||
                pDirEntry->TargetNameLength == 0)
            {

                if( pDirEntry != NULL)
                {

                    ntStatus = STATUS_ACCESS_DENIED;
                }

                try_return( ntStatus);
            }

            AFSAcquireExcl( &DirectoryCB->NonPaged->Lock,
                            TRUE);

            if( DirectoryCB->NameInformation.TargetName.Length == 0)
            {

                //
                // Update the target name
                //

                ntStatus = AFSUpdateTargetName( &DirectoryCB->NameInformation.TargetName,
                                                &DirectoryCB->Flags,
                                                (WCHAR *)((char *)pDirEntry + pDirEntry->TargetNameOffset),
                                                (USHORT)pDirEntry->TargetNameLength);

                if( !NT_SUCCESS( ntStatus))
                {

                    AFSReleaseResource( &DirectoryCB->NonPaged->Lock);

                    try_return( ntStatus);
                }
            }

            AFSConvertToShared( &DirectoryCB->NonPaged->Lock);
        }

        //
        // Need to pass the full path in for parsing.
        //

        uniFullPathName.Length = 0;
        uniFullPathName.MaximumLength = DirectoryCB->NameInformation.TargetName.Length;

        uniFullPathName.Buffer = (WCHAR *)AFSExAllocatePoolWithTag( PagedPool,
                                                                    uniFullPathName.MaximumLength,
                                                                    AFS_NAME_BUFFER_EIGHT_TAG);

        if( uniFullPathName.Buffer == NULL)
        {

            AFSReleaseResource( &DirectoryCB->NonPaged->Lock);

            try_return( ntStatus = STATUS_INSUFFICIENT_RESOURCES);
        }

        pwchBuffer = uniFullPathName.Buffer;

        RtlZeroMemory( uniFullPathName.Buffer,
                       uniFullPathName.MaximumLength);

        RtlCopyMemory( uniFullPathName.Buffer,
                       DirectoryCB->NameInformation.TargetName.Buffer,
                       DirectoryCB->NameInformation.TargetName.Length);

        uniFullPathName.Length = DirectoryCB->NameInformation.TargetName.Length;

        //
        // This name should begin with the \afs server so parse it off and chech it
        //

        FsRtlDissectName( uniFullPathName,
                          &uniComponentName,
                          &uniRemainingPath);

        if( RtlCompareUnicodeString( &uniComponentName,
                                     &AFSServerName,
                                     TRUE) != 0)
        {

            //
            // Try evaluating the full path
            //

            uniFullPathName.Buffer = pwchBuffer;

            uniFullPathName.Length = DirectoryCB->NameInformation.TargetName.Length;

            uniFullPathName.MaximumLength = uniFullPathName.Length;
        }
        else
        {

            uniFullPathName = uniRemainingPath;
        }

        uniParsedName = uniFullPathName;

        ulNameDifference = (ULONG)(uniFullPathName.Length > 0 ? ((char *)uniFullPathName.Buffer - (char *)pwchBuffer) : 0);

        AFSReleaseResource( &DirectoryCB->NonPaged->Lock);

        //
        // Our name array
        //

        pNameArray = AFSInitNameArray( AFSGlobalRoot->DirectoryCB,
                                       0);

        if( pNameArray == NULL)
        {

            try_return( ntStatus = STATUS_INSUFFICIENT_RESOURCES);
        }

        pVolumeCB = AFSGlobalRoot;

        AFSAcquireShared( pVolumeCB->VolumeLock,
                          TRUE);

        pParentDirEntry = AFSGlobalRoot->DirectoryCB;

        InterlockedIncrement( &pVolumeCB->VolumeReferenceCount);

        AFSDbgLogMsg( AFS_SUBSYSTEM_VOLUME_REF_COUNTING,
                      AFS_TRACE_LEVEL_VERBOSE,
                      "AFSEvaluateRootEntry Increment count on volume %08lX Cnt %d\n",
                      pVolumeCB,
                      pVolumeCB->VolumeReferenceCount);

        InterlockedIncrement( &pParentDirEntry->OpenReferenceCount);

        AFSDbgLogMsg( AFS_SUBSYSTEM_DIRENTRY_REF_COUNTING,
                      AFS_TRACE_LEVEL_VERBOSE,
                      "AFSEvaluateRootEntry Increment count on %wZ DE %p Ccb %p Cnt %d\n",
                      &pParentDirEntry->NameInformation.FileName,
                      pParentDirEntry,
                      NULL,
                      pParentDirEntry->OpenReferenceCount);

        ntStatus = AFSLocateNameEntry( NULL,
                                       NULL,
                                       &uniFullPathName,
                                       &uniParsedName,
                                       pNameArray,
                                       0,
                                       &pVolumeCB,
                                       &pParentDirEntry,
                                       &pDirectoryEntry,
                                       NULL);

        if( !NT_SUCCESS( ntStatus))
        {

            //
            // The volume lock was released on failure above
            // Except for STATUS_OBJECT_NAME_NOT_FOUND
            //

            if( ntStatus == STATUS_OBJECT_NAME_NOT_FOUND)
            {

                if( pVolumeCB != NULL)
                {

                    InterlockedDecrement( &pVolumeCB->VolumeReferenceCount);

                    AFSDbgLogMsg( AFS_SUBSYSTEM_VOLUME_REF_COUNTING,
                                  AFS_TRACE_LEVEL_VERBOSE,
                                  "AFSEvaluateRootEntry Decrement count on volume %08lX Cnt %d\n",
                                  pVolumeCB,
                                  pVolumeCB->VolumeReferenceCount);

                    AFSReleaseResource( pVolumeCB->VolumeLock);
                }

                if( pDirectoryEntry != NULL)
                {

                    InterlockedDecrement( &pDirectoryEntry->OpenReferenceCount);

                    AFSDbgLogMsg( AFS_SUBSYSTEM_DIRENTRY_REF_COUNTING,
                                  AFS_TRACE_LEVEL_VERBOSE,
                                  "AFSEvaluateRootEntry Decrement1 count on %wZ DE %p Ccb %p Cnt %d\n",
                                  &pDirectoryEntry->NameInformation.FileName,
                                  pDirectoryEntry,
                                  NULL,
                                  pDirectoryEntry->OpenReferenceCount);
                }
                else
                {

                    InterlockedDecrement( &pParentDirEntry->OpenReferenceCount);

                    AFSDbgLogMsg( AFS_SUBSYSTEM_DIRENTRY_REF_COUNTING,
                                  AFS_TRACE_LEVEL_VERBOSE,
                                  "AFSEvaluateRootEntry Decrement1 count on %wZ DE %p Ccb %p Cnt %d\n",
                                  &pParentDirEntry->NameInformation.FileName,
                                  pParentDirEntry,
                                  NULL,
                                  pParentDirEntry->OpenReferenceCount);
                }
            }

            pVolumeCB = NULL;

            try_return( ntStatus);
        }

        //
        // Pass back the target dir entry for this request
        //

        *TargetDirEntry = pDirectoryEntry;

try_exit:

        if( pDirEntry != NULL)
        {

            AFSExFreePool( pDirEntry);
        }

        if( pVolumeCB != NULL)
        {

            InterlockedDecrement( &pVolumeCB->VolumeReferenceCount);

            AFSDbgLogMsg( AFS_SUBSYSTEM_VOLUME_REF_COUNTING,
                          AFS_TRACE_LEVEL_VERBOSE,
                          "AFSEvaluateRootEntry2 Decrement count on volume %08lX Cnt %d\n",
                          pVolumeCB,
                          pVolumeCB->VolumeReferenceCount);

            AFSReleaseResource( pVolumeCB->VolumeLock);
        }

        if( pNameArray != NULL)
        {

            AFSFreeNameArray( pNameArray);
        }

        if( pwchBuffer != NULL)
        {

            //
            // Always free the buffer that we allocated as AFSLocateNameEntry
            // will not free it.  If uniFullPathName.Buffer was allocated by
            // AFSLocateNameEntry, then we must free that as well.
            // Check that the uniFullPathName.Buffer in the string is not the same
            // offset by the length of the server name
            //

            AFSExFreePool( pwchBuffer);

            if( uniFullPathName.Length > 0 &&
                pwchBuffer != (WCHAR *)((char *)uniFullPathName.Buffer - ulNameDifference))
            {

                AFSExFreePool( uniFullPathName.Buffer);
            }
        }
    }

    return ntStatus;
}

NTSTATUS
AFSCleanupFcb( IN AFSFcb *Fcb,
               IN BOOLEAN ForceFlush)
{

    NTSTATUS ntStatus = STATUS_SUCCESS;
    AFSDeviceExt *pRDRDeviceExt = NULL, *pControlDeviceExt = NULL;
    LARGE_INTEGER liTime;
    IO_STATUS_BLOCK stIoStatus;

    __Enter
    {

        pControlDeviceExt = (AFSDeviceExt *)AFSControlDeviceObject->DeviceExtension;

        pRDRDeviceExt = (AFSDeviceExt *)AFSRDRDeviceObject->DeviceExtension;

        if( BooleanFlagOn( pRDRDeviceExt->DeviceFlags, AFS_DEVICE_FLAG_REDIRECTOR_SHUTDOWN))
        {

            if( !BooleanFlagOn( Fcb->ObjectInformation->Flags, AFS_OBJECT_FLAGS_OBJECT_INVALID) &&
                !BooleanFlagOn( Fcb->ObjectInformation->Flags, AFS_OBJECT_FLAGS_DELETED))
            {

                AFSAcquireExcl( &Fcb->NPFcb->Resource,
                                TRUE);

                if( Fcb->OpenReferenceCount > 0)
                {

                    __try
                    {

                        CcFlushCache( &Fcb->NPFcb->SectionObjectPointers,
                                      NULL,
                                      0,
                                      &stIoStatus);

                        if( !NT_SUCCESS( stIoStatus.Status))
                        {

                            AFSDbgLogMsg( AFS_SUBSYSTEM_IO_PROCESSING,
                                          AFS_TRACE_LEVEL_ERROR,
                                          "AFSCleanupFcb CcFlushCache [1] failure FID %08lX-%08lX-%08lX-%08lX Status 0x%08lX Bytes 0x%08lX\n",
                                          Fcb->ObjectInformation->FileId.Cell,
                                          Fcb->ObjectInformation->FileId.Volume,
                                          Fcb->ObjectInformation->FileId.Vnode,
                                          Fcb->ObjectInformation->FileId.Unique,
                                          stIoStatus.Status,
                                          stIoStatus.Information);

                            ntStatus = stIoStatus.Status;
                        }

                        CcPurgeCacheSection( &Fcb->NPFcb->SectionObjectPointers,
                                             NULL,
                                             0,
                                             FALSE);
                    }
                    __except( EXCEPTION_EXECUTE_HANDLER)
                    {
                        ntStatus = GetExceptionCode();
                    }
                }

                AFSReleaseResource( &Fcb->NPFcb->Resource);

                //
                // Wait for any currently running flush or release requests to complete
                //

                AFSWaitOnQueuedFlushes( Fcb);

                //
                // Now perform another flush on the file
                //

                if( !NT_SUCCESS( AFSFlushExtents( Fcb)))
                {

                    AFSReleaseExtentsWithFlush( Fcb);
                }
            }

            if( Fcb->OpenReferenceCount == 0 ||
                BooleanFlagOn( Fcb->ObjectInformation->Flags, AFS_OBJECT_FLAGS_OBJECT_INVALID) ||
                BooleanFlagOn( Fcb->ObjectInformation->Flags, AFS_OBJECT_FLAGS_DELETED))
            {

                AFSTearDownFcbExtents( Fcb);
            }

            try_return( ntStatus);
        }

        KeQueryTickCount( &liTime);

        //
        // First up are there dirty extents in the cache to flush?
        //

        if( ForceFlush ||
            ( !BooleanFlagOn( Fcb->ObjectInformation->Flags, AFS_OBJECT_FLAGS_OBJECT_INVALID) &&
              !BooleanFlagOn( Fcb->ObjectInformation->Flags, AFS_OBJECT_FLAGS_DELETED) &&
              ( Fcb->Specific.File.ExtentsDirtyCount ||
                Fcb->Specific.File.ExtentCount) &&
              (liTime.QuadPart - Fcb->Specific.File.LastServerFlush.QuadPart)
                                                    >= pControlDeviceExt->Specific.Control.FcbFlushTimeCount.QuadPart))
        {

            if( !NT_SUCCESS( AFSFlushExtents( Fcb)) &&
                Fcb->OpenReferenceCount == 0)
            {

                AFSReleaseExtentsWithFlush( Fcb);
            }
        }
        else if( BooleanFlagOn( Fcb->ObjectInformation->Flags, AFS_OBJECT_FLAGS_OBJECT_INVALID) ||
                 BooleanFlagOn( Fcb->ObjectInformation->Flags, AFS_OBJECT_FLAGS_DELETED))
        {

            //
            // The file has been marked as invalid.  Dump it
            //

            AFSTearDownFcbExtents( Fcb);
        }

        //
        // If there are extents and they haven't been used recently *and*
        // are not being used
        //

        if( ( ForceFlush ||
              ( 0 != Fcb->Specific.File.ExtentCount &&
                0 != Fcb->Specific.File.LastExtentAccess.QuadPart &&
                (liTime.QuadPart - Fcb->Specific.File.LastExtentAccess.QuadPart) >=
                                        (AFS_SERVER_PURGE_SLEEP * pControlDeviceExt->Specific.Control.FcbPurgeTimeCount.QuadPart))) &&
            AFSAcquireExcl( &Fcb->NPFcb->Resource,
                            ForceFlush))
        {

            __try
            {

                CcFlushCache( &Fcb->NPFcb->SectionObjectPointers,
                              NULL,
                              0,
                              &stIoStatus);

                if( !NT_SUCCESS( stIoStatus.Status))
                {

                    AFSDbgLogMsg( AFS_SUBSYSTEM_IO_PROCESSING,
                                  AFS_TRACE_LEVEL_ERROR,
                                  "AFSCleanupFcb CcFlushCache [2] failure FID %08lX-%08lX-%08lX-%08lX Status 0x%08lX Bytes 0x%08lX\n",
                                  Fcb->ObjectInformation->FileId.Cell,
                                  Fcb->ObjectInformation->FileId.Volume,
                                  Fcb->ObjectInformation->FileId.Vnode,
                                  Fcb->ObjectInformation->FileId.Unique,
                                  stIoStatus.Status,
                                  stIoStatus.Information);

                    ntStatus = stIoStatus.Status;
                }

                if( ForceFlush)
                {

                    CcPurgeCacheSection( &Fcb->NPFcb->SectionObjectPointers,
                                         NULL,
                                         0,
                                         FALSE);
                }
            }
            __except( EXCEPTION_EXECUTE_HANDLER)
            {
                ntStatus = GetExceptionCode();
            }

            AFSReleaseResource( &Fcb->NPFcb->Resource);

            if( Fcb->OpenReferenceCount == 0)
            {

                //
                // Tear em down we'll not be needing them again
                //

                AFSTearDownFcbExtents( Fcb);
            }
        }

try_exit:

        NOTHING;
    }

    return ntStatus;
}

NTSTATUS
AFSUpdateDirEntryName( IN AFSDirectoryCB *DirectoryCB,
                       IN UNICODE_STRING *NewFileName)
{

    NTSTATUS ntStatus = STATUS_SUCCESS;
    WCHAR *pTmpBuffer = NULL;

    __Enter
    {

        if( NewFileName->Length > DirectoryCB->NameInformation.FileName.Length)
        {

            if( BooleanFlagOn( DirectoryCB->Flags, AFS_DIR_RELEASE_NAME_BUFFER))
            {

                AFSExFreePool( DirectoryCB->NameInformation.FileName.Buffer);

                ClearFlag( DirectoryCB->Flags, AFS_DIR_RELEASE_NAME_BUFFER);

                DirectoryCB->NameInformation.FileName.Buffer = NULL;
            }

            //
            // OK, we need to allocate a new name buffer
            //

            pTmpBuffer = (WCHAR *)AFSExAllocatePoolWithTag( PagedPool,
                                                            NewFileName->Length,
                                                            AFS_NAME_BUFFER_NINE_TAG);

            if( pTmpBuffer == NULL)
            {

                try_return( ntStatus = STATUS_INSUFFICIENT_RESOURCES);
            }

            DirectoryCB->NameInformation.FileName.Buffer = pTmpBuffer;

            DirectoryCB->NameInformation.FileName.MaximumLength = NewFileName->Length;

            SetFlag( DirectoryCB->Flags, AFS_DIR_RELEASE_NAME_BUFFER);
        }

        DirectoryCB->NameInformation.FileName.Length = NewFileName->Length;

        RtlCopyMemory( DirectoryCB->NameInformation.FileName.Buffer,
                       NewFileName->Buffer,
                       NewFileName->Length);

try_exit:

        NOTHING;
    }

    return ntStatus;
}

NTSTATUS
AFSReadCacheFile( IN void *ReadBuffer,
                  IN LARGE_INTEGER *ReadOffset,
                  IN ULONG RequestedDataLength,
                  IN OUT PULONG BytesRead)
{

    NTSTATUS            ntStatus = STATUS_SUCCESS;
    PIRP                pIrp = NULL;
    KEVENT              kEvent;
    PIO_STACK_LOCATION  pIoStackLocation = NULL;
    AFSDeviceExt       *pRdrDevExt = (AFSDeviceExt *)AFSRDRDeviceObject->DeviceExtension;
    DEVICE_OBJECT      *pTargetDeviceObject = NULL;
    FILE_OBJECT        *pCacheFileObject = NULL;

    __Enter
    {

        pCacheFileObject = AFSReferenceCacheFileObject();

        if( pCacheFileObject == NULL)
        {
            try_return( ntStatus = STATUS_DEVICE_NOT_READY);
        }

        pTargetDeviceObject = IoGetRelatedDeviceObject( pCacheFileObject);

        //
        // Initialize the event
        //

        KeInitializeEvent( &kEvent,
                           SynchronizationEvent,
                           FALSE);

        //
        // Allocate an irp for this request.  This could also come from a
        // private pool, for instance.
        //

        pIrp = IoAllocateIrp( pTargetDeviceObject->StackSize,
                              FALSE);

        if( pIrp == NULL)
        {

            try_return( ntStatus = STATUS_INSUFFICIENT_RESOURCES);
        }

        //
        // Build the IRP's main body
        //

        pIrp->UserBuffer = ReadBuffer;

        pIrp->Tail.Overlay.Thread = PsGetCurrentThread();
        pIrp->RequestorMode = KernelMode;
        pIrp->Flags |= IRP_READ_OPERATION;

        //
        // Set up the I/O stack location.
        //

        pIoStackLocation = IoGetNextIrpStackLocation( pIrp);
        pIoStackLocation->MajorFunction = IRP_MJ_READ;
        pIoStackLocation->DeviceObject = pTargetDeviceObject;
        pIoStackLocation->FileObject = pCacheFileObject;
        pIoStackLocation->Parameters.Read.Length = RequestedDataLength;

        pIoStackLocation->Parameters.Read.ByteOffset.QuadPart = ReadOffset->QuadPart;

        //
        // Set the completion routine.
        //

        IoSetCompletionRoutine( pIrp,
                                AFSIrpComplete,
                                &kEvent,
                                TRUE,
                                TRUE,
                                TRUE);

        //
        // Send it to the FSD
        //

        ntStatus = IoCallDriver( pTargetDeviceObject,
                                 pIrp);

        if( NT_SUCCESS( ntStatus))
        {

            //
            // Wait for the I/O
            //

            ntStatus = KeWaitForSingleObject( &kEvent,
                                              Executive,
                                              KernelMode,
                                              FALSE,
                                              0);

            if( NT_SUCCESS( ntStatus))
            {

                ntStatus = pIrp->IoStatus.Status;

                *BytesRead = (ULONG)pIrp->IoStatus.Information;
            }
        }

try_exit:

        if( pCacheFileObject != NULL)
        {
            AFSReleaseCacheFileObject( pCacheFileObject);
        }

        if( pIrp != NULL)
        {

            if( pIrp->MdlAddress != NULL)
            {

                if( FlagOn( pIrp->MdlAddress->MdlFlags, MDL_PAGES_LOCKED))
                {

                    MmUnlockPages( pIrp->MdlAddress);
                }

                IoFreeMdl( pIrp->MdlAddress);
            }

            pIrp->MdlAddress = NULL;

            //
            // Free the Irp
            //

            IoFreeIrp( pIrp);
        }
    }

    return ntStatus;
}

NTSTATUS
AFSIrpComplete( IN PDEVICE_OBJECT DeviceObject,
                IN PIRP           Irp,
                IN PVOID          Context)
{

    KEVENT *pEvent = (KEVENT *)Context;

    KeSetEvent( pEvent,
                0,
                FALSE);

    return STATUS_MORE_PROCESSING_REQUIRED;
}

BOOLEAN
AFSIsDirectoryEmptyForDelete( IN AFSFcb *Fcb)
{

    BOOLEAN bIsEmpty = FALSE;
    AFSDirectoryCB *pDirEntry = NULL;

    __Enter
    {

        AFSAcquireShared( Fcb->ObjectInformation->Specific.Directory.DirectoryNodeHdr.TreeLock,
                          TRUE);

        bIsEmpty = TRUE;

        if( Fcb->ObjectInformation->Specific.Directory.DirectoryNodeListHead != NULL)
        {

            pDirEntry = Fcb->ObjectInformation->Specific.Directory.DirectoryNodeListHead;

            while( pDirEntry != NULL)
            {

                if( !BooleanFlagOn( pDirEntry->Flags, AFS_DIR_ENTRY_FAKE) &&
                    !BooleanFlagOn( pDirEntry->Flags, AFS_DIR_ENTRY_DELETED))
                {

                    bIsEmpty = FALSE;

                    break;
                }

                pDirEntry = (AFSDirectoryCB *)pDirEntry->ListEntry.fLink;
            }

        }

        AFSReleaseResource( Fcb->ObjectInformation->Specific.Directory.DirectoryNodeHdr.TreeLock);
    }

    return bIsEmpty;
}

void
AFSRemoveNameEntry( IN AFSObjectInfoCB *ParentObjectInfo,
                    IN AFSDirectoryCB *DirEntry)
{

    NTSTATUS ntStatus = STATUS_SUCCESS;

    __Enter
    {

        if( BooleanFlagOn( DirEntry->Flags, AFS_DIR_ENTRY_NOT_IN_PARENT_TREE))
        {

            try_return( ntStatus);
        }

        ASSERT( ExIsResourceAcquiredExclusiveLite( ParentObjectInfo->Specific.Directory.DirectoryNodeHdr.TreeLock));

        //
        // Remove the entry from the parent tree
        //

        AFSRemoveCaseSensitiveDirEntry( &ParentObjectInfo->Specific.Directory.DirectoryNodeHdr.CaseSensitiveTreeHead,
                                        DirEntry);

        AFSRemoveCaseInsensitiveDirEntry( &ParentObjectInfo->Specific.Directory.DirectoryNodeHdr.CaseInsensitiveTreeHead,
                                          DirEntry);

        if( ParentObjectInfo->Specific.Directory.ShortNameTree &&
            DirEntry->Type.Data.ShortNameTreeEntry.HashIndex != 0)
        {

            //
            // From the short name tree
            //

            AFSRemoveShortNameDirEntry( &ParentObjectInfo->Specific.Directory.ShortNameTree,
                                        DirEntry);
        }

        SetFlag( DirEntry->Flags, AFS_DIR_ENTRY_NOT_IN_PARENT_TREE);

try_exit:

        NOTHING;
    }

    return;
}

LARGE_INTEGER
AFSGetAuthenticationId()
{

    LARGE_INTEGER liAuthId = {0,0};
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PACCESS_TOKEN hToken = NULL;
    PTOKEN_STATISTICS pTokenInfo = NULL;
    BOOLEAN bCopyOnOpen = FALSE;
    BOOLEAN bEffectiveOnly = FALSE;
    BOOLEAN bPrimaryToken = FALSE;
    SECURITY_IMPERSONATION_LEVEL stImpersonationLevel;

    __Enter
    {

        hToken = PsReferenceImpersonationToken( PsGetCurrentThread(),
                                                &bCopyOnOpen,
                                                &bEffectiveOnly,
                                                &stImpersonationLevel);

        if( hToken == NULL)
        {

            hToken = PsReferencePrimaryToken( PsGetCurrentProcess());

            if( hToken == NULL)
            {

                AFSDbgLogMsg( AFS_SUBSYSTEM_NETWORK_PROVIDER,
                              AFS_TRACE_LEVEL_ERROR,
                              "AFSGetAuthenticationId Failed to retrieve impersonation or primary token\n");

                try_return( ntStatus);
            }

            bPrimaryToken = TRUE;
        }

        ntStatus = SeQueryInformationToken( hToken,
                                            TokenStatistics,
                                            (PVOID *)&pTokenInfo);

        if( !NT_SUCCESS( ntStatus))
        {

            AFSDbgLogMsg( AFS_SUBSYSTEM_NETWORK_PROVIDER,
                          AFS_TRACE_LEVEL_ERROR,
                          "AFSGetAuthenticationId Failed to retrieve information Status %08lX\n", ntStatus);

            try_return( ntStatus);
        }

        liAuthId.HighPart = pTokenInfo->AuthenticationId.HighPart;
        liAuthId.LowPart = pTokenInfo->AuthenticationId.LowPart;

        AFSDbgLogMsg( AFS_SUBSYSTEM_NETWORK_PROVIDER,
                      AFS_TRACE_LEVEL_VERBOSE,
                      "AFSGetAuthenticationId Successfully retrieved authentication ID %I64X\n",
                      liAuthId.QuadPart);

try_exit:

        if( hToken != NULL)
        {

            if( !bPrimaryToken)
            {

                PsDereferenceImpersonationToken( hToken);
            }
            else
            {

                PsDereferencePrimaryToken( hToken);
            }
        }

        if( pTokenInfo != NULL)
        {

            AFSExFreePool( pTokenInfo);
        }
    }

    return liAuthId;
}

void
AFSUnwindFileInfo( IN AFSFcb *Fcb,
                   IN AFSCcb *Ccb)
{

    if( Ccb->FileUnwindInfo.FileAttributes != (ULONG)-1)
    {
        Ccb->DirectoryCB->ObjectInformation->FileAttributes = Ccb->FileUnwindInfo.FileAttributes;
    }

    if( Ccb->FileUnwindInfo.CreationTime.QuadPart != (ULONGLONG)-1)
    {
        Ccb->DirectoryCB->ObjectInformation->CreationTime.QuadPart = Ccb->FileUnwindInfo.CreationTime.QuadPart;
    }

    if( Ccb->FileUnwindInfo.LastAccessTime.QuadPart != (ULONGLONG)-1)
    {
        Ccb->DirectoryCB->ObjectInformation->LastAccessTime.QuadPart = Ccb->FileUnwindInfo.LastAccessTime.QuadPart;
    }

    if( Ccb->FileUnwindInfo.LastWriteTime.QuadPart != (ULONGLONG)-1)
    {
        Ccb->DirectoryCB->ObjectInformation->LastWriteTime.QuadPart = Ccb->FileUnwindInfo.LastWriteTime.QuadPart;
    }

    if( Ccb->FileUnwindInfo.ChangeTime.QuadPart != (ULONGLONG)-1)
    {
        Ccb->DirectoryCB->ObjectInformation->ChangeTime.QuadPart = Ccb->FileUnwindInfo.ChangeTime.QuadPart;
    }

    return;
}

BOOLEAN
AFSValidateDirList( IN AFSObjectInfoCB *ObjectInfo)
{

    BOOLEAN bIsValid = TRUE;
    ULONG ulCount = 0;
    AFSDirectoryCB *pCurrentDirEntry = NULL;

    pCurrentDirEntry = ObjectInfo->Specific.Directory.DirectoryNodeListHead;

    while( pCurrentDirEntry != NULL)
    {

        if( !BooleanFlagOn( pCurrentDirEntry->Flags, AFS_DIR_ENTRY_FAKE))
        {
            ulCount++;
        }

        pCurrentDirEntry = (AFSDirectoryCB *)pCurrentDirEntry->ListEntry.fLink;
    }

    if( ulCount != ObjectInfo->Specific.Directory.DirectoryNodeCount)
    {

        AFSPrint("AFSValidateDirList Count off Calc: %d Stored: %d\n",
                  ulCount,
                  ObjectInfo->Specific.Directory.DirectoryNodeCount);

        ObjectInfo->Specific.Directory.DirectoryNodeCount = ulCount;

        bIsValid = FALSE;
    }

    return bIsValid;
}

PFILE_OBJECT
AFSReferenceCacheFileObject()
{

    AFSDeviceExt       *pRdrDevExt = (AFSDeviceExt *)AFSRDRDeviceObject->DeviceExtension;
    FILE_OBJECT        *pCacheFileObject = NULL;

    AFSAcquireShared( &pRdrDevExt->Specific.RDR.CacheFileLock,
                      TRUE);

    pCacheFileObject = pRdrDevExt->Specific.RDR.CacheFileObject;

    if( pCacheFileObject != NULL)
    {
        ObReferenceObject( pCacheFileObject);
    }

    AFSReleaseResource( &pRdrDevExt->Specific.RDR.CacheFileLock);

    return pCacheFileObject;
}

void
AFSReleaseCacheFileObject( IN PFILE_OBJECT CacheFileObject)
{

    ASSERT( CacheFileObject != NULL);

    ObDereferenceObject( CacheFileObject);

    return;
}

NTSTATUS
AFSInitializeLibrary( IN AFSLibraryInitCB *LibraryInit)
{

    NTSTATUS ntStatus = STATUS_SUCCESS;
    AFSDeviceExt *pControlDevExt = NULL;
    ULONG ulTimeIncrement = 0;

    __Enter
    {

        AFSControlDeviceObject = LibraryInit->AFSControlDeviceObject;

        AFSRDRDeviceObject = LibraryInit->AFSRDRDeviceObject;

        AFSServerName = LibraryInit->AFSServerName;

        AFSDebugFlags = LibraryInit->AFSDebugFlags;

        //
        // Callbacks in the framework
        //

        AFSProcessRequest = LibraryInit->AFSProcessRequest;

        AFSDbgLogMsg = LibraryInit->AFSDbgLogMsg;

        AFSAddConnectionEx = LibraryInit->AFSAddConnectionEx;

        AFSExAllocatePoolWithTag = LibraryInit->AFSExAllocatePoolWithTag;

        AFSExFreePool = LibraryInit->AFSExFreePool;

        AFSDumpTraceFilesFnc = LibraryInit->AFSDumpTraceFiles;

        AFSRetrieveAuthGroupFnc = LibraryInit->AFSRetrieveAuthGroup;

        AFSLibCacheManagerCallbacks = LibraryInit->AFSCacheManagerCallbacks;

        if( LibraryInit->AFSCacheBaseAddress != NULL)
        {

            SetFlag( AFSLibControlFlags, AFS_REDIR_LIB_FLAGS_NONPERSISTENT_CACHE);

            AFSLibCacheBaseAddress = LibraryInit->AFSCacheBaseAddress;

            AFSLibCacheLength = LibraryInit->AFSCacheLength;
        }

        //
        // Initialize some flush parameters
        //

        pControlDevExt = (AFSDeviceExt *)AFSControlDeviceObject->DeviceExtension;

        ulTimeIncrement = KeQueryTimeIncrement();

        pControlDevExt->Specific.Control.ObjectLifeTimeCount.QuadPart = (ULONGLONG)((ULONGLONG)AFS_OBJECT_LIFETIME / (ULONGLONG)ulTimeIncrement);
        pControlDevExt->Specific.Control.FcbPurgeTimeCount.QuadPart = AFS_ONE_SECOND;
        pControlDevExt->Specific.Control.FcbPurgeTimeCount.QuadPart *= AFS_SERVER_PURGE_DELAY;
        pControlDevExt->Specific.Control.FcbPurgeTimeCount.QuadPart /= ulTimeIncrement;
        pControlDevExt->Specific.Control.FcbFlushTimeCount.QuadPart = (ULONGLONG)((ULONGLONG)(AFS_ONE_SECOND * AFS_SERVER_FLUSH_DELAY) / (ULONGLONG)ulTimeIncrement);
        pControlDevExt->Specific.Control.ExtentRequestTimeCount.QuadPart = (ULONGLONG)((ULONGLONG)AFS_EXTENT_REQUEST_TIME/(ULONGLONG)ulTimeIncrement);

        //
        // Initialize the global root entry
        //

        ntStatus = AFSInitVolume( NULL,
                                  &LibraryInit->GlobalRootFid,
                                  &AFSGlobalRoot);

        if( !NT_SUCCESS( ntStatus))
        {

            AFSDbgLogMsg( AFS_SUBSYSTEM_LOAD_LIBRARY | AFS_SUBSYSTEM_INIT_PROCESSING,
                          AFS_TRACE_LEVEL_ERROR,
                          "AFSInitializeLibrary AFSInitVolume failure %08lX\n",
                          ntStatus);

            try_return( ntStatus);
        }

        ntStatus = AFSInitRootFcb( (ULONGLONG)PsGetCurrentProcessId(),
                                   AFSGlobalRoot);

        if( !NT_SUCCESS( ntStatus))
        {

            AFSDbgLogMsg( AFS_SUBSYSTEM_LOAD_LIBRARY | AFS_SUBSYSTEM_INIT_PROCESSING,
                          AFS_TRACE_LEVEL_ERROR,
                          "AFSInitializeLibrary AFSInitRootFcb failure %08lX\n",
                          ntStatus);

            AFSReleaseResource( AFSGlobalRoot->VolumeLock);

            try_return( ntStatus);
        }

        //
        // Update the node type code to AFS_ROOT_ALL
        //

        AFSGlobalRoot->ObjectInformation.Fcb->Header.NodeTypeCode = AFS_ROOT_ALL;

        SetFlag( AFSGlobalRoot->Flags, AFS_VOLUME_ACTIVE_GLOBAL_ROOT);

        //
        // Drop the locks acquired above
        //

        AFSInitVolumeWorker( AFSGlobalRoot);

        AFSReleaseResource( AFSGlobalRoot->VolumeLock);

        AFSReleaseResource( AFSGlobalRoot->ObjectInformation.Fcb->Header.Resource);

try_exit:

        NOTHING;
    }

    return ntStatus;
}

NTSTATUS
AFSCloseLibrary()
{

    NTSTATUS ntStatus = STATUS_SUCCESS;
    AFSDirectoryCB *pDirNode = NULL, *pLastDirNode = NULL;

    __Enter
    {

        if( AFSGlobalDotDirEntry != NULL)
        {

            AFSDeleteObjectInfo( AFSGlobalDotDirEntry->ObjectInformation);

            ExDeleteResourceLite( &AFSGlobalDotDirEntry->NonPaged->Lock);

            ExFreePool( AFSGlobalDotDirEntry->NonPaged);

            ExFreePool( AFSGlobalDotDirEntry);

            AFSGlobalDotDirEntry = NULL;
        }

        if( AFSGlobalDotDotDirEntry != NULL)
        {

            AFSDeleteObjectInfo( AFSGlobalDotDotDirEntry->ObjectInformation);

            ExDeleteResourceLite( &AFSGlobalDotDotDirEntry->NonPaged->Lock);

            ExFreePool( AFSGlobalDotDotDirEntry->NonPaged);

            ExFreePool( AFSGlobalDotDotDirEntry);

            AFSGlobalDotDotDirEntry = NULL;
        }

        if( AFSSpecialShareNames != NULL)
        {

            pDirNode = AFSSpecialShareNames;

            while( pDirNode != NULL)
            {

                pLastDirNode = (AFSDirectoryCB *)pDirNode->ListEntry.fLink;

                AFSDeleteObjectInfo( pDirNode->ObjectInformation);

                ExDeleteResourceLite( &pDirNode->NonPaged->Lock);

                ExFreePool( pDirNode->NonPaged);

                ExFreePool( pDirNode);

                pDirNode = pLastDirNode;
            }

            AFSSpecialShareNames = NULL;
        }
    }

    return ntStatus;
}

NTSTATUS
AFSDefaultLogMsg( IN ULONG Subsystem,
                  IN ULONG Level,
                  IN PCCH Format,
                  ...)
{

    NTSTATUS ntStatus = STATUS_SUCCESS;
    va_list va_args;
    char chDebugBuffer[ 256];

    __Enter
    {

        va_start( va_args, Format);

        ntStatus = RtlStringCbVPrintfA( chDebugBuffer,
                                        256,
                                        Format,
                                        va_args);

        if( NT_SUCCESS( ntStatus))
        {
            DbgPrint( chDebugBuffer);
        }

        va_end( va_args);
    }

    return ntStatus;
}

NTSTATUS
AFSGetObjectStatus( IN AFSGetStatusInfoCB *GetStatusInfo,
                    IN ULONG InputBufferLength,
                    IN AFSStatusInfoCB *StatusInfo,
                    OUT ULONG *ReturnLength)
{

    NTSTATUS ntStatus = STATUS_SUCCESS;
    AFSFcb   *pFcb = NULL;
    AFSVolumeCB *pVolumeCB = NULL;
    AFSDeviceExt *pDevExt = (AFSDeviceExt *) AFSRDRDeviceObject->DeviceExtension;
    AFSObjectInfoCB *pObjectInfo = NULL;
    ULONGLONG   ullIndex = 0;
    UNICODE_STRING uniFullPathName, uniRemainingPath, uniComponentName, uniParsedName;
    AFSNameArrayHdr *pNameArray = NULL;
    AFSDirectoryCB *pDirectoryEntry = NULL, *pParentDirEntry = NULL;

    __Enter
    {

        //
        // If we are given a FID then look up the entry by that, otherwise
        // do it by name
        //

        if( GetStatusInfo->FileID.Cell != 0 &&
            GetStatusInfo->FileID.Volume != 0 &&
            GetStatusInfo->FileID.Vnode != 0 &&
            GetStatusInfo->FileID.Unique != 0)
        {

            AFSAcquireShared( &pDevExt->Specific.RDR.VolumeTreeLock, TRUE);

            //
            // Locate the volume node
            //

            ullIndex = AFSCreateHighIndex( &GetStatusInfo->FileID);

            ntStatus = AFSLocateHashEntry( pDevExt->Specific.RDR.VolumeTree.TreeHead,
                                           ullIndex,
                                           (AFSBTreeEntry **)&pVolumeCB);

            if( pVolumeCB != NULL)
            {

                InterlockedIncrement( &pVolumeCB->VolumeReferenceCount);

                AFSDbgLogMsg( AFS_SUBSYSTEM_VOLUME_REF_COUNTING,
                              AFS_TRACE_LEVEL_VERBOSE,
                              "AFSGetObjectStatus Increment count on volume %08lX Cnt %d\n",
                              pVolumeCB,
                              pVolumeCB->VolumeReferenceCount);
            }

            AFSReleaseResource( &pDevExt->Specific.RDR.VolumeTreeLock);

            if( !NT_SUCCESS( ntStatus) ||
                pVolumeCB == NULL)
            {
                try_return( ntStatus = STATUS_INVALID_PARAMETER);
            }

            if( AFSIsVolumeFID( &GetStatusInfo->FileID))
            {

                pObjectInfo = &pVolumeCB->ObjectInformation;

                InterlockedIncrement( &pObjectInfo->ObjectReferenceCount);

                InterlockedDecrement( &pVolumeCB->VolumeReferenceCount);
            }
            else
            {

                AFSAcquireShared( pVolumeCB->ObjectInfoTree.TreeLock,
                                  TRUE);

                InterlockedDecrement( &pVolumeCB->VolumeReferenceCount);

                AFSDbgLogMsg( AFS_SUBSYSTEM_VOLUME_REF_COUNTING,
                              AFS_TRACE_LEVEL_VERBOSE,
                              "AFSGetObjectStatus Decrement count on volume %08lX Cnt %d\n",
                              pVolumeCB,
                              pVolumeCB->VolumeReferenceCount);

                ullIndex = AFSCreateLowIndex( &GetStatusInfo->FileID);

                ntStatus = AFSLocateHashEntry( pVolumeCB->ObjectInfoTree.TreeHead,
                                               ullIndex,
                                               (AFSBTreeEntry **)&pObjectInfo);

                if( pObjectInfo != NULL)
                {

                    //
                    // Reference the node so it won't be torn down
                    //

                    InterlockedIncrement( &pObjectInfo->ObjectReferenceCount);

                    AFSDbgLogMsg( AFS_SUBSYSTEM_OBJECT_REF_COUNTING,
                                  AFS_TRACE_LEVEL_VERBOSE,
                                  "AFSGetObjectStatus Increment count on object %08lX Cnt %d\n",
                                  pObjectInfo,
                                  pObjectInfo->ObjectReferenceCount);
                }

                AFSReleaseResource( pVolumeCB->ObjectInfoTree.TreeLock);

                if( !NT_SUCCESS( ntStatus) ||
                    pObjectInfo == NULL)
                {
                    try_return( ntStatus = STATUS_INVALID_PARAMETER);
                }
            }
        }
        else
        {

            if( GetStatusInfo->FileNameLength == 0 ||
                InputBufferLength < (ULONG)(FIELD_OFFSET( AFSGetStatusInfoCB, FileName) + GetStatusInfo->FileNameLength))
            {
                try_return( ntStatus = STATUS_INVALID_PARAMETER);
            }

            uniFullPathName.Length = GetStatusInfo->FileNameLength;
            uniFullPathName.MaximumLength = uniFullPathName.Length;

            uniFullPathName.Buffer = (WCHAR *)GetStatusInfo->FileName;

            //
            // This name should begin with the \afs server so parse it off and check it
            //

            FsRtlDissectName( uniFullPathName,
                              &uniComponentName,
                              &uniRemainingPath);

            if( RtlCompareUnicodeString( &uniComponentName,
                                         &AFSServerName,
                                         TRUE) != 0)
            {
                AFSDbgLogMsg( AFS_SUBSYSTEM_FILE_PROCESSING,
                              AFS_TRACE_LEVEL_ERROR,
                              "AFSGetObjectStatus Name %wZ contains invalid server name\n",
                              &uniFullPathName);

                try_return( ntStatus = STATUS_OBJECT_PATH_INVALID);
            }

            uniFullPathName = uniRemainingPath;

            uniParsedName = uniFullPathName;

            //
            // Our name array
            //

            pNameArray = AFSInitNameArray( AFSGlobalRoot->DirectoryCB,
                                           0);

            if( pNameArray == NULL)
            {
                try_return( ntStatus = STATUS_INSUFFICIENT_RESOURCES);
            }

            pVolumeCB = AFSGlobalRoot;

            AFSAcquireShared( pVolumeCB->VolumeLock,
                              TRUE);

            pParentDirEntry = AFSGlobalRoot->DirectoryCB;

            //
            // Increment the ref count on the volume and dir entry for correct processing below
            //

            InterlockedIncrement( &pVolumeCB->VolumeReferenceCount);

            AFSDbgLogMsg( AFS_SUBSYSTEM_VOLUME_REF_COUNTING,
                          AFS_TRACE_LEVEL_VERBOSE,
                          "AFSGetObjectStatus Increment count on volume %08lX Cnt %d\n",
                          pVolumeCB,
                          pVolumeCB->VolumeReferenceCount);

            InterlockedIncrement( &pParentDirEntry->OpenReferenceCount);

            AFSDbgLogMsg( AFS_SUBSYSTEM_DIRENTRY_REF_COUNTING,
                          AFS_TRACE_LEVEL_VERBOSE,
                          "AFSGetObjectStatus Increment count on %wZ DE %p Ccb %p Cnt %d\n",
                          &pParentDirEntry->NameInformation.FileName,
                          pParentDirEntry,
                          NULL,
                          pParentDirEntry->OpenReferenceCount);

            ntStatus = AFSLocateNameEntry( NULL,
                                           NULL,
                                           &uniFullPathName,
                                           &uniParsedName,
                                           pNameArray,
                                           AFS_LOCATE_FLAGS_NO_MP_TARGET_EVAL |
                                                        AFS_LOCATE_FLAGS_NO_SL_TARGET_EVAL,
                                           &pVolumeCB,
                                           &pParentDirEntry,
                                           &pDirectoryEntry,
                                           NULL);

            if( !NT_SUCCESS( ntStatus))
            {

                //
                // The volume lock was released on failure above
                // Except for STATUS_OBJECT_NAME_NOT_FOUND
                //

                if( ntStatus == STATUS_OBJECT_NAME_NOT_FOUND)
                {

                    if( pVolumeCB != NULL)
                    {

                        InterlockedDecrement( &pVolumeCB->VolumeReferenceCount);

                        AFSDbgLogMsg( AFS_SUBSYSTEM_VOLUME_REF_COUNTING,
                                      AFS_TRACE_LEVEL_VERBOSE,
                                      "AFSGetObjectStatus Decrement count on volume %08lX Cnt %d\n",
                                      pVolumeCB,
                                      pVolumeCB->VolumeReferenceCount);

                        AFSReleaseResource( pVolumeCB->VolumeLock);
                    }

                    if( pDirectoryEntry != NULL)
                    {

                        InterlockedDecrement( &pDirectoryEntry->OpenReferenceCount);

                        AFSDbgLogMsg( AFS_SUBSYSTEM_DIRENTRY_REF_COUNTING,
                                      AFS_TRACE_LEVEL_VERBOSE,
                                      "AFSGetObjectStatus Decrement1 count on %wZ DE %p Ccb %p Cnt %d\n",
                                      &pDirectoryEntry->NameInformation.FileName,
                                      pDirectoryEntry,
                                      NULL,
                                      pDirectoryEntry->OpenReferenceCount);
                    }
                    else
                    {

                        InterlockedDecrement( &pParentDirEntry->OpenReferenceCount);

                        AFSDbgLogMsg( AFS_SUBSYSTEM_DIRENTRY_REF_COUNTING,
                                      AFS_TRACE_LEVEL_VERBOSE,
                                      "AFSGetObjectStatus Decrement2 count on %wZ DE %p Ccb %p Cnt %d\n",
                                      &pParentDirEntry->NameInformation.FileName,
                                      pParentDirEntry,
                                      NULL,
                                      pParentDirEntry->OpenReferenceCount);
                    }
                }

                pVolumeCB = NULL;

                try_return( ntStatus);
            }

            //
            // Remove the reference made above
            //

            InterlockedDecrement( &pDirectoryEntry->OpenReferenceCount);

            pObjectInfo = pDirectoryEntry->ObjectInformation;

            InterlockedIncrement( &pObjectInfo->ObjectReferenceCount);

            if( pVolumeCB != NULL)
            {

                InterlockedDecrement( &pVolumeCB->VolumeReferenceCount);

                AFSDbgLogMsg( AFS_SUBSYSTEM_VOLUME_REF_COUNTING,
                              AFS_TRACE_LEVEL_VERBOSE,
                              "AFSRetrieveFileAttributes Decrement2 count on volume %08lX Cnt %d\n",
                              pVolumeCB,
                              pVolumeCB->VolumeReferenceCount);

                AFSReleaseResource( pVolumeCB->VolumeLock);
            }
        }

        //
        // At this point we have an object info block, return the information
        //

        StatusInfo->FileId = pObjectInfo->FileId;

        StatusInfo->TargetFileId = pObjectInfo->TargetFileId;

        StatusInfo->Expiration = pObjectInfo->Expiration;

        StatusInfo->DataVersion = pObjectInfo->DataVersion;

        StatusInfo->FileType = pObjectInfo->FileType;

        StatusInfo->ObjectFlags = pObjectInfo->Flags;

        StatusInfo->CreationTime = pObjectInfo->CreationTime;

        StatusInfo->LastAccessTime = pObjectInfo->LastAccessTime;

        StatusInfo->LastWriteTime = pObjectInfo->LastWriteTime;

        StatusInfo->ChangeTime = pObjectInfo->ChangeTime;

        StatusInfo->FileAttributes = pObjectInfo->FileAttributes;

        StatusInfo->EndOfFile = pObjectInfo->EndOfFile;

        StatusInfo->AllocationSize = pObjectInfo->AllocationSize;

        StatusInfo->EaSize = pObjectInfo->EaSize;

        StatusInfo->Links = pObjectInfo->Links;

        //
        // Return the information length
        //

        *ReturnLength = sizeof( AFSStatusInfoCB);

try_exit:

        if( pObjectInfo != NULL)
        {

            InterlockedDecrement( &pObjectInfo->ObjectReferenceCount);
        }

        if( pNameArray != NULL)
        {

            AFSFreeNameArray( pNameArray);
        }
    }

    return ntStatus;
}

NTSTATUS
AFSCheckSymlinkAccess( IN AFSDirectoryCB *ParentDirectoryCB,
                       IN UNICODE_STRING *ComponentName)
{

    NTSTATUS ntStatus = STATUS_SUCCESS;
    AFSDirectoryCB *pDirEntry = NULL;
    ULONG ulCRC = 0;

    __Enter
    {

        //
        // Search for the entry in the parent
        //

        AFSDbgLogMsg( AFS_SUBSYSTEM_FILE_PROCESSING,
                      AFS_TRACE_LEVEL_VERBOSE_2,
                      "AFSCheckSymlinkAccess Searching for entry %wZ case sensitive\n",
                      ComponentName);

        ulCRC = AFSGenerateCRC( ComponentName,
                                FALSE);

        AFSAcquireShared( ParentDirectoryCB->ObjectInformation->Specific.Directory.DirectoryNodeHdr.TreeLock,
                          TRUE);

        AFSLocateCaseSensitiveDirEntry( ParentDirectoryCB->ObjectInformation->Specific.Directory.DirectoryNodeHdr.CaseSensitiveTreeHead,
                                        ulCRC,
                                        &pDirEntry);

        if( pDirEntry == NULL)
        {

            //
            // Missed so perform a case insensitive lookup
            //

            AFSDbgLogMsg( AFS_SUBSYSTEM_FILE_PROCESSING,
                          AFS_TRACE_LEVEL_VERBOSE_2,
                          "AFSCheckSymlinkAccess Searching for entry %wZ case insensitive\n",
                          ComponentName);

            ulCRC = AFSGenerateCRC( ComponentName,
                                    TRUE);

            AFSLocateCaseInsensitiveDirEntry( ParentDirectoryCB->ObjectInformation->Specific.Directory.DirectoryNodeHdr.CaseInsensitiveTreeHead,
                                              ulCRC,
                                              &pDirEntry);

            if( pDirEntry == NULL)
            {

                //
                // OK, if this component is a valid short name then try
                // a lookup in the short name tree
                //

                if( RtlIsNameLegalDOS8Dot3( ComponentName,
                                            NULL,
                                            NULL))
                {

                    AFSDbgLogMsg( AFS_SUBSYSTEM_FILE_PROCESSING,
                                  AFS_TRACE_LEVEL_VERBOSE_2,
                                  "AFSCheckSymlinkAccess Searching for entry %wZ short name\n",
                                  ComponentName);

                    AFSLocateShortNameDirEntry( ParentDirectoryCB->ObjectInformation->Specific.Directory.ShortNameTree,
                                                ulCRC,
                                                &pDirEntry);
                }
            }
        }

        if( pDirEntry != NULL)
        {
            InterlockedIncrement( &pDirEntry->OpenReferenceCount);
        }

        AFSReleaseResource( ParentDirectoryCB->ObjectInformation->Specific.Directory.DirectoryNodeHdr.TreeLock);

        if( pDirEntry == NULL)
        {

            AFSDbgLogMsg( AFS_SUBSYSTEM_FILE_PROCESSING,
                          AFS_TRACE_LEVEL_VERBOSE_2,
                          "AFSCheckSymlinkAccess Failed to locate entry %wZ\n",
                          ComponentName);

            try_return( ntStatus = STATUS_OBJECT_NAME_NOT_FOUND);
        }

        //
        // We have the symlink object but previously failed to process it so return access
        // denied.
        //

        AFSDbgLogMsg( AFS_SUBSYSTEM_FILE_PROCESSING,
                      AFS_TRACE_LEVEL_VERBOSE_2,
                      "AFSCheckSymlinkAccess Failing symlink access to entry %wZ ACCESS_DENIED\n",
                      ComponentName);

        ntStatus = STATUS_ACCESS_DENIED;

        InterlockedDecrement( &pDirEntry->OpenReferenceCount);

try_exit:

        NOTHING;
    }

    return ntStatus;
}

NTSTATUS
AFSRetrieveFinalComponent( IN UNICODE_STRING *FullPathName,
                           OUT UNICODE_STRING *ComponentName)
{

    NTSTATUS ntStatus = STATUS_SUCCESS;
    UNICODE_STRING uniFullPathName, uniRemainingPath, uniComponentName;

    uniFullPathName = *FullPathName;

    while( TRUE)
    {

        FsRtlDissectName( uniFullPathName,
                          &uniComponentName,
                          &uniRemainingPath);

        if( uniRemainingPath.Length == 0)
        {
            break;
        }

        uniFullPathName = uniRemainingPath;
    }

    if( uniComponentName.Length > 0)
    {
        *ComponentName = uniComponentName;
    }

    return ntStatus;
}

void
AFSDumpTraceFiles_Default()
{
    return;
}

BOOLEAN
AFSValidNameFormat( IN UNICODE_STRING *FileName)
{

    BOOLEAN bIsValidName = TRUE;
    USHORT usIndex = 0;

    __Enter
    {

        while( usIndex < FileName->Length/sizeof( WCHAR))
        {

            if( FileName->Buffer[ usIndex] == L':' ||
                FileName->Buffer[ usIndex] == L'*' ||
                FileName->Buffer[ usIndex] == L'?' ||
                FileName->Buffer[ usIndex] == L'"' ||
                FileName->Buffer[ usIndex] == L'<' ||
                FileName->Buffer[ usIndex] == L'>')
            {
                bIsValidName = FALSE;
                break;
            }

            usIndex++;
        }
    }

    return bIsValidName;
}

NTSTATUS
AFSCreateDefaultSecurityDescriptor()
{

    NTSTATUS ntStatus = STATUS_SUCCESS;
    PACL pSACL = NULL;
    ULONG ulSACLSize = 0;
    SYSTEM_MANDATORY_LABEL_ACE* pACE = NULL;
    ULONG ulACESize = 0;
    SECURITY_DESCRIPTOR *pSecurityDescr = NULL;
    ULONG ulSDLength = 0;
    SECURITY_DESCRIPTOR *pRelativeSecurityDescr = NULL;

    __Enter
    {

        if( AFSRtlSetSaclSecurityDescriptor == NULL)
        {

            AFSPrint( "AFSCreateDefaultSecurityDescriptor AFSRtlSetSaclSecurityDescriptor == NULL\n");
        }
        else
        {

            ulACESize = sizeof( SYSTEM_MANDATORY_LABEL_ACE) + 128;

            pACE = (SYSTEM_MANDATORY_LABEL_ACE *)ExAllocatePoolWithTag( PagedPool,
                                                                        ulACESize,
                                                                        AFS_GENERIC_MEMORY_29_TAG);

            if( pACE == NULL)
            {

                AFSPrint( "AFSCreateDefaultSecurityDescriptor unable to allocate AFS_GENERIC_MEMORY_29_TAG\n");

                try_return( ntStatus = STATUS_INSUFFICIENT_RESOURCES);
            }

            RtlZeroMemory( pACE,
                           ulACESize);

            pACE->Header.AceFlags = OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE;
            pACE->Header.AceType = SYSTEM_MANDATORY_LABEL_ACE_TYPE;
            pACE->Header.AceSize = FIELD_OFFSET( SYSTEM_MANDATORY_LABEL_ACE, SidStart) + (USHORT)RtlLengthSid( SeExports->SeLowMandatorySid);
            pACE->Mask = SYSTEM_MANDATORY_LABEL_NO_WRITE_UP;

            RtlCopySid( RtlLengthSid( SeExports->SeLowMandatorySid),
                        &pACE->SidStart,
                        SeExports->SeLowMandatorySid);

            ulSACLSize = sizeof(ACL) + RtlLengthSid( SeExports->SeLowMandatorySid) +
                FIELD_OFFSET( SYSTEM_MANDATORY_LABEL_ACE, SidStart) + ulACESize;

            pSACL = (PACL)ExAllocatePoolWithTag( PagedPool,
                                                 ulSACLSize,
                                                 AFS_GENERIC_MEMORY_29_TAG);

            if( pSACL == NULL)
            {

                AFSPrint( "AFSCreateDefaultSecurityDescriptor unable to allocate AFS_GENERIC_MEMORY_29_TAG\n");

                try_return( ntStatus = STATUS_INSUFFICIENT_RESOURCES);
            }

            ntStatus = RtlCreateAcl( pSACL,
                                     ulSACLSize,
                                     ACL_REVISION);

            if( !NT_SUCCESS( ntStatus))
            {

                AFSPrint( "AFSCreateDefaultSecurityDescriptor RtlCreateAcl ntStatus %08lX\n",
                          ntStatus);

                try_return( ntStatus);
            }

            ntStatus = RtlAddAce( pSACL,
                                  ACL_REVISION,
                                  0,
                                  pACE,
                                  pACE->Header.AceSize);

            if( !NT_SUCCESS( ntStatus))
            {

                AFSPrint( "AFSCreateDefaultSecurityDescriptor RtlAddAce ntStatus %08lX\n",
                          ntStatus);

                try_return( ntStatus);
            }
        }

        pSecurityDescr = (SECURITY_DESCRIPTOR *)ExAllocatePoolWithTag( NonPagedPool,
                                                                       sizeof( SECURITY_DESCRIPTOR),
                                                                       AFS_GENERIC_MEMORY_27_TAG);

        if( pSecurityDescr == NULL)
        {

            AFSPrint( "AFSCreateDefaultSecurityDescriptor unable to allocate AFS_GENERIC_MEMORY_27_TAG\n");

            try_return( ntStatus = STATUS_INSUFFICIENT_RESOURCES);
        }

        ntStatus = RtlCreateSecurityDescriptor( pSecurityDescr,
                                                SECURITY_DESCRIPTOR_REVISION);

        if( !NT_SUCCESS( ntStatus))
        {

            AFSPrint( "AFSCreateDefaultSecurityDescriptor RtlCreateSecurityDescriptor ntStatus %08lX\n",
                      ntStatus);

            try_return( ntStatus);
        }

        if( AFSRtlSetSaclSecurityDescriptor != NULL)
        {
            ntStatus = AFSRtlSetSaclSecurityDescriptor( pSecurityDescr,
                                                        TRUE,
                                                        pSACL,
                                                        FALSE);

            if( !NT_SUCCESS( ntStatus))
            {

                AFSPrint( "AFSCreateDefaultSecurityDescriptor AFSRtlSetSaclSecurityDescriptor ntStatus %08lX\n",
                          ntStatus);

                try_return( ntStatus);
            }
        }

        if( !RtlValidSecurityDescriptor( pSecurityDescr))
        {

            AFSPrint( "AFSCreateDefaultSecurityDescriptor RtlValidSecurityDescriptor NOT\n");

            try_return( ntStatus = STATUS_INVALID_PARAMETER);
        }

        pRelativeSecurityDescr = (SECURITY_DESCRIPTOR *)ExAllocatePoolWithTag( NonPagedPool,
                                                                               PAGE_SIZE,
                                                                               AFS_GENERIC_MEMORY_27_TAG);

        if( pRelativeSecurityDescr == NULL)
        {

            AFSPrint( "AFSCreateDefaultSecurityDescriptor unable to allocate AFS_GENERIC_MEMORY_27_TAG\n");

            try_return( ntStatus = STATUS_INSUFFICIENT_RESOURCES);
        }

        ulSDLength = PAGE_SIZE;

        ntStatus = RtlAbsoluteToSelfRelativeSD( pSecurityDescr,
                                                pRelativeSecurityDescr,
                                                &ulSDLength);

        if( !NT_SUCCESS( ntStatus))
        {

            AFSPrint( "AFSCreateDefaultSecurityDescriptor RtlAbsoluteToSelfRelativeSD ntStatus %08lX\n",
                      ntStatus);

            try_return( ntStatus);
        }

        AFSDefaultSD = pRelativeSecurityDescr;

try_exit:

        if( !NT_SUCCESS( ntStatus))
        {

            if( pRelativeSecurityDescr != NULL)
            {
                ExFreePool( pRelativeSecurityDescr);
            }
        }

        if( pSecurityDescr != NULL)
        {
            ExFreePool( pSecurityDescr);
        }

        if( pSACL != NULL)
        {
            ExFreePool( pSACL);
        }

        if( pACE != NULL)
        {
            ExFreePool( pACE);
        }
    }

    return ntStatus;
}