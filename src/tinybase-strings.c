//========================================
// Creation
//========================================

external string
String(void* Base, usz WriteCur, usz Size, encoding EC)
{
    string Result = { (char*)Base, WriteCur, Size, EC };
    return Result;
}

external string
StringC(void* Base, encoding Enc)
{
    string Result = String(Base, 0, 0, Enc);
    Result.WriteCur = StringLen(Result, LEN_CSTRING);
    return Result;
}

//========================================
// Read
//========================================

external u32
GetMultibyteCharSize(mb_char Char, encoding Enc)
{
    switch (Enc)
    {
        case EC_UTF16LE:
        case EC_UTF16BE: return (Char <= 0xFFFF) ? 2 : 4;
        case EC_UTF32LE:
        case EC_UTF32BE: return 4;
        default:
        {
            if (Char <= 0xFF) return 1;
            if (Char <= 0xFFFF) return 2;
            if (Char <= 0xFFFFFF) return 3;
            return 4;
        }
    }
}

external u32
GetNextCharSize(void* Src, encoding Enc)
{
    switch (Enc)
    {
        case EC_UTF8:    return _UTF8CharSize(Src);
        case EC_UTF16LE: return _UTF16LECharSize(Src);
        case EC_UTF16BE: return _UTF16BECharSize(Src);
        case EC_UTF32LE:
        case EC_UTF32BE: return 4;
        default:         return 1;
    }
}

internal mb_char
_AdjustEndianness(mb_char Char, encoding Enc)
{
    if (Enc == EC_UTF16BE) Char = FlipEndian16x2(Char);
    else if (Enc == EC_UTF32BE) Char = FlipEndian32(Char);
    return Char;
}

external mb_char
GetNextChar(void* Src, encoding Enc)
{
    u8* Ptr = (u8*)Src;
    u32 Size = GetNextCharSize(Src, Enc);
    mb_char Bytes = 0;
    switch (Size)
    {
        case 1: Bytes = Ptr[0]; break;
        case 2: Bytes = Ptr[1] << 8  | Ptr[0]; break;
        case 3: Bytes = Ptr[2] << 16 | Ptr[1] << 8  | Ptr[0]; break;
        case 4: Bytes = Ptr[3] << 24 | Ptr[2] << 16 | Ptr[1] << 8 | Ptr[0]; break;
    }
    Bytes = _AdjustEndianness(Bytes, Enc);
    return Bytes;
}

external mb_char
EatNextChar(void** Src, encoding Enc)
{
    u8* Ptr = *(u8**)Src;
    u32 Size = GetNextCharSize(Src, Enc);
    mb_char Bytes = 0;
    switch (Size)
    {
        case 1: Bytes = Ptr[0]; break;
        case 2: Bytes = Ptr[1] << 8  | Ptr[0]; break;
        case 3: Bytes = Ptr[2] << 16 | Ptr[1] << 8  | Ptr[0]; break;
        case 4: Bytes = Ptr[3] << 24 | Ptr[2] << 16 | Ptr[1] << 8 | Ptr[0]; break;
    }
    Bytes = _AdjustEndianness(Bytes, Enc);
    *Src = Ptr;
    return Bytes;
}

external usz
StringLen(string A, len_type LenType)
{
    if (LenType == LEN_CODEPOINT)
    {
        switch (A.Enc)
        {
            case EC_ASCII:   return A.WriteCur;
            case EC_UTF8:    return _UTF8LenCodePoints(A);
            case EC_UTF16LE: return _UTF16LELenCodePoints(A);
            case EC_UTF16BE: return _UTF16BELenCodePoints(A);
            case EC_UTF32LE:
            case EC_UTF32BE: return A.WriteCur/4;
            default: return 0;
        }
    }
    
    else if (LenType == LEN_PRINTCHAR)
    {
        switch (A.Enc)
        {
            case EC_ASCII:   return _AsciiLenPrintChar(A);
            case EC_UTF8:    return _UTF8LenPrintChar(A);
            case EC_UTF16LE: return _UTF16LELenPrintChar(A);
            case EC_UTF16BE: return _UTF16BELenPrintChar(A);
            case EC_UTF32LE: return _UTF32LELenPrintChar(A);
            case EC_UTF32BE: return _UTF32BELenPrintChar(A);
            default: return 0;
        }
    }
    
    else // LEN_CSTRING
    {
        switch (A.Enc)
        {
            case EC_UTF16LE:
            case EC_UTF16BE: return _UTF16LenCString(A);
            case EC_UTF32LE:
            case EC_UTF32BE: return _UTF32LenCString(A);
            default:         return strlen(A.Base);
        }
    }
}

