#ifndef _C_MASK_SERACH_H_
#define _C_MASK_SERACH_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>

#define MASKS_MAX_FIELD     64
#define MASKS_MAX_LEN       512

/*
*   ͨ���ƥ����
*/
class MaskSearch
{
private:
    char        maskMultiChar;  //  ƥ�����ַ����ַ���ȱʡΪ'*';
    char        maskSingleChar; //  ƥ�䵥���ַ����ַ���ȱʡΪ'?';

    char        m_value[MASKS_MAX_FIELD][MASKS_MAX_LEN];
    int         m_vnum;
    int         m_lcur;
    int         m_lleft;
    //  int         m_lright;
    int         m_rcur;
    //  int         m_rleft;
    int         m_rright;

    char        m_osrc[1024];

public:
    MaskSearch(
        char        multiChar = '*',
        char        singleChar = '?') {
        maskMultiChar = multiChar;
        maskSingleChar = singleChar;
    };
    ~MaskSearch() {

    };
    void        setMaskSingleChar(char c) {
        maskSingleChar = c;
    };
    void        setMaskMultiChar(char c) {
        maskMultiChar = c;
    };

    /*
    *   ��maskƥ��src,ȱʡ��ͨ�����*�ͣ�,�����ڶ����ʼ��ʱ����ͳ���
    *   ���磺
    *       match ("ab*ef", "abcdef") = true
    *       match ("ab*cd", "abcd") = true
    *       match ("ab???ef", "abcdef") = false
    */
    int match(const char* mask, const char* src);

    int FormatMask(const char* mask, char* n_mask);
    int FormatMaskMul(const char* mask, char* n_mask);
    int SingleHeadMatch(const char* mask, const char* src);
    int SingleTailMatch(const char* mask, const char* src);
    int SingleTotalMatch(const char* mask, const char* src);
    int TotalMatch(char* mask, char* src);
    int GetMaskValue(int p_i, char *p_pchStr);
};

#endif  //  _C_MASK_SERACH_H_

