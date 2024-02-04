#ifndef TINYBASE_STRINGS_H
//=========================================================================
// tinybase-strings.h
//
// Module for working with strings of different encodings. It works as a
// wrapper over tinybase-memory.h, where a buffer and an ASCII-encoded
// string work basically the same way.
//
// The encodings supported are defined in the [encoding] enum, and the
// implementations of the different string manipulation methods for each
// encoding is done in tinybase-encoding.h
//=========================================================================
#define TINYBASE_STRINGS_H

#include "tinybase-memory.h"

typedef enum encoding
{
    EC_ASCII   = 1,
    EC_UTF8    = 2,
    EC_UTF16LE = 3,
    EC_UTF16BE = 4,
    EC_UTF32LE = 5,
    EC_UTF32BE = 6
} encoding;

typedef u32 mb_char; // multibyte char.
typedef u32 uchar;   // unicode char.
/* Multibyte characters have at most 4 bytes, so they fit in an int. */

typedef struct string
{
    union
    {
        struct
        {
            char* Base;
            usz WriteCur;
            usz Size;
        };
        buffer Buffer;
    };
    encoding Enc;
} string;

/* Structure used in the library. Basically a buffer with encoding information.
|  Both [.WriteCur] and [.Size] are always in bytes. */


//========================================
// Creation
//========================================

external string String(void* Base, usz WriteCur, usz Size, encoding Enc);

/* Creates a new string object.
 |--- Return: string. */

external string StringC(void* Base, encoding Enc);

/* Creates a new string object from a null-terminated CString, and calculates
|  its size.
|--- Return: string. */

#define StringLit(S) String((S), sizeof(S)-1, 0, EC_ASCII)

/* Creates a new string struct from a C string literal.
 |--- Return: string. */

//========================================
// Read
//========================================

typedef enum len_type
{
    LEN_CSTRING,   // Same as using strlen() or wcslen() from the C standard lib.
    LEN_CODEPOINT, // Returns the number of codepoints in a string.
    LEN_PRINTCHAR  // Returns the number of printable characters in a string.
} len_type;

external mb_char GetNextChar(void* Src, encoding Enc);

/* Get the next multibyte character in [Src], given [Enc] encoding.
 |--- Return: multibyte char at address [Src]. */

external mb_char EatNextChar(void** Src, encoding Enc);

/* Get the next multibyte character in [Src], given [Enc] encoding, and advances
|  [Src] by the number of bytes of that character.
 |--- Return: multibyte char at address [Src]. */

external u32 GetNextCharSize(void* Src, encoding Enc);

/* Get the size in bytes of the next character in [Src], given [Enc] encoding.
|--- Return: size of multibyte char at address [Src]. */

external u32 GetMultibyteCharSize(mb_char Char, encoding Enc);

/* Get the size in bytes of a multibyte char.
|--- Return: size of multibyte char. */

external usz StringLen(string A, len_type LenType);

/* Get the length of [A]. Type of length changed by [LenType].
|--- Return: length based on len_type flag. */


//========================================
// Interpret
//========================================

external bool CharIsAlphanum(mb_char Char);

/* Determines if a multibyte char is a~z, A~Z, or 0~9.
|--- Return: 1 if char is alphanum, false if not. */

external bool CharIsDigit(mb_char Char);

/* Determines if a multibyte char is 0~9.
|--- Return: 1 if char is digit, false if not. */

external bool CharIsLetter(mb_char Char);

/* Determines if a multibyte char is a~z or A~Z.
|--- Return: 1 if char is letter, false if not. */

external bool StringIsAlphanum(string Src);

/* Determines if all chars in [Src] are a~z, A~Z, or 0~9.
|--- Return: 1 if all chars are alphanum, false if not. */

external bool StringIsDigit(string Src);

/* Determines if all chars in [Src] are 0~9.
|--- Return: 1 if all chars are digits, false if not. */

external bool StringIsLetter(string Src);

/* Determines if all chars in [Src] are a~z or A~Z.
|--- Return: 1 if all chars are letters, false if not. */

external isz StringToInt(string Src);

