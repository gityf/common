
#include "scs/maskatom.h"
#include "base/localdef.h"
MaskAtom::MaskAtom() {
    memset(m_szMask, 0, CMaskAtom_MAX_LEN);
    memset(m_szMeMask, 0, CMaskAtom_MAX_LEN);

    ClearCMaskAtomFieldInfo(0);

    m_iCurVar = -1;
}

MaskAtom::~MaskAtom() {
    memset(m_szMask, 0, CMaskAtom_MAX_LEN);
    memset(m_szMeMask, 0, CMaskAtom_MAX_LEN);

    ClearCMaskAtomFieldInfo(0);

    m_iCurVar = -1;
}

/*************************************************
private
*************************************************/

void MaskAtom::ClearCMaskAtomFieldInfo(int p_iFlag) {
    int i, len;
    if (p_iFlag == 0) len = CMaskAtom_MAX_FIELD;
    else {
        len = m_SAtomNum;
        if (len == 0) return;

        if (len < 0 || len > CMaskAtom_MAX_FIELD)
            len = CMaskAtom_MAX_FIELD;
    }

    for (i = 0; i < len; i++) {
        memset(&m_SAtom[i], 0, LEN_MASK_ATOM_FIELD_INFO);
        m_SAtom[i].m_iType = CMaskAtom_TYPE_INVALID;
        m_SAtom[i].m_iValid = -1;
    }
    m_SAtomNum = 0;

    return;
}


int MaskAtom::GetAtomStrType(int p_iFlag, char p_pch) {
    if (p_pch == '*') {
        return CMaskAtom_TYPE_WILDCARD;
    } else if (p_pch == '$') {
        if (p_iFlag == CMaskAtom_TYPE_VARIABLE) {
            return CMaskAtom_TYPE_INVALID;
        } else {
            return CMaskAtom_TYPE_VARIABLE;
        }
    } else {
        if ((p_iFlag == CMaskAtom_TYPE_VARIABLE) &&
            (p_pch >= '0' && p_pch <= '9')) {
            return CMaskAtom_TYPE_VARIABLE;
        } else {
            return CMaskAtom_TYPE_CONSTANT;
        }
    }
}

int MaskAtom::GetAtomStrFromString(char *p_pchStr,
                                   int *p_iFlag, int *p_iLen, char *p_pchOut) {
    int i, len;
    int oflag, nflag;
    char l_buff[CMaskAtom_MAX_LEN];

    if ((p_pchStr == NULL) ||
        (*p_pchStr == 0x00) ||
        (p_iFlag == NULL) ||
        (p_iLen == NULL) ||
        (p_pchOut == NULL)) {
        return RET_ERROR;
    }

    memset(l_buff, 0, sizeof(l_buff));
    strcpy(l_buff, p_pchStr);

    *p_iFlag = CMaskAtom_TYPE_INVALID;
    *p_iLen = 0;

    len = strlen(l_buff);

    oflag = CMaskAtom_TYPE_INVALID;
    nflag = CMaskAtom_TYPE_INVALID;
    for (i = 0; i < len; i++) {
        if (i == 0) {
            oflag = GetAtomStrType(oflag, l_buff[i]);
        } else {
            nflag = GetAtomStrType(oflag, l_buff[i]);
            if (nflag != oflag) {
                break;
            }
        }
    }

    *p_iFlag = oflag;
    *p_iLen = i;
    memcpy(p_pchOut, l_buff, i);

    return 0;
}

int MaskAtom::GetVariableLoc(int p_iVID, int p_iLoc) {
    if (p_iVID < 0) return RET_ERROR;

    if ((p_iLoc < 0) || (p_iLoc > m_SAtomNum)) {
        p_iLoc = 0;
    }

    if (p_iLoc == m_SAtomNum) {
        return RET_ERROR;
    }

    for (int i = p_iLoc; i < m_SAtomNum; i++) {
        if ((m_SAtom[i].m_iValid == 1) &&
            (m_SAtom[i].m_iType == CMaskAtom_TYPE_VARIABLE) &&
            (m_SAtom[i].m_iVariableID == p_iVID)) {
            return i;
        }
    }

    return RET_ERROR;
}

