
#ifndef _C_SERVICE_LIST_H_
#define _C_SERVICE_LIST_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include "scs/serviceatom.h"
#define MAX_SERVICE_ATOM_COUNTS 32
class ServiceList
{
private:
    ServiceAtom         *iServiceAtom;
    int                 m_iAtomNum;
    int                 m_iIDList[MAX_SERVICE_ATOM_COUNTS];

    int                 m_iSortMax;
    int                 m_iTotal;
public:
    ServiceList();
    ~ServiceList();

    int Init(const string& aFormatFile);
    int ProcessLog(char *p_pchLog, char *p_pchOut);

    int SortCServiceAtoms(int p_iSortFlag, int p_iZeroFlag);
};

#endif // _C_SERVICE_LIST_H_

