//========================================
// Support functions
//========================================

internal int
_GetECEndian(encoding Enc)
{
    return (Enc == EC_UTF16BE || Enc == EC_UTF32BE) ? 0 : 1;
}

//========================================
// Ascii
//========================================

internal usz
_AsciiLenPrintChar(string A)
{
    usz Result = 0;
    for (usz Idx = 0; Idx < A.WriteCur; Idx++)
    {
        if (A.Base[Idx] >= 0x20 && A.Base[Idx] < 0x7f)
        {
            Result++;
        }
    }
    return Result;
}

//========================================
// UTF-8
//========================================

internal u32
_UTF8CharSize(void* Ptr)
{
    u8* Char = (u8*)Ptr;
    if (Char[0] <= 0x7f)      return 1;
    if (Char[0] >> 5 == 0x6)  return 2;
    if (Char[0] >> 4 == 0xe)  return 3;
    if (Char[0] >> 3 == 0x1e) return 4;
    else return 1; // OBS: In case of error.
}

internal usz
_UTF8LenCodePoints(string A)
{
    usz Result = 0;
    for (usz Idx = 0; Idx < A.WriteCur; Idx++)
    {
        if ((A.Base[Idx] & 0xc0) != 0x80)
        {
            Result++;
        }
    }
    return Result;
}

internal usz
_UTF8LenPrintChar(string A)
{
    // TODO: Verify combining characters and reserved characters.
    usz Result = 0;
    for (char* Ptr = A.Base; Ptr < A.Base + A.WriteCur; )
    {
        u32 Size = _UTF8CharSize(Ptr);
        u32 Bytes = 0;
        switch (Size)
        {
            case 1: Bytes = Ptr[0]; break;
            case 2: Bytes = Ptr[1] << 8  | Ptr[0]; break;
            case 3: Bytes = Ptr[2] << 16 | Ptr[1] << 8  | Ptr[0]; break;
            case 4: Bytes = Ptr[3] << 24 | Ptr[2] << 16 | Ptr[1] << 8 | Ptr[0]; break;
        }
        
        if ((Bytes >= 0x20 && Bytes < 0x7f) || Bytes > 0x9f) Result++;
        Ptr += Size;
    }
    return Result;
}

internal uchar
_UTF8ToUnicode(mb_char Char)
{
	if (Char < 0x80) return Char;
	if (Char < 0xFFFF) return (((Char & 0x1f)<<6) | ((Char & 0x3f00)>>8));
	if (Char < 0xFFFFFF) return (((Char & 0x0f)<<12) | ((Char & 0x3f00)>>2) | ((Char & 0x3f0000)>>16));
	return (((Char & 0x07)<<18) | ((Char & 0x3F00)<<4) | ((Char & 0x3f0000)>>10) | (Char & 0x3f000000)>>24);
}

internal mb_char
_UnicodeToUTF8(uchar Char)
{
    if (Char < 0x80) return Char;
	if (Char < 0x800) return ((Char & 0x3f | 0x80) << 8) + ((Char >> 6 & 0x1F | 0xc0));
	if (Char < 0x10000) return ((Char & 0x3f | 0x80) << 16) + ((Char >> 6 & 0x3f | 0x80) << 8) + ((Char >> 12 & 0xf | 0xe0));
	return ((Char & 0x3f | 0x80) << 24) + ((Char >> 6 & 0x3f | 0x80) << 16) + ((Char >> 12 & 0x3f | 0x80) << 8) + ((Char >> 18 & 0x7 | 0xf0));
}

//========================================
// UTF-16
//========================================

internal u32
_UTF16LECharSize(void* Ptr)
{
    u8* Char = (u8*)Ptr;
    u32 Result = (Char[1] >= 0xd8 && Char[1] < 0xe0) ? 4 : 2;
    return Result;
}

internal u32
_UTF16BECharSize(void* Ptr)
{
    u8* Char = (u8*)Ptr;
    u32 Result = (Char[0] >= 0xd8 && Char[0] < 0xe0) ? 4 : 2;
    return Result;
}

internal usz
_UTF16LELenCodePoints(string A)
{
    usz Result = 0;
    for (usz Idx = 0; Idx < A.WriteCur; )
    {
        Result++;
        Idx += _UTF16LECharSize(A.Base + Idx);
    }
    return Result;
}

internal usz
_UTF16BELenCodePoints(string A)
{
    usz Result = 0;
    for (usz Idx = 0; Idx < A.WriteCur; )
    {
        Result++;
        Idx += _UTF16BECharSize(A.Base + Idx);
    }
    return Result;
}

internal usz
_UTF16LELenPrintChar(string A)
{
    // TODO: Verify combining characters and reserved characters.
    usz Result = 0;
    
    mb_char Char = 0;
    for (char* Ptr = A.Base; Ptr < A.Base + A.WriteCur; )
    {
        u32 Size = _UTF16LECharSize(Ptr);
        switch (Size)
        {
            case 2: Char = Ptr[1] << 8  | Ptr[0]; break;
            case 4: Char = Ptr[3] << 24 | Ptr[2] << 16 | Ptr[1] << 8 | Ptr[0]; break;
        }
        Result += ((Char >= 0x20 && Char < 0x7f) || Char > 0x9f);
        Ptr += (Char > 0xFFFF) ? 4 : 2;
    }
    
    return Result;
}

