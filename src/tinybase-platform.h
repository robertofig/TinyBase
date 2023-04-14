#ifndef TINYBASE_PLATFORM_H
//==========================================================================
// tinybase-platform.h
//
// Module to generalise platform-specific operations. Some of the functions
// are similar to ones provided by the C standard library, but most have no
// equivalent in it.
//
// The goal of this module is to provide an API that can quickly and easily
// turn code that builds upon it multiplatform.
//==========================================================================
#define TINYBASE_PLATFORM_H

#include "tinybase-types.h"
#include "tinybase-memory.h"
#include "tinybase-strings.h"

//========================================
// Config
//========================================

typedef struct sys_info
{
    usz PageSize;
    usz MemBlockSize;
    usz NumThreads;
    isz AddressRange[2];
    f64 TimingFreq;
    char OSVersion[8];
} sys_info;
global sys_info gSysInfo = {0};

external void LoadSystemInfo(void);

/* Reads system information and saves it to [gSysInfo] global variable.
 |  Must be called only once, before calling other functions in this library.
 |--- Return: nothing. */

//========================================
// Memory
//========================================

#define MEM_READ     0x1  // Marks memory pages for read.
#define MEM_WRITE    0x2  // Marks memory pages for read and write.
#define MEM_GUARD    0x4  // Marks memory pages as untouchable (takes precedence over others).
#define MEM_EXEC     0x8  // Marks memory pages as executable.
#define MEM_HUGEPAGE 0x10 // Reserves large memory pages, if the system supports.

external buffer GetMemory(usz Size, _opt void* Address, _opt int AccessFlags);

/* Allocates memory block of [Size] bytes, rounded up to system page size.
 |  Block is guaranteed to be zeroed. A start [Address] can optionally be passed
 |  (system will choose random address if this is NULL). [AccessFlags] determines
 |  memory block behaviour; if none is passed, block is set to read-only.
 |--- Return: buffer of allocated memory if successful, empty otherwise. */

external void ClearMemory(buffer* Mem);

/* Clears entire buffer in [Mem] to zero.
 |--- Return: nothing. */

external void FreeMemory(buffer* Mem);

/* Frees memory block allocated with GetMemory(). Frees the entire block at once.
 |--- Return: nothing. */

external buffer GetMemoryFromHeap(usz Size);

/* Gets a chunk of memory of [Size] bytes from the main process heap.
|--- Return: buffer of heap memory if successful, empty otherwise. */

external void FreeMemoryFromHeap(buffer* Mem);

/* Frees memory chunk allocated with GetMemoryFromHeap().
 |--- Return: nothing. */


//========================================
// FileIO
//========================================

#define READ_SOLO    0x1   // Open file with solo read access.
#define WRITE_SOLO   0x2   // Open file with solo write access.
#define READ_SHARE   0x4   // Open file with shared read access.
#define WRITE_SHARE  0x8   // Open file with shared write access.
#define DELETE_SHARE 0x10  // Open file with shared delete access.
#define HIDDEN_FILE  0x20  // Create file in hidden form.
#define ASYNC_FILE   0x40  // Open file in async IO mode.
#define APPEND_FILE  0x80  // Open file in write-append mode (must have write access).
#define FORCE_CREATE 0x100 // Overwrite existing file during creation.
#define FORCE_OPEN   0x200 // Create new file if one does not exist already.

typedef usz file;

typedef struct async
{
    u8 Data[ASYNC_DATA_SIZE]; // OVERLAPPED on Windows.
} async;

external file CreateNewFile(void* Filename, i32 Flags);

/* Creates and opens new file at path [Filename]. Path must be at the Unicode
 |  encoding native to the system (e.g. UTF16 on Windows, UTF8 on Linux).
 |  Creation flags must be passed to [Flags].
|--- Return: file handle if successful, or INVALID_FILE if not. */

external file OpenFileHandle(void* Filename, i32 Flags);

