#include "tinybase-platform.h"

#include <stdio.h>
#include <varargs.h>

bool Error = false;
#define Test(Callback, ...) \
do { \
if (!Test##Callback(__VA_ARGS__)) { \
Error = true; \
printf(" [%3d] %-40s ERRO.\n", __LINE__, #Callback##"()"); } \
} while (0); \

//
// Platform tests
//

bool TestGetMemory(usz Size, void* Address)
{
    Size = Align(Size, gSysInfo.PageSize);
    void* Mem = GetMemory(Size, Address, MEM_READ|MEM_WRITE);
    if (Mem)
    {
        try { *(u8*)Mem = 0; }
        catch (...) { return false; }
        
        try { *((u8*)Mem + Size - 1) = 0; }
        catch (...) { return false; }
        
        try { *((u8*)Mem + Size) = 0; }
        catch (...) { return Address ? (isz)Mem == (isz)Address : true; }
    }
    return false;
}

bool TestGetMemoryGuard(usz Size, void* Address)
{
    void* Mem = GetMemory(Size, Address, MEM_GUARD);
    if (Mem)
    {
        try { *(u8*)Mem = 0; }
        catch (...) { return true; }
    }
    return false;
}

bool TestGetMemoryReadOnly(usz Size, void* Address)
{
    void* Mem = GetMemory(Size, Address, MEM_READ);
    if (Mem)
    {
        try { u8 Z = ((u8*)Mem)[0]; }
        catch (...) { return false;}
        
        try { *(u8*)Mem = 0; }
        catch (...) { return true; }
    }
    return false;
}

bool TestFreeMemory(void* Mem)
{
    FreeMemory(Mem);
    try { *(u8*)Mem = 0; }
    catch (...) { return true; }
    return false;
}

bool TestGetMemoryFromHeap(usz SizeToAllocate)
{
    u8* Mem = (u8*)GetMemoryFromHeap(SizeToAllocate);
    
    try { *Mem = 0; }
    catch (...) { return false; }
    
    try { *(Mem + SizeToAllocate - 1) = 0; }
    catch (...) { return false; }
    
    for (usz Idx = 0; Idx < SizeToAllocate; Idx++)
    {
        if (Mem[Idx] != 0) return false;
    }
    
    return true;
}

bool TestCreateNewFile(void* Filename)
{
    file File = CreateNewFile(Filename, 0);
    bool Result = File != INVALID_FILE;
    CloseFileHandle(File);
    return Result;
}

bool TestCreateHiddenFile(void* Filename)
{
    file File = CreateNewFile(Filename, HIDDEN_FILE);
    bool Result = (File != INVALID_FILE
                   && IsFileHidden(Filename));
    CloseFileHandle(File);
    return Result;
}

bool TestOpenFileReadOnly(void* Filename)
{
    file File = OpenFileHandle(Filename, READ_SHARE);
    buffer Buffer = { (u8*)"Test", 4, 5 };
    bool Result = (File != INVALID_FILE
                   && WriteEntireFile(File, Buffer) == false);
    CloseFileHandle(File);
    return Result;
}

bool TestOpenFileReadWrite(void* Filename)
{
    file File = OpenFileHandle(Filename, WRITE_SHARE);
    buffer Buffer = { (u8*)"Test", 4, 5 };
    bool Result = (File != INVALID_FILE
                   && WriteEntireFile(File, Buffer));
    CloseFileHandle(File);
    return Result;
}

bool TestReadFromFile(file FileHandle, usz StartPos, usz AmountToRead, buffer Expected1, bool Expected2)
{
    void* Mem = GetMemory(AmountToRead, 0, MEM_READ|MEM_WRITE);
    if (ReadFromFile(FileHandle, Mem, AmountToRead, StartPos))
    {
        buffer File = Buffer(Mem, AmountToRead, 0);
        if (EqualBuffers(File, Expected1) && (File.Base[File.WriteCur-1] != 0))
        {
            return true == Expected2;
        }
    }
    return false == Expected2;
}

bool TestReadEntireFile(file FileHandle, buffer Expected1, bool Expected2)
{
    buffer File = ReadEntireFile(FileHandle);
    if (File.Base)
    {
        if (EqualBuffers(File, Expected1) && (File.Base[File.WriteCur-1] != 0))
        {
            return true == Expected2;
        }
    }
    return false == Expected2;
}

bool TestWriteEntireFile(file FileHandle, buffer Content)
{
    bool Result = WriteEntireFile(FileHandle, Content);
    if (Result)
    {
        buffer File = ReadEntireFile(FileHandle);
        Result = EqualBuffers(File, Content);
    }
    return Result;
}

bool TestWriteToFile(file FileHandle, buffer Content, usz StartPos)
{
    bool Result = WriteToFile(FileHandle, Content, StartPos);
    if (Result)
    {
        void* Mem = GetMemory(Content.WriteCur, 0, MEM_READ|MEM_WRITE);
        ReadFromFile(FileHandle, Mem, Content.WriteCur, StartPos);
        buffer File = Buffer(Mem, Content.WriteCur, Content.WriteCur);
        Result = EqualBuffers(File, Content);
    }
    return Result;
}

bool TestFilesAreEqual(void* APath, void* BPath, b32 Expected)
{
    file A = OpenFileHandle(APath, READ_SHARE);
    file B = OpenFileHandle(BPath, READ_SHARE);
    b32 Result = FilesAreEqual(A, B);
    
    CloseFileHandle(A);
    CloseFileHandle(B);
    return Result == Expected;
}

bool TestDuplicateFile(void* SrcPath, void* DstPath, bool OverwriteIfExists, b32 Expected)
{
    b32 Result = DuplicateFile(SrcPath, DstPath, OverwriteIfExists);
    if (Result == 1)
    {
        file A = OpenFileHandle(SrcPath, READ_SHARE);
        file B = OpenFileHandle(DstPath, READ_SHARE);
        Result = FilesAreEqual(A, B);
        
        CloseFileHandle(A);
        CloseFileHandle(B);
    }
    return Result == Expected;
}

bool TestRemoveFile(void* Filename, b32 Expected)
{
    return RemoveFile(Filename) == Expected;
}

bool TestChangeFileLocation(void* SrcPath, void* DstPath)
{
    return (ChangeFileLocation(SrcPath, DstPath)
            && !IsExistingPath(SrcPath)
            && IsExistingPath(DstPath));
}

bool TestMakeDir(void* Path)
{
    return MakeDir(Path);
}

bool TestIsExistingPath(void* Path, b32 Expected)
{
    return IsExistingPath(Path) == Expected;
}

bool TestIsExistingDir(void* Path, b32 Expected)
{
    return IsExistingDir(Path) == Expected;
}

bool TestAppendPathToPath(path NewPart, path* Path, path Expected)
{
    AppendPathToPath(NewPart, Path);
    return EqualStrings(*Path, Expected);
}

bool TestMoveUpPath(path* Path, usz MoveUpCount, path Expected)
{
    MoveUpPath(Path, MoveUpCount);
    return EqualStrings(*Path, Expected);
}

bool TestListFiles(path DirPath, path Expected)
{
    char CompareBuf[2048] = {0};
    string Compare = String(CompareBuf, 0, sizeof(CompareBuf), EC_UTF16LE);
    
    iter_dir Iter = {0};
    InitIterDir(&Iter, DirPath);
    while (ListFiles(&Iter))
    {
        //usz FilenameSize = Len(Path(Iter.Filename), LEN_CSTRING);
        usz FilenameSize = 2 * wcslen((wchar_t*)Iter.Filename);
        AppendStringToString(String(Iter.Filename, FilenameSize, 0, EC_UTF16LE), &Compare);
    }
    
    return EqualStrings(Compare, Expected);
}

bool TestRemoveDir(void* Path, bool RemoveAllFiles, b32 Expected)
{
    return RemoveDir(Path, RemoveAllFiles) == Expected;
}

bool TestChangeDirLocation(void* SrcPath, void* DstPath)
{
    return (ChangeDirLocation(SrcPath, DstPath)
            && !IsExistingDir(SrcPath)
            && IsExistingDir(DstPath));
}

bool TestLoadExternalLibrary(void* LibPath)
{
    file Lib = LoadExternalLibrary(LibPath);
    bool Result = Lib != NULL;
    UnloadExternalLibrary(Lib);
    return Result;
}

typedef int (*add_two)(int);
bool TestLoadExternalSymbol(void* LibPath, char* SymbolName, int TestNumber, int Expected)
{
    bool Result = false;
    file Lib = LoadExternalLibrary(LibPath);
    if (Lib)
    {
        add_two AddTwo = (add_two)LoadExternalSymbol(Lib, SymbolName);
        Result = AddTwo && AddTwo(TestNumber) == Expected;
        UnloadExternalLibrary(Lib);
    }
    return Result;
}


//
// Test program
//

int main()
{
    LoadSystemInfo();
    
#if defined(TT_WINDOWS)
    wchar_t* LibPath = L"..\\tests\\a.dll";
    wchar_t* TestDir = L"test";
    wchar_t* _TempA = L"test\\temp.a";
    wchar_t* _TempB = L"test\\temp.b";
    wchar_t* _TempC = L"test\\temp.c";
    wchar_t* _TempD = L"test\\temp.d";
    wchar_t* Tempdir = L"test\\tempdir\\";
    wchar_t* Tempdir_TempB = L"test\\tempdir\\temp.b";
    wchar_t* TempdirDir1 = L"test\\tempdir\\dir1\\";
    wchar_t* TempdirDir1Dir2 = L"test\\tempdir\\dir1\\dir2";
    wchar_t* TempdirDir2 = L"test\\tempdir\\dir2";
    wchar_t* Tempdir2 = L"test\\tempdir2";
    wchar_t* CompareListDir = L"temp.btemp.ctempdir";
    wchar_t* Dir1Lit = L"dir1";
    wchar_t* Dir2Lit = L"dir2";
    wchar_t* EmptyLit = L"";
    wchar_t* TestLit = L"test";
    wchar_t* TempdirLit = L"tempdir";
    wchar_t* TempDLit = L"temp.d";
    
#define VF2(F) L##F
#define VF1(F) VF2(F)
#define __VFILE__ VF1(__FILE__)
    
#else
    char* LibPath = "../tests/a.dll";
    char* TestDir = "test";
    char* _TempA = "test/temp.a";
    char* _TempB = "test/temp.b";
    char* _TempC = "test/temp.c";
    char* _TempD = "test/temp.d";
    char* Tempdir = "test/tempdir";
    char* Tempdir_TempB = "test/tempdir/temp.b";
    char* TempdirDir1 = "test/tempdir/dir1/";
    char* TempdirDir1Dir2 = "test/tempdir/dir1/dir2";
    char* TempdirDir2 = "test/tempdir/dir2";
    char* Tempdir2 = "test/tempdir2";
    char* CompareListDir = "temp.btemp.ctempdir";
    char* Dir1Lit = "dir1";
    char* Dir2Lit = "dir2";
    char* EmptyLit = "";
    char* TestLit = "test";
    char* TempdirLit = "tempdir";
    char* TempDLit = "temp.d";
    
#define __VFILE__ __FILE__
    
#endif
    
    // Memory
    void* Address = (void*)(isz)0x2000000;
    void* Mem1 = GetMemory(Kilobyte(1), NULL, MEM_READ);
    
    Test(GetMemory, gSysInfo.PageSize, Address);
    Test(FreeMemory, Mem1);
    Test(GetMemoryReadOnly, gSysInfo.PageSize, NULL);
    Test(GetMemoryGuard, gSysInfo.PageSize, NULL);
    Test(GetMemoryFromHeap, 100);
    
    // FileIO
    file File;
    buffer FileBaseText = { (u8*)"abcdefghijklmnopqrstuvwxyz1234567890", 36, 37 };
    buffer FileNewText = { (u8*)"!@#$%¨&*()", 10, 11 };
    MakeDir(TestDir);
    
    Test(CreateNewFile, _TempA);
    Test(CreateHiddenFile, _TempB);
    Test(OpenFileReadOnly, _TempA);
    Test(OpenFileReadWrite, _TempA);
    {
        File = OpenFileHandle(_TempA, READ_SHARE|WRITE_SHARE|DELETE_SHARE);
    }
    Test(WriteEntireFile, File, FileBaseText);
    Test(ReadEntireFile, File, FileBaseText, true);
    Test(ReadEntireFile, File, FileNewText, false);
    Test(WriteToFile, File, FileNewText, 20);
    Test(ReadFromFile, File, 20, 10, FileNewText, true);
    Test(ReadFromFile, File, 21, 11, FileNewText, false);
    Test(FilesAreEqual, __VFILE__, __VFILE__, true);
    Test(FilesAreEqual, __VFILE__, _TempA, false);
    Test(DuplicateFile, __VFILE__, _TempC, false, 1);
    Test(DuplicateFile, __VFILE__, _TempC, true, 1);
    Test(DuplicateFile, __VFILE__, _TempC, false, 4);
    Test(RemoveFile, _TempA, 1);
    Test(RemoveFile, _TempD, 2);
    
    // Filesystem
    char DirPathBuf[MAX_PATH_SIZE] = {0};
    path DirPath = Path(DirPathBuf);
    path DirExpected = PathLit(CompareListDir);
    path AppendCompare1 = PathLit(TempdirDir1Dir2);
    path MoveUpCompare1 = PathLit(TempdirDir1);
    path MoveUpCompare2 = PathLit(EmptyLit);
    AppendPathToPath(MoveUpCompare1, &DirPath);
    
    Test(MakeDir, Tempdir);
    Test(MakeDir, TempdirDir1Dir2);
    Test(IsExistingPath, __VFILE__, true);
    Test(IsExistingPath, Tempdir, true);
    Test(IsExistingPath, Tempdir2, false);
    Test(IsExistingDir, Tempdir, true);
    Test(IsExistingDir, __VFILE__, false);
    Test(AppendPathToPath, PathLit(Dir2Lit), &DirPath, AppendCompare1);
    Test(MoveUpPath, &DirPath, 1, MoveUpCompare1);
    Test(MoveUpPath, &DirPath, 3, MoveUpCompare2);
    Test(MoveUpPath, &DirPath, 1, MoveUpCompare2);
    {
        AppendPathToPath(PathLit(TestLit), &DirPath);
        AppendPathToPath(PathLit(TempdirLit), &DirPath);
        AppendPathToPath(PathLit(Dir1Lit), &DirPath);
        AppendPathToPath(PathLit(Dir2Lit), &DirPath);
        AppendPathToPath(PathLit(TempDLit), &DirPath);
        file NewFile = CreateNewFile(DirPath.Base, READ_SHARE);
        CloseFileHandle(NewFile);
        CloseFileHandle(File);
        MoveUpPath(&DirPath, 4);
    }
    Test(ListFiles, DirPath, DirExpected);
    Test(ChangeFileLocation, _TempB, Tempdir_TempB);
    Test(ChangeDirLocation, TempdirDir1Dir2, TempdirDir2);
    Test(RemoveDir, TestDir, false, false);
    Test(RemoveDir, TestDir, true, true);
    
    // External Libraries
    Test(LoadExternalLibrary, LibPath);
    Test(LoadExternalSymbol, LibPath, "AddTwo", 3, 5);
    
    if (!Error) printf("All tests passed!\n");
    return 0;
}