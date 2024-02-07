#ifndef _GNU_SOURCE
# define _GNU_SOURCE
#endif

#include <aio.h>
#include <dlfcn.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/fs.h>
#include <linux/version.h>
#include <pthread.h>
#include <sched.h>
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/sysinfo.h>
#include <sys/utsname.h>
#include <sys/wait.h>
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
    
#if defined(CLOCK_BOOTTIME)
    gSysInfo.TimingFreq = CLOCK_BOOTTIME;
#else
    gSysInfo.TimingFreq = CLOCK_MONOTONIC;
#endif
    
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
_NewFile(void* Filename, bool IsCreate, i32 Flags)
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

external bool
RemoveFile(void* Filename)
{
    int Result = unlink((const char*)Filename);
    return !Result;
}

external usz
FileSizeOf(file File)
{
    usz Result = USZ_MAX;
    
    struct stat Stat;
    if (fstat((int)File, &Stat) == 0)
    {
        if (S_ISREG(Stat.st_mode))
        {
            Result = Stat.st_size;
        }
        else if (S_ISBLK(Stat.st_mode))
        {
            u64 Size;
            if (ioctl((int)File, BLKGETSIZE64, &Size) == 0)
            {
                Result = (usz)Size;
            }
        }
    }
    return Result;
}

external bool
SeekFile(file File, usz Pos)
{
    off_t Result = lseek((int)File, Pos, SEEK_SET);
    return (Result == Pos);
}

external bool
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

external bool
ReadFileAsync(file File, buffer* Dst, usz AmountToRead, usz StartPos, async* Async)
{
    if (AmountToRead <= (Dst->Size - Dst->WriteCur))
    {
        struct aiocb* Context = (struct aiocb*)Async->Data;
        Context->aio_fildes = (int)File;
        Context->aio_buf = (void*)Dst->Base;
        Context->aio_nbytes = AmountToRead;
        Context->aio_offset = (off_t)StartPos;
        
        if (aio_read(Context) == 0)
        {
            Dst->WriteCur += AmountToRead;
            return true;
        }
    }
    return false;
}

external bool
AppendToFile(file File, buffer Content)
{
    ssize_t BytesWritten = write((int)File, Content.Base, Content.WriteCur);
    return (BytesWritten == Content.WriteCur);
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
    struct aiocb* Context = (struct aiocb*)Async->Data;
    Context->aio_fildes = (int)File;
    Context->aio_buf = Src;
    Context->aio_nbytes = AmountToWrite;
    Context->aio_offset = (off_t)StartPos;
    
    bool Result = !aio_write(Context);
    return Result;
}

external usz
WaitOnIoCompletion(file File, async* Async, bool Block)
{
    struct aiocb* Context = (struct aiocb*)Async->Data;
    struct timespec* WaitTime = 0;
    if (!Block)
    {
        WaitTime = (struct timespec*)&Context[1];
        WaitTime->tv_sec = 0;
        WaitTime->tv_nsec = 0;
    }
    
    const struct aiocb* const CtxList[1] = { Context };
    if (aio_suspend(CtxList, 1, WaitTime) == 0)
    {
        usz BytesTransferred = aio_return(Context);
        return BytesTransferred;
    }
    
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
    bool Result = false;
    
    file SrcFile = OpenFileHandle(SrcPath, READ_SHARE);
    if (SrcFile != INVALID_FILE)
    {
        i32 Flags = (OverwriteIfExists) ? WRITE_SHARE|FORCE_CREATE : WRITE_SHARE;
        file DstFile = CreateNewFile(DstPath, Flags);
        if (DstFile != INVALID_FILE)
        {
            usz Size = FileSizeOf(SrcFile);
            ssize_t BytesWritten = syscall(SYS_copy_file_range, (int)SrcFile, 0,
                                           (int)DstFile, 0, Size, 0);
            CloseFileHandle(SrcFile);
            CloseFileHandle(DstFile);
            Result = (BytesWritten == Size);
        }
        CloseFileHandle(SrcFile);
    }
    
    return Result;
}

external bool
IsFileHidden(void* Filename)
{
    bool Result = false;
    
    if (IsExistingPath(Filename))
    {
        string File = String(Filename, 0, 0, EC_UTF8);
        File.WriteCur = StringLen(File, LEN_CSTRING);
        
        usz Slash = CharInString('/', File, RETURN_IDX_AFTER|SEARCH_REVERSE);
        char FirstByte = (Slash != INVALID_IDX) ? File.Base[Slash] : File.Base[0];
        
        Result = (FirstByte == '.');
    }
    
    return Result;
}

//========================================
// Filesystem
//========================================

external bool
IsExistingPath(void* Filepath)
{
    int Result = access(Filepath, F_OK);
    return !Result;
}

external bool
IsExistingDir(void* Filepath)
{
    struct stat PathStat;
    stat(Filepath, &PathStat);
    return S_ISDIR(PathStat.st_mode);
}

