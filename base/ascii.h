/*
** Copyright (C) 2015 Wang Yaofu
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: Header file of ASCII.
** the file is copy from Jack <lapsangx@gmail.com>.
*/

#pragma once

#include <limits.h>
#include <stdint.h>

struct Ascii
{
private:
    Ascii();
    ~Ascii();
private:
    // mask for character.
    enum CharTypeMask
    {
        kUpper    = 1 << 0,
        kLower    = 1 << 1,
        kDigit    = 1 << 2,
        kHexDigit = 1 << 3,
        kBlank    = 1 << 4,
        kSpace    = 1 << 5,
        kControl  = 1 << 6,
        kPunct    = 1 << 7,
        kPrint    = 1 << 8,
        kGraph    = 1 << 9,
    };
public:
    // the character is ASCII or not.
    static bool IsValid(char c)
    {
        return (c & 0x80) == 0;
    }

    static inline bool IsLower(char c)
    {
        return CharIncludeAnyTypeMask(c, kLower);
    }

    static inline bool IsUpper(char c)
    {
        return CharIncludeAnyTypeMask(c, kUpper);
    }

    // the character is alpha or not.
    static bool IsAlpha(char c)
    {
        return CharIncludeAnyTypeMask(c, kUpper | kLower);
    }

    // the character is number or not.
    static bool IsDigit(char c)
    {
        return CharIncludeAnyTypeMask(c, kDigit);
    }

    // the character is number or alpha or not.
    static bool IsAlphaNumber(char c)
    {
        return CharIncludeAnyTypeMask(c, kUpper | kLower | kDigit);
    }

    // the character is space,\t etc or not.
    static bool IsBlank(char c)
    {
        return CharIncludeAnyTypeMask(c, kBlank);
    }

    // the character is space,\t etc or not.
    static inline bool IsSpace(char c)
    {
        return CharIncludeAnyTypeMask(c, kSpace);
    }

    // the character is control or not.
    static bool IsControl(char c)
    {
        return CharIncludeAnyTypeMask(c, kControl);
    }

    // the character is symbol or not.
    static inline bool IsPunct(char c)
    {
        return CharIncludeAnyTypeMask(c, kPunct);
    }

    // the character is 0-9,a-f or not.
    static inline bool IsHexDigit(char c)
    {
        return CharIncludeAnyTypeMask(c, kHexDigit);
    }

    // the character can be graphable or not.
    static inline bool IsGraph(char c)
    {
        return CharIncludeAnyTypeMask(c, kGraph);
    }

    // the character can be printable or not.
    static inline bool IsPrint(char c)
    {
        return CharIncludeAnyTypeMask(c, kPrint);
    }

    static inline char ToAscii(char c)
    {
        return c & 0x7F;
    }

    static inline char ToLower(char c)
    {
        return IsUpper(c) ? c + ('a' - 'A') : c;
    }

