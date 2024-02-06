#define _WINSOCKAPI_ // Prevents windows.h from including winsock.h, so windock2.h
//                      can be used instead.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <versionhelpers.h>

#pragma comment(lib, "kernel32")
#pragma comment(lib, "ntdll")

#define _RECURSIVE_

//========================================
// Config
//========================================

external void
LoadSystemInfo(void)
{
    SYSTEM_INFO Info;
    GetSystemInfo(&Info);
    gSysInfo.PageSize = Info.dwPageSize;
    gSysInfo.AddressRange[0] = (isz)Info.lpMinimumApplicationAddress;
    gSysInfo.AddressRange[1] = (isz)Info.lpMaximumApplicationAddress;
    gSysInfo.MemBlockSize = Info.dwAllocationGranularity;
    gSysInfo.NumThreads = Info.dwNumberOfProcessors;
    
    LARGE_INTEGER Freq;
    QueryPerformanceFrequency(&Freq);
    gSysInfo.TimingFreq = (f64)Freq.QuadPart;
    
    usz VerSize = sizeof(gSysInfo.OSVersion);
    if (IsWindowsServer())
    {
        if (IsWindows10OrGreater()) CopyData(gSysInfo.OSVersion, VerSize, "S16R1", 5);
        else if (IsWindows8Point1OrGreater()) CopyData(gSysInfo.OSVersion, VerSize,
                                                       "S12R2", 5);
        else if (IsWindows8OrGreater()) CopyData(gSysInfo.OSVersion, VerSize, "S12R1", 5);
        else if (IsWindows7OrGreater()) CopyData(gSysInfo.OSVersion, VerSize, "S08R2", 5);
        else if (IsWindowsVistaOrGreater()) CopyData(gSysInfo.OSVersion, VerSize,
                                                     "S08R1", 5);
        else CopyData(gSysInfo.OSVersion, VerSize, "S...", 4);
    }
    else
    {
        if (IsWindows10OrGreater()) CopyData(gSysInfo.OSVersion, VerSize, "W10.0", 5);
        else if (IsWindows8Point1OrGreater()) CopyData(gSysInfo.OSVersion, VerSize, "W08.1", 5);
        else if (IsWindows8OrGreater()) CopyData(gSysInfo.OSVersion, VerSize, "W08.0", 5);
        else if (IsWindows7OrGreater()) CopyData(gSysInfo.OSVersion, VerSize, "W07.0", 5);
        else if (IsWindowsVistaOrGreater()) CopyData(gSysInfo.OSVersion, VerSize, "WVista", 6);
        else CopyData(gSysInfo.OSVersion, VerSize, "W...", 4);
    }
}

//========================================
// Memory
//========================================

external void
ClearMemory(buffer* Mem)
{
    memset(Mem->Base, 0, Mem->Size);
    Mem->WriteCur = 0;
}

external buffer
GetMemory(usz Size, void* Address, int Flags)
{
    buffer Result = {0};
    
    DWORD Access = 0;
    if (Flags & MEM_GUARD)
    {
        Access = PAGE_NOACCESS;
    }
    else if (Flags & MEM_EXEC)
    {
        if (Flags & MEM_WRITE) Access = PAGE_EXECUTE_READWRITE;
        else if (Flags & MEM_READ) Access = PAGE_EXECUTE_READ;
        else Access = PAGE_EXECUTE;
    }
    else if (Flags & MEM_WRITE)
    {
        Access = PAGE_READWRITE;
    }
    else
    {
        Access = PAGE_READONLY;
    }
    
    DWORD HugePages = ((Flags & MEM_HUGEPAGE) > 0) ? MEM_LARGE_PAGES : 0;
    DWORD AllocType = MEM_RESERVE | MEM_COMMIT | HugePages;
    
    void* Ptr = VirtualAlloc(Address, Size, AllocType, Access);
    if (Ptr)
    {
        Result.Base = (u8*)Ptr;
        Result.Size = (gSysInfo.PageSize) ? Align(Size, gSysInfo.PageSize) : Size;
    }
    return Result;
}

external void
FreeMemory(buffer* Mem)
{
    VirtualFree(Mem->Base, 0, MEM_RELEASE);
    memset(Mem, 0, sizeof(buffer));
}