/* Opens file at path [Filename]. Path must be at the Unicode encoding native
 |  to the system (e.g. UTF16 on Windows, UTF8 on Linux). Open flags must be
 |  passed to [Flags].
|--- Return: file handle if successful, or INVALID_FILE if not.*/

external void CloseFileHandle(file File);

/* Closes file handle acquired with CreateNewFile() or OpenFileHandle().
 |--- Return: nothing. */

external buffer ReadEntireFile(file File);

/* Takes in an open [File] handle, allocated memory for it with read/write
|  access, and copies entire file content to it.
|--- Return: buffer with memory if successful, or empty buffer if not. */

external b32 ReadFromFile(file File, buffer* Dst, usz AmountToRead, usz StartPos);

/* Copies [AmountToRead] bytes from [File] at [StartPos] offset into [Dst]
|  memory. Memory must already be allocated. If [StartPos] + [AmountToRead]
|  goes beyond EOF, nothing is copied and function fails.
 |--- Return: 1 if read operation was successfully started, or 0 if not. */

external b32 ReadFileAsync(file File, buffer* Dst, usz AmountToRead, async* Async);

/* Reads file in non-blocking manner. Memory must already be allocated and
|  passed at [Dst], with at least [AmountToRead] size. Platform-specific
|  async data for async IO must be passed in [Async]. File is read in chunks
 |  of U32_MAX, so if [AmountToRead] is larger than that multiple reads will
 |  be posted.
|--- Return: 1 if read operation was successfully started, or 0 if not. */

external b32 AppendToFile(file File, buffer Content);

/* Writes data in [Content] at EOF of [File] (it needs not have been opened
 |  with APPEND_FILE flag).
 |--- Return: 1 if successful, or 0 if not. */

external b32 WriteEntireFile(file File, buffer Content);

/* Writes data in [Content] at beginning of [File]. If file was opened with
|  APPEND_FILE flag, writes at EOF.
|--- Return: 1 if successful, or 0 is not. */

external b32 WriteToFile(file File, buffer Content, usz StartPos);

/* Writes data in [Content] at [StartPos] of [File]. If file was opened with
|  APPEND_FILE flag, [StartPos] is ignored and it writes at EOF.
 |--- Return: 1 if successful, or 0 if not. */

external b32 WriteFileAsync(file File, void* Src, usz AmountToWrite, async* Async);

/* Writes [AmounttoWrite] bytes from [Src] at beginning of [File] in non-blocking
 |  manner. If file was opened with APPEND_FILE flag, writes at EOF. Platform-
|  -specific data for async IO must be passed in [Async]. File is written to in
 |  chunks of U32_MAX, so if [AmountToRead] is larger than that multiple reads
 |  will be posted.
 |--- Return: 1 if write operation was successfully started, or 0 if not. */

external u32 WaitOnIoCompletion(file File, async* Async);

/* Waits until an async IO operation done on [File] completes. [Async] is a
 |  pointer to the same async object used when start the IO.
|--- Return: number of bytes transferred, or 0 in case of failure. */

external usz FileLastWriteTime(file File);

/* Gets last time [File] was written to, in system units.
|--- Return: time of write if successful, or USZ_MAX if not. */

external usz FileSizeOf(file File);

/* Gets the size of [File] in bytes.
|--- Return: size of file if successful, or USZ_MAX if not. */

external b32 FilesAreEqual(file FileA, file FileB);

/* Given two open file handles, determines if the content of both files is equal.
 |--- Return: 1 if they are equal, 0 if not, and 2 in case of system error. */

external b32 IsFileHidden(void* Filename);

/* Checks if file in [Filename] is hidden. Path must be at the Unicode encoding native
 |  to the system (e.g. UTF16 on Windows, UTF8 on Linux).
|--- Return: 1 if hidden, 0 if not, and 2 in case of system error. */

external b32 DuplicateFile(void* SrcPath, void* DstPath, bool OverwriteIfExists);