//========================================
// Interpret
//========================================

external bool
CharIsDigit(mb_char Char)
{
    return (Char >= '0' && Char <= '9');
}

external bool
CharIsHexDigit(mb_char Char)
{
    return ((Char >= '0' && Char <= '9') || (Char >= 'A' && Char <= 'F') || (Char >= 'a' && Char <= 'f'));
}

external bool
CharIsLetter(mb_char Char)
{
    return ((Char >= 'a' && Char <= 'z') || (Char >= 'A' && Char <= 'Z'));
}

external bool
CharIsAlphanum(mb_char Char)
{
    return ((Char >= 'a' && Char <= 'z') || (Char >= 'A' && Char <= 'Z') || (Char >= '0' && Char <= '9'));
}

external bool
StringIsDigit(string Src)
{
    encoding EC = (Src.Enc == EC_UTF8) ? EC_ASCII : Src.Enc;
    usz Offset = EC >= EC_UTF32LE ? 4 : EC >= EC_UTF16LE ? 2 : 1;
    for (usz Idx = 0; Idx < Src.WriteCur; Idx += Offset)
    {
        mb_char Char = GetNextChar(Src.Base + Idx, EC);
        if (!CharIsDigit(Char))
        {
            return false;
        }
    }
    return true;
}

external bool
StringIsLetter(string Src)
{
    encoding EC = (Src.Enc == EC_UTF8) ? EC_ASCII : Src.Enc;
    usz Offset = EC >= EC_UTF32LE ? 4 : EC >= EC_UTF16LE ? 2 : 1;
    for (usz Idx = 0; Idx < Src.WriteCur; Idx += Offset)
    {
        mb_char Char = GetNextChar(Src.Base + Idx, EC);
        if (!CharIsLetter(Char))
        {
            return false;
        }
    }
    return true;
}

external bool
StringIsAlphanum(string Src)
{
    encoding EC = (Src.Enc == EC_UTF8) ? EC_ASCII : Src.Enc;
    usz Offset = EC >= EC_UTF32LE ? 4 : EC >= EC_UTF16LE ? 2 : 1;
    for (usz Idx = 0; Idx < Src.WriteCur; Idx += Offset)
    {
        mb_char Char = GetNextChar(Src.Base + Idx, EC);
        if (!CharIsAlphanum(Char))
        {
            return false;
        }
    }
    return true;
}

external isz
StringToInt(string Src)
{
    i64 Result = 0;
    
    mb_char Char = GetNextChar(Src.Base, Src.Enc);
    i32 CharSize = Src.Enc >= EC_UTF32LE ? 4 : Src.Enc >= EC_UTF16LE ? 2 : 1;
    
    i64 Sign = (Char == '-') ? -1 : 1;
    usz Idx = (Char == '-') ? CharSize : 0;
    while (Idx < Src.WriteCur)
    {
        Char = GetNextChar(Src.Base + Idx, Src.Enc);
        if (!CharIsDigit(Char))
        {
            return ISZ_MAX;
        }
        Result = Result * 10 + Char - '0';
        Idx += CharSize;
    }
    return Result * Sign;
}

external usz
StringToUInt(string Src)
{
    u64 Result = 0;
    
    i32 CharSize = Src.Enc >= EC_UTF32LE ? 4 : Src.Enc >= EC_UTF16LE ? 2 : 1;
    for (usz Idx = 0; Idx < Src.WriteCur; )
    {
        mb_char Char = GetNextChar(Src.Base + Idx, Src.Enc);
        if (!CharIsDigit(Char))
        {
            return USZ_MAX;
        }
        Result = Result * 10 + Char - '0';
        Idx += CharSize;
    }
    return Result;
}

