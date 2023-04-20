#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/sysinfo.h>
#include <sys/utsname.h>
#include <time.h>
#include <unistd.h>

//========================================
// Config
//========================================

external void
LoadSystemInfo(void)
{
    struct sysinfo SysInfo;
    sysinfo(&SysInfo);
    gSysInfo.PageSize = getpagesize();
    gSysInfo.AddressRange[0] = 0; // TODO: get value from mmap_min_addr.
    gSysInfo.AddressRange[1] = SysInfo.totalram; // TODO: mmap_max_addr? 
    gSysInfo.MemBlockSize = gSysInfo.PageSize;
    gSysInfo.NumThreads = get_nprocs();
    gSysInfo.TimingFreq = 1.0;
    
    struct utsname OSInfo;
    uname(&OSInfo);
    gSysInfo.OSVersion[0] = 'L';
    usz VerSize = sizeof(gSysInfo.OSVersion);
    CopyData(gSysInfo.OSVersion+1, VerSize-1, OSInfo.release, 4);
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
    
    int Prot = 0;
    if (Flags & MEM_READ) Prot |= PROT_READ;
    if (Flags & MEM_WRITE) Prot |= PROT_READ | PROT_WRITE;
    if (Flags & MEM_EXEC) Prot |= PROT_EXEC;
    if (Flags & MEM_GUARD) Prot = PROT_NONE;
    void* Ptr = mmap(Address, Size, Prot, MAP_PRIVATE|MAP_ANONYMOUS, 0, 0);
    if (Ptr != MAP_FAILED)
    {
        Result.Base = (u8*)Ptr;
        Result.Size = (gSysInfo.PageSize) ? Align(Size, gSysInfo.PageSize) : Size;
    }
    
    return Result;
}

external void
FreeMemory(buffer* Mem)
{
    munmap(Mem->Base, Mem->Size);
    memset(Mem, 0, sizeof(buffer));
}

external buffer
GetMemoryFromHeap(usz Size)
{
    buffer Result = {0};
    void* Ptr = calloc(1, Size);
    if (Ptr)
    {
        Result.Base = (u8*)Ptr;
        Result.Size = Size;
    }
    return Result;
}

external void
FreeMemoryFromHeap(buffer* Mem)
{
    if (Mem->Base)
    {
        free(Mem->Base);
    }
    memset(Mem, 0, sizeof(buffer));
}


//========================================
// FileIO
//========================================

internal file
_NewFile(void* Filename, b32 IsCreate, i32 Flags)
{
    int OpenOpts = 0, Mode = 0;
    if (IsCreate)
    {
        OpenOpts |= O_CREAT;
        OpenOpts |= (Flags & FORCE_CREATE) ? O_TRUNC : O_EXCL;
        Mode = S_IRUSR|S_IWUSR;
    }
    bool Read = (Flags & (READ_SOLO|READ_SHARE)) > 0;
    bool Write = (Flags & (WRITE_SOLO|WRITE_SHARE)) > 0;
    
    if (Read && Write) OpenOpts |= O_RDWR;
    else if (Read) OpenOpts |= O_RDONLY;
    else if (Write) OpenOpts |= O_WRONLY;
    if (Flags & APPEND_FILE) OpenOpts |= O_APPEND;
    if (Flags & ASYNC_FILE) OpenOpts |= (O_ASYNC|O_NONBLOCK);
    
    char NewBuf[MAX_PATH_SIZE];
    if (Flags & HIDDEN_FILE)
    {
        string New = String(NewBuf, 0, MAX_PATH_SIZE, EC_UTF8);
        ClearMemory(&New.Buffer);
        string Old = String((char*)Filename, 0, MAX_PATH_SIZE, EC_UTF8);
        Old.WriteCur = StringLen(Old, LEN_CSTRING);
        
        usz Slash = CharInString('/', Old, RETURN_IDX_AFTER|SEARCH_REVERSE);
        if (Slash != INVALID_IDX)
        {
            AppendDataToString(Old.Base, Slash, &New);
        }
        if (Old.Base[Slash] != '.')
        {
            New.Base[New.WriteCur++] = '.';
        }
        AppendDataToString(Old.Base+Slash, Old.WriteCur-Slash, &New);
        
        Filename = (void*)NewBuf;
    }
    
    file File = open(Filename, OpenOpts, Mode);
    return File;
}