external buffer
GetMemoryFromHeap(usz SizeToAllocate)
{
    buffer Result = {0};
    void* Ptr = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, SizeToAllocate);
    if (Ptr)
    {
        Result.Base = (u8*)Ptr;
        Result.Size = SizeToAllocate;
    }
    return Result;
}

external void
FreeMemoryFromHeap(buffer* Mem)
{
    HeapFree(GetProcessHeap(), 0, Mem->Base);
    memset(Mem, 0, sizeof(buffer));
}

//========================================
// FileIO
//========================================

internal file
_NewFile(void* Filename, DWORD CreationMode, i32 Flags)
{
    DWORD Read = ((Flags & (READ_SOLO|READ_SHARE)) > 0) ? GENERIC_READ : 0;
    DWORD Write = ((Flags & (WRITE_SOLO|WRITE_SHARE)) > 0) ? GENERIC_WRITE : 0;
    DWORD Append = ((Flags & APPEND_FILE) > 0) ? FILE_APPEND_DATA : 0;
    DWORD AccessRights = Read | Write | Append;
    
    DWORD ReadShare = ((Flags & READ_SHARE) > 0) ? FILE_SHARE_READ : 0;
    DWORD WriteShare = ((Flags & WRITE_SHARE) > 0) ? FILE_SHARE_WRITE : 0;
    DWORD DeleteShare = ((Flags & DELETE_SHARE) > 0) ? FILE_SHARE_DELETE : 0;
    DWORD ShareMode = ReadShare | WriteShare | DeleteShare;
    
    DWORD Async = ((Flags & ASYNC_FILE) > 0) ? FILE_FLAG_OVERLAPPED : 0;
    DWORD Hidden = ((Flags & HIDDEN_FILE) > 0) ? FILE_ATTRIBUTE_HIDDEN : 0;
    DWORD Attributes = Async | Hidden;
    
    file File = (file)CreateFileW((wchar_t*)Filename, AccessRights, ShareMode, NULL,
                                  CreationMode, Attributes, NULL);
    return File;
}

external file
CreateNewFile(void* Filename, i32 Flags)
{
    DWORD CreationMode = (Flags & FORCE_CREATE) ? CREATE_ALWAYS : CREATE_NEW;
    file Result = _NewFile(Filename, CreationMode, Flags);
    return Result;
}

external file
OpenFileHandle(void* Filename, i32 Flags)
{
    DWORD CreationMode = (Flags & FORCE_OPEN) ? OPEN_ALWAYS : OPEN_EXISTING;
    file Result = _NewFile(Filename, CreationMode, Flags);
    return Result;
}

external void
CloseFileHandle(file File)
{
    CloseHandle((HANDLE)File);
}

external bool
RemoveFile(void* Filename)
{
    if (!DeleteFileW((wchar_t*)Filename))
    {
        int Error = GetLastError();
        if (Error == ERROR_ACCESS_DENIED)
        {
            SetFileAttributesW((wchar_t*)Filename, FILE_ATTRIBUTE_NORMAL);
            return DeleteFileW((wchar_t*)Filename);
        }
        return false;
    }
    return true;
}

external usz
FileSizeOf(file File)
{
    usz Result = USZ_MAX;
    DWORD FileSizeHi;
    DWORD FileSizeLo = GetFileSize((HANDLE)File, &FileSizeHi);
    if (FileSizeLo != INVALID_FILE_SIZE)
    {
        Result = (usz)FileSizeHi << 32 | FileSizeLo;
    }
    return Result;
}

external bool
SeekFile(file File, usz Pos)
{
    LONG PosHi = (LONG)(Pos >> 32);
    DWORD Result = SetFilePointer((HANDLE)File, Pos & 0xFFFFFFFF, &PosHi, FILE_BEGIN);
    return Result != INVALID_SET_FILE_POINTER;
}

external bool
ReadFromFile(file File, buffer* Dst, usz AmountToRead, usz StartPos)
{
    usz FileSize = FileSizeOf(File);
    if ((StartPos + AmountToRead) <= FileSize
        && AmountToRead <= (Dst->Size - Dst->WriteCur))
    {
        SeekFile(File, StartPos);
        usz RemainsToRead = AmountToRead;
        u8* Ptr = Dst->Base + Dst->WriteCur;
        while (RemainsToRead > 0)
        {
            DWORD ReadSize = (RemainsToRead > U32_MAX) ? U32_MAX : (DWORD)RemainsToRead;
            DWORD BytesRead = 0;
            if (!ReadFile((HANDLE)File, Ptr, ReadSize, &BytesRead, NULL))
            {
                return false;
            }
            RemainsToRead -= BytesRead;
            Ptr += BytesRead;
        }
        Dst->WriteCur += AmountToRead;
        return true;
    }
    return false;
}

