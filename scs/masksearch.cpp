#include "base/log.h"
#include "scs/masksearch.h"
/*
*  match
*/
int MaskSearch::match(const char* mask, const char* src) {
    char n_mask[1024] = {0};
    int mask_len;
    int src_len;
    char *l_pch;
    char buff[1024] = {0};
    char n_src[1024] = {0};
    int s_left;
    int s_right;
    int m_left;
    int m_right;

    if ((mask == NULL) || (src == NULL)) {
        return 0;
    }

    /*  如果mask的长度为0,返回不匹配    */
    //bzero(n_mask, sizeof(n_mask));
    //  mask_len = FormatMask(mask, n_mask);
    //memset(n_mask, 0, sizeof(n_mask));
    mask_len = FormatMaskMul(mask, n_mask);
    DEBUG(LL_DBG, "MaskSearch::match:mask:(%s),newmask:(%s)", mask, n_mask);

    if (mask_len == 0) {
        return 0;
    }
    src_len = strlen(src);

    memset(m_osrc, 0, sizeof(m_osrc));
    strcpy(m_osrc, src);

    /*  去头尾匹配  */
    l_pch = strchr(n_mask, maskMultiChar);
    if (l_pch == NULL) {
        /*  无'*'   */
        if (mask_len == src_len) {
            if (SingleHeadMatch(n_mask, src) == -1) {
                return 0;
            } else {
                return 1;
            }
        } else {
            return 0;
        }
    } else {
        m_left = l_pch - n_mask;

        m_lcur = 1;
        m_lleft = m_left;

        if (m_left != 0) {
            bzero(buff, sizeof(buff));
            memcpy(buff, n_mask, m_left);

            if (SingleHeadMatch(buff, src) == -1) {
                return 0;
            }
        }
    }

    /*  此处l_pch不可能为NULL   */
    l_pch = strrchr(n_mask, maskMultiChar);
    m_right = mask_len - (l_pch - n_mask) - 1;

    m_rcur = m_vnum;
    m_rright = src_len - m_right;

    if (m_right != 0) {
        bzero(buff, sizeof(buff));
        strcpy(buff, l_pch + 1);

        if (SingleTailMatch(buff, src) == -1) {
            return 0;
        }
    }

    if (src_len < (m_left + m_right)) {
        return 0;
    }

    if (m_left + m_right + 1 == mask_len) {
        memcpy(m_value[m_lcur - 1], m_osrc + m_lleft, m_rright - m_lleft);

        return 1;
    }

    bzero(buff, sizeof(buff));
    memcpy(buff, n_mask + m_left + 1, mask_len - m_left - m_right - 2);

    bzero(n_src, sizeof(n_src));
    memcpy(n_src, src + m_left, src_len - m_left - m_right);

    memset(m_osrc, 0, sizeof(m_osrc));
    strcpy(m_osrc, n_src);
    m_lleft = 0;
    m_rright = strlen(m_osrc);

    if (TotalMatch(buff, n_src) == 0) return 1;
    else return 0;
}

/*
*  FormatMaskMul
*
*  只将**转化为*
*/
int MaskSearch::FormatMaskMul(const char* mask, char* n_mask) {
    int flag = 0;
    int i;
    int j;
    int len;
    char mc;

    len = strlen(mask);
    if (len == 0) {
        n_mask[0] = 0;
        return 0;
    }

    /* start jiangxh 20060531 */

    for (i = 0; i < MASKS_MAX_FIELD; i++) {
        memset(m_value[i], 0, MASKS_MAX_LEN);
    }
    m_vnum = 0;
    /* end jiangxh 20060531 */

    j = 0;
    for (i = 0; i < len; i++) {
        mc = mask[i];

        if (mc == maskMultiChar) {
            if (flag == 0) {
                n_mask[j] = mc;
                flag = 1;
                j++;
                m_vnum++;
            }
        } else {
            n_mask[j] = mc;
            j++;
            flag = 0;
        }
    }
    n_mask[j] = 0x00;

    return j;
}

