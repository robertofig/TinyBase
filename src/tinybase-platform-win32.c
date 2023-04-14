#define _WINSOCKAPI_ // OBS: Prevents windows.h from including winsock.h, so winsock2.h can be used instead.
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
        else if (IsWindows8Point1OrGreater()) CopyData(gSysInfo.OSVersion, VerSize, "S12R2", 5);
        else if (IsWindows8OrGreater()) CopyData(gSysInfo.OSVersion, VerSize, "S12R1", 5);
        else if (IsWindows7OrGreater()) CopyData(gSysInfo.OSVersion, VerSize, "S08R2", 5);
        else if (IsWindowsVistaOrGreater()) CopyData(gSysInfo.OSVersion, VerSize, "S08R1", 5);
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
    if (Flags & MEM_GUARD) Access = PAGE_NOACCESS;
    else if (Flags & MEM_EXEC)
    {
        if (Flags & MEM_WRITE) Access = PAGE_EXECUTE_READWRITE;
        else if (Flags & MEM_READ) Access = PAGE_EXECUTE_READ;
        else Access = PAGE_EXECUTE;
    }
    else if (Flags & MEM_WRITE) Access = PAGE_READWRITE;
    else Access = PAGE_READONLY;
    DWORD AllocType = MEM_RESERVE | MEM_COMMIT | ((Flags & MEM_HUGEPAGE) ? MEM_LARGE_PAGES : 0);
    
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
    bool Read = (Flags & (READ_SOLO|READ_SHARE)) > 0;
    bool Write = (Flags & (WRITE_SOLO|WRITE_SHARE)) > 0;
    bool Append = (Flags & APPEND_FILE) > 0;
    DWORD AccessRights = ((GENERIC_READ) * Read) | ((GENERIC_WRITE) * Write) | ((FILE_APPEND_DATA) * Append);
    
    bool ReadShare = (Flags & READ_SHARE) > 0;
    bool WriteShare = (Flags & WRITE_SHARE) > 0;
    bool DeleteShare = (Flags & DELETE_SHARE) > 0;
    DWORD ShareMode = (FILE_SHARE_READ * ReadShare) | (FILE_SHARE_WRITE * WriteShare)
        | (FILE_SHARE_DELETE * DeleteShare);
    
    bool Async = (Flags & ASYNC_FILE) > 0;
    bool Hidden = (Flags & HIDDEN_FILE) > 0;
    DWORD Attributes = (FILE_FLAG_OVERLAPPED * Async) | (FILE_ATTRIBUTE_HIDDEN * Hidden);
    
    file File = (file)CreateFileW((wchar_t*)Filename, AccessRights, ShareMode, NULL, CreationMode,
                                  Attributes, NULL);
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

external b32
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
        if (Error == ERROR_FILE_NOT_FOUND
            || Error == ERROR_PATH_NOT_FOUND) return 2;
        return 0;
    }
    return 1;
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

external b32
SeekFile(file File, usz Pos)
{
    LONG PosHi = (LONG)(Pos >> 32);
    DWORD Result = SetFilePointer((HANDLE)File, Pos & 0xFFFFFFFF, &PosHi, FILE_BEGIN);
    return Result != INVALID_SET_FILE_POINTER;
}

external b32
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

external b32
ReadFileAsync(file File, buffer* Dst, usz AmountToRead, async* Async)
{
    if (AmountToRead <= (Dst->Size - Dst->WriteCur))
    {
        OVERLAPPED* Overlapped = (OVERLAPPED*)&Async->Data;
        
        usz ReadChunk = Min(AmountToRead, U32_MAX);
        u8* Ptr = Dst->Base + Dst->WriteCur;
        for (usz AmountRead = 0; AmountRead < AmountToRead; )
        {
            DWORD BytesToRead = (DWORD)Min(AmountToRead - AmountRead, ReadChunk);
            DWORD BytesRead = 0;
            if (!ReadFile((HANDLE)File, Ptr, BytesToRead, &BytesRead, Overlapped)
                && GetLastError() != ERROR_IO_PENDING)
            {
                return false;
            }
            Ptr += BytesToRead;
            AmountRead += BytesToRead;
            Overlapped->Offset = AmountRead & 0xFFFFFFFF;
            Overlapped->OffsetHigh = (AmountRead >> 32) & 0xFFFFFFFF;
        }
        Dst->WriteCur += AmountToRead;
        return true;
    }
    return false;
}

external b32
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

external b32
WriteEntireFile(file File, buffer Content)
{
    SeekFile(File, 0);
    b32 Result = AppendToFile(File, Content);
    return Result;
}

external b32
WriteToFile(file File, buffer Content, usz StartPos)
{
    SeekFile(File, StartPos);
    b32 Result = AppendToFile(File, Content);
    return Result;
}