/* Copies file in [Src] to new file in [Dst] in the file system. The entire
|  directory tree in [Dst] must already exist. Paths must be at the Unicode
 |  encoding native to the system (e.g. UTF16 on Windows, UTF8 on Linux).
|  [OverwriteIfExists] controls whether an existing file should be overwritten.
|--- Return: 1 if successful, 0 in case of system error, 2 if [Src] does not exist,
 |    3 if [Dst] directory does not exist, and 4 if [Dst] already existed (and was
|    not meant to be overwritten). */

external b32 RemoveFile(void* Filename);

/* Deletes file in [Src]. Path must be at the Unicode encoding native to the
 |  system (e.g. UTF16 on Windows, UTF8 on Linux).
|--- Return: 1 if successful, 0 in case of system error, 2 if [Src] does not exist. */

external b32 SeekFile(file File, usz Pos);

/* Sets the pointer of [File] to the offset [Pos]. [Pos] must not be greater
|  than file size, or else the function fails.
|--- Return: 1 if successful, or 0 if not. */


//========================================
// Filesystem
//========================================

typedef string path;

/* Type [path] is a string with MAX_PATH_SIZE [.Size] and in the system native
|  unicode encoding (e.g. UTF-16 on Windows, UTF-8 on Linux). */

external path Path(void* Mem);

/* Creates a path from [Mem] memory region, and sets [.WriteCur] to zero. [Mem] must
|  be at least MAX_PATH_SIZE long.
|--- Return: new path object.*/

external path PathLit(void* CString);

/* Creates a path form [CString] memory region, and sets [.WriteCur] to its length.
|  This length is the number of bytes until the zero-termination. If [CString] is
|  not zero-terminated, a buffer overflow may occur.
 |--- Return: new path object, or empty path if length cannot be determined. */

external b32 AppendArrayToPath(void* NewPart, path* Dst);

/* Appends a new sub-dir or file [NewPart] to [Dst]. [NewPart] needs not start or
|  end with path separators (slash or backslash), this is taken care of by the
|  function. [NewPart] is assumed to be in the same encoding as [Dst], and to be
|  zero-terminated. It also must fit entirely in [Dst].
|--- Return: 1 if successful, 0 if not. */

external b32 AppendDataToPath(void* NewPart, usz NewPartSize, path* Dst);

/* Same as AppendArrayToPath(), but with [NewPartSize] determining the size of
|  [NewPart], meaning it does not need to be zero-terminated.
|--- Return: 1 if successful, 0 if not. */

external b32 AppendPathToPath(path NewPart, path* Dst);

/* Appends [NewPart] to [Dst], so long as it fits entirely. [NewPart] must be
|  a valid path object, or else the function fails.
|--- Return: 1 if successful, 0 if not. */

external b32 AppendStringToPath(string NewPart, path* Dst);

/* Appends [NewPart] to [Dst], so long as it fits entirely. If [NewPart] is of
|  a different encoding than [Dst], it is transcoded to it.
|--- Return: 1 if successful, 0 if not. */

external b32 AppendCWDToPath(path* Dst);

/* Appends the current working directory to [Dst], so long as it fits entirely.
|--- Return: 1 if successful, 0 if not, and 2 if system error. */

external b32 ChangeFileLocation(void* Src, void* Dst);

/* Changes location of file in [Src] to [Dst] in the file system. The entire
|  directory tree in [Dst] must already exist. Paths must be at the Unicode
 |  encoding native to the system (e.g. UTF16 on Windows, UTF8 on Linux).
|--- Return: 1 if successful, 0 in case of system error, 2 if [Src] does not exist,
 |    3 if [Dst] directory does not exist, and 4 if [Dst] already existed. */

external b32 ChangeDirLocation(void* Src, void* Dst);

/* Same usage as ChangeFileLocation(), but with a directory as [Src].
|--- Return: 1 if successful, 0 in case of system error, 2 if [Src] does not exist,
 |    3 if [Dst] directory tree does not exist, and 4 if [Dst] already exists. */

external b32 IsExistingPath(void* Path);