/*
*  FormatMask
*
*  规范mask的格式，如**转化为*，*???转化为*
*/
int MaskSearch::FormatMask(const char* mask, char* n_mask) {
    int flag = 0;           /*  是否为'*'   */
    int i;
    int j;
    int k;
    int len;
    char mc;
    int single_num = 0;     /*  '?'的个数   */

    len = strlen(mask);
    if (len == 0) {
        n_mask[0] = 0;
        return 0;
    }

    j = 0;
    for (i = 0; i < len; i++) {
        mc = mask[i];

        if (mc == maskMultiChar) {
            single_num = 0;     //  清空'?'数
            if (flag == 0) {
                n_mask[j] = mc;
                flag = 1;
                j++;
            }
        } else if (mc == maskSingleChar) {
            if (flag == 0) {
                single_num++;
            }
        } else {
            if (single_num != 0) {
                for (k = 0; k < single_num; k++) {
                    n_mask[j] = maskSingleChar;
                    j++;
                }
            }

            single_num = 0;
            flag = 0;
            n_mask[j] = mc;
            j++;
        }
    }

    if (single_num != 0) {
        for (k = 0; k < single_num; k++) {
            n_mask[j] = maskSingleChar;
            j++;
        }
    }

    return j;
}

/*
*  SingleHeadMatch
*
*  只有?,且从str头开始匹配
*/
int MaskSearch::SingleHeadMatch(const char* mask, const char* str) {
    int mask_len;
    int str_len;
    int i;

    mask_len = strlen(mask);
    str_len = strlen(str);

    if ((mask_len == 0) ||
        (str_len == 0) ||
        (mask_len > str_len)) {
        return -1;
    }

    for (i = 0; i < mask_len; i++) {
        if ((mask[i] != maskSingleChar) &&
            (mask[i] != str[i])) {
            return -1;
        }
    }

    return 0;
}

/*
*  SingleTailMatch
*
*  只有?,且匹配str尾
*/
int MaskSearch::SingleTailMatch(const char* mask, const char* str) {
    int mask_len;
    int str_len;
    int i, j;

    mask_len = strlen(mask);
    str_len = strlen(str);

    if ((mask_len == 0) ||
        (str_len == 0) ||
        (mask_len > str_len)) {
        return -1;
    }

    j = str_len - mask_len;
    for (i = 0; i < mask_len; i++) {
        if ((mask[i] != maskSingleChar) &&
            (mask[i] != str[j])) {
            return -1;
        }
        j++;
    }

    return 0;
}

/*
*  SingleTotalMatch
*
*/
int MaskSearch::SingleTotalMatch(const char* mask, const char* src) {
    int mask_len;
    int src_len;
    int i;

    mask_len = strlen(mask);
    src_len = strlen(src);

    if (mask_len > src_len) {
        return -1;
    } else if (mask_len == src_len) {
        return SingleHeadMatch(mask, src);
    } else {
        for (i = 0; i <= src_len - mask_len; i++) {
            if (SingleHeadMatch(mask, src + i) != -1) {
                return i;
            }
        }
    }

    return -1;
}

/*
*  TotalMatch
*
*/
int MaskSearch::TotalMatch(char* mask, char* src) {
    int ret;
    char *l_pch;
    char n_mask[1024];
    char *n_src;

    l_pch = strchr(mask, maskMultiChar);
    if (l_pch == NULL) {
        ret = SingleTotalMatch(mask, src);
        if (ret == -1) {
            return -1;
        } else {
            memcpy(m_value[m_lcur - 1], m_osrc + m_lleft, ret);
            strcpy(m_value[m_rcur - 1], m_osrc + m_lleft + ret + strlen(mask));

            return 0;
        }
    }

    bzero(n_mask, sizeof(n_mask));
    memcpy(n_mask, mask, l_pch - mask);

    ret = SingleTotalMatch(n_mask, src);
    if (ret == -1) {
        return -1;
    } else {
        n_src = src + ret + strlen(n_mask);

        memcpy(m_value[m_lcur - 1], m_osrc + m_lleft, ret);
        m_lcur++;
        m_lleft = m_lleft + ret + strlen(n_mask);

        bzero(n_mask, sizeof(n_mask));
        strcpy(n_mask, l_pch + 1);

        return TotalMatch(n_mask, n_src);
    }
}

int MaskSearch::GetMaskValue(int p_i, char *p_pchStr) {
    if (p_i < 0 || p_i >= m_vnum) return -1;
    if (p_pchStr == NULL) return -1;
    strcpy(p_pchStr, m_value[p_i]);
    return 0;
}