external path
Path(void* Mem)
{
    // Expects [Mem] to have at least MAX_PATH_SIZE of size.
    path Result = String(Mem, 0, MAX_PATH_SIZE - sizeof(char), EC_UTF8);
    return Result;
}

external path
PathCString(void* CString)
{
    // Assumes [CString] is in UTF-8.
    path Result = { CString, 0, 0, EC_UTF8 };
    usz CStringSize = strlen((char*)CString);
    Result.WriteCur = Min(CStringSize, MAX_PATH_SIZE);
    return Result;
}

external bool
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
    return (Path->WriteCur != CurrentWriteCur);
}

internal bool
_AppendPathToPath(path Src, path* Dst)
{
    bool Result = true;
    path Tmp = *Dst;
    
    if (Tmp.WriteCur > 0
        && Tmp.Base[Tmp.WriteCur-1] != '/')
    {
        if (!AppendCharToString('/', &Tmp)) return false;
    }
    
    if (Dst->WriteCur > 0
        && Src.Base[0] == '/')
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

external bool
AppendDataToPath(void* NewPart, usz NewPartSize, path* Dst)
{
    // OBS: Assumes [NewPart] is UTF-8 compatible.
    path NewPartPath = Path(NewPart);
    NewPartPath.WriteCur = NewPartSize;
    return _AppendPathToPath(NewPartPath, Dst);
}

external bool
AppendArrayToPath(void* NewPart, path* Dst)
{
    // OBS: Assumes [NewPart] is UTF-8 compatible.
    path NewPartPath = PathCString(NewPart);
    return _AppendPathToPath(NewPartPath, Dst);
}

external bool
AppendCWDToPath(path* Dst)
{
    bool Result = false;
    
    char CWD[MAX_PATH_SIZE] = {0};
    path CWDPath = Path(CWD);
    if (getcwd(CWD, sizeof(CWD)))
    {
        CWDPath.WriteCur = StringLen(CWDPath, LEN_CSTRING);
        Result = _AppendPathToPath(CWDPath, Dst);
    }
    
    return Result;
}

external bool
MakeDir(void* DirPath)
{
    int Result = mkdir((char*)DirPath, S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH);
    if (Result == -1)
    {
        if (errno == ENOENT)
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
                    return false;
                }
                WorkPath.Base[Slash] = '/';
            }
            
            if (WorkPath.WriteCur > 0)
            {
                Result = mkdir((char*)DirPath, S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH);
            }
        }
    }
    return !Result;
}