external buffer
ReadEntireFile(file File)
{
    usz FileSize = FileSizeOf(File);
    buffer Mem = GetMemory(FileSize, 0, MEM_READ|MEM_WRITE);
    if (Mem.Base)
    {
        if (ReadFromFile(File, &Mem, FileSize, 0))
        {
            Mem.WriteCur = FileSize;
        }
        else
        {
            FreeMemory(&Mem);
        }
    }
    return Mem;
}

external bool
ReadFileAsync(file File, buffer* Dst, usz AmountToRead, usz StartPos, async* Async)
{
    if (AmountToRead <= (Dst->Size - Dst->WriteCur))
    {
        OVERLAPPED* Overlapped = (OVERLAPPED*)Async->Data;
        u8* Ptr = Dst->Base + Dst->WriteCur;
        for (usz AmountRead = 0; AmountRead < AmountToRead; )
        {
            Overlapped->Offset = StartPos & 0xFFFFFFFF;
            Overlapped->OffsetHigh = (StartPos >> 32) & 0xFFFFFFFF;
            DWORD BytesToRead = (DWORD)Min(AmountToRead - AmountRead, U32_MAX);
            if (!ReadFile((HANDLE)File, Ptr, BytesToRead, NULL, Overlapped)
                && GetLastError() != ERROR_IO_PENDING)
            {
                return false;
            }
            Ptr += BytesToRead;
            AmountRead += BytesToRead;
            StartPos += BytesToRead;
        }
        *((file*)&Overlapped[1]) = File;
        Dst->WriteCur += AmountToRead;
        return true;
    }
    return false;
}

external bool
AppendToFile(file File, buffer Content)
{
    usz RemainsToWrite = Content.WriteCur;
    u8* Ptr = Content.Base;
    while (RemainsToWrite > 0)
    {
        DWORD WriteSize = (DWORD)Min(RemainsToWrite, U32_MAX);
        DWORD BytesWritten = 0;
        if (!WriteFile((HANDLE)File, Ptr, WriteSize, &BytesWritten, NULL))
        {
            return false;
        }
        RemainsToWrite -= BytesWritten;
        Ptr += BytesWritten;
    }
    return true;
}

external bool
WriteEntireFile(file File, buffer Content)
{
    SeekFile(File, 0);
    bool Result = AppendToFile(File, Content);
    return Result;
}

external bool
WriteToFile(file File, buffer Content, usz StartPos)
{
    SeekFile(File, StartPos);
    bool Result = AppendToFile(File, Content);
    return Result;
}

external bool
WriteFileAsync(file File, void* Src, usz AmountToWrite, usz StartPos, async* Async)
{
    OVERLAPPED* Overlapped = (OVERLAPPED*)Async->Data;
    
    usz WriteChunk = Min(AmountToWrite, U32_MAX);
    u8* Ptr = (u8*)Src;
    for (usz AmountWritten = 0; AmountWritten < AmountToWrite; )
    {
        Overlapped->Offset = StartPos & 0xFFFFFFFF;
        Overlapped->OffsetHigh = (StartPos >> 32) & 0xFFFFFFFF;
        DWORD BytesToWrite = (DWORD)Min(AmountToWrite - AmountWritten, WriteChunk);
        if (!WriteFile((HANDLE)File, Ptr, BytesToWrite, 0, Overlapped)
            && GetLastError() != ERROR_IO_PENDING)
        {
            return false;
        }
        Ptr += BytesToWrite;
        AmountWritten += BytesToWrite;
        StartPos += BytesToWrite;
    }
    
    *((file*)&Overlapped[1]) = File;
    return true;
}

external usz
WaitOnIoCompletion(file File, async* Async, bool Block)
{
    DWORD BytesTransferred = 0;
    OVERLAPPED* Overlapped = (OVERLAPPED*)Async->Data;
    GetOverlappedResult((HANDLE)File, Overlapped, &BytesTransferred, Block);
    return BytesTransferred;
}