external file
CreateNewFile(void* Filename, i32 Flags)
{
    file Result = _NewFile(Filename, 1, Flags);
    return Result;
}

external file
OpenFileHandle(void* Filename, i32 Flags)
{
    file Result = _NewFile(Filename, 0, Flags);
    if ((Flags & FORCE_OPEN) && Result == INVALID_FILE)
    {
        Result = _NewFile(Filename, 1, Flags);
    }
    return Result;
}

external void
CloseFileHandle(file File)
{
    close((int)File);
}

external b32
RemoveFile(void* Filename)
{
    int Result = unlink((const char*)Filename);
    if (Result == -1 && errno == ENOENT) return 2;
    else return !Result;
}

external usz
FileSizeOf(file File)
{
    struct stat FileStat;
    fstat((int)File, &FileStat);
    usz Result = FileStat.st_size;
    return Result;
}

external b32
SeekFile(file File, usz Pos)
{
    off_t Result = lseek((int)File, Pos, SEEK_SET);
    return (Result == Pos);
}

external b32
ReadFromFile(file File, buffer* Dst, usz AmountToRead, usz StartPos)
{
    usz FileSize = FileSizeOf(File);
    if ((StartPos + AmountToRead) <= FileSize
        && AmountToRead <= (Dst->Size - Dst->WriteCur))
    {
        SeekFile(File, StartPos);
        ssize_t BytesRead = read((int)File, Dst->Base + Dst->WriteCur, AmountToRead);
        if (BytesRead != AmountToRead)
        {
            return false;
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
    buffer Mem = GetMemory(FileSize, 0, MEM_WRITE);
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
    return 0;
}

external b32
AppendToFile(file File, buffer Content)
{
    ssize_t BytesWritten = write((int)File, Content.Base, Content.WriteCur);
    return (BytesWritten == Content.WriteCur);
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
    return 0;
}

external u32
WaitOnIoCompletion(file File, async* Async)
{
    return 0;
}

external usz
FileLastWriteTime(file File)
{
    struct stat FileStat;
    fstat((int)File, &FileStat);
    usz Result = FileStat.st_mtim.tv_nsec;
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
    file SrcFile = OpenFileHandle(SrcPath, READ_SHARE);
    if (SrcFile != INVALID_FILE)
    {
        i32 Flags = (OverwriteIfExists) ? WRITE_SHARE|FORCE_CREATE : WRITE_SHARE;
        file DstFile = CreateNewFile(DstPath, Flags);
        if (DstFile != INVALID_FILE)
        {
            usz Size = FileSizeOf(SrcFile);
            ssize_t BytesWritten = copy_file_range(SrcFile, 0, DstFile, 0, Size, 0);
            CloseFileHandle(SrcFile);
            CloseFileHandle(DstFile);
            return (BytesWritten == Size);
        }
        CloseFileHandle(SrcFile);
        
        if (errno == EEXIST) return 4;
        if (errno == ENOTDIR) return 3;
        return 0;
    }
    return 2;
}

external b32
ChangeFileLocation(void* SrcPath, void* DstPath)
{
    int Result = rename((const char*)SrcPath, (const char*)DstPath);
    if (Result == -1 && errno == EXDEV)
    {
        if (DuplicateFile(SrcPath, DstPath, false))
        {
            return RemoveFile(SrcPath);
        }
    }
    return !Result;
}

external b32
IsFileHidden(void* Filename)
{
    if (IsExistingPath(Filename))
    {
        string File = String(Filename, 0, 0, EC_UTF8);
        File.WriteCur = StringLen(File, LEN_CSTRING);
        
        usz Slash = CharInString('/', File, RETURN_IDX_AFTER|SEARCH_REVERSE);
        char FirstByte = (Slash != INVALID_IDX) ? File.Base[Slash] : File.Base[0];
        
        return FirstByte == '.';
    }
    return 0;
}

//========================================
// Filesystem
//========================================

external b32
IsExistingPath(void* Filepath)
{
    int Result = access(Filepath, F_OK);
    return !Result;
}

external b32
IsExistingDir(void* Filepath)
{
    struct stat PathStat;
    stat(Filepath, &PathStat);
    return S_ISDIR(PathStat.st_mode);
}

external path
Path(void* Mem)
{
    // Path [.Size] is set to 1 byte less than MAX_PATH_SIZE, so there's always room for \0.
    path Result = String((u8*)Mem, 0, MAX_PATH_SIZE-1, EC_UTF8);
    return Result;
}

external path
PathLit(void* CString)
{
    // Assumes [CString] is in UTF-16LE.
    path Result = { 0, 0, 0, EC_UTF8 };
    usz CStringSize = strlen((char*)CString);
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
    if (Path->Base[Path->WriteCur-1] == '/') { Path->WriteCur--; }
    usz CurrentWriteCur = Path->WriteCur;
    
    for (usz Count = 0; Count < MoveUpCount; Count++)
    {
        usz BackslashIdx = CharInString('/', *Path, RETURN_IDX_FIND|SEARCH_REVERSE);
        if (BackslashIdx == INVALID_IDX)
        {
            Path->WriteCur = 0;
            break;
        }
        Path->WriteCur = BackslashIdx;
    }
    
    if (Path->WriteCur > 0) { Path->WriteCur++; }
    return Path->WriteCur != CurrentWriteCur;
}

internal b32
_AppendPathToPath(path Src, path* Dst)
{
    b32 Result = true;
    path Tmp = *Dst;
    
    if (Tmp.WriteCur > 0
        && Tmp.Base[Tmp.WriteCur-1] != '/')
    {
        if (!AppendCharToString('/', &Tmp)) return false;
    }
    
    if (Src.Base[0] == '/')
    {
        AdvanceBuffer(&Src.Buffer, 1);
    }
    
    char C0 = 0, C1 = 0, C2 = 0;
    usz ReadIdx = 0, DirCharCount = 0, ReadStart = 0;
    for (; ReadIdx < Src.WriteCur && Result == 1; ReadIdx++)
    {
        C0 = C1;
        C1 = C2;
        C2 = Src.Base[ReadIdx];
        
        if (C2 == '/')
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
                if (!AppendCharToString('/', &Tmp)) return false;
            }
            DirCharCount = 0;
            ReadStart = ReadIdx + 1;
        }
        else
        {
            DirCharCount++;
        }
    }
    
    // Does a final copy in case [NewPart] does not end in '/'.
    if (DirCharCount > 0)
    {
        string Dir = String(Src.Base + ReadStart, ReadIdx - ReadStart, 0, Src.Enc);
        if (!AppendStringToString(Dir, &Tmp)) return false;
    }
    
    // Makes sure last byte is 0, so array can be passed to path-reading functions.
    Tmp.Base[Tmp.WriteCur] = 0;
    
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
        char NewPartUTF8[MAX_PATH_SIZE] = {0};
        path Src = Path(NewPartUTF8);
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
    // OBS: Assumes [NewPart] is UTF-8 compatible.
    path NewPartPath = Path(NewPart);
    NewPartPath.WriteCur = NewPartSize;
    return _AppendPathToPath(NewPartPath, Dst);
}