external void
InitIterDir(iter_dir* Iter, path DirPath)
{
    ClearBuffer(&Iter->AllFiles.Buffer);
    Iter->AllFiles = Path(Iter->AllFilesBuf);
    AppendStringToString(DirPath, &Iter->AllFiles);
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

external bool
ListFiles(iter_dir* Iter)
{
    DIR** Dir = (DIR**)&Iter->OSData[0];
    if (*Dir == 0)
    {
        *Dir = opendir(Iter->AllFilesBuf);
    }
    struct dirent** EntryPtr = (struct dirent**)&Dir[1];
    struct dirent* Entry = (struct dirent*)&EntryPtr[1];
    
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

#pragma GCC diagnostic pop

external bool
RemoveDir(void* DirPath, bool RemoveAllFiles)
{
    // Assumes [DirPath] is UTF-8 compatible.
    if (RemoveAllFiles)
    {
        path RemovePath = PathCString(DirPath);
        iter_dir Iter = {0};
        InitIterDir(&Iter, RemovePath);
        
        while (ListFiles(&Iter))
        {
            path ScratchPath = Iter.AllFiles;
            path Filename = PathCString(Iter.Filename);
            AppendPathToPath(Filename, &ScratchPath);
            
            bool Result = (Iter.IsDir) ? RemoveDir(ScratchPath.Base, true) : RemoveFile(ScratchPath.Base);
            MoveUpPath(&ScratchPath, 1);
            
            if (!Result) return false;
        }
    }
    return !rmdir((const char*)DirPath);
}

external bool
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

external bool
ChangeDirLocation(void* SrcPath, void* DstPath)
{
    int Result = rename((const char*)SrcPath, (const char*)DstPath);
    return !Result;
}

//========================================
// Timing
//========================================

external void
StartTiming(timing* Info)
{
    struct timespec Now;
    clock_gettime(gSysInfo.TimingFreq, &Now);
    Info->Start = (isz)Now.tv_sec * 1000000000 + (isz)Now.tv_nsec;
}

external void
StopTiming(timing* Info)
{
    struct timespec Now;
    clock_gettime(gSysInfo.TimingFreq, &Now);
    Info->End = (isz)Now.tv_sec * 1000000000 + (isz)Now.tv_nsec;
    Info->Diff = (f64)(Info->End - Info->Start) / 1000000000;
}

external datetime
CurrentSystemTime(void)
{
    time_t Now = time(NULL);
    struct tm DT = {0};
    gmtime_r(&Now, &DT);
    
    datetime Result = { DT.tm_year+1900, DT.tm_mon+1, DT.tm_mday, DT.tm_hour, DT.tm_min, DT.tm_sec, (weekday)DT.tm_wday };
    return Result;
}

external datetime
CurrentLocalTime(void)
{
    time_t Now = time(NULL);
    struct tm DT = {0};
    localtime_r(&Now, &DT);
    
    datetime Result = { DT.tm_year+1900, DT.tm_mon+1, DT.tm_mday, DT.tm_hour, DT.tm_min, DT.tm_sec, (weekday)DT.tm_wday };
    return Result;
}

//========================================
// External Libraries
//========================================

external file
LoadExternalLibrary(void* LibPath)
{
    file Result = (file)dlopen((const char*)LibPath, RTLD_NOW);
    return Result;
}

external void*
LoadExternalSymbol(file Library, char* SymbolName)
{
    void* Result = dlsym((void*)Library, SymbolName);
    return Result;
}

external bool
UnloadExternalLibrary(file Library)
{
    bool Result = !dlclose((void*)Library);
    return Result;
}

//========================================
// Threading
//========================================

// TODO: Change method for getting tid to something more reliable.
internal pid_t
_GetThreadID(file ThreadHandle)
{
    // This struct is only to cast pthread_t and get the tid.
    struct _pthread_hack_
    {
        u8 _Padding[720];
        pid_t ThreadID;
    };
    
    struct _pthread_hack_* PThread = (struct _pthread_hack_*)ThreadHandle;
    pid_t TID = PThread->ThreadID;
    
    return TID;
}

external thread
InitThread(thread_proc ThreadProc, void* ThreadArg, bool Waitable)
{
    thread Result = {0};
    
    int Detach = (Waitable) ? PTHREAD_CREATE_JOINABLE : PTHREAD_CREATE_DETACHED;
    pthread_attr_t Attr = {0};
    pthread_t Thread = 0;
    if (!pthread_attr_init(&Attr)
        && !pthread_attr_setdetachstate(&Attr, Detach)
        && !pthread_attr_setguardsize(&Attr, gSysInfo.PageSize)
        && !pthread_attr_setstacksize(&Attr, Megabyte(2))
        && !pthread_create(&Thread, &Attr, ThreadProc, ThreadArg))
    {
        Result.Handle = (file)Thread;
    }
    
    return Result;
}

external bool
ChangeThreadScheduling(thread* Thread, int NewScheduling)
{
    int Priority;
    switch (NewScheduling)
    {
        case SCHEDULE_HIGH: Priority = -20; break;
        case SCHEDULE_LOW: Priority = 19; break;
        default: Priority = 0;
    }
    int Error = setpriority(PRIO_PROCESS, _GetThreadID(Thread->Handle), Priority);
    return !Error;
}

external i32
GetThreadScheduling(thread Thread)
{
    int Priority = getpriority(PRIO_PROCESS, _GetThreadID(Thread.Handle));
    i32 Result = (Priority >= 7) ? SCHEDULE_LOW : (Priority < -7) ? SCHEDULE_HIGH : SCHEDULE_NORMAL;
    return Result;
}

external bool
CloseThread(thread* Thread)
{
    // Function currently is a stub, to be expanded in the future.
    Thread->Handle = 0;
    return true;
}

external bool
WaitOnThread(thread* Thread)
{
    if (!pthread_join((pthread_t)Thread->Handle, 0))
    {
        return CloseThread(Thread);
    }
    return false;
}

external bool
KillThread(thread* Thread)
{
    if (!pthread_kill((pthread_t)Thread->Handle, SIGKILL))
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
    pthread_mutex_init((pthread_mutex_t*)Result.Handle, NULL);
    return Result;
}

external bool
CloseMutex(mutex* Mutex)
{
    int Result = pthread_mutex_destroy((pthread_mutex_t*)Mutex->Handle);
    return (Result == 0);
}

external bool
LockOnMutex(mutex* Mutex)
{
    int Result = pthread_mutex_lock((pthread_mutex_t*)Mutex->Handle);
    return (Result == 0);
}

external bool
UnlockMutex(mutex* Mutex)
{
    int Result = pthread_mutex_unlock((pthread_mutex_t*)Mutex->Handle);
    return (Result == 0);
}

external semaphore
InitSemaphore(i32 InitCount)
{
    semaphore Result = {0};
    sem_init((sem_t*)Result.Handle, 0, InitCount);
    return Result;
}

external bool
CloseSemaphore(semaphore* Semaphore)
{
    int Result = sem_destroy((sem_t*)Semaphore->Handle);
    return (Result == 0);
}

external bool
IncreaseSemaphore(semaphore* Semaphore)
{
    int Result = sem_post((sem_t*)Semaphore->Handle);
    return (Result == 0);
}

external bool
WaitOnSemaphore(semaphore* Semaphore)
{
    int Result = sem_wait((sem_t*)Semaphore->Handle);
    return Result;
}
