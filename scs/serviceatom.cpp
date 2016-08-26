#include "scs/serviceatom.h"
#include "base/conf.h"
#include "base/log.h"
#include "base/localdef.h"

ServiceAtom::ServiceAtom() {
    memset(m_szServiceName, 0, sizeof(m_szServiceName));
    memset(m_szServiceMask, 0, sizeof(m_szServiceMask));

    m_iFlag = 0;
    m_iRecNum = 0;
}


ServiceAtom::~ServiceAtom() {
    m_iFlag = 0;
}


int ServiceAtom::Init(const string& aServiceName) {
    // service name format=serviceSectionName;*keyword*
    int off_set = aServiceName.find_first_of(';');
    if (off_set > 0) {
        strcpy(m_szServiceName, aServiceName.substr(0, off_set).c_str());
        strcpy(m_szServiceMask, aServiceName.substr(off_set+1, aServiceName.length()).c_str());
    } else {
        strcpy(m_szServiceName, aServiceName.c_str());
        strcpy(m_szServiceMask, "*");
    }

    DEBUG(LL_DBG, "ServiceAtom::Init:ServiceName:(%s),ServiceMask:(%s).",
        m_szServiceName, m_szServiceMask);

    if (m_CCRecordFormat.Init(m_szServiceName) == RET_ERROR) {
        printf("ServiceAtom::Init ---- RecordFormat::Init<%s> Error!\n",
                aServiceName.c_str());
        return RET_ERROR;
    }

    m_iFlag = 1;
    return RET_OK;
}

int ServiceAtom::MatchRecord(char *p_pchRec, char *p_pchOut) {
    int i;

    if ((p_pchRec == NULL) || (*p_pchRec == 0x00) ||
        (p_pchOut == NULL)) {
        return RET_ERROR;
    }

    if (m_iFlag == 0) return RET_ERROR;

    if (m_CCMaskSearch.match(m_szServiceMask, p_pchRec) == 0) {
        return RET_ERROR;
    }

    if (m_CCRecordFormat.GetFormatString(p_pchRec, p_pchOut) == 0) {
        m_iRecNum++;
        return 0;
    } else {
        return RET_ERROR;
    }
}

int ServiceAtom::GetFlag() {
    return m_iFlag;
}

int ServiceAtom::SetFlag(int p_iFlag) {
    m_iFlag = (p_iFlag == 0) ? 0 : 1;
    return 0;
}

int ServiceAtom::GetRecNum() {
    return m_iRecNum;
}

int ServiceAtom::SetRecNum(int p_iRecNum) {
    m_iRecNum = (p_iRecNum <= 0) ? 0 : p_iRecNum;
    return 0;
}

