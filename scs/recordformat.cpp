#include "base/conf.h"
#include "base/log.h"
#include "base/localdef.h"
#include "scs/recordformat.h"
using common::Conf;
RecordFormat::RecordFormat() {
    m_CCRecordFormatAtoms = NULL;
    m_iNum = 0;
}

RecordFormat::~RecordFormat() {
    if (m_CCRecordFormatAtoms != NULL) {
        delete [] m_CCRecordFormatAtoms;
        m_CCRecordFormatAtoms = NULL;
    }
    m_iNum = 0;
}

int RecordFormat::Init(const string& aRecFmttName) {
    DEBUG(LL_ALL, "Begin");
    iRecordFormatName = aRecFmttName;
    if (m_CCRecordFormatAtoms != NULL) {
        delete [] m_CCRecordFormatAtoms;
        m_CCRecordFormatAtoms = NULL;
    }
    Conf* configPtr = Singleton<Conf>::Instance();
    m_iNum = configPtr->getInteger(aRecFmttName, ".Formats", 1);
    if (m_iNum < 0) {
        LOG(LL_ERROR, "get %s.Formats=(%d) error.",
            aRecFmttName.c_str(), m_iNum);
        return RET_ERROR;
    }
    if (m_iNum > MAX_RECORD_FORMATS)
        m_iNum = MAX_RECORD_FORMATS;


    DEBUG(LL_VARS, "RecordFormatAtom num:(%d).", m_iNum);
    DEBUG(LL_INFO, "try to new RecordFormatAtom[%d].", m_iNum);
    m_CCRecordFormatAtoms = new RecordFormatAtom[m_iNum];
    if (m_CCRecordFormatAtoms == NULL) {
        LOG(LL_ERROR, "CRecordFormatAtoms[%d] -- new error.",
            m_iNum);
        m_iNum = 0;
        return RET_ERROR;
    }
    int l_f_flag = 0;

    char section_name[MAX_SIZE_1K] = {0};
    char tmpBuffer[MAX_SIZE_2K] = {0};
    for (int ii = 0; ii < m_iNum; ii++) {
        sprintf(section_name, "ORIFormat%d", ii+1);
        string oriFormat =
            configPtr->get(aRecFmttName, section_name, " ");
        LOG(LL_VARS, "%s=(%s).", section_name, oriFormat.c_str());
        if (oriFormat == " ") {
            LOG(LL_ERROR, "get section ori %s error.", section_name);
            m_CCRecordFormatAtoms[ii].SetValidFormat(0);
            continue;
        }

        sprintf(section_name, "NEWFormat%d", ii+1);
        string newFormat =
            configPtr->get(aRecFmttName, section_name, " ");
        LOG(LL_VARS, "%s=(%s).", section_name, newFormat.c_str());
        if (newFormat == " ") {
            LOG(LL_ERROR, "get section new %s error.", section_name);
            m_CCRecordFormatAtoms[ii].SetValidFormat(0);
            continue;
        }

        DEBUG(LL_VARS, "try init format[%d]:ori=(%s),new=(%s).",
            ii, oriFormat.c_str(), newFormat.c_str());
        if (m_CCRecordFormatAtoms[ii].InitFormat(oriFormat.c_str(), newFormat.c_str()) == RET_ERROR) {
            LOG(LL_ERROR, "CRecordFormatAtoms[%d].InitFormat Error!",ii);
            m_CCRecordFormatAtoms[ii].SetValidFormat(0);
            continue;
        }
        DEBUG(LL_VARS, "init format[%d] success:ori=(%s),new=(%s).",
            ii, oriFormat.c_str(), newFormat.c_str());
        m_CCRecordFormatAtoms[ii].SetValidFormat(1);
        l_f_flag = 1;
    }

    if (l_f_flag == 0) {
        if (m_CCRecordFormatAtoms != NULL) {
            delete [] m_CCRecordFormatAtoms;
            m_CCRecordFormatAtoms = NULL;
        }
        m_iNum = 0;

        return RET_ERROR;
    }

    DEBUG(LL_ALL, "End");
    return RET_OK;
}


int RecordFormat::GetFormatString(char *p_pchOri, char *p_pchNew) {
    if ((p_pchOri == NULL) ||
        (*p_pchOri == 0x00) ||
        (p_pchNew == NULL)) {
        return RET_ERROR;
    }

    if ((m_iNum == 0) || (m_CCRecordFormatAtoms == NULL)) {
        return RET_ERROR;
    }

    for (int i = 0; i < m_iNum; i++) {
        if (m_CCRecordFormatAtoms[i].FormatString(p_pchOri, p_pchNew) == RET_OK) {
            return RET_OK;
        }
    }

    return RET_ERROR;
}

int RecordFormat::GetFormatNumber() {
    return m_iNum;
}

