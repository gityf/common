
#ifndef _C_RECORD_FORMAT_H_
#define _C_RECORD_FORMAT_H_

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <string>
#include "scs/recordformatatom.h"
#define MAX_RECORD_FORMATS 16
using std::string;
class RecordFormat
{
private:
    string iRecordFormatName;

    RecordFormatAtom        *m_CCRecordFormatAtoms;
    int                     m_iNum;

public:
    RecordFormat();
    ~RecordFormat();

    int     Init(const string& aRecordFormatName);
    int     GetFormatString(char *p_pchOri, char *p_pchNew);
    int     GetFormatNumber();
};

#endif // _C_RECORD_FORMAT_H_

