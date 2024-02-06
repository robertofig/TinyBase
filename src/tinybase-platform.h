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
// Platform types and defines
//========================================

#if defined(TT_WINDOWS)
# define MAX_PATH_SIZE 520 // 260 wchar_t elements.
# define INVALID_FILE USZ_MAX
# define ASYNC_DATA_SIZE 40 // OVERLAPPED struct.
# define DYNAMIC_LIB_EXT ".dll"
# define MUTEX_SIZE 8 // Size of HANDLE
# define SEMAPHORE_SIZE 8 // Size of HANDLE
# define THREAD_PROC(Name) u32 Name(void* Arg)
typedef u32 (*thread_proc)(void*);
#elif defined(TT_LINUX)
# define MAX_PATH_SIZE 4096
# define INVALID_FILE USZ_MAX
# define ASYNC_DATA_SIZE 184 // aiocb struct + timespec struct.
# define DYNAMIC_LIB_EXT ".so"
# define MUTEX_SIZE 40 // Size of pthread_mutex_t
# define SEMAPHORE_SIZE 32 // Size of sem_t
# define THREAD_PROC(Name) void* Name(void* Arg)
typedef void* (*thread_proc)(void*);
#else // Reserved for other platforms;
#endif


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

/* Reads system information and saves it to [gSysInfo] global variable. Must be called
 |  only once, before calling other functions in this library.
 |--- Return: nothing. */

//========================================
// Memory
//========================================

#define MEM_READ     0x1  // Marks memory pages for read.
#define MEM_WRITE    0x2  // Marks memory pages for read and write.
#define MEM_GUARD    0x4  // Marks memory pages as untouchable (takes precedence over
//                           others).
#define MEM_EXEC     0x8  // Marks memory pages as executable.
#define MEM_HUGEPAGE 0x10 // Reserves large memory pages, if the system supports.

external buffer GetMemory(usz Size, _opt void* Address, _opt int AccessFlags);

/* Allocates memory block of [Size] bytes, rounded up to system page size. Block is
 |  guaranteed to be zeroed. A start [Address] can optionally be passed (system will
 |  choose random address if this is NULL). [AccessFlags] determines memory block
 |  behaviour; if none is passed, block is set to read-only.
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
    u8 Data[ASYNC_DATA_SIZE];
} async;

external file CreateNewFile(void* Filename, i32 Flags);

/* Creates and opens new file at path [Filename]. Path must be at the Unicode encoding
 |  native to the system (e.g. UTF16 on Windows, UTF8 on Linux). Creation flags must be
 |  passed to [Flags].
|--- Return: file handle if successful, or INVALID_FILE if not. */

external file OpenFileHandle(void* Filename, i32 Flags);

/* Opens file at path [Filename]. Path must be at the Unicode encoding native to the
 |  system (e.g. UTF16 on Windows, UTF8 on Linux). Open flags must be passed to [Flags].
|--- Return: file handle if successful, or INVALID_FILE if not.*/

external void CloseFileHandle(file File);

/* Closes file handle acquired with CreateNewFile() or OpenFileHandle().
 |--- Return: nothing. */

external buffer ReadEntireFile(file File);

/* Takes in an open [File] handle, allocated memory for it with read/write access, and
 |  copies entire file content to it.
|--- Return: buffer with memory if successful, or empty buffer if not. */

external bool ReadFromFile(file File, buffer* Dst, usz AmountToRead, usz StartPos);

/* Copies [AmountToRead] bytes from [File] at [StartPos] offset into [Dst] memory.
 |  Memory must already be allocated. If [StartPos] + [AmountToRead] goes beyond EOF,
 | nothing is copied and function fails.
 |--- Return: true if read operation was successfully started, false if not. */

external bool ReadFileAsync(file File, buffer* Dst, usz AmountToRead, usz StartPos,
                            async* Async);

/* Reads file in non-blocking manner. Memory must already be allocated and passed at
 |  [Dst], with at least [AmountToRead] size. [Async] is a pointer to an unitialized
 |  object that will receive platform-specific async IO data, to be later passed to the
 |  completion function. File is read in chunks of U32_MAX, so if [AmountToRead] is
 |  larger than that multiple reads will be posted.
|--- Return: true if read operation was successfully started, false if not. */

external bool AppendToFile(file File, buffer Content);

/* Writes data in [Content] at EOF of [File]. This function is for files that were NOT
 |  opened with APPEND_FILE flag; for those that were, any regular write call will always
 |  append.
 |--- Return: true if successful, false if not. */

external bool WriteEntireFile(file File, buffer Content);

