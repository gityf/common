
#ifndef _C_RECORD_FORMAT_ATOM_H_
#define _C_RECORD_FORAMT_ATOM_H_

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include "scs/maskatom.h"

class RecordFormatAtom
{
private:
    int         m_iFlag;
    MaskAtom    m_CORIFormat;
    MaskAtom    m_CNEWFormat;

public:
    RecordFormatAtom();
    ~RecordFormatAtom();

    int InitFormat(const char* p_pchORIFormat, const char* p_pchNEWFormat);
    int FormatString(char *p_pchORI, char *p_pchNEW);

    int IsValidFormat();
    int SetValidFormat(int p_iFlag);
};

#endif // _C_RECORD_FORMAT_ATOM_H_