external usz
StringToHex(string Src)
{
    usz Result = 0;
    
    i32 CharSize = Src.Enc >= EC_UTF32LE ? 4 : Src.Enc >= EC_UTF16LE ? 2 : 1;
    for (usz Idx = 0; Idx < Src.WriteCur; )
    {
        mb_char Char = GetNextChar(Src.Base + Idx, Src.Enc);
        if (CharIsDigit(Char))
        {
            Result = Result * 16 + Char - '0';
        }
        else if (CharIsHexDigit(Char))
        {
            Result = Result * 16 + Char - 'A' + 10;
        }
        else
        {
            return USZ_MAX;
        }
        
        Idx += CharSize;
    }
    return Result;
}

external f64
StringToFloat(string Src)
{
    mb_char Char = GetNextChar(Src.Base, Src.Enc);
    i32 CharSize = Src.Enc >= EC_UTF32LE ? 4 : Src.Enc >= EC_UTF16LE ? 2 : 1;
    
    isz NumberSign = (Char == '-') ? -1 : 1;
    isz ExponentSign = 1;
    
    usz Integer = 0;
    usz Fraction = 0;
    usz Exponent = 0;
    
    usz ParsingStage = 0; // OBS: 0 (Integer), 1 (Fractional), 2 (Exponent).
    usz Idx = (Char == '-') ? CharSize : 0;
    usz FractionCount = 0;
    
    while (Idx < Src.WriteCur)
    {
        Char = GetNextChar(Src.Base + Idx, Src.Enc);
        
        if (ParsingStage == 0)
        {
            if (Char == '.') ParsingStage = 1;
            else if (Char == 'e') ParsingStage = 2;
            else if (CharIsDigit(Char)) Integer = Integer * 10 + Char - '0';
            else return DBL_MAX;
        }
        
        else if (ParsingStage == 1)
        {
            if (Char == 'e') ParsingStage = 2;
            else if (CharIsDigit(Char))
            {
                Fraction = Fraction * 10 + Char - '0';
                FractionCount++;
            }
            else return DBL_MAX;
        }
        
        else
        {
            if (Char == '-') ExponentSign = -1;
            else if (CharIsDigit(Char)) Exponent = Exponent * 10 + Char - '0';
            else return DBL_MAX;
        }
        
        Idx += CharSize;
    }
    
    f64 Result = NumberSign * ((f64)Integer + ((f64)Fraction / Pow(10, (f64)FractionCount)));
    Result *= Pow(10, (f64)Exponent * ExponentSign);
    return Result;
}

//========================================
// Query
//========================================

external usz
CharInString(mb_char Needle, string Haystack, int Flags)
{
    usz Result = INVALID_IDX;
    u32 NeedleSize = GetMultibyteCharSize(Needle, Haystack.Enc);
    if (NeedleSize == 1)
    {
        Result = ByteInBuffer((u8)Needle, Haystack.Buffer, Flags);
    }
    else
    {
        buffer NeedleBuf = Buffer((u8*)&Needle, NeedleSize, 0);
        Result = BufferInBuffer(NeedleBuf, Haystack.Buffer, Flags);
    }
    return Result;
}

external usz
StringInString(string Needle, string Haystack, int Flags)
{
    usz Result = INVALID_IDX;
    if (Needle.Enc == Haystack.Enc)
    {
        Result = BufferInBuffer(Needle.Buffer, Haystack.Buffer, Flags);
    }
    return Result;
}

external usz
CountCharInString(mb_char Needle, string Haystack)
{
    // Assumes Needle is in the same encoding as Haystack.
    usz Result = 0;
    for (usz Idx = 0
         ; (Idx = CharInString(Needle, Haystack, RETURN_IDX_AFTER)) != INVALID_IDX
         ; AdvanceBuffer(&Haystack.Buffer, Idx))
    {
        Result++;
    }
    return Result;
}

external usz
CompareStrings(string A, string B, usz AmountToCompare, int Flag)
{
    usz Result = INVALID_IDX;
    if (A.Enc == B.Enc)
    {
        Result = CompareBuffers(A.Buffer, B.Buffer, AmountToCompare, Flag);
    }
    return Result;
}

external bool
EqualStrings(string A, string B)
{
    bool Result = (A.Enc == B.Enc) && (EqualBuffers(A.Buffer, B.Buffer));
    return Result;
}

