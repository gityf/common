
#include "scs/servicelist.h"
#include "base/localdef.h"
#include "base/log.h"
#include "base/conf.h"
#include "base/singleton.h"
 using common::Conf;
ServiceList::ServiceList() {
    DEBUG(LL_ALL, "ServiceList::ServiceList():Begin");
    iServiceAtom = NULL;
    m_iAtomNum = 0;
    for (int i = 0; i < MAX_SERVICE_ATOM_COUNTS; i++) {
        m_iIDList[i] = i;
    }
    m_iSortMax = 0;
    m_iTotal = 0;
    DEBUG(LL_ALL, "ServiceList::ServiceList():End");
}

ServiceList::~ServiceList() {
    DEBUG(LL_ALL, "ServiceList::~ServiceList():Begin");
    if (iServiceAtom != NULL) {
        delete [] iServiceAtom;
        iServiceAtom = NULL;
    }

    m_iAtomNum = 0;
    DEBUG(LL_ALL, "ServiceList::~ServiceList():End");
}

int ServiceList::Init(const string& aFormatFile) {
    DEBUG(LL_ALL, "Begin");
    DEBUG(LL_INFO, "try to InitConfig...");
    Conf* configPtr = Singleton<Conf>::Instance();
    if (configPtr->parse(aFormatFile) == RET_ERROR) {
        LOG(LL_ERROR, "InitConfig Error -- |%s|\n",
            aFormatFile.c_str());
        return RET_ERROR;
    }

    DEBUG(LL_INFO, "InitConfig success.");

    m_iSortMax = 10000;
    m_iAtomNum = configPtr->getInteger("GLOBAL","ServiceNames", 0);
    LOG(LL_VARS, "GLOBAL.ServiceNames:(%d).", m_iAtomNum);
    if (m_iAtomNum == 0) {
        return RET_ERROR;
    } else if (m_iAtomNum > MAX_SERVICE_ATOM_COUNTS) {
        m_iAtomNum = MAX_SERVICE_ATOM_COUNTS;
    }
    DEBUG(LL_INFO, "try to new ServiceAtom:(%d).", m_iAtomNum);
    iServiceAtom = new ServiceAtom[m_iAtomNum];
    if (iServiceAtom == NULL) {
        LOG(LL_ERROR, "new ServiceAtom[%d] Error!\n",
            m_iAtomNum);
        m_iAtomNum = 0;
        return RET_ERROR;
    }
    DEBUG(LL_INFO, "new ServiceAtom success.");

    char section_name[MAX_SIZE_1K] = {0};
    char tmpBuffer[MAX_SIZE_2K] = {0};
    for (int ii = 0; ii < m_iAtomNum; ii++) {
        sprintf(section_name, "ServiceName%d", ii+1);
        string serviceName = configPtr->get("GLOBAL", section_name, " ");
        LOG(LL_VARS, "%s=(%s).",
            section_name, serviceName.c_str());
        if (serviceName == " ") {
            LOG(LL_ERROR, "get section %s error.", section_name);
            return RET_ERROR;
        }

        DEBUG(LL_INFO, "try to init service atom:(%d).", ii+1);
        if (iServiceAtom[ii].Init(serviceName) == RET_ERROR) {
            LOG(LL_ERROR, "CServiceAtoms[%d].Init error, service name:(%s).",
                ii+1, serviceName.c_str());
            return RET_ERROR;
        }

        m_iIDList[ii] = ii;
    }
    DEBUG(LL_ALL, "End");
    return RET_OK;
}


int ServiceList::ProcessLog(char *p_pchLog, char *p_pchOut) {
    DEBUG(LL_ALL, "Begin");
    if ((p_pchLog == NULL) || (*p_pchLog == 0x00) ||
        (p_pchOut == NULL)) {
        LOG(LL_ERROR, "IO buffer null.");
        return RET_ERROR;
    }
    for (int ii = 0; ii < m_iAtomNum; ii++) {
        if (iServiceAtom[m_iIDList[ii]].MatchRecord(p_pchLog, p_pchOut) == RET_OK) {
            DEBUG(LL_VARS, "ServiceAtom[%d] match ok,out buffer=(%s).",
                ii, p_pchOut);
            m_iTotal++;
            return RET_OK;
        }
        DEBUG(LL_VARS, "ServiceAtom[%d] match error.", ii);
    }
    DEBUG(LL_ALL, "End");
    return RET_ERROR;
}

int ServiceList::SortCServiceAtoms(int p_iSortFlag, int p_iZeroFlag) {
    int l_curloc;
    int l_curnum;
    int l_tmploc;
    int l_tmpnum;
    int i, j;

    if (m_iAtomNum <= 0) {
        return -1;
    }

    if (m_iSortMax == 0) return 0;

    if (m_iTotal < m_iSortMax) {
        return 0;
    }

    for (i = 0; i < m_iAtomNum; i++) {
        l_curloc = m_iIDList[i];
        l_curnum = iServiceAtom[l_curloc].GetRecNum();

        for (j = i + 1; j < m_iAtomNum; j++) {
            l_tmploc = m_iIDList[j];
            l_tmpnum = iServiceAtom[l_tmploc].GetRecNum();

            if (p_iSortFlag == 0) {
                if (l_tmpnum > l_curnum) {
                    l_curloc = l_tmploc;
                    l_curnum = l_tmpnum;
                }
            } else {
                if (l_tmpnum < l_curnum) {
                    l_curloc = l_tmploc;
                    l_curnum = l_tmpnum;
                }
            }
        }
        if (l_curloc != m_iIDList[i]) {
            l_tmploc = m_iIDList[i];
            m_iIDList[i] = m_iIDList[l_curloc];
            m_iIDList[l_curloc] = l_tmploc;
        }

        if (p_iZeroFlag != 0) {
            iServiceAtom[l_curloc].SetRecNum(0);
        }
    }

    if (p_iZeroFlag != 0) {
        m_iTotal = 0;
    }

    return 0;
}