    static inline char ToUpper(char c)
    {
        return IsLower(c) ? c - ('a' - 'A') : c;
    }

private:
    static inline int GetCharTypeMask(char c)
    {
#if 0
        // the table is created by code below.
        // #include <ctype.h>
        // #include <stdio.h>

        // int main()
        // {
        //     for (int i = 0; i < 128; ++i)
        //     {
        //         printf("            /* 0x%02x(%c) */ ", i, isgraph(i) ? i : ' ');
        //         if (isblank(i)) printf("kBlank | ");
        //         if (isspace(i)) printf("kSpace | ");
        //         if (isupper(i)) printf("kUpper | ");
        //         if (islower(i)) printf("kLower | ");
        //         if (isdigit(i)) printf("kDigit | ");
        //         if (isxdigit(i)) printf("kHexDigit | ");
        //         if (ispunct(i)) printf("kPunct | ");
        //         if (iscntrl(i)) printf("kControl | ");
        //         if (isgraph(i)) printf("kGraph | ");
        //         if (isprint(i)) printf("kPrint | ");
        //         printf("0,\n");
        //     }
        // }

        // build and run command as below.
        // $ LC_ALL=C ./a.out
#endif
        static const uint16_t table[UCHAR_MAX + 1] =
        {
            /* 0x00( ) */ kControl | 0,
            /* 0x01( ) */ kControl | 0,
            /* 0x02( ) */ kControl | 0,
            /* 0x03( ) */ kControl | 0,
            /* 0x04( ) */ kControl | 0,
            /* 0x05( ) */ kControl | 0,
            /* 0x06( ) */ kControl | 0,
            /* 0x07( ) */ kControl | 0,
            /* 0x08( ) */ kControl | 0,
            /* 0x09( ) */ kBlank | kSpace | kControl | 0,
            /* 0x0a( ) */ kSpace | kControl | 0,
            /* 0x0b( ) */ kSpace | kControl | 0,
            /* 0x0c( ) */ kSpace | kControl | 0,
            /* 0x0d( ) */ kSpace | kControl | 0,
            /* 0x0e( ) */ kControl | 0,
            /* 0x0f( ) */ kControl | 0,
            /* 0x10( ) */ kControl | 0,
            /* 0x11( ) */ kControl | 0,
            /* 0x12( ) */ kControl | 0,
            /* 0x13( ) */ kControl | 0,
            /* 0x14( ) */ kControl | 0,
            /* 0x15( ) */ kControl | 0,
            /* 0x16( ) */ kControl | 0,
            /* 0x17( ) */ kControl | 0,
            /* 0x18( ) */ kControl | 0,
            /* 0x19( ) */ kControl | 0,
            /* 0x1a( ) */ kControl | 0,
            /* 0x1b( ) */ kControl | 0,
            /* 0x1c( ) */ kControl | 0,
            /* 0x1d( ) */ kControl | 0,
            /* 0x1e( ) */ kControl | 0,
            /* 0x1f( ) */ kControl | 0,
            /* 0x20( ) */ kBlank | kSpace | kPrint | 0,
            /* 0x21(!) */ kPunct | kGraph | kPrint | 0,
            /* 0x22(") */ kPunct | kGraph | kPrint | 0,
            /* 0x23(#) */ kPunct | kGraph | kPrint | 0,
            /* 0x24($) */ kPunct | kGraph | kPrint | 0,
            /* 0x25(%) */ kPunct | kGraph | kPrint | 0,
            /* 0x26(&) */ kPunct | kGraph | kPrint | 0,
            /* 0x27(') */ kPunct | kGraph | kPrint | 0,
            /* 0x28(() */ kPunct | kGraph | kPrint | 0,
            /* 0x29()) */ kPunct | kGraph | kPrint | 0,
            /* 0x2a(*) */ kPunct | kGraph | kPrint | 0,
            /* 0x2b(+) */ kPunct | kGraph | kPrint | 0,
            /* 0x2c(,) */ kPunct | kGraph | kPrint | 0,
            /* 0x2d(-) */ kPunct | kGraph | kPrint | 0,
            /* 0x2e(.) */ kPunct | kGraph | kPrint | 0,
            /* 0x2f(/) */ kPunct | kGraph | kPrint | 0,
            /* 0x30(0) */ kDigit | kHexDigit | kGraph | kPrint | 0,
            /* 0x31(1) */ kDigit | kHexDigit | kGraph | kPrint | 0,
            /* 0x32(2) */ kDigit | kHexDigit | kGraph | kPrint | 0,
            /* 0x33(3) */ kDigit | kHexDigit | kGraph | kPrint | 0,
            /* 0x34(4) */ kDigit | kHexDigit | kGraph | kPrint | 0,
            /* 0x35(5) */ kDigit | kHexDigit | kGraph | kPrint | 0,
            /* 0x36(6) */ kDigit | kHexDigit | kGraph | kPrint | 0,
            /* 0x37(7) */ kDigit | kHexDigit | kGraph | kPrint | 0,
            /* 0x38(8) */ kDigit | kHexDigit | kGraph | kPrint | 0,
            /* 0x39(9) */ kDigit | kHexDigit | kGraph | kPrint | 0,
            /* 0x3a(:) */ kPunct | kGraph | kPrint | 0,
            /* 0x3b(;) */ kPunct | kGraph | kPrint | 0,
            /* 0x3c(<) */ kPunct | kGraph | kPrint | 0,
            /* 0x3d(=) */ kPunct | kGraph | kPrint | 0,
            /* 0x3e(>) */ kPunct | kGraph | kPrint | 0,
            /* 0x3f(?) */ kPunct | kGraph | kPrint | 0,
            /* 0x40(@) */ kPunct | kGraph | kPrint | 0,
            /* 0x41(A) */ kUpper | kHexDigit | kGraph | kPrint | 0,
            /* 0x42(B) */ kUpper | kHexDigit | kGraph | kPrint | 0,
            /* 0x43(C) */ kUpper | kHexDigit | kGraph | kPrint | 0,
            /* 0x44(D) */ kUpper | kHexDigit | kGraph | kPrint | 0,
            /* 0x45(E) */ kUpper | kHexDigit | kGraph | kPrint | 0,
            /* 0x46(F) */ kUpper | kHexDigit | kGraph | kPrint | 0,
            /* 0x47(G) */ kUpper | kGraph | kPrint | 0,
            /* 0x48(H) */ kUpper | kGraph | kPrint | 0,
            /* 0x49(I) */ kUpper | kGraph | kPrint | 0,
            /* 0x4a(J) */ kUpper | kGraph | kPrint | 0,
            /* 0x4b(K) */ kUpper | kGraph | kPrint | 0,
            /* 0x4c(L) */ kUpper | kGraph | kPrint | 0,
            /* 0x4d(M) */ kUpper | kGraph | kPrint | 0,
            /* 0x4e(N) */ kUpper | kGraph | kPrint | 0,
            /* 0x4f(O) */ kUpper | kGraph | kPrint | 0,
            /* 0x50(P) */ kUpper | kGraph | kPrint | 0,
            /* 0x51(Q) */ kUpper | kGraph | kPrint | 0,
            /* 0x52(R) */ kUpper | kGraph | kPrint | 0,
            /* 0x53(S) */ kUpper | kGraph | kPrint | 0,
            /* 0x54(T) */ kUpper | kGraph | kPrint | 0,
            /* 0x55(U) */ kUpper | kGraph | kPrint | 0,
            /* 0x56(V) */ kUpper | kGraph | kPrint | 0,
            /* 0x57(W) */ kUpper | kGraph | kPrint | 0,
            /* 0x58(X) */ kUpper | kGraph | kPrint | 0,
            /* 0x59(Y) */ kUpper | kGraph | kPrint | 0,
            /* 0x5a(Z) */ kUpper | kGraph | kPrint | 0,
            /* 0x5b([) */ kPunct | kGraph | kPrint | 0,
            /* 0x5c(\) */ kPunct | kGraph | kPrint | 0,
            /* 0x5d(]) */ kPunct | kGraph | kPrint | 0,
            /* 0x5e(^) */ kPunct | kGraph | kPrint | 0,
            /* 0x5f(_) */ kPunct | kGraph | kPrint | 0,
            /* 0x60(`) */ kPunct | kGraph | kPrint | 0,
            /* 0x61(a) */ kLower | kHexDigit | kGraph | kPrint | 0,
            /* 0x62(b) */ kLower | kHexDigit | kGraph | kPrint | 0,
            /* 0x63(c) */ kLower | kHexDigit | kGraph | kPrint | 0,
            /* 0x64(d) */ kLower | kHexDigit | kGraph | kPrint | 0,
            /* 0x65(e) */ kLower | kHexDigit | kGraph | kPrint | 0,
            /* 0x66(f) */ kLower | kHexDigit | kGraph | kPrint | 0,
            /* 0x67(g) */ kLower | kGraph | kPrint | 0,
            /* 0x68(h) */ kLower | kGraph | kPrint | 0,
            /* 0x69(i) */ kLower | kGraph | kPrint | 0,
            /* 0x6a(j) */ kLower | kGraph | kPrint | 0,
            /* 0x6b(k) */ kLower | kGraph | kPrint | 0,
            /* 0x6c(l) */ kLower | kGraph | kPrint | 0,
            /* 0x6d(m) */ kLower | kGraph | kPrint | 0,
            /* 0x6e(n) */ kLower | kGraph | kPrint | 0,
            /* 0x6f(o) */ kLower | kGraph | kPrint | 0,
            /* 0x70(p) */ kLower | kGraph | kPrint | 0,
            /* 0x71(q) */ kLower | kGraph | kPrint | 0,
            /* 0x72(r) */ kLower | kGraph | kPrint | 0,
            /* 0x73(s) */ kLower | kGraph | kPrint | 0,
            /* 0x74(t) */ kLower | kGraph | kPrint | 0,
            /* 0x75(u) */ kLower | kGraph | kPrint | 0,
            /* 0x76(v) */ kLower | kGraph | kPrint | 0,
            /* 0x77(w) */ kLower | kGraph | kPrint | 0,
            /* 0x78(x) */ kLower | kGraph | kPrint | 0,
            /* 0x79(y) */ kLower | kGraph | kPrint | 0,
            /* 0x7a(z) */ kLower | kGraph | kPrint | 0,
            /* 0x7b({) */ kPunct | kGraph | kPrint | 0,
            /* 0x7c(|) */ kPunct | kGraph | kPrint | 0,
            /* 0x7d(}) */ kPunct | kGraph | kPrint | 0,
            /* 0x7e(~) */ kPunct | kGraph | kPrint | 0,
            /* 0x7f( ) */ kControl | 0,
            // ����ȫΪ 0
        };
        return table[static_cast<unsigned char>(c)];
    }

    static bool CharIncludeAnyTypeMask(char c, int mask)
    {
        return (GetCharTypeMask(c) & mask) != 0;
    }

    static bool CharIncludeAallTypeMask(char c, int mask)
    {
        return (GetCharTypeMask(c) & mask) == mask;
    }
};

#endif // _COMMON_ASCII_H
