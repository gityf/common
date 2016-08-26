
#ifndef _C_MASK_ATOM_H_
#define _C_MASK_ATOM_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include "scs/masksearch.h"

#define CMaskAtom_MAX_VARY           64
#define CMaskAtom_MAX_FIELD          128
#define CMaskAtom_MAX_LEN            1024

#define CMaskAtom_TYPE_INVALID           -1
#define CMaskAtom_TYPE_CONSTANT          0
#define CMaskAtom_TYPE_WILDCARD          1
#define CMaskAtom_TYPE_VARIABLE          2

typedef struct mask_atom_field_info
{
    int m_iNum;
    int m_iType;
    char m_szValue[CMaskAtom_MAX_LEN];
    int m_iLoc;
    int m_iLen;

    int m_iValid;
    char m_szValidValue[CMaskAtom_MAX_LEN];
    int m_iVariableID;

    int m_iSeq;
    char m_szMatch[CMaskAtom_MAX_LEN];

} MaskAtomFieldInfo;

#define LEN_MASK_ATOM_FIELD_INFO    sizeof(MaskAtomFieldInfo)

class MaskAtom
{
private:
    char m_szMask[CMaskAtom_MAX_LEN];
    char m_szMeMask[CMaskAtom_MAX_LEN];
    MaskAtomFieldInfo m_SAtom[CMaskAtom_MAX_FIELD];
    int m_SAtomNum;

    MaskSearch m_CMaskSearch;

    int m_iCurVar;

    void ClearCMaskAtomFieldInfo(int p_iFlag);
    int GetAtomStrType(int p_iFlag, char p_pch);
    int GetAtomStrFromString(char *p_pchStr,
        int *p_iFlag, int *p_iLen, char *p_pchOut);
    int GetVariableLoc(int p_iVID, int p_iLoc);
    int IsLawCMaskAtomFields();
    int LawCMaskAtomFields();
    int FormatMask(int p_iCheckFlag = 0);
    void ClearCMaskAtomMatchValue();

public:
    MaskAtom();
    ~MaskAtom();

    int InitMask(const char* p_pchMask, int p_iCheckFlag = 0);
    int IsMatchString(char *p_pchString, int p_iFlag);


    int GetMatchString(int p_iVID, char *p_pchStr);

    int GetFormatMatchString(int p_iFlag, char *p_pchStr);

    int GetSeqVarID(int *p_iLoc);

    int IsIncludeCMaskAtom(MaskAtom *p_CCMaskAtom, int p_iFlag = 0);
    int GetStringValueFromAtom(MaskAtom *p_CCMaskAtom, char *p_pchStr);

    int UpdateMeMask(int p_iFlag);
    int UpdateMeMask(char *p_pchMeMask);

    int UpdateToMulChar(int p_iLoc, int p_iVID, int p_iFlag = 0);
};


#endif // _C_MASK_ATOM_H_