//========================================
// Transcode
//========================================

external uchar
DecodeChar(mb_char Char, encoding EC)
{
    switch (EC)
    {
        case EC_UTF8:    return _UTF8ToUnicode(Char);
        case EC_UTF16LE:
        case EC_UTF16BE: return _UTF16ToUnicode(Char);
        default:         return Char;
    }
}

external mb_char
EncodeChar(uchar Char, encoding EC)
{
    switch (EC)
    {
        case EC_UTF8:    return _UnicodeToUTF8(Char);
        case EC_UTF16LE:
        case EC_UTF16BE: return _UnicodeToUTF16(Char);
        default:         return Char;
    }
}

internal bool
_Transcode(string Src, string* Dst)
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
                case 1: { _Dst[0] = _Src[0]; } break;
                case 2: { _Dst[0] = _Src[0]; _Dst[1] = _Src[1]; } break;
                case 3: { _Dst[0] = _Src[0]; _Dst[1] = _Src[1]; _Dst[2] = _Src[2]; } break;
                case 4: { _Dst[0] = _Src[0]; _Dst[1] = _Src[1]; _Dst[2] = _Src[2]; _Dst[3] = _Src[3]; } break;
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

external bool
Transcode(string Src, string* Dst)
{
    switch (Dst->Enc)
    {
        case EC_UTF16BE: return _TranscodeUTF16BE(Src, Dst);
        case EC_UTF32BE: return _TranscodeUTF32BE(Src, Dst);
        default: return _Transcode(Src, Dst);
    }
}

external bool
ReplaceCharInString(mb_char Old, mb_char New, string A)
{
    // Assumes [Old], [New] and [A] are same EC.
    
    // TODO: Change it so [Old] and [New] can be different length.
    u32 OldLen = GetMultibyteCharSize(Old, A.Enc);
    u32 NewLen = GetMultibyteCharSize(New, A.Enc);
    if (OldLen != NewLen)
    {
        return false;
    }
    
    if (NewLen == 1)
    {
        ReplaceByteInBuffer(Old, New, A.Buffer);
    }
    else
    {
        usz FoundIdx = 0;
        while ((FoundIdx = CharInString(Old, A, RETURN_IDX_FIND)) != INVALID_IDX)
        {
            AdvanceBuffer(&A.Buffer, FoundIdx);
            CopyData(A.Base, A.WriteCur, &New, NewLen);
            AdvanceBuffer(&A.Buffer, NewLen);
        }
    }
    
    return true;
}

//========================================
// Write
//========================================

external void
AdvanceString(string* Dst, usz NumChars)
{
    while (NumChars--)
    {
        u32 BytesToAdvance = GetNextCharSize(Dst->Base, Dst->Enc);
        Dst->Base += BytesToAdvance;
        Dst->WriteCur -= BytesToAdvance;
        Dst->Size -= BytesToAdvance;
    }
}

external bool
AppendCharToString(mb_char Src, string* Dst)
{
    // Assumes [Src] is same EC as [Dst].
    u32 CharSize = GetMultibyteCharSize(Src, Dst->Enc);
    Src = _AdjustEndianness(Src, Dst->Enc);
    buffer SrcBuffer = Buffer((u8*)&Src, CharSize, 0);
    return AppendBufferToBuffer(SrcBuffer, &Dst->Buffer);
}

external bool
AppendCharToStringNTimes(mb_char Src, usz Count, string* Dst)
{
    // Assumes [Src] is same EC as [Dst].
    u32 CharSize = GetMultibyteCharSize(Src, Dst->Enc);
    Src = _AdjustEndianness(Src, Dst->Enc);
    buffer SrcBuffer = Buffer((u8*)&Src, CharSize, 0);
    return AppendBufferToBufferNTimes(SrcBuffer, Count, &Dst->Buffer);
}

external bool
AppendArrayToString(void* Src, string* Dst)
{
    // Assumes [Src] is same EC as [Dst], and zero-terminated.
    usz NULIdx = StringLen(String(Src, 0, 0, Dst->Enc), LEN_CSTRING);
    return AppendDataToBuffer(Src, NULIdx, &Dst->Buffer);
}

