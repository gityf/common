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
*   通配符匹配类
*/
class MaskSearch
{
private:
    char        maskMultiChar;  //  匹配多个字符的字符，缺省为'*';
    char        maskSingleChar; //  匹配单个字符的字符，缺省为'?';

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
    *   用mask匹配src,缺省的通配符有*和？,可以在对象初始化时定义统配符
    *   例如：
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