internal usz
_UTF16BELenPrintChar(string A)
{
    // TODO: Verify combining characters and reserved characters.
    usz Result = 0;
    
    mb_char Char = 0;
    for (char* Ptr = A.Base; Ptr < A.Base + A.WriteCur; )
    {
        u32 Size = _UTF16BECharSize(Ptr);
        switch (Size)
        {
            case 2: Char = Ptr[0] << 8  | Ptr[1]; break;
            case 4: Char = Ptr[2] << 24 | Ptr[3] << 16 | Ptr[0] << 8 | Ptr[1]; break;
        }
        Result += ((Char >= 0x20 && Char < 0x7f) || Char > 0x9f);
        Ptr += (Char > 0xFFFF) ? 4 : 2;
    }
    
    return Result;
}

internal usz
_UTF16LenCString(string A)
{
    u16* Ptr = (u16*)A.Base;
    while (*Ptr) { Ptr++; }
    usz Result = (usz)Ptr - (usz)A.Base;
    return Result;
}

internal uchar
_UTF16ToUnicode(mb_char Char)
{
	if (Char > 0xFFFF)
	{
		u32 A = ((Char & 0xFFFF) - 0xD800) * 0x400;
		u32 B = ((Char >> 16) & 0xFFFF) - 0xDC00;
		uchar R = 0x10000 + A + B;
		return R;
	}
	return Char;
}

internal mb_char
_UnicodeToUTF16(uchar Char)
{
    mb_char Result = Char;
	if (Char > 0xFFFF)
	{
		Char -= 0x10000;
		u32 A = (Char >> 10) + 0xd800;
		u32 B = (Char & 0x3ff) + 0xdc00;
		Result = (B << 16) + A;
	}
    return Result;
}

internal bool
_TranscodeUTF16BE(string Src, string* Dst)
{
    usz StartWriteCur = Dst->WriteCur;
    usz SrcIdx = 0;
    while (SrcIdx < Src.WriteCur)
    {
        mb_char Char = GetNextChar(Src.Base+SrcIdx, Src.Enc);
        u32 CharSize = GetMultibyteCharSize(Char, Src.Enc);
        
        uchar Decoded = DecodeChar(Char, Src.Enc);
        mb_char Encoded = EncodeChar(Decoded, Dst->Enc);
        u32 EncodedSize = GetMultibyteCharSize(Encoded, Dst->Enc);
        
        if (Dst->WriteCur + EncodedSize <= Dst->Size)
        {
            u8* _Dst = (u8*)Dst->Base + Dst->WriteCur;
            u8* _Src = (u8*)&Encoded;
            switch (EncodedSize)
            {
                case 2: { _Dst[0] = _Src[1]; _Dst[1] = _Src[0]; } break;
                case 4: { _Dst[0] = _Src[1]; _Dst[1] = _Src[0]; _Dst[2] = _Src[3]; _Dst[3] = _Src[2]; } break;
            }
            Dst->WriteCur += EncodedSize;
            SrcIdx += CharSize;
        }
        else
        {
            Dst->WriteCur = StartWriteCur;
            return false;
        }
    }
    return true;
}

//========================================
// UTF-32
//========================================

internal usz
_UTF32LELenPrintChar(string A)
{
    // TODO: Verify combining characters and reserved characters.
    usz Result = 0;
    
    for (char* Ptr = A.Base; Ptr < A.Base + A.WriteCur; Ptr += 4)
    {
        mb_char Bytes = Ptr[3] << 24 | Ptr[2] << 16 | Ptr[1] << 8 | Ptr[0];
        Result += ((Bytes >= 0x20 && Bytes < 0x7f) || Bytes > 0x9f);
    }
    
    return Result;
}

internal usz
_UTF32BELenPrintChar(string A)
{
    // TODO: Verify combining characters and reserved characters.
    usz Result = 0;
    
    for (char* Ptr = A.Base; Ptr < A.Base + A.WriteCur; Ptr += 4)
    {
        mb_char Bytes = Ptr[0] << 24 | Ptr[1] << 16 | Ptr[2] << 8 | Ptr[3];
        Result += ((Bytes >= 0x20 && Bytes < 0x7f) || Bytes > 0x9f);
    }
    
    return Result;
}

internal usz
_UTF32LenCString(string A)
{
    u32* Ptr = (u32*)A.Base;
    while (*Ptr) { Ptr++; }
    usz Result = (usz)Ptr - (usz)A.Base;
    return Result;
}

internal bool
_TranscodeUTF32BE(string Src, string* Dst)
{
    usz StartWriteCur = Dst->WriteCur;
    usz SrcIdx = 0;
    while (SrcIdx < Src.WriteCur)
    {
        mb_char Char = GetNextChar(Src.Base+SrcIdx, Src.Enc);
        u32 CharSize = GetMultibyteCharSize(Char, Src.Enc);
        
        uchar Decoded = DecodeChar(Char, Src.Enc);
        mb_char Encoded = EncodeChar(Decoded, Dst->Enc);
        u32 EncodedSize = 4;
        
        if (Dst->WriteCur + EncodedSize <= Dst->Size)
        {
            u8* _Dst = (u8*)Dst->Base + Dst->WriteCur;
            u8* _Src = (u8*)&Encoded;
            _Dst[0] = _Src[3]; _Dst[1] = _Src[2]; _Dst[2] = _Src[1]; _Dst[3] = _Src[0];
            Dst->WriteCur += EncodedSize;
            SrcIdx += CharSize;
        }
        else
        {
            Dst->WriteCur = StartWriteCur;
            return false;
        }
    }
    return true;
}