external b32
WriteFileAsync(file File, void* Src, usz AmountToWrite, async* Async)
{
    OVERLAPPED* Overlapped = (OVERLAPPED*)&Async->Data;
    
    usz WriteChunk = Min(AmountToWrite, U32_MAX);
    u8* Ptr = (u8*)Src;
    for (usz AmountWritten = 0; AmountWritten < AmountToWrite; )
    {
        DWORD BytesToWrite = (DWORD)Min(AmountToWrite - AmountWritten, WriteChunk);
        DWORD BytesWritten = 0;
        if (!WriteFile((HANDLE)File, Ptr, BytesToWrite, &BytesWritten, Overlapped)
            && GetLastError() != ERROR_IO_PENDING)
        {
            return false;
        }
        Ptr += BytesToWrite;
        AmountWritten += BytesToWrite;
        Overlapped->Offset = AmountWritten & 0xFFFFFFFF;
        Overlapped->OffsetHigh = (AmountWritten >> 32) & 0xFFFFFFFF;
    }
    
    return true;
}

external u32
WaitOnIoCompletion(file File, async* Async)
{
    OVERLAPPED* Overlapped = (OVERLAPPED*)&Async->Data;
    DWORD BytesTransferred;
    GetOverlappedResult((HANDLE)File, Overlapped, &BytesTransferred, TRUE);
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

external b32
FilesAreEqual(file A, file B)
{
    b32 Result = 0;
    
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
        else
        {
            Result = 2;
        }
    }
    
    return Result;
}

external b32
DuplicateFile(void* SrcPath, void* DstPath, bool OverwriteIfExists)
{
    if (!CopyFileW((wchar_t*)SrcPath, (wchar_t*)DstPath, !OverwriteIfExists))
    {
        int Error = GetLastError();
        if (Error == ERROR_FILE_NOT_FOUND) return 2;
        if (Error == ERROR_PATH_NOT_FOUND) return 3;
        if (Error == ERROR_FILE_EXISTS) return 4;
        return 0;
    }
    return 1;
}

external b32
ChangeFileLocation(void* SrcPath, void* DstPath)
{
    if (!MoveFileW((wchar_t*)SrcPath, (wchar_t*)DstPath))
    {
        int Error = GetLastError();
        if (Error == ERROR_FILE_NOT_FOUND) return 2;
        if (Error == ERROR_PATH_NOT_FOUND) return 3;
        if (Error == ERROR_ALREADY_EXISTS) return 4;
        return 0;
    }
    return 1;
}

external b32
IsFileHidden(void* Filename)
{
    DWORD Result = GetFileAttributesW((wchar_t*)Filename);
    if (Result == INVALID_FILE_ATTRIBUTES) return 2;
    return Result & FILE_ATTRIBUTE_HIDDEN;
}

//========================================
// Filesystem
//========================================

external b32
IsExistingPath(void* Path)
{
    // Assumes [Path] is in UTF-16LE.
    DWORD Attributes = GetFileAttributesW((wchar_t*)Path);
    return Attributes != INVALID_FILE_ATTRIBUTES;
}

external b32
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
    // Path [.Size] is set to 2 byte less than MAX_PATH_SIZE, so there's always room for \0.
    path Result = String((u8*)Mem, 0, MAX_PATH_SIZE - sizeof(wchar_t), EC_UTF16LE);
    return Result;
}

external path
PathLit(void* CString)
{
    // Assumes [CString] is in UTF-16LE.
    path Result = { 0, 0, 0, EC_UTF16LE };
    usz CStringSize = 2 * wcslen((wchar_t*)CString);
    if (CStringSize <= MAX_PATH_SIZE)
    {
        Result.Base = (char*)CString;
        Result.WriteCur = CStringSize;
    }
    return Result;
}

external b32
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
    return Path->WriteCur != CurrentWriteCur;
}

