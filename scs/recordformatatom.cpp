
#include "scs/recordformatatom.h"
#include "base/localdef.h"

RecordFormatAtom::RecordFormatAtom() {
    m_iFlag = 0;
}

RecordFormatAtom::~RecordFormatAtom() {
    m_iFlag = 0;
}

int RecordFormatAtom::InitFormat(const char* p_pchORIFormat, const char* p_pchNEWFormat) {
    m_iFlag = 0;

    if ((m_CORIFormat.InitMask(p_pchORIFormat) == RET_ERROR) ||
        (m_CNEWFormat.InitMask(p_pchNEWFormat, 1) == RET_ERROR)) {
        return RET_ERROR;
    }

    if (m_CORIFormat.IsIncludeCMaskAtom(&m_CNEWFormat) == RET_ERROR) {
        return RET_ERROR;
    }

    m_iFlag = 1;

    return RET_OK;
}

int RecordFormatAtom::FormatString(char *p_pchORI, char *p_pchNEW) {
    if ((p_pchORI == NULL) || (*p_pchORI == 0x00) ||
        (p_pchNEW == NULL)) {
        return RET_ERROR;
    }

    if (m_iFlag == 0) return RET_ERROR;

    if (m_CORIFormat.IsMatchString(p_pchORI, 1) == RET_ERROR) {
        return RET_ERROR;
    }

    if (m_CNEWFormat.GetStringValueFromAtom(&m_CORIFormat, p_pchNEW) == RET_ERROR) {
        return RET_ERROR;
    }
    return RET_OK;
}

int RecordFormatAtom::IsValidFormat() {
    return m_iFlag;
}

int RecordFormatAtom::SetValidFormat(int p_iFlag) {
    if (p_iFlag == 0) {
        m_iFlag = 0;
    } else {
        m_iFlag = 1;
    }
    return 0;
}