/* Checks if [Path] points to an existing file or directory. Path must be at the
 |  Unicode encoding native to the system (e.g. UTF16 on Windows, UTF8 on Linux).
|--- Return: 1 if it exists, 0 if not. */

external b32 IsExistingDir(void* Path);
/* Checks if [Path] points to an existing directory. Path must be at the
 |  Unicode encoding native to the system (e.g. UTF16 on Windows, UTF8 on Linux).
|--- Return: 1 if it exists, 0 if not. */

external b32 MakeDir(void* Path);

/* Creates directory pointed at by [Path]. Path must be at the Unicode encoding
 |  native to the system (e.g. UTF16 on Windows, UTF8 on Linux). Creates all
 |  dirs up the directory tree if they don't exist already.
|--- Return: 1 if successful, 0 if not, 2 if [Path] already exists. */

external b32 MoveUpPath(path* Path, usz MoveUpCount);

/* Moves the [.WriteCur] of [Path] to point to the directory [MoveUpCount] dirs
 |  above. Does not, however, clear the buffer.
 |--- Return: 1 if successful, 0 if not. */

external b32 RemoveDir(void* Path, bool RemoveAllFiles);

/* Deletes directory pointed at by [Path]. Path must be at the Unicode encoding
 |  native to the system (e.g. UTF16 on Windows, UTF8 on Linux). If [RemoveAllFiles]
|  flag is not active and there are files inside [Path], the function fails.
 |--- Return: 1 if successful, 0 if not. */

// TODO: IsValidPath(Path);

typedef struct iter_dir
{
    char AllFilesBuf[MAX_PATH_SIZE];
    path AllFiles;
    char* Filename;
    u8 OSData[600]; // OBS: If Windows, bytes 0~7 are HANDLE to First File, and
    //                 bytes 8~599 are WIN32_FIND_DATAW struct. If POSIX, bytes 0~7 are
    //                 DIR* pointer, and bytes 8~n are dirent struct.
    bool IsDir;
} iter_dir;

/* Structure for walking through direcytory trees and listing files in it. */

external void InitIterDir(iter_dir* Iter, path DirPath);

/* Initializes a new iter_dir struct [Iter]. [DirPath] is the base path from which
 |  it starts navigating, and is copied to [.AllFilesBuf], so it can be freed after.
|--- Return: nothing. */

external b32 ListFiles(iter_dir* Iter);

/* Call this function iteratively to list all files and directories inside [.AllFiles].
 |  Each time the function is called, a new path will be returned in [.Filename], and
|  the flag [.IsDir] will be set to indicate if the path is a directory (true) or a
|  file (false).
|--- Return: 1 if function fetched a new path into [Iter], or 0 if it has reached the
|    end of the base path and there are no more files or dirs to read. */


//========================================
// Timing
//========================================

typedef enum weekday
{
    WeekDay_Sunday,
    WeekDay_Monday,
    WeekDay_Tuesday,
    WeekDay_Wednesday,
    WeekDay_Thursday,
    WeekDay_Friday,
    WeekDay_Saturday
} weekday;

typedef struct datetime
{
    i32 Year;
    i32 Month;
    i32 Day;
    i32 Hour;
    i32 Minute;
    i32 Second;
    weekday WeekDay;
} datetime;

typedef struct timing
{
    isz Start;
    isz End;
    f64 Diff; // Time ellapsed in seconds.
} timing;

/* Structure for timing code execution. Pass the same object to StartTiming() and
 |  StopTiming() to get the time ellapsed. */

external datetime CurrentLocalTime(void);

/* Gets the date and time at function call adjusted for system timezone.
 |--- Return: datetime info. */

external datetime CurrentSystemTime(void);

/* Gets the date and time at function call in UTC+0 timezone.
 |--- Return: datetime info. */

external void StartTiming(timing* Info);

/* Starts the clock for timing.
|--- Return: nothing. */

external void StopTiming(timing* Info);