external bool
AppendDataToString(void* Src, usz SrcSize, string* Dst)
{
    // Assumes [Src] is same EC as [Dst].
    return AppendDataToBuffer(Src, SrcSize, &Dst->Buffer);
}

external bool
AppendStringToString(string Src, string* Dst)
{
    if (Src.Enc == Dst->Enc)
    {
        return AppendBufferToBuffer(Src.Buffer, &Dst->Buffer);
    }
    else
    {
        return Transcode(Src, Dst);
    }
}

external bool
AppendStringToStringNTimes(string Src, usz Count, string* Dst)
{
    if (Src.Enc == Dst->Enc)
    {
        return AppendBufferToBufferNTimes(Src.Buffer, Count, &Dst->Buffer);
    }
    else
    {
        for (usz Idx = 0; Idx < Count; Idx++)
        {
            Transcode(Src, Dst);
        }
        return true;
    }
}

external bool
AppendUIntToString(usz Integer, string* Dst)
{
    usz NumDigits = NumberOfDigits(Integer);
    if ((Dst->Size - Dst->WriteCur) >= NumDigits)
    {
        char Digits[USZ_MAX_DIGITS * 4] = {0};
        usz CharSize = (Dst->Enc >= EC_UTF32LE) ? 4 : (Dst->Enc >= EC_UTF16LE) ? 2 : 1;
        usz Offset = (Dst->Enc == EC_UTF32BE) ? 3 : (Dst->Enc == EC_UTF16BE) ? 1 : 0;
        
        char* Ptr = Digits;
        for (usz Digit = 0; Digit < NumDigits; Digit++)
        {
            usz Idx = (NumDigits - Digit - 1) * CharSize + Offset;
            Ptr[Idx] = (Integer % 10) + '0';
            Integer /= 10;
        }
        return AppendDataToBuffer(Digits, NumDigits * CharSize, &Dst->Buffer);
    }
    return false;
}

external bool
AppendIntToString(isz Integer, string* Dst)
{
    usz NumDigits = NumberOfDigits(Integer);
    usz TotalChars = NumDigits + (Integer < 0);
    if ((Dst->Size - Dst->WriteCur) >= TotalChars)
    {
        char Digits[USZ_MAX_DIGITS * 4] = {0};
        usz CharSize = (Dst->Enc >= EC_UTF32LE) ? 4 : (Dst->Enc >= EC_UTF16LE) ? 2 : 1;
        usz Offset = (Dst->Enc == EC_UTF32BE) ? 3 : (Dst->Enc == EC_UTF16BE) ? 1 : 0;
        
        char* Ptr = Digits;
        if (Integer < 0)
        {
            Ptr[Offset] = '-';
            Ptr += CharSize;
        }
        
        Integer = Abs(Integer);
        for (usz Digit = 0; Digit < NumDigits; Digit++)
        {
            usz Idx = (NumDigits - Digit - 1) * CharSize + Offset;
            Ptr[Idx] = (Integer % 10) + '0';
            Integer /= 10;
        }
        
        return AppendDataToBuffer(Digits, TotalChars * CharSize, &Dst->Buffer);
    }
    return false;
}

internal bool
_AppendFloatToStringRegular(f64 Float, usz DecimalPlaces, string* Dst)
{
    isz Hi = Abs((isz)Float);
    f64 Lo = Float - (f64)Hi;
    usz NumDigits = NumberOfDigits(Hi);
    usz TotalChars = NumDigits + (Float < 0);
    
    if (DecimalPlaces > 0)
    {
        usz SpaceLeftForDecimal = USZ_MAX_DIGITS - NumDigits - (Float < 0) - 1;
        DecimalPlaces = Min(SpaceLeftForDecimal, DecimalPlaces);
        TotalChars += DecimalPlaces + 1; // OBS: This +1 is for the . separating High and Low parts.
    }
    
    usz CharSize = (Dst->Enc >= EC_UTF32LE) ? 4 : (Dst->Enc >= EC_UTF16LE) ? 2 : 1;
    if ((Dst->Size - Dst->WriteCur) >= (TotalChars * CharSize))
    {
        char Digits[128] = {0};
        usz Offset = (Dst->Enc == EC_UTF32BE) ? 3 : (Dst->Enc == EC_UTF16BE) ? 1 : 0;
        
        char* Ptr = Digits;
        if (Float < 0)
        {
            Ptr[Offset] = '-';
            Ptr += CharSize;
        }
        
        for (usz Digit = 0; Digit < NumDigits; Digit++)
        {
            usz Idx = (NumDigits - Digit - 1) * CharSize + Offset;
            Ptr[Idx] = (Hi % 10) + '0';
            Hi /= 10;
        }
        
        if (DecimalPlaces > 0)
        {
            Ptr += (NumDigits * CharSize);
            Ptr[Offset] = '.';
            Ptr += CharSize;
            
            usz Decimal = (usz)(Abs(Lo) * Pow(10, (f64)DecimalPlaces));
            for (usz DecimalDigit = 0; DecimalDigit < DecimalPlaces; DecimalDigit++)
            {
                usz Idx = (DecimalPlaces - DecimalDigit - 1) * CharSize + Offset;
                Ptr[Idx] = (Decimal % 10) + '0';
                Decimal /= 10;
            }
        }
        
        return AppendDataToBuffer(Digits, TotalChars * CharSize, &Dst->Buffer);
    }
    return false;
}

