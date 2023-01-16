#include "tinybase-strings.h"

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
// String tests
//

bool TestStrLit(string A, string Expected)
{
    return EqualStrings(A, Expected);
}

bool TestGetNextCharSize(void* Char, encoding Enc, usz Expected)
{
    return GetNextCharSize(Char, Enc) == Expected;
}

bool TestLenCodePoint(string A, usz Expected)
{
    return StringLen(A, LEN_CODEPOINT) == Expected;
}

bool TestAppendCharToStringNTimes(void* Src, usz Count, string* Dst, string Expected)
{
    mb_char Char = GetNextChar(Src, Dst->Enc);
    return AppendCharToStringNTimes(Char, Count, Dst) && EqualStrings(*Dst, Expected);
}

bool TestAppendArrayToString(void* Src, string* Dst, string Expected)
{
    return AppendArrayToString(Src, Dst) && EqualStrings(*Dst, Expected);
}

bool TestAppendStringToString(string Src, string* Dst, string Expected)
{
    return AppendStringToString(Src, Dst) && EqualStrings(*Dst, Expected);
}

bool TestAppendStringToStringNTimes(string Src, usz Count, string* Dst, string Expected)
{
    return AppendStringToStringNTimes(Src, Count, Dst) && EqualStrings(*Dst, Expected);
}

bool TestAppendUIntToString(usz Integer, string* Dst, string Expected)
{
    return AppendUIntToString(Integer, Dst) && EqualStrings(*Dst, Expected);
}

bool TestTranscode(string Src, string* Dst, string Expected)
{
    return Transcode(Src, Dst) && EqualStrings(*Dst, Expected);
}

//
// Test program
//