/* Writes data in [Content] at beginning of [File]. If file was opened with APPEND_FILE
 |  flag, writes at EOF.
|--- Return: true if successful, false is not. */

external bool WriteToFile(file File, buffer Content, usz StartPos);

/* Writes data in [Content] at [StartPos] of [File]. If file was opened with APPEND_FILE
 |  flag, [StartPos] is ignored and it writes at EOF.
 |--- Return: true if successful, false if not. */

external bool WriteFileAsync(file File, void* Src, usz AmountToWrite, usz StartPos,
                             async* Async);

/* Writes [AmounttoWrite] bytes from [Src] at beginning of [File] in non-blocking
 |  manner. If file was opened with APPEND_FILE flag, writes at EOF. [Async] is a pointer
 |  to an unitialized object that will receive platform-specific async IO data, to be
 |  later passed to the completion function. File is written to in chunks of U32_MAX, so
 |  if [AmountToRead] is larger than that multiple reads will be posted.
 |--- Return: true if write operation was successfully started, false if not. */

external usz WaitOnIoCompletion(file File, async* Async, bool Block);

/* Waits until an async IO operation done on [File] completes. [Async] is a pointer to
 |  the same object used when start the IO. [Block] determines if the call waits
 |  indefinitely or returns immediately if it'd block.
|--- Return: number of bytes transferred; 0 means an error if [Block], and a timeout
 |            if not. */

external usz FileLastWriteTime(file File);

/* Gets last time [File] was written to, in system units.
|--- Return: time of write if successful, or USZ_MAX if not. */

external usz FileSizeOf(file File);

/* Gets the size of [File] in bytes.
|--- Return: size of file if successful, or USZ_MAX if not. */

external bool FilesAreEqual(file FileA, file FileB);

/* Given two open file handles, determines if the content of both files is equal.
 |--- Return: true if they are equal, false otherwise. */

external bool IsFileHidden(void* Filename);

/* Checks if file in [Filename] is hidden. Path must be at the Unicode encoding native
 |  to the system (e.g. UTF16 on Windows, UTF8 on Linux).
|--- Return: true if hidden, false if not. */

external bool DuplicateFile(void* SrcPath, void* DstPath, bool OverwriteIfExists);

/* Copies file in [Src] to new file in [Dst] in the file system. The entire directory
 |  tree in [Dst] must already exist. Paths must be at the Unicode encoding native to
 |  the system (e.g. UTF16 on Windows, UTF8 on Linux). [OverwriteIfExists] controls
 |  whether an existing file should be overwritten.
|--- Return: true if successful, false if not. */

external bool RemoveFile(void* Filename);

/* Deletes file in [Src]. Path must be at the Unicode encoding native to the system
 |  (e.g. UTF16 on Windows, UTF8 on Linux).
|--- Return: true if successful, false if not. */

external bool SeekFile(file File, usz Pos);

/* Sets the pointer of [File] to the offset [Pos]. [Pos] must not be greater than
 |  file size, or else the function fails.
|--- Return: true if successful, false if not. */


//========================================
// Filesystem
//========================================

typedef string path;

// Type [path] is a string with MAX_PATH_SIZE [.Size] and in the system native
// unicode encoding (e.g. UTF-16 on Windows, UTF-8 on Linux).

external path Path(void* Mem);

/* Creates a path from [Mem] buffer, and sets its [.Size] to MAX_PATH_SIZE, minus
|  one char to guarantee it will always be null-terminated.
|--- Return: new path object.*/

external path PathCString(void* CString);

/* Creates a path from [CString] memory region, and sets [.WriteCur] to its length
 |  or MAX_PATH_SIZE, whichever is smaller. This length is the number of bytes until
 |  the null-termination. If [CString] is not null-terminated, an overflow may occur.
 |--- Return: new path object. */

external bool AppendArrayToPath(void* NewPart, path* Dst);

/* Appends a new sub-dir or file [NewPart] to [Dst]. [NewPart] needs not start or end
 |  with path separators (slash or backslash), this is taken care of by the function.
 |  [NewPart] is assumed to be in the same encoding as [Dst], and to be zero-terminated.
 |  It also must fit entirely in [Dst].
|--- Return: true if successful, false if not. */

external bool AppendDataToPath(void* NewPart, usz NewPartSize, path* Dst);

/* Same as AppendArrayToPath(), but with [NewPartSize] determining the size of [NewPart],
 |  meaning it does not need to be zero-terminated.
|--- Return: true if successful, false if not. */

external bool AppendPathToPath(path NewPart, path* Dst);