external usz
FileLastWriteTime(file File)
{
    usz Result = USZ_MAX;
    FILETIME LastWrite;
    if (GetFileTime((HANDLE)File, NULL, NULL, &LastWrite))
    {
        Result = (usz)LastWrite.dwHighDateTime << 32 | (usz)LastWrite.dwLowDateTime;
    }
    return Result;
}

external bool
FilesAreEqual(file A, file B)
{
    bool Result = false;
    
    usz ASize = FileSizeOf(A);
    usz BSize = FileSizeOf(B);
    if (ASize == BSize)
    {
        buffer ABuf = ReadEntireFile(A);
        buffer BBuf = ReadEntireFile(B);
        if (ABuf.Base && BBuf.Base)
        {
            Result = EqualBuffers(ABuf, BBuf);
            FreeMemory(&ABuf);
            FreeMemory(&BBuf);
        }
    }
    
    return Result;
}

external bool
DuplicateFile(void* SrcPath, void* DstPath, bool OverwriteIfExists)
{
    bool Result = CopyFileW((wchar_t*)SrcPath, (wchar_t*)DstPath, !OverwriteIfExists);
    return Result;
}

external bool
ChangeFileLocation(void* SrcPath, void* DstPath)
{
    bool Result = MoveFileW((wchar_t*)SrcPath, (wchar_t*)DstPath);
    return Result;
}

external bool
IsFileHidden(void* Filename)
{
    DWORD Result = GetFileAttributesW((wchar_t*)Filename);
    return (Result & FILE_ATTRIBUTE_HIDDEN);
}

//========================================
// Filesystem
//========================================

external bool
IsExistingPath(void* Path)
{
    // Assumes [Path] is in UTF-16LE.
    DWORD Attributes = GetFileAttributesW((wchar_t*)Path);
    return (Attributes != INVALID_FILE_ATTRIBUTES);
}

external bool
IsExistingDir(void* Path)
{
    // Assumes [Path] is in UTF-16LE.
    DWORD Attributes = GetFileAttributesW((wchar_t*)Path);
    return (Attributes != INVALID_FILE_ATTRIBUTES
            && Attributes & FILE_ATTRIBUTE_DIRECTORY);
}

external path
Path(void* Mem)
{
    // Expects [Mem] to have at least MAX_PATH_SIZE of size.
    path Result = String(Mem, 0, MAX_PATH_SIZE - sizeof(wchar_t), EC_UTF16LE);
    return Result;
}

external path
PathCString(void* CString)
{
    // Assumes [CString] is in UTF-16LE.
    path Result = { (char*)CString, 0, 0, EC_UTF16LE };
    usz CStringSize = 2 * wcslen((wchar_t*)CString);
    Result.WriteCur = Min(CStringSize, MAX_PATH_SIZE);
    return Result;
}

external bool
MoveUpPath(path* Path, usz MoveUpCount)
{
    wchar_t* LastChar = (wchar_t*)(Path->Base + Path->WriteCur) - 1;
    Path->WriteCur -= (*LastChar == '\\') ? sizeof(wchar_t) : 0;
    usz CurrentWriteCur = Path->WriteCur;
    
    string Backslash = String(L"\\", sizeof(wchar_t), 0, EC_UTF16LE);
    for (usz Count = 0; Count < MoveUpCount; Count++)
    {
        usz BackslashIdx = StringInString(Backslash, *Path, RETURN_IDX_FIND|SEARCH_REVERSE);
        if (BackslashIdx == INVALID_IDX)
        {
            Path->WriteCur = 0;
            break;
        }
        Path->WriteCur = BackslashIdx;
    }
    
    if (Path->WriteCur > 0)
    {
        Path->WriteCur += sizeof(wchar_t);
    }
    
    return (Path->WriteCur != CurrentWriteCur);
}

