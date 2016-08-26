
#ifndef _C_SERVICE_ATOM_H_
#define _C_SERVICE_ATOM_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <string>
#include "scs/recordformat.h"
#include "scs/masksearch.h"
using std::string;
class ServiceAtom
{
private:
    char            m_szServiceName[1024];
    char            m_szServiceMask[1024];
    RecordFormat    m_CCRecordFormat;
    MaskSearch      m_CCMaskSearch;

    int             m_iFlag;
    int             m_iRecNum;

public:
    ServiceAtom();
    ~ServiceAtom();

    int Init(const string& aServiceName);
    int MatchRecord(char *p_pchRec, char *p_pchOut);

    int GetFlag();
    int SetFlag(int p_iFlag);
    int GetRecNum();
    int SetRecNum(int p_iRecNum);
};


#endif // _C_SERVICE_ATOM_H_