/* Appends [NewPart] to [Dst], so long as it fits entirely. [NewPart] must be a valid
 |  path object, or else the function fails.
|--- Return: true if successful, false if not. */

external bool AppendStringToPath(string NewPart, path* Dst);

/* Appends [NewPart] to [Dst], so long as it fits entirely. If [NewPart] is of a
 |  different encoding than [Dst], it is transcoded to it.
|--- Return: true if successful, false if not. */

external bool AppendCWDToPath(path* Dst);

/* Appends the current working directory to [Dst], so long as it fits entirely.
|--- Return: true if successful, false if not. */

external bool ChangeFileLocation(void* Src, void* Dst);

/* Changes location of file in [Src] to [Dst] in the file system. The entire
 |  directory tree in [Dst] must already exist. Paths must be at the Unicode encoding
 |  native to the system (e.g. UTF16 on Windows, UTF8 on Linux).
|--- Return: true if successful, false if not. */

external bool ChangeDirLocation(void* Src, void* Dst);

/* Same usage as ChangeFileLocation(), but with a directory as [Src].
|--- Return: true if successful, false if not. */

external bool IsExistingPath(void* Path);

/* Checks if [Path] points to an existing file or directory. Path must be at the
 |  Unicode encoding native to the system (e.g. UTF16 on Windows, UTF8 on Linux).
|--- Return: true if it exists, false if not. */

external bool IsExistingDir(void* Path);
/* Checks if [Path] points to an existing directory. Path must be at the Unicode
 |  encoding native to the system (e.g. UTF16 on Windows, UTF8 on Linux).
|--- Return: true if it exists, false if not. */

external bool MakeDir(void* Path);

/* Creates directory pointed at by [Path]. Path must be at the Unicode encoding native
 |  to the system (e.g. UTF16 on Windows, UTF8 on Linux). Creates all dirs up the
 |  directory tree if they don't exist already.
|--- Return: true if successful, false if not. */

external bool MoveUpPath(path* Path, usz MoveUpCount);

/* Moves the [.WriteCur] of [Path] to point to the directory [MoveUpCount] dirs above.
 |  Does not, however, clear the buffer.
 |--- Return: true if successful, false if not. */

external bool RemoveDir(void* Path, bool RemoveAllFiles);

/* Deletes directory pointed at by [Path]. Path must be at the Unicode encoding native
 |  to the system (e.g. UTF16 on Windows, UTF8 on Linux). If [RemoveAllFiles] flag is
 |  not active and there are files inside [Path], the function fails.
 |--- Return: true if successful, false if not. */

// TODO: IsValidPath(Path);

typedef struct iter_dir
{
    char AllFilesBuf[MAX_PATH_SIZE];
    path AllFiles;
    char* Filename;
    u8 OSData[600]; // If Windows, bytes 0~7 are HANDLE to First File, and bytes 8~599
    //                 are WIN32_FIND_DATAW struct. If POSIX, bytes 0~7 are DIR* pointer,
    //                 and bytes 8~n are dirent struct.
    bool IsDir;
} iter_dir;

/* Structure for walking through direcytory trees and listing files in it. */

external void InitIterDir(iter_dir* Iter, path DirPath);

/* Initializes a new iter_dir struct [Iter]. [DirPath] is the base path from which it
 |  starts navigating, and is copied to [.AllFilesBuf], so it can be freed after.
|--- Return: nothing. */

external bool ListFiles(iter_dir* Iter);

/* Call this function iteratively to list all files and directories inside [.AllFiles].
 |  Each time the function is called, a new path will be returned in [.Filename], and
 |  the flag [.IsDir] will be set to indicate if the path is a directory (true) or a
 |  file (false).
|--- Return: true if function fetched a new path into [Iter], false if it has reached
 |            the end of the base path and there are no more files or dirs to read. */


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

// Structure for timing code execution. Pass the same object to StartTiming() and
// StopTiming() to get the time ellapsed.

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

/* Stops the clock for timing, and saved the time ellapsed in [.Diff]. StartTiming()
 |  needs to have been called first, else the result will be wrong.
|--- Return: nothing. */


//========================================
// External Libraries
//========================================

external file LoadExternalLibrary(void* LibPath);

/* Loads an external shared library. Path of [LibPath] must point to valid external or
 |  import library (.so, .dll, .lib), and must be at the Unicode encoding native to the
 |  system (e.g. UTF16 on Windows, UTF8 on Linux).
|--- Return: file handle to library if successful, or INVALID_FILE if not. */