int MaskAtom::IsLawCMaskAtomFields() {
    int l_oflag, l_flag;
    int i;

    if (m_SAtomNum < 2) {
        return 0;
    }

    l_oflag = CMaskAtom_TYPE_INVALID;
    for (i = 0; i < m_SAtomNum; i++) {
        l_flag = m_SAtom[i].m_iType;

        if (l_flag == CMaskAtom_TYPE_INVALID) {
            return RET_ERROR;
        } else if (l_flag == CMaskAtom_TYPE_CONSTANT) {
            l_oflag = CMaskAtom_TYPE_INVALID;
        } else if (l_flag == CMaskAtom_TYPE_VARIABLE) {
            if (l_oflag == CMaskAtom_TYPE_INVALID) {
                l_oflag = l_flag;
            } else {
                return RET_ERROR;
            }
        } else {
        }
    }

    for (i = 0; i < m_SAtomNum; i++) {
        if ((m_SAtom[i].m_iValid == 1) &&
            (m_SAtom[i].m_iType == CMaskAtom_TYPE_VARIABLE)) {
            if (GetVariableLoc(m_SAtom[i].m_iVariableID, i + 1) != -1) {
                return RET_ERROR;
            }
        }
    }

    return 0;
}


int MaskAtom::LawCMaskAtomFields() {
    int i;

    if (m_SAtomNum < 2) return 0;

    for (i = 0; i < m_SAtomNum; i++) {
        if (m_SAtom[i].m_iType == CMaskAtom_TYPE_VARIABLE) {
            if (i - 1 >= 0) {
                if (m_SAtom[i - 1].m_iType == CMaskAtom_TYPE_WILDCARD) {
                    m_SAtom[i - 1].m_iValid = -1;
                }
            }

            if (i + 1 < m_SAtomNum) {
                if (m_SAtom[i + 1].m_iType == CMaskAtom_TYPE_WILDCARD) {
                    m_SAtom[i + 1].m_iValid = -1;
                }
            }
        }
    }

    return 0;
}

int MaskAtom::FormatMask(int p_iCheckFlag) {
    int i, j, len;

    char l_buff[CMaskAtom_MAX_LEN];
    char l_tmp[CMaskAtom_MAX_LEN];
    char l_new[CMaskAtom_MAX_LEN];
    int l_flag;
    int l_len;

    len = strlen(m_szMask);
    if (len == 0) {
        return 0;
    }

    if (m_SAtomNum != 0) {
        ClearCMaskAtomFieldInfo(1);
    }

    i = 0;
    while (i < len) {
        memset(l_tmp, 0, sizeof(l_tmp));
        strcpy(l_tmp, m_szMask + i);

        l_flag = CMaskAtom_TYPE_INVALID;
        l_len = 0;
        memset(l_buff, 0, sizeof(l_buff));
        if (GetAtomStrFromString(l_tmp, &l_flag, &l_len, l_buff) == 0) {
            if (l_len == 0) {
                break;
            }
            m_SAtom[m_SAtomNum].m_iNum = m_SAtomNum;
            m_SAtom[m_SAtomNum].m_iType = l_flag;
            strcpy(m_SAtom[m_SAtomNum].m_szValue, l_buff);
            m_SAtom[m_SAtomNum].m_iLoc = i;
            m_SAtom[m_SAtomNum].m_iLen = l_len;

            m_SAtom[m_SAtomNum].m_iValid = 1;
            if (l_flag == CMaskAtom_TYPE_VARIABLE) {
                strcpy(m_SAtom[m_SAtomNum].m_szValidValue, l_buff);
                strcpy(m_SAtom[m_SAtomNum].m_szMatch, "*");
                if (strlen(l_buff) == 1) {
                    printf("$ == 1\n");
                    ClearCMaskAtomFieldInfo(1);
                    return -1;
                }
                memset(l_new, 0, sizeof(l_new));
                strcpy(l_new, l_buff + 1);
                m_SAtom[m_SAtomNum].m_iVariableID = atoi(l_new);
            } else if (l_flag == CMaskAtom_TYPE_WILDCARD) {
                m_SAtom[m_SAtomNum].m_szValidValue[0] = '*';
                m_SAtom[m_SAtomNum].m_szValidValue[1] = 0;
                m_SAtom[m_SAtomNum].m_iVariableID = -1;
                strcpy(m_SAtom[m_SAtomNum].m_szMatch, "*");
            } else {
                strcpy(m_SAtom[m_SAtomNum].m_szValidValue, l_buff);
                m_SAtom[m_SAtomNum].m_iVariableID = -1;
                strcpy(m_SAtom[m_SAtomNum].m_szMatch, l_buff);
            }

            m_SAtom[m_SAtomNum].m_iSeq = -1;

            m_SAtomNum++;
            i += l_len;
        } else {
            ClearCMaskAtomFieldInfo(1);
            printf("GetAtomStrFromString return -1\n");
            return RET_ERROR;
        }
    }

    if (m_SAtomNum == 0) {
        printf("m_SAtomNum == 0\n");
        return RET_ERROR;
    } else {
        if (p_iCheckFlag == 0) {
            if (IsLawCMaskAtomFields() == RET_ERROR) {
                printf("IsLawCMaskAtomFields return -1\n");
                ClearCMaskAtomFieldInfo(1);
                return RET_ERROR;
            }

            if (LawCMaskAtomFields() == RET_ERROR) {
                printf("LawCMaskAtomFields return -1\n");
                ClearCMaskAtomFieldInfo(0);
                return RET_ERROR;
            }
        }
    }

    memset(m_szMeMask, 0, sizeof(m_szMeMask));

    //  printf("get m_szMeMask ......\n");

    j = 1;
    for (i = 0; i < m_SAtomNum; i++) {
        if (m_SAtom[i].m_iValid == 1) {
            //          strcat(m_szMeMask, m_SAtom[i].m_szValidValue);
            strcat(m_szMeMask, m_SAtom[i].m_szMatch);

            switch (m_SAtom[i].m_iType) {
            case CMaskAtom_TYPE_WILDCARD:
            case CMaskAtom_TYPE_VARIABLE:
                //              strcat(m_szMeMask, "*");
                m_SAtom[i].m_iSeq = j++;
                break;
            default:
                break;
            }
        }
    }

    if (j > CMaskAtom_MAX_VARY) {
        printf("j > 32\n");
        ClearCMaskAtomFieldInfo(1);
        return RET_ERROR;
    }

    if (strlen(m_szMeMask) == 0) {
        printf("m_szMeMask is null\n");
        ClearCMaskAtomFieldInfo(1);
        return RET_ERROR;
    }

    return 0;
}