internal bool
_AppendPathToPath(path Src, path* Dst)
{
    path Tmp = *Dst;
    string Backslash = String(L"\\", sizeof(wchar_t), 0, EC_UTF16LE);
    
    if (Tmp.WriteCur > 0
        && Tmp.Base[Tmp.WriteCur-sizeof(wchar_t)] != '\\')
    {
        if (!AppendStringToString(Backslash, &Tmp)) return false;
    }
    
    if (Src.Base[0] == '\\' || Src.Base[0] == '/')
    {
        AdvanceBuffer(&Src.Buffer, sizeof(wchar_t));
    }
    
    wchar_t C0 = 0, C1 = 0, C2 = 0;
    usz ReadIdx = 0, DirCharCount = 0, ReadStart = 0;
    while (ReadIdx < Src.WriteCur)
    {
        C0 = C1;
        C1 = C2;
        C2 = *(wchar_t*)(Src.Base + ReadIdx);
        
        if (C2 == '\\' || C2 == '/')
        {
            if (DirCharCount == 1 && C1 == '.') { /* OBS: Do nothing. */ }
            else if (DirCharCount == 2 && C0 == '.' && C1 == '.')
            {
                MoveUpPath(&Tmp, 1);
            }
            else
            {
                string Dir = String(Src.Base + ReadStart, ReadIdx - ReadStart, 0, Src.Enc);
                if (!AppendStringToString(Dir, &Tmp)) return false;
                if (!AppendStringToString(Backslash, &Tmp)) return false;
            }
            DirCharCount = 0;
            ReadStart = ReadIdx + sizeof(wchar_t);
        }
        else
        {
            DirCharCount++;
        }
        
        ReadIdx += sizeof(wchar_t);
    }
    
    // Does a final copy in case [NewPart] does not end in '\'.
    if (DirCharCount > 0)
    {
        string Dir = String(Src.Base + ReadStart, ReadIdx - ReadStart, 0, Src.Enc);
        if (!AppendStringToString(Dir, &Tmp)) return false;
    }
    
    // Makes sure last byte is 0, so array can be passed to path-reading functions.
    *(wchar_t*)(Tmp.Base + Tmp.WriteCur) = '\0';
    
    *Dst = Tmp;
    return true;
}

external bool
AppendPathToPath(path NewPart, path* Dst)
{
    if (NewPart.Enc == Dst->Enc
        && NewPart.WriteCur <= (Dst->Size - Dst->WriteCur))
    {
        return _AppendPathToPath(NewPart, Dst);
    }
    return false;
}

external bool
AppendStringToPath(string NewPart, path* Dst)
{
    if (NewPart.Enc != Dst->Enc)
    {
        char NewPartUTF16[MAX_PATH_SIZE] = {0};
        path Src = Path(NewPartUTF16);
        return (Transcode(NewPart, &Src)
                && _AppendPathToPath(Src, Dst));
    }
    else
    {
        return _AppendPathToPath(NewPart, Dst);
    }
}

external bool
AppendDataToPath(void* NewPart, usz NewPartSize, path* Dst)
{
    // OBS: Assumes [NewPart] is in UTF-16LE.
    path NewPartPath = Path(NewPart);
    NewPartPath.WriteCur = NewPartSize;
    return _AppendPathToPath(NewPartPath, Dst);
}

external bool
AppendArrayToPath(void* NewPart, path* Dst)
{
    // OBS: Assumes [NewPart] is in UTF-16LE.
    path NewPartPath = PathCString(NewPart);
    return _AppendPathToPath(NewPartPath, Dst);
}

external bool
AppendCWDToPath(path* Dst)
{
    bool Result = false;
    
    char CWD[MAX_PATH_SIZE] = {0};
    path CWDPath = Path(CWD);
    DWORD Length = 2 * GetCurrentDirectoryW(sizeof(CWD) / 2, (wchar_t*)CWD);
    if (Length > 0 && Length < MAX_PATH_SIZE)
    {
        CWDPath.WriteCur = Length;
        Result = _AppendPathToPath(CWDPath, Dst);
    }
    
    return Result;
}