external void* LoadExternalSymbol(file Library, char* SymbolName);

/* Loads a function of name [SymbolName] from an open [Library] handle.
|--- Return: pointer to function loaded if successful, or NULL if not. */

external bool UnloadExternalLibrary(file Library);

/* Unloads an open [Library] handle, that was open with LoadExternalLibrary().
 |--- Return: true if successful, false if not. */


//========================================
// Threading
//========================================

#define SCHEDULE_UNKNOWN 0
#define SCHEDULE_NORMAL  1
#define SCHEDULE_LOW     2
#define SCHEDULE_HIGH    4

typedef struct thread
{
    file Handle;
    // Space reserved for expansion.
} thread;

external thread InitThread(thread_proc ThreadProc, void* ThreadArg, bool Waitable);

/* Creates a new thread. [ThreadProc] specifies the entry point, and should be a
 |  function of type thread_proc. [ThreadArg] is the parameter passed to the function
 |  pointed to at [ThreadProc]. [Waitable] determines how to close the thread; if true,
 |  the parent thread must wait on the child thread with WaitOnThread() to close it;
 |  otherwise it must call CloseThread() after it knows the thread has finished.
|--- Return: thread object, or empty object if creation failed. */

external bool ChangeThreadScheduling(thread* Thread, int NewScheduling);

/* Change how frequently [Thread] is awoken by the system. [NewScheduling] can either
 |  be SCHEDULE_NORMAL, SCHEDULE_LOW or SCHEDULE_HIGH.
|--- Return: true if successful, false if not. */

external i32 GetThreadScheduling(thread Thread);

/* Get current scheduling rule for [Thread]. That can be SCHEDULE_NORMAL (1),
 |  SCHEDULE_LOW (2) or SCHEDULE_HIGH (4).
|--- Return: number referring to thread schedule. */

external bool CloseThread(thread* Thread);

/* Cleans up thread, after it has finished running. Must only be called on non-waitable
 |  threads. Failure to call it may result in memory leaks.
 |--- Return: true if successful, false if not. */

external bool WaitOnThread(thread* Thread);

/* Waits on a thread created with Waitable status. This will block the calling thread
 |  until [Thread] finishes running. If it has already finished, returns immediately.
 |  This also cleans up the thread, so no need to call CloseThread().
 |--- Return: true if successful, false if not. */

external bool KillThread(thread* Thread);

/* Forcefully terminates the running thread, and cleans up its resources. Calling this
 |  function on a thread that has already been closed may have unpredictable behaviour,
 |  and should be avoided.
 |--- Return: true if successful, false if not. */


//========================================
// Synchronization
//========================================

typedef struct mutex
{
    u8 Handle[MUTEX_SIZE];
} mutex;

external mutex InitMutex(void);

/* Inits a mutex in an unlocked state.
|--- Return: mutex object, or empty object in failure. */

external bool CloseMutex(mutex* Mutex);

/* Closes the [Mutex] object.
|--- Return: true if successful, false if not. */

external bool LockOnMutex(mutex* Mutex);

/* Acquires the lock on the mutex. If the mutex is already locked by another thread,
|  it will block until the mutex is released.
|--- Return: true if successful, false if not. */

external bool UnlockMutex(mutex* Mutex);

/* Releases the lock on the mutex.
|--- Return: true if successful, false if not. */


typedef struct semaphore
{
    u8 Handle[SEMAPHORE_SIZE];
} semaphore;

external semaphore InitSemaphore(i32 InitCount);

/* Inits a semaphore with a [InitCount] value.
|--- Return: semaphore object, or empty object in failure. */

external bool CloseSemaphore(semaphore* Semaphore);

/* Closes the [Semaphore] object.
|--- Return: true if successful, false if not. */

external bool WaitOnSemaphore(semaphore* Semaphore);

/* Decreases the count of [Semaphore] by 1. If the count is 0, this function will
|  block until another thread increases the count with IncreaseSemaphore().
|--- Return: true if successful, false if not. */

external bool IncreaseSemaphore(semaphore* Semaphore);

/* Increases the count of [Semaphore] by 1. For each time this function is called,
|  some thread blocking on WaitOnSemaphore() will resume execution.
|--- Return: true if successful, false if not. */


#if !defined(TT_STATIC_LINKING)
# if defined(TT_WINDOWS)
#  include "tinybase-platform-win32.c"
# elif defined(TT_LINUX)
#  include "tinybase-platform-linux.c"
# endif //TT_WINDOWS
#endif //TT_STATIC_LINKING

#endif //TINYBASE_PLATFORM_H