void MaskAtom::ClearCMaskAtomMatchValue() {
    for (int i = 0; i < m_SAtomNum; i++) {
        memset(m_SAtom[i].m_szMatch, 0, CMaskAtom_MAX_LEN);
    }
    return;
}


/*************************************************
public
*************************************************/

int MaskAtom::InitMask(const char* p_pchMask, int p_iCheckFlag) {
    ClearCMaskAtomFieldInfo(0);

    if ((p_pchMask == NULL) || (strlen(p_pchMask) == 0)) {
        return RET_ERROR;
    }

    memset(m_szMask, 0, CMaskAtom_MAX_LEN);
    snprintf(m_szMask, CMaskAtom_MAX_LEN-1, "%s", p_pchMask);

    if (FormatMask(p_iCheckFlag) == RET_ERROR) {
        printf("MaskAtom::InitMask - FormatMask return -1\n");
        return RET_ERROR;
    }

    return 0;
}

int MaskAtom::IsMatchString(char *p_pchString, int p_iFlag) {
    char l_buff[1024];

    if (m_CMaskSearch.match(m_szMeMask, p_pchString) == 0) {
        //      printf("match(<%s>, <%s> return error\n", m_szMeMask, p_pchString);
        return RET_ERROR;
    }

    //  printf("match(<%s>, <%s> return success\n", m_szMeMask, p_pchString);

    if (p_iFlag == 1) {
        for (int i = 0; i < m_SAtomNum; i++) {
            if ((m_SAtom[i].m_iValid == 1) &&
                (m_SAtom[i].m_iType > 0)) {
                memset(m_SAtom[i].m_szMatch, 0, sizeof(m_SAtom[i].m_szMatch));
                memset(l_buff, 0, sizeof(l_buff));
                if (m_CMaskSearch.GetMaskValue(m_SAtom[i].m_iSeq - 1, l_buff) == 0)
                    strcpy(m_SAtom[i].m_szMatch, l_buff);
                else
                    return RET_ERROR;
            }
        }
    }

    return RET_OK;
}

int MaskAtom::GetMatchString(int p_iVID, char *p_pchStr) {
    int l_loc;

    if (p_pchStr == NULL) return RET_ERROR;

    l_loc = GetVariableLoc(p_iVID, 0);

    if (l_loc >= 0 && l_loc < m_SAtomNum) {
        strcpy(p_pchStr, m_SAtom[l_loc].m_szMatch);
        return RET_OK;
    } else {
        return RET_ERROR;
    }
}

int MaskAtom::GetFormatMatchString(int p_iFlag, char *p_pchStr) {
    int i;

    if (p_pchStr == NULL) return RET_ERROR;
    p_pchStr[0] = 0x00;

    for (i = 0; i < m_SAtomNum; i++) {
        if (m_SAtom[i].m_iValid == 1) {
            switch (m_SAtom[i].m_iType) {
            case CMaskAtom_TYPE_VARIABLE:
                if (p_iFlag == 0) {
                    strcat(p_pchStr, m_SAtom[i].m_szValidValue);
                } else {
                    strcat(p_pchStr, m_SAtom[i].m_szMatch);
                }
                break;
            default:
                strcat(p_pchStr, m_SAtom[i].m_szValidValue);
                break;
            }
        }
    }

    return RET_OK;
}


