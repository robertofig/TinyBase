#include <fcntl.h>
#include <stdlib.h>
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
    int OpenFlags = 0, Mode = 0;
    if (IsCreate)
    {
        OpenFlags |= O_CREAT;
        if (OpenFlags & FORCE_CREATE) OpenFlags |= O_TRUNC;
        Mode = S_IRUSR|S_IWUSR;
    }
    bool Read = (Flags & (READ_SOLO|READ_SHARE)) > 0;
    bool Write = (Flags & (WRITE_SOLO|WRITE_SHARE)) > 0;
    
    if (Read && Write) OpenFlags |= O_RDWR;
    else if (Read) OpenFlags |= O_RDONLY;
    else if (Write) OpenFlags |= O_WRONLY;
    if (Flags & APPEND_FILE) OpenFlags |= O_APPEND;
    if (Flags & ASYNC_FILE) OpenFlags |= (O_ASYNC|O_NONBLOCK);
    
    file File = open(Filename, OpenFlags, Mode);
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