internal b32
_AppendPathToPath(path Src, path* Dst)
{
    b32 Result = true;
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
    while (ReadIdx < Src.WriteCur && Result == 1)
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

external b32
AppendPathToPath(path NewPart, path* Dst)
{
    if (NewPart.Enc == Dst->Enc
        && NewPart.WriteCur <= (Dst->Size - Dst->WriteCur))
    {
        return _AppendPathToPath(NewPart, Dst);
    }
    return false;
}

external b32
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

external b32
AppendDataToPath(void* NewPart, usz NewPartSize, path* Dst)
{
    // OBS: Assumes [NewPart] is in UTF-16LE.
    path NewPartPath = Path(NewPart);
    NewPartPath.WriteCur = NewPartSize;
    return _AppendPathToPath(NewPartPath, Dst);
}

external b32
AppendArrayToPath(void* NewPart, path* Dst)
{
    // OBS: Assumes [NewPart] is in UTF-16LE.
    path NewPartPath = PathLit(NewPart);
    return _AppendPathToPath(NewPartPath, Dst);
}

external b32
AppendCWDToPath(path* Dst)
{
    char CWD[MAX_PATH_SIZE] = {0};
    path CWDPath = Path(CWD);
    DWORD Result = 2 * GetCurrentDirectoryW(sizeof(CWD) / 2, (wchar_t*)CWD);
    
    if (Result == 0) return 2;
    else if (Result > MAX_PATH_SIZE) return 0;
    else CWDPath.WriteCur = Result;
    
    return _AppendPathToPath(CWDPath, Dst);
}

external b32
MakeDir(void* DirPath)
{
    // Assumes [DirPath] is in UTF-16LE.
    
    b32 Result = CreateDirectoryW((wchar_t*)DirPath, NULL);
    if (!Result)
    {
        int Error = GetLastError();
        if (Error == ERROR_ALREADY_EXISTS)
        {
            Result = 2;
        }
        else if (Error == ERROR_PATH_NOT_FOUND)
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

external b32
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

external _RECURSIVE_ b32
RemoveDir(void* DirPath, bool RemoveAllFiles)
{
    // Assumes [DirPath] is in UTF-16LE.
    if (RemoveAllFiles)
    {
        path RemovePath = PathLit(DirPath);
        iter_dir Iter = {0};
        InitIterDir(&Iter, RemovePath);
        
        while (ListFiles(&Iter))
        {
            path ScratchPath = Iter.AllFiles;
            MoveUpPath(&ScratchPath, 1);
            path Filename = PathLit(Iter.Filename);
            AppendPathToPath(Filename, &ScratchPath);
            
            if (Iter.IsDir)
            {
                if (!RemoveDir(ScratchPath.Base, true))
                {
                    return false;
                }
            }
            else
            {
                if (!RemoveFile(ScratchPath.Base))
                {
                    return false;
                }
            }
        }
    }
    return RemoveDirectoryW((wchar_t*)DirPath);
}

external b32
ChangeDirLocation(void* SrcPath, void* DstPath)
{
    // Assumes [SrcPath] and [DstPath] are in UTF-16LE.
    if (!MoveFileW((wchar_t*)SrcPath, (wchar_t*)DstPath))
    {
        int Error = GetLastError();
        if (Error == ERROR_FILE_NOT_FOUND) return 2;
        if (Error == ERROR_PATH_NOT_FOUND) return 3;
        if (Error == ERROR_ALREADY_EXISTS) return 4;
        return 0;
    }
    return 1;
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
        SystemTime.wHour, SystemTime.wMinute, SystemTime.wSecond, (weekday)SystemTime.wDayOfWeek };
    
    return TimeFormat;
}

external datetime
CurrentLocalTime(void)
{
    SYSTEMTIME SystemTime;
    GetLocalTime(&SystemTime);
    
    datetime TimeFormat = { SystemTime.wYear, SystemTime.wMonth, SystemTime.wDay,
        SystemTime.wHour, SystemTime.wMinute, SystemTime.wSecond, (weekday)SystemTime.wDayOfWeek };
    
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

external b32
UnloadExternalLibrary(file Library)
{
    return FreeLibrary((HMODULE)Library);
}

//========================================
// Threading
//========================================

external file
ThreadCreate(void* ThreadProc, void* ThreadArg, usz* ThreadId, bool CreateAndRun)
{
    DWORD Creation = !CreateAndRun * 4;
    file Thread = (file)CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadProc, ThreadArg, Creation, (LPDWORD)ThreadId);
    return Thread;
}

external b32
ThreadChangeScheduling(file Thread, int NewScheduling)
{
    DWORD Priority = NewScheduling * NORMAL_PRIORITY_CLASS;
    return SetPriorityClass((HANDLE)Thread, Priority);
}

external i32
ThreadGetScheduling(file Thread)
{
    switch (GetPriorityClass((HANDLE)Thread))
    {
        case ABOVE_NORMAL_PRIORITY_CLASS:
        case HIGH_PRIORITY_CLASS:
        case REALTIME_PRIORITY_CLASS: return SCHEDULE_HIGH;
        
        case BELOW_NORMAL_PRIORITY_CLASS:
        case IDLE_PRIORITY_CLASS:     return SCHEDULE_LOW;
        
        default:                      return SCHEDULE_NORMAL;
    }
}

external void
ThreadExit(isz ExitCode)
{
    ExitThread((DWORD)ExitCode);
}

external b32
ThreadSuspend(file Thread)
{
    DWORD Result = SuspendThread((HANDLE)Thread);
    return (Result >= 0) ? true : false;
}

external bool
ThreadUnsuspend(file Thread)
{
    DWORD Result = ResumeThread((HANDLE)Thread);
    return (Result >= 0) ? true : false;
}

//========================================
// Atomic
//========================================

external i16
AtomicExchange16(void* volatile Dst, i16 Value)
{
    i32 Result = InterlockedExchange16((i16*)Dst, Value);
    return Result;
}

external i32
AtomicExchange32(void* volatile Dst, i32 Value)
{
    i32 Result = InterlockedExchange((long*)Dst, (long)Value);
    return Result;
}

external i64
AtomicExchange64(void* volatile Dst, i64 Value)
{
    i64 Result = InterlockedExchange64((i64*)Dst, Value);
    return Result;
}

external void*
AtomicExchangePtr(void* volatile* Dst, void* Value)
{
    void* Result = InterlockedExchangePointer(Dst, Value);
    return Result;
}