external bool
MakeDir(void* DirPath)
{
    // Assumes [DirPath] is in UTF-16LE.
    
    bool Result = CreateDirectoryW((wchar_t*)DirPath, NULL);
    if (!Result)
    {
        int Error = GetLastError();
        if (Error == ERROR_PATH_NOT_FOUND)
        {
            char WorkPathBuf[MAX_PATH_SIZE] = {0};
            path WorkPath = Path(WorkPathBuf);
            AppendArrayToPath(DirPath, &WorkPath);
            
            string Backslash = String(L"\\", sizeof(wchar_t), 0, EC_UTF16LE);
            isz BackslashIdx;
            while ((BackslashIdx = StringInString(Backslash, WorkPath, RETURN_IDX_FIND))
                   != INVALID_IDX)
            {
                wchar_t* BackslashPtr = (wchar_t*)(WorkPath.Base + BackslashIdx);
                *BackslashPtr = 0;
                Result = CreateDirectoryW((wchar_t*)WorkPathBuf, NULL);
                *BackslashPtr = '\\';
                WorkPath.Base += (BackslashIdx + sizeof(wchar_t));
                WorkPath.WriteCur -= (BackslashIdx + sizeof(wchar_t));
            }
            
            if (WorkPath.WriteCur > 0)
            {
                Result = CreateDirectoryW((wchar_t*)WorkPathBuf, NULL);
            }
        }
    }
    return Result;
}

external void
InitIterDir(iter_dir* Iter, path DirPath)
{
    ClearBuffer(&Iter->AllFiles.Buffer);
    Iter->AllFiles = Path(Iter->AllFilesBuf);
    AppendPathToPath(DirPath, &Iter->AllFiles);
    AppendDataToPath(L"*", sizeof(wchar_t), &Iter->AllFiles);
}

external bool
ListFiles(iter_dir* Iter)
{
    HANDLE* File = (HANDLE*)&Iter->OSData[0];
    WIN32_FIND_DATAW* Data = (WIN32_FIND_DATAW*)&Iter->OSData[sizeof(HANDLE)];
    if (*File == 0)
    {
        *File = FindFirstFileW((wchar_t*)Iter->AllFilesBuf, Data); // .
        FindNextFileW(*File, Data); // ..
    }
    
    if (!FindNextFileW(*File, Data))
    {
        FindClose(*File);
        *File = INVALID_HANDLE_VALUE;
        return false;
    }
    Iter->Filename = (char*)Data->cFileName;
    Iter->IsDir = Data->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
    
    return true;
}

external _RECURSIVE_ bool
RemoveDir(void* DirPath, bool RemoveAllFiles)
{
    // Assumes [DirPath] is in UTF-16LE.
    if (RemoveAllFiles)
    {
        path RemovePath = PathCString(DirPath);
        iter_dir Iter = {0};
        InitIterDir(&Iter, RemovePath);
        
        while (ListFiles(&Iter))
        {
            path ScratchPath = Iter.AllFiles;
            MoveUpPath(&ScratchPath, 1);
            path Filename = PathCString(Iter.Filename);
            AppendPathToPath(Filename, &ScratchPath);
            
            if (Iter.IsDir)
            {
                return RemoveDir(ScratchPath.Base, 1);
            }
            else
            {
                return RemoveFile(ScratchPath.Base);
            }
        }
    }
    return RemoveDirectoryW((wchar_t*)DirPath);
}

external bool
ChangeDirLocation(void* SrcPath, void* DstPath)
{
    // Assumes [SrcPath] and [DstPath] are in UTF-16LE.
    bool Result = MoveFileW((wchar_t*)SrcPath, (wchar_t*)DstPath);
    return Result;
}

//========================================
// Timing
//========================================

external void
StartTiming(timing* Info)
{
    LARGE_INTEGER Counter;
    QueryPerformanceCounter(&Counter);
    Info->Start = (isz)Counter.QuadPart;
}

external void
StopTiming(timing* Info)
{
    LARGE_INTEGER Counter;
    QueryPerformanceCounter(&Counter);
    Info->End = (isz)Counter.QuadPart;
    Info->Diff = (f64)(Info->End - Info->Start)/gSysInfo.TimingFreq;
}

external datetime
CurrentSystemTime(void)
{
    SYSTEMTIME SystemTime;
    GetSystemTime(&SystemTime);
    
    datetime TimeFormat = { SystemTime.wYear, SystemTime.wMonth, SystemTime.wDay,
        SystemTime.wHour, SystemTime.wMinute, SystemTime.wSecond,
        (weekday)SystemTime.wDayOfWeek };
    
    return TimeFormat;
}

external datetime
CurrentLocalTime(void)
{
    SYSTEMTIME SystemTime;
    GetLocalTime(&SystemTime);
    
    datetime TimeFormat = { SystemTime.wYear, SystemTime.wMonth, SystemTime.wDay,
        SystemTime.wHour, SystemTime.wMinute, SystemTime.wSecond,
        (weekday)SystemTime.wDayOfWeek };
    
    return TimeFormat;
}