/* Converts [Src] to signed integer.
|--- Return: string as number if successful, ISZ_MAX if not. */

external usz StringToUInt(string Src);

/* Converts [Src] to unsigned integer.
|--- Return: string as number if successful, USZ_MAX if not. */

external usz StringToHex(string Src);

/* Converts hex string in [Src] to unsigned integer.
|--- Return: string as number if successful, USZ_MAX if not. */

external f64 StringToFloat(string Src);

/* Converts [Src] to 64-bit float.
|--- Return: f64 if conversion is successful, DBL_MAX if not. */


//========================================
// Query
//========================================

/* Flags for CharInString() and StringInString() are the same as defined in "memory.h".
 |
| RETURN_BOOL        Returns 1 if query was successful, false if not.
| RETURN_IDX_FIND    Returns offset into first byte of instance found, or INVALID_IDX if none.
 | RETURN_IDX_AFTER   Returns offset one mb_char after first instance found, or INVALID_IDX if none.
 | RETURN_IDX_DIFF    Returns offset where two buffers start to differ, or the length if they don't.
 | RETURN_PTR_FIND    Returns pointer of first byte of instance found, or NULL if none.
 | RETURN_PTR_AFTER   Returns pointer one mb_char after first instance found, or INVALID_IDX if none.
 | RETURN_PTR_DIFF    Returns pointer where two buffers start to differ, or the length if they don't.
 | SEARCH_REVERSE     Performs query backwards from the end of buffers. */

external usz CharInString(mb_char Needle, string Haystack, int Flags);

/* Searches for multibyte char [Needle] in [Haystack]. [Needle] is expected to be of same
|  encoding as [Haystack]. Pass either RETURN_BOOL, RETURN_IDX_FIND, RETURN_IDX_AFTER,
 |  RETURN_PTR_FIND, or RETURN_PTR_AFTER to [Flags] to determine return type. Return type
 |  flag can be OR'd together with SEARCH_REVERSE.
|--- Return: based on return type flag. */

external usz StringInString(string Needle, string Haystack, int Flags);

/* Searches for string [Needle] in [Haystack]. [Needle] must be of same encoding as
 |  [Haystack], otherwise the function fails. Pass either RETURN_BOOL, RETURN_IDX_FIND,
|  RETURN_IDX_AFTER, RETURN_PTR_FIND, or RETURN_PTR_AFTER to [Flags] to determine return type.
|  Return flag can be OR'd together with SEARCH_REVERSE. If a RETURN_(...)_AFTER flag is used,
|  returns one byte after the [Needle] length (e.g. if [Needle] has 35 bytes of length and
|  was found on offset 400, return is at 436).
|--- Return: based on return type flag. */

external usz CountCharInString(mb_char Needle, string Haystack);

/* Counts number of multibyte char [Needle] in [Haystack].
|--- Return: number of chars found in string. */

external usz CompareStrings(string A, string B, usz AmountToCompare, int Flag);

/* Compares two strings byte by byte for [AmountToCompare] bytes, until they differ, or
|  until the smaller of each string's [.WriteCur]. Both strings must be of same encoding,
|  otherwise the function fails. Pass either RETURN_IDX_DIFF or RETURN_PTR_DIFF to [Flags],
 |  returns the point where they differ, or the amount compared if not.
|--- Return: based on return type flag. */

external bool EqualStrings(string A, string B);

/* Compares two strings byte by byte, to see if they are identical in size and content.
|  Both strings must be of same encoding, otherwise the function fails.
 |--- Return: true if successful, false if not. */


//========================================
// Transcode
//========================================

external uchar DecodeChar(mb_char Char, encoding Enc);

/* Takes one multibyte [Char] of encoding [Enc] and decodes it to Unicode codepoint.
|--- Return: unicode char. */

external mb_char EncodeChar(uchar Char, encoding Enc);

/* Takes one Unicode codepoint [Char] and encodes it to multibyte char of encoding [Enc].
|--- Return: encoded char. */

external bool Transcode(string Src, string* Dst);