internal bool
_AppendFloatToStringInSN(f64 Float, usz DecimalPlaces, string* Dst)
{
    // OBS: Format is (-)I.(D * DecimalPlaces)e(+/-)XXX.
    //      Ex1: 3.8904e-003 , Ex2: -0.000001e+012 , Ex3: 5.0e+000
    usz TotalChars = (Float < 0) + 2 + DecimalPlaces + 5;
    usz CharSize = (Dst->Enc >= EC_UTF32LE) ? 4 : (Dst->Enc >= EC_UTF16LE) ? 2 : 1;
    if ((Dst->Size - Dst->WriteCur) >= (TotalChars * CharSize))
    {
        usz Exp = 0;
        isz ExpSign = 1;
        isz FloatSign = 1;
        usz Offset = (Dst->Enc == EC_UTF32BE) ? 3 : (Dst->Enc == EC_UTF16BE) ? 1 : 0;
        
        char Digits[128] = {0};
        char* Ptr = Digits;
        
        if (Float < 0)
        {
            FloatSign = -1;
            Float *= -1;
            Ptr[Offset] = '-';
            Ptr += CharSize;
        }
        
        if (Float < 1)
        {
            ExpSign = -1;
            while (Float < 1) { Float *= 10; Exp++; }
        }
        else
        {
            while (Float > 10) { Float /= 10; Exp++; }
        }
        
        Ptr[Offset] = (char)Float + '0';
        Ptr[CharSize + Offset] = '.';
        Ptr += CharSize * 2;
        
        if (DecimalPlaces > 0)
        {
            // TODO: Proper rounding.
            f64 Lo = Float - (usz)Float;
            usz Decimal = (usz)(Abs(Lo) * Pow(10, (f64)(DecimalPlaces)));
            for (usz DecimalDigit = 0; DecimalDigit < DecimalPlaces; DecimalDigit++)
            {
                usz Idx = (DecimalPlaces - DecimalDigit - 1) * CharSize + Offset;
                Ptr[Idx] = (Decimal % 10) + '0';
                Decimal /= 10;
            }
            Ptr += DecimalPlaces * CharSize;
        }
        else
        {
            Ptr[Offset] = '0';
            Ptr += CharSize;
        }
        
        Ptr[Offset] = 'e';
        Ptr[CharSize + Offset] = (ExpSign > 0) ? '+' : '-';
        Ptr += CharSize * 2;
        
        for (usz Digit = 0; Digit < 3; Digit++)
        {
            usz Idx = (3 - Digit - 1) * CharSize + Offset;
            Ptr[Idx] = (Exp % 10) + '0';
            Exp /= 10;
        }
        
        return AppendDataToBuffer(Digits, TotalChars * CharSize, &Dst->Buffer);
    }
    return false;
}

external bool
AppendFloatToString(f64 Float, usz DecimalPlaces, bool InScientificNotation, string* Dst)
{
    if (InScientificNotation) return _AppendFloatToStringInSN(Float, DecimalPlaces, Dst);
    else return _AppendFloatToStringRegular(Float, DecimalPlaces, Dst);
}