int MaskAtom::GetSeqVarID(int *p_iLoc) {
    if (m_iCurVar == -1) {
        m_iCurVar = 0;
    }

    if (m_iCurVar == m_SAtomNum) {
        m_iCurVar = 0;
        return RET_ERROR;
    }

    for (int i = m_iCurVar; i < m_SAtomNum; i++) {
        if ((m_SAtom[i].m_iValid == 1) &&
            (m_SAtom[i].m_iType == CMaskAtom_TYPE_VARIABLE)) {
            m_iCurVar = i + 1;
            *p_iLoc = i;
            return m_SAtom[i].m_iVariableID;
        }
    }
    return RET_ERROR;
}

int MaskAtom::UpdateToMulChar(int p_iLoc, int p_iVID, int p_iFlag) {
    if (p_iLoc < 0 || p_iLoc > m_SAtomNum) {
        return RET_ERROR;
    }

    if ((m_SAtom[p_iLoc].m_iValid == 1) &&
        (m_SAtom[p_iLoc].m_iType == CMaskAtom_TYPE_VARIABLE) &&
        (m_SAtom[p_iLoc].m_iVariableID == p_iVID)) {
        m_SAtom[p_iLoc].m_iType = CMaskAtom_TYPE_WILDCARD;
        memset(m_SAtom[p_iLoc].m_szValidValue, 0, CMaskAtom_MAX_LEN);
        m_SAtom[p_iLoc].m_szValidValue[0] = '*';
        m_SAtom[p_iLoc].m_szValidValue[1] = 0;
    } else {
        return RET_OK;
    }

    if (p_iFlag == 1) {
        UpdateMeMask(0);
    }

    return RET_OK;
}

int MaskAtom::IsIncludeCMaskAtom(MaskAtom *p_CCMaskAtom, int p_iFlag) {
    int l_VID;
    int l_Loc;

    if (p_CCMaskAtom == NULL) return RET_OK;

    while ((l_VID = p_CCMaskAtom->GetSeqVarID(&l_Loc)) != RET_ERROR) {
        if (GetVariableLoc(l_VID, 0) == RET_ERROR) {
            if (p_iFlag == 1) {
                p_CCMaskAtom->UpdateToMulChar(l_Loc, l_VID, 1);
            } else {
                return RET_ERROR;
            }
        }
    }

    return RET_OK;
}

int MaskAtom::GetStringValueFromAtom(MaskAtom *p_CCMaskAtom, char *p_pchStr) {
    if ((p_CCMaskAtom == NULL) ||
        (p_pchStr == NULL)) {
        return RET_ERROR;
    }

    ClearCMaskAtomMatchValue();

    for (int i = 0; i < m_SAtomNum; i++) {
        if ((m_SAtom[i].m_iValid == 1) &&
            (m_SAtom[i].m_iType == CMaskAtom_TYPE_VARIABLE)) {
            if (p_CCMaskAtom->GetMatchString(m_SAtom[i].m_iVariableID, m_SAtom[i].m_szMatch) == RET_ERROR) {
                ClearCMaskAtomMatchValue();
                return RET_ERROR;
            }
        }
    }

    return GetFormatMatchString(1, p_pchStr);
}


int MaskAtom::UpdateMeMask(int p_iFlag) {
    memset(m_szMeMask, 0, sizeof(m_szMeMask));

    for (int i = 0; i < m_SAtomNum; i++) {
        if (m_SAtom[i].m_iValid == 1) {
            switch (m_SAtom[i].m_iType) {
            case CMaskAtom_TYPE_VARIABLE:
                if (p_iFlag == 0) {
                    strcat(m_szMeMask, m_SAtom[i].m_szValidValue);
                } else {
                    strcat(m_szMeMask, m_SAtom[i].m_szMatch);
                }
                break;
            default:
                strcat(m_szMeMask, m_SAtom[i].m_szValidValue);
                break;
            }
        }
    }

    return RET_OK;
}

int MaskAtom::UpdateMeMask(char *p_pchMeMask) {
    if ((p_pchMeMask == NULL) || *p_pchMeMask == 0x00)
        return RET_ERROR;

    strcpy(m_szMeMask, p_pchMeMask);
    return RET_OK;
}