/* Copies [Src] into [Dst], transcoding it into [Dst]'s encoding. Function fails if the
|  transcoded [Src] does not fit entirely in [Dst].
 |--- Return: true if successful, false if not. */

external bool ReplaceCharInString(mb_char Old, mb_char New, string A);

/* Replaces every instance of multibyte [Old] in [A] with [New]. Assumes [Old] and [New]
 |  are in the same encoding as [A], and same number of bytes (function fails otherwise).
|--- Return: true if successful, false if not. */


//========================================
// Write
//========================================

external void AdvanceString(string* Dst, usz NumChars);

/* Modifies [Dst] to advance its [.Base] by [NumChars], and shrink [.WriteCur] and [.Size] by
 |  the same amount.
 |--- Return: nothing. */

external bool AppendArrayToString(void* Src, string* Dst);

/* Appends content of [Src] into [Dst]. [Src] must be zero-terminated, and its content must
 |  fit entirely in [Dst]. Assumes both are in same encoding.
 |--- Return: true if successful, false if not. */

external bool AppendCharToString(mb_char Src, string* Dst);

/* Appends multibyte char [Src] into [Dst]. Multibyte must fit entirely in [Dst]. Assumes both
 |  are in same encoding.
|--- Return: true if successful, false if not. */

external bool AppendCharToStringNTimes(mb_char Src, usz Count, string* Dst);

/* Appends the multibyte char [Src] into [Dst], [Count] number of times. All multibyte chars
|  must fit in [Dst]. Assumes both are in same encoding.
 |--- Return: true if successful, false if not. */

external bool AppendDataToString(void* Src, usz SrcSize, string* Dst);

/* Appends [SrcSize] bytes of [Src] into [Dst]. [Src] content myst fit entirely in [Dst].
|  Assumes both at in same encoding.
 |--- Return: true if successful, false if not. */

external bool AppendStringToString(string Src, string* Dst);

/* Appends [Src] into [Dst]. Both need to be in same encoding. [Src] content must fit 
|  entirely in [Dst].
 |--- Return: true if successful, false if not. */

external bool AppendStringToStringNTimes(string Src, usz Count, string* Dst);

/* Appends  [Src] into [Dst], [Count] number of times. Both need to be in same encoding.
|  [Src] content must fit entirely in [Dst].
 |--- Return: true if successful, false if not. */

external bool AppendIntToString(isz Integer, string* Dst);

/* Appends signed integer into [Dst], in the encode specified by [Dst]. String representation
 |  of [Integer] must fit entirely in [Dst].
|--- Return: true if successful, false if not. */

external bool AppendUIntToString(usz Integer, string* Dst);

/* Appends unsigned integer into [Dst], in the encode specified by [Dst]. String representation
 |  of [Integer] must fit entirely in [Dst].
|--- Return: true if successful, false if not. */

external bool AppendFloatToString(f64 Float, usz DecimalPlaces, bool InScientificNotation, string* Dst);

/* Appends float into [Dst], to [DecimalPlaces] precision, in the encode specified by [Dst].
 |  [InScientificNotation] controls the format (e.g. 0.0045 without, or 4.5e-002 with). String
 |  representation of [Float] must fit entirely in [Dst].
|--- Return: true if successful, false if not. */


//========================================
// C++ operators
//========================================

/* Experimental C++ interface. */

#if defined(__cplusplus)
inline string operator+(string A, void* Src) { AppendArrayToString(Src, &A); return A; }
inline string& operator+=(string& A, void* Src) { A = A + Src; return A; }
inline string operator+(string A, string Src) { AppendStringToString(Src, &A); return A; }
inline string& operator+=(string& A, string Src) { A = A + Src; return A; }
inline string operator+(string A, isz Integer) { AppendIntToString(Integer, &A); return A; }
inline string& operator+=(string& A, isz Integer) { A = A + Integer; return A; }
inline bool operator==(string A, string B) { return EqualBuffers(A.Buffer, B.Buffer); }

#endif //__cplusplus

#if !defined(TT_STATIC_LINKING)
#include "tinybase-encoding.c"
#include "tinybase-strings.c"
#endif

#endif //TINYBASE_STRINGS_H