//========================================
// External Libraries
//========================================

external file
LoadExternalLibrary(void* LibPath)
{
    file Library = (file)LoadLibraryW((wchar_t*)LibPath);
    return Library;
}

external void*
LoadExternalSymbol(file Library, char* SymbolName)
{
    void* Symbol = (void*)GetProcAddress((HMODULE)Library, SymbolName);
    return Symbol;
}

external bool
UnloadExternalLibrary(file Library)
{
    return FreeLibrary((HMODULE)Library);
}

//========================================
// Threading
//========================================

external thread
InitThread(thread_proc ThreadProc, void* ThreadArg, bool Waitable)
{
    // [Waitable] does nothing on Windows.
    HANDLE Thread = CreateThread(NULL, Megabyte(2), (LPTHREAD_START_ROUTINE)ThreadProc,
                                 ThreadArg, 0, 0);
    thread Result = { (file)Thread };
    return Result;
}

external bool
ChangeThreadScheduling(thread* Thread, int NewScheduling)
{
    DWORD Priority = NewScheduling * NORMAL_PRIORITY_CLASS;
    return SetPriorityClass((HANDLE)Thread->Handle, Priority);
}

external i32
GetThreadScheduling(thread Thread)
{
    switch (GetPriorityClass((HANDLE)Thread.Handle))
    {
        case ABOVE_NORMAL_PRIORITY_CLASS:
        case HIGH_PRIORITY_CLASS:
        case REALTIME_PRIORITY_CLASS: return SCHEDULE_HIGH;
        
        case BELOW_NORMAL_PRIORITY_CLASS:
        case IDLE_PRIORITY_CLASS:     return SCHEDULE_LOW;
        
        case NORMAL_PRIORITY_CLASS:   return SCHEDULE_NORMAL;
        default:                      return SCHEDULE_UNKNOWN;
    }
}

external bool
CloseThread(thread* Thread)
{
    // Function currently is a stub, to be expanded in the future.
    CloseHandle((HANDLE)Thread->Handle);
    return true;
}

external bool
WaitOnThread(thread* Thread)
{
    if (WaitForSingleObject((HANDLE)Thread->Handle, INFINITE) != WAIT_FAILED)
    {
        return CloseThread(Thread);
    }
    return false;
}

external bool
KillThread(thread* Thread)
{
    if (TerminateThread((HANDLE)Thread->Handle, 0))
    {
        return CloseThread(Thread);
    }
    return false;
}


//========================================
// Synchronization
//========================================

external mutex
InitMutex(void)
{
    mutex Result = {0};
    *(HANDLE*)Result.Handle = CreateMutexA(NULL, FALSE, NULL);
    return Result;
}

external bool
CloseMutex(mutex* Mutex)
{
    BOOL Result = CloseHandle(*(HANDLE*)Mutex->Handle);
    return Result;
}

external bool
LockOnMutex(mutex* Mutex)
{
    DWORD Result = WaitForSingleObject(*(HANDLE*)Mutex->Handle, INFINITE);
    return (Result != WAIT_FAILED);
}

external bool
UnlockMutex(mutex* Mutex)
{
    BOOL Result = ReleaseMutex(*(HANDLE*)Mutex->Handle);
    return Result;
}

external semaphore
InitSemaphore(i32 InitCount)
{
    semaphore Result = {0};
    *(HANDLE*)Result.Handle = CreateSemaphoreA(0, InitCount, I32_MAX, NULL);
    return Result;
}

external bool
CloseSemaphore(semaphore* Semaphore)
{
    BOOL Result = CloseHandle(*(HANDLE*)Semaphore->Handle);
    return Result;
}

external bool
IncreaseSemaphore(semaphore* Semaphore)
{
    BOOL Result = ReleaseSemaphore(*(HANDLE*)Semaphore->Handle, 1, NULL);
    return Result;
}

external bool
WaitOnSemaphore(semaphore* Semaphore)
{
    DWORD Result = WaitForSingleObject(*(HANDLE*)Semaphore->Handle, INFINITE);
    return (Result != WAIT_FAILED);
}