external b32
AppendArrayToPath(void* NewPart, path* Dst)
{
    // OBS: Assumes [NewPart] is UTF-8 compatible.
    path NewPartPath = PathLit(NewPart);
    return _AppendPathToPath(NewPartPath, Dst);
}

external b32
AppendCWDToPath(path* Dst)
{
    char CWD[MAX_PATH_SIZE] = {0};
    path CWDPath = Path(CWD);
    
    if (getcwd(CWD, sizeof(CWD)))
    {
        CWDPath.WriteCur = StringLen(CWDPath, LEN_CSTRING);
        return _AppendPathToPath(CWDPath, Dst);
    }
    return 2; // System error.
}

external b32
MakeDir(void* DirPath)
{
    int Result = !mkdir((char*)DirPath, S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH);
    if (!Result)
    {
        if (errno == EEXIST)
        {
            Result = 2;
        }
        else if (errno == ENOENT)
        {
            char WorkPathBuf[MAX_PATH_SIZE] = {0};
            path WorkPath = Path(WorkPathBuf);
            AppendArrayToPath(DirPath, &WorkPath);
            
            for (usz Slash
                 ; (Slash = CharInString('/', WorkPath, RETURN_IDX_FIND)) != INVALID_IDX
                 ; AdvanceBuffer(&WorkPath.Buffer, Slash+1))
            {
                WorkPath.Base[Slash] = 0;
                if (mkdir(WorkPathBuf, S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH) == -1
                    && errno != EEXIST)
                {
                    return 0;
                }
                WorkPath.Base[Slash] = '/';
            }
            
            if (WorkPath.WriteCur > 0)
            {
                Result = !mkdir((char*)DirPath, S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH);
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
    AppendStringToString(DirPath, &Iter->AllFiles);
}

external b32
ListFiles(iter_dir* Iter)
{
    DIR** Dir = (DIR**)&Iter->OSData[0];
    struct dirent** EntryPtr = (struct dirent**)&Dir[1];
    struct dirent* Entry = (struct dirent*)&EntryPtr[1];
    if (*Dir == 0)
    {
        *Dir = opendir(Iter->AllFilesBuf);
    }
    
    while (!readdir_r(*Dir, Entry, EntryPtr)
           && *EntryPtr)
    {
        if (Entry->d_name[0] == '.')
        {
            if (Entry->d_name[1] == 0
                || (Entry->d_name[1] == '.' && Entry->d_name[2] == 0))
            {
                continue;
            }
        }
        
        Iter->Filename = Entry->d_name;
        Iter->IsDir = Entry->d_type & DT_DIR;
        return true;
    }
    
    closedir(*Dir);
    *Dir = 0;
    return false;
}

external b32
RemoveDir(void* DirPath, bool RemoveAllFiles)
{
    // Assumes [DirPath] is UTF-8 compatible.
    if (RemoveAllFiles)
    {
        path RemovePath = PathLit(DirPath);
        iter_dir Iter = {0};
        InitIterDir(&Iter, RemovePath);
        
        while (ListFiles(&Iter))
        {
            path ScratchPath = Iter.AllFiles;
            path Filename = PathLit(Iter.Filename);
            AppendPathToPath(Filename, &ScratchPath);
            
            b32 Result = (Iter.IsDir) ? RemoveDir(ScratchPath.Base, true) : RemoveFile(ScratchPath.Base);
            MoveUpPath(&ScratchPath, 1);
            
            if (!Result) return false;
        }
    }
    return !rmdir((const char*)DirPath);
}

external b32
ChangeDirLocation(void* SrcPath, void* DstPath)
{
    int Result = rename((const char*)SrcPath, (const char*)DstPath);
    if (Result == -1)
    {
        if (!IsExistingDir(SrcPath)) return 2;
        if (errno == ENOENT) return 3;
        if (errno == ENOTEMPTY || errno == EEXIST) return 4;
    }
    return !Result;
}

//========================================
// Timing
//========================================

external void
StartTiming(timing* Info)
{
    return;
}

external void
StopTiming(timing* Info)
{
    return;
}

external datetime
CurrentSystemTime(void)
{
    return {0};
}

external datetime
CurrentLocalTime(void)
{
    return {0};
}

//========================================
// External Libraries
//========================================

external file
LoadExternalLibrary(void* LibPath)
{
    return 0;
}

external void*
LoadExternalSymbol(file Library, char* SymbolName)
{
    return 0;
}

external b32
UnloadExternalLibrary(file Library)
{
    return 0;
}

//========================================
// Threading
//========================================

external file
ThreadCreate(void* ThreadProc, void* ThreadArg, usz* ThreadId, bool CreateAndRun)
{
    return 0;
}

external b32
ThreadChangeScheduling(file Thread, int NewScheduling)
{
    return 0;
}

external i32
ThreadGetScheduling(file Thread)
{
    return 0;
}

external void
ThreadExit(isz ExitCode)
{
    return;
}

external b32
ThreadSuspend(file Thread)
{
    return 0;
}

external bool
ThreadUnsuspend(file Thread)
{
    return 0;
}

//========================================
// Atomic
//========================================

external i16
AtomicExchange16(void* volatile Dst, i16 Value)
{
    return 0;
}

external i32
AtomicExchange32(void* volatile Dst, i32 Value)
{
    return 0;
}

external i64
AtomicExchange64(void* volatile Dst, i64 Value)
{
    return 0;
}

external void*
AtomicExchangePtr(void* volatile* Dst, void* Value)
{
    return 0;
}