/* Stops the clock for timing, and saved the time ellapsed in [.Diff].
 |  StartTiming() needs to have been called first, else the result will be wrong.
|--- Return: nothing. */


//========================================
// External Libraries
//========================================

external file LoadExternalLibrary(void* LibPath);

/* Loads an external shared library. Path of [LibPath] must point to valid external
 |  or import library (.so, .dll, .lib), and must be at the Unicode encoding native
 |  to the system (e.g. UTF16 on Windows, UTF8 on Linux).
|--- Return: file handle to library if successful, or INVALID_FILE if not. */

external void* LoadExternalSymbol(file Library, char* SymbolName);

/* Loads a function of name [SymbolName] from an open [Library] handle.
|--- Return: pointer to function loaded if successful, or NULL if not. */

external b32 UnloadExternalLibrary(file Library);

/* Unloads an open [Library] handle, that was open with LoadExternalLibrary().
 |--- Return: 1 if successful, 0 if not. */


//========================================
// Threading
//========================================

#define SCHEDULE_NORMAL 1
#define SCHEDULE_LOW    2
#define SCHEDULE_HIGH   4

external file ThreadCreate(void* ThreadProc, void* ThreadArg, _opt usz* ThreadId, bool CreateAndRun);

/* Creates a new thread. [ThreadProc] specifies the entry point, and should be a function
 |  that takes one void* as input parameter, and returns a void*. [ThreadArg] is the parameter
|  passed to the function pointed to at [ThreadProc]. [ThreadId] is a pointer to an usz variable
|  that received the thread id back (can be NULL). [CreateAndRun] determines if the thread
|  if start suspended or not.
|--- Return: file handle to thread, or NULL if creation failed. */

external b32 ThreadChangeScheduling(file Thread, int NewScheduling);

/* Change how frequently [Thread] is awoken by the system. [NewScheduling] can either
 |  be SCHEDULE_NORMAL, SCHEDULE_LOW or SCHEDULE_HIGH.
|--- Return: 1 if successful, 0 if not. */

external i32 ThreadGetScheduling(file Thread);

/* Get current scheduling rule for [Thread]. Value can be SCHEDULE_NORMAL (1), SCHEDULE_LOW (2)
 |  or SCHEDULE_HIGH (4).
|--- Return: number referring to thread schedule. */

external b32 ThreadSuspend(file Thread);

/* Suspends thread at file handle [Thread].
|--- Return: 1 if successful, 0 if not. */

external bool ThreadUnsuspend(file Thread);

/* Unsuspends thread at file handle [Thread].
|--- Return: 1 if successful, 0 if not. */

external void ThreadExit(isz ExitCode);

/* Exits the thread that calls this function.
 |--- Return: nothing. */


//========================================
// Atomic
//========================================

external i16 AtomicExchange16(void* volatile Dst, i16 Value);

/* Saves 16-bit [Value] into [Dst] in a thread-safe manner, and returns previous value.
|--- Result: 16-bit value at [Dst] before function call. */

external i32 AtomicExchange32(void* volatile Dst, i32 Value);

/* Saves 32-bit [Value] into [Dst] in a thread-safe manner, and returns previous value.
|--- Result: 32-bit value at [Dst] before function call. */

external i64 AtomicExchange64(void* volatile Dst, i64 Value);

/* Saves 64-bit [Value] into [Dst] in a thread-safe manner, and returns previous value.
|--- Result: 64-bit value at [Dst] before function call. */

external void* AtomicExchangePtr(void* volatile* Dst, void* Value);

/* Saves pointer [Value] into [Dst] in a thread-safe manner, and returns previous value.
|--- Result: pointer address at [Dst] before function call. */


#if !defined(TT_STATIC_LINKING)
#if defined(TT_WINDOWS)
#include "tinybase-platform-win32.c"
#elif defined(TT_LINUX)
#include "tinybase-platform-linux.c"
#endif //TT_WINDOWS
#endif //TT_STATIC_LINKING

#endif //TINYBASE_PLATFORM_H