int main()
{
    // Ascii tests
    
    char Ascii[] = { 'A', 'l', 'f', 'a', '1', '2', '3', '?', '?', '?', 'B', 'e', 't', 'a', 'B', 'e', 't', 'a', 'O', 'm', 'e', 'g', 'a', 0 };
    string Lit1 = String(Ascii, sizeof(Ascii)-1, sizeof(Ascii), EC_ASCII);
    u8 Arr1[128] = {0};
    string Buf1 = String(Arr1, 0, sizeof(Arr1), EC_ASCII);
    
    Test(GetNextCharSize, Ascii, EC_ASCII, 1);
    Test(LenCodePoint, Lit1, 23);
    Test(AppendStringToString, String(Ascii, 4, 0, EC_ASCII), &Buf1, String(Ascii, 4, 0, EC_ASCII));
    Test(AppendUIntToString, 123, &Buf1, String(Ascii, 7, 0, EC_ASCII));
    Test(AppendCharToStringNTimes, "?", 3, &Buf1, String(Ascii, 10, 0, EC_ASCII));
    Test(AppendStringToStringNTimes, String(Ascii+10, 4, 0, EC_ASCII), 2, &Buf1, String(Ascii, 18, 0, EC_ASCII));
    Test(AppendArrayToString, Ascii + 18, &Buf1, Lit1);
    
    // UTF-8 tests
    
    char UTF8[] = { 'A', 'l', 'f', 'a', '1', '2', '3', 0xc4, 0x80, 0xc4, 0x80, 0xc4, 0x80, 0xe4, 0xbb, 0x8a, 0xf0, 0xa4, 0xad, 0xa2, 0xe4, 0xbb, 0x8a, 0xf0, 0xa4, 0xad, 0xa2, 0xe6, 0x80, 0xaa, 0xe3, 0x81, 0x97, 0xe3, 0x81, 0x84, 0 };
    string Lit2 = String(UTF8, sizeof(UTF8)-1, sizeof(UTF8), EC_UTF8);
    u8 Arr2[128] = {0};
    string Buf2 = String(Arr2, 0, sizeof(Arr2), EC_UTF8);
    
    Test(GetNextCharSize, UTF8, EC_UTF8, 1);
    Test(GetNextCharSize, UTF8+7, EC_UTF8, 2);
    Test(GetNextCharSize, UTF8+13, EC_UTF8, 3);
    Test(GetNextCharSize, UTF8+16, EC_UTF8, 4);
    Test(LenCodePoint, Lit2, 17);
    Test(AppendStringToString, String(UTF8, 4, 0, EC_UTF8), &Buf2, String(UTF8, 4, 0, EC_UTF8));
    Test(AppendUIntToString, 123, &Buf2, String(UTF8, 7, 0, EC_UTF8));
    Test(AppendCharToStringNTimes, UTF8+7, 3, &Buf2, String(UTF8, 13, 0, EC_UTF8));
    Test(AppendStringToStringNTimes, String(UTF8+13, 7, 0, EC_UTF8), 2, &Buf2, String(UTF8, 27, 0, EC_UTF8));
    Test(AppendArrayToString, UTF8+27, &Buf2, Lit2);
    
    // UTF-16LE tests
    
    char UTF16LE[] = { 'A', 0, 'l', 0, 'f', 0, 'a', 0, '1', 0, '2', 0, '3', 0, 0, 1, 0, 1, 0, 1, 0xca, 0x4e, 0x52, 0xd8, 0x62, 0xdf, 0xca, 0x4e, 0x52, 0xd8, 0x62, 0xdf, 0x2a, 0x60, 0x57, 0x30, 0x44, 0x30, 0, 0 };
    string Lit3 = String(UTF16LE, sizeof(UTF16LE)-2, sizeof(UTF16LE), EC_UTF16LE);
    u8 Arr3[128] = {0};
    string Buf3 = String(Arr3, 0, sizeof(Arr3), EC_UTF16LE);
    
    Test(GetNextCharSize, UTF16LE, EC_UTF16LE, 2);
    Test(GetNextCharSize, UTF16LE+14, EC_UTF16LE, 2);
    Test(GetNextCharSize, UTF16LE+20, EC_UTF16LE, 2);
    Test(GetNextCharSize, UTF16LE+22, EC_UTF16LE, 4);
    Test(LenCodePoint, Lit3, 17);
    Test(AppendStringToString, String(UTF16LE, 8, 0, EC_UTF16LE), &Buf3, String(UTF16LE, 8, 0, EC_UTF16LE));
    Test(AppendUIntToString, 123, &Buf3, String(UTF16LE, 14, 0, EC_UTF16LE));
    Test(AppendCharToStringNTimes, UTF16LE+14, 3, &Buf3, String(UTF16LE, 20, 0, EC_UTF16LE));
    Test(AppendStringToStringNTimes, String(UTF16LE+20, 6, 0, EC_UTF16LE), 2, &Buf3, String(UTF16LE, 32, 0, EC_UTF16LE));
    Test(AppendArrayToString, UTF16LE+32, &Buf3, Lit3);
    
    // UTF-16BE tests
    
    char UTF16BE[] = { 0, 'A', 0, 'l', 0, 'f', 0, 'a', 0, '1', 0, '2', 0, '3', 1, 0, 1, 0, 1, 0, 0x4e, 0xca, 0xd8, 0x52, 0xdf, 0x62, 0x4e, 0xca, 0xd8, 0x52, 0xdf, 0x62, 0x60, 0x2a, 0x30, 0x57, 0x30, 0x44, 0, 0 };
    string Lit4 = String(UTF16BE, sizeof(UTF16BE)-2, sizeof(UTF16BE), EC_UTF16BE);
    u8 Arr4[128] = {0};
    string Buf4 = String(Arr4, 0, sizeof(Arr4), EC_UTF16BE);
    
    Test(GetNextCharSize, UTF16BE, EC_UTF16BE, 2);
    Test(GetNextCharSize, UTF16BE+14, EC_UTF16BE, 2);
    Test(GetNextCharSize, UTF16BE+20, EC_UTF16BE, 2);
    Test(GetNextCharSize, UTF16BE+22, EC_UTF16BE, 4);
    Test(LenCodePoint, Lit4, 17);
    Test(AppendStringToString, String(UTF16BE, 8, 0, EC_UTF16BE), &Buf4, String(UTF16BE, 8, 0, EC_UTF16BE));
    Test(AppendUIntToString, 123, &Buf4, String(UTF16BE, 14, 0, EC_UTF16BE));
    Test(AppendCharToStringNTimes, UTF16BE+14, 3, &Buf4, String(UTF16BE, 20, 0, EC_UTF16BE));
    Test(AppendStringToStringNTimes, String(UTF16BE+20, 6, 0, EC_UTF16BE), 2, &Buf4, String(UTF16BE, 32, 0, EC_UTF16BE));
    Test(AppendArrayToString, UTF16BE+32, &Buf4, Lit4);
    
    // UTF-32LE tests
    
    char UTF32LE[] = { 'A', 0, 0, 0, 'l', 0, 0, 0, 'f', 0, 0, 0, 'a', 0, 0, 0, '1', 0, 0, 0, '2', 0, 0, 0, '3', 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0xca, 0x4e, 0, 0, 0x62, 0x4b, 0x02, 0, 0xca, 0x4e, 0, 0, 0x62, 0x4b, 0x02, 0, 0x2a, 0x60, 0, 0, 0x57, 0x30, 0, 0, 0x44, 0x30, 0, 0, 0, 0, 0, 0 };
    string Lit5 = String(UTF32LE, sizeof(UTF32LE)-4, sizeof(UTF32LE), EC_UTF32LE);
    u8 Arr5[128] = {0};
    string Buf5 = String(Arr5, 0, sizeof(Arr5), EC_UTF32LE);
    
    Test(GetNextCharSize, UTF32LE, EC_UTF32LE, 4);
    Test(GetNextCharSize, UTF32LE+28, EC_UTF32LE, 4);
    Test(GetNextCharSize, UTF32LE+40, EC_UTF32LE, 4);
    Test(GetNextCharSize, UTF32LE+44, EC_UTF32LE, 4);
    Test(LenCodePoint, Lit5, 17);
    Test(AppendStringToString, String(UTF32LE, 16, 0, EC_UTF32LE), &Buf5, String(UTF32LE, 16, 0, EC_UTF32LE));
    Test(AppendUIntToString, 123, &Buf5, String(UTF32LE, 28, 0, EC_UTF32LE));
    Test(AppendCharToStringNTimes, UTF32LE+28, 3, &Buf5, String(UTF32LE, 40, 0, EC_UTF32LE));
    Test(AppendStringToStringNTimes, String(UTF32LE+40, 8, 0, EC_UTF32LE), 2, &Buf5, String(UTF32LE, 56, 0, EC_UTF32LE));
    Test(AppendArrayToString, UTF32LE+56, &Buf5, Lit5);
    
    // UTF-32BE tests
    
    char UTF32BE[] = { 0, 0, 0, 'A', 0, 0, 0, 'l', 0, 0, 0, 'f', 0, 0, 0, 'a', 0, 0, 0, '1', 0, 0, 0, '2', 0, 0, 0, '3', 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0x4e, 0xca, 0, 0x02, 0x4b, 0x62, 0, 0, 0x4e, 0xca, 0, 0x02, 0x4b, 0x62, 0, 0, 0x60, 0x2a, 0, 0, 0x30, 0x57, 0, 0, 0x30, 0x44, 0, 0, 0, 0 };
    string Lit6 = String(UTF32BE, sizeof(UTF32BE)-4, sizeof(UTF32BE), EC_UTF32BE);
    u8 Arr6[128] = {0};
    string Buf6 = String(Arr6, 0, sizeof(Arr6), EC_UTF32BE);
    
    Test(GetNextCharSize, UTF32BE, EC_UTF32BE, 4);
    Test(GetNextCharSize, UTF32BE+28, EC_UTF32BE, 4);
    Test(GetNextCharSize, UTF32BE+40, EC_UTF32BE, 4);
    Test(GetNextCharSize, UTF32BE+44, EC_UTF32BE, 4);
    Test(LenCodePoint, Lit6, 17);
    Test(AppendStringToString, String(UTF32BE, 16, 0, EC_UTF32BE), &Buf6, String(UTF32BE, 16, 0, EC_UTF32BE));
    Test(AppendUIntToString, 123, &Buf6, String(UTF32BE, 28, 0, EC_UTF32BE));
    Test(AppendCharToStringNTimes, UTF32BE+28, 3, &Buf6, String(UTF32BE, 40, 0, EC_UTF32BE));
    Test(AppendStringToStringNTimes, String(UTF32BE+40, 8, 0, EC_UTF32BE), 2, &Buf6, String(UTF32BE, 56, 0, EC_UTF32BE));
    Test(AppendArrayToString, UTF32BE+56, &Buf6, Lit6);
    
    // Transcoding tests
    
    char Transcoded[128] = {0};
    string UTF8ToUTF16LE = { Transcoded, 0, sizeof(Transcoded), EC_UTF16LE };
    string UTF8ToUTF16BE = { Transcoded, 0, sizeof(Transcoded), EC_UTF16BE };
    string UTF8ToUTF32LE = { Transcoded, 0, sizeof(Transcoded), EC_UTF32LE };
    string UTF8ToUTF32BE = { Transcoded, 0, sizeof(Transcoded), EC_UTF32BE };
    string UTF16LEToUTF8 = { Transcoded, 0, sizeof(Transcoded), EC_UTF8 };
    string UTF16LEToUTF32LE = { Transcoded, 0, sizeof(Transcoded), EC_UTF32LE };
    string UTF16LEToUTF32BE = { Transcoded, 0, sizeof(Transcoded), EC_UTF32BE };
    string UTF32BEToUTF8 = { Transcoded, 0, sizeof(Transcoded), EC_UTF8 };
    
    Test(Transcode, Lit2, &UTF8ToUTF16LE, Lit3);
    Test(Transcode, Lit2, &UTF8ToUTF16BE, Lit4);
    Test(Transcode, Lit2, &UTF8ToUTF32LE, Lit5);
    Test(Transcode, Lit2, &UTF8ToUTF32BE, Lit6);
    Test(Transcode, Lit3, &UTF16LEToUTF8, Lit2);
    Test(Transcode, Lit3, &UTF16LEToUTF32LE, Lit5);
    Test(Transcode, Lit3, &UTF16LEToUTF32BE, Lit6);
    Test(Transcode, Lit6, &UTF32BEToUTF8, Lit2);
    
    if (!Error) printf("All tests passed!\n");
    return 0;
}