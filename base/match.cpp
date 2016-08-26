/*
* EPSRevision History
*
* J. Kercheval  Wed, 02/20/1991  22:29:01  Released to Public Domain
* J. Kercheval  Fri, 02/22/1991  15:29:01  fix '\' bugs (two :(of them)
* J. Kercheval  Sun, 03/10/1991  19:31:29  add error return to matche()
* J. Kercheval  Sun, 03/10/1991  20:11:11  add is_valid_pattern code
* J. Kercheval  Sun, 03/10/1991  20:37:11  beef up main()
* J. Kercheval  Tue, 03/12/1991  22:25:10  Released as V1.1 to Public Domain
* J. Kercheval  Thu, 03/14/1991  22:22:25  remove '\' for DOS file parsing
* J. Kercheval  Mon, 05/13/1991  21:49:05  ifdef full match code
* J. Kercheval  Mon, 01/06/1992  21:31:44  add match character defines
* Chengdong Lu  Thu, 08/17/2000            add repeated range support
* Chengdong Lu  Wed, 08/23/2000  19:30:13  add group and option support
* Chengdong Lu  Thu, 08/24/2000  09:31:33  fix greedy group bug
* Chengdong Lu  Thu, 01/04/2001  18:01:00  add support for numbered repeat
* Chengdong Lu  Wed, 08/21/2002  14:57:13  fix bug of * in is_null_pattern
* Chengdong Lu  Wed, 09/24/2002  17:10:00  real fix of * and range test in is_null_pattern
*/

/*
* Wildcard Pattern Matching
*/

#define MAXINT 0x7FFFFFFF

#include "match.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

/* character defines */
#define MATCH_CHAR_SINGLE               '?'
#define MATCH_CHAR_KLEENE_CLOSURE       '*'
#define MATCH_CHAR_CLOSURE              '+'
#define MATCH_GROUP_OPEN                '('
#define MATCH_GROUP_OR                  '|'
#define MATCH_GROUP_CLOSE               ')'
#define MATCH_CHAR_RANGE_OPEN           '['
#define MATCH_CHAR_RANGE                '-'
#define MATCH_CHAR_RANGE_CLOSE          ']'
#define MATCH_REPEAT_OPEN               '{'
#define MATCH_REPEAT_CLOSE              '}'
#define MATCH_CHAR_LITERAL              '\\'
#define MATCH_CHAR_NULL                 '\0'
#define MATCH_CHAR_CARAT_NEGATE         '^'
#define MATCH_CHAR_EXCLAMATION_NEGATE   '!'
#define TRUE  1
#define FALSE 0
/* forward function prototypes */
int consume_repeat(char **p,int *min,int *max,int *len);
int matche_after_star(char *pattern, char *text);
int fast_match_after_star(char *pattern, char *text);

/*----------------------------------------------------------------------------
*
* Return TRUE if PATTERN has any special wildcard characters
*
---------------------------------------------------------------------------*/

int is_pattern(char *p)
{
    while (*p) {
        switch (*p++) {
            case MATCH_CHAR_SINGLE:
            case MATCH_CHAR_KLEENE_CLOSURE:
            case MATCH_CHAR_CLOSURE:
            case MATCH_GROUP_OPEN:
            case MATCH_REPEAT_OPEN:
            case MATCH_CHAR_RANGE_OPEN:
            case MATCH_CHAR_LITERAL:
                return TRUE;
        }
    }
    return FALSE;
}


/*----------------------------------------------------------------------------
*
* Return TRUE if PATTERN has is a well formed regular expression according
* to the above syntax
*
* error_type is a return code based on the type of pattern error.  Zero is
* returned in error_type if the pattern is a valid one.  error_type return
* values are as follows:
*
*   PATTERN_VALID - pattern is well formed
*   PATTERN_ESC   - pattern has invalid escape ('\' at end of pattern)
*   PATTERN_RANGE - [..] construct has a no end range in a '-' pair (ie [a-])
*   PATTERN_CLOSE - [..] construct has no end bracket (ie [abc-g)
*   PATTERN_EMPTY - [..] construct is empty (ie [])
*
---------------------------------------------------------------------------*/

int is_valid_pattern(char *p, int *error_type)
{
    /* init error_type */
    *error_type = PATTERN_VALID;

    /* loop through pattern to EOS */
    while (*p) {

        /* determine pattern type */
        switch (*p) {

            /* check literal escape, it cannot be at end of pattern */
            case MATCH_CHAR_LITERAL:
                if (!*++p) {
                    *error_type = PATTERN_ESC;
                    return FALSE;
                }
                p++;
                break;

                /* the [..] construct must be well formed */
            case MATCH_CHAR_RANGE_OPEN:
                p++;

                /* if the next character is ']' then bad pattern */
                if (*p == MATCH_CHAR_RANGE_CLOSE) {
                    *error_type = PATTERN_EMPTY;
                    return FALSE;
                }

                /* if end of pattern here then bad pattern */
                if (!*p) {
                    *error_type = PATTERN_CLOSE;
                    return FALSE;
                }

                /* loop to end of [..] construct */
                while (*p != MATCH_CHAR_RANGE_CLOSE) {

                    /* check for literal escape */
                    if (*p == MATCH_CHAR_LITERAL) {
                        p++;

                        /* if end of pattern here then bad pattern */
                        if (!*p++) {
                            *error_type = PATTERN_ESC;
                            return FALSE;
                        }
                    }
                    else
                        p++;

                    /* if end of pattern here then bad pattern */
                    if (!*p) {
                        *error_type = PATTERN_CLOSE;
                        return FALSE;
                    }

                    /* if this a range */
                    if (*p == MATCH_CHAR_RANGE) {

                        /* we must have an end of range */
                        if (!*++p || *p == MATCH_CHAR_RANGE_CLOSE) {
                            *error_type = PATTERN_RANGE;
                            return FALSE;
                        }
                        else {

                            /* check for literal escape */
                            if (*p == MATCH_CHAR_LITERAL)
                                p++;

                            /* if end of pattern here then bad pattern */
                            if (!*p++) {
                                *error_type = PATTERN_ESC;
                                return FALSE;
                            }
                        }
                    }
                }
                break;

                /* the {..} construct must be well formed */
            case MATCH_REPEAT_OPEN:
                p++;

                /* if the next character is '}' then bad pattern */
                if (*p == MATCH_REPEAT_CLOSE) {
                    *error_type = PATTERN_EMPTY;
                    return FALSE;
                }

                /* if end of pattern here then bad pattern */
                if (!*p) {
                    *error_type = PATTERN_CLOSE;
                    return FALSE;
                }

                /* loop to end of {..} construct */
                while (*p != MATCH_REPEAT_CLOSE) {

                    /* check for literal escape */
                    if (*p == MATCH_CHAR_LITERAL) {
                        p++;

                        /* if end of pattern here then bad pattern */
                        if (!*p++) {
                            *error_type = PATTERN_ESC;
                            return FALSE;
                        }
                    }
                    else
                        p++;

                    /* if end of pattern here then bad pattern */
                    if (!*p) {
                        *error_type = PATTERN_CLOSE;
                        return FALSE;
                    }
                }
                break;

                /* the (..) construct must be well formed */
            case MATCH_GROUP_OPEN: {
                int glvl;
                p++;

                /* if the next character is ')' then bad pattern */
                if (*p == MATCH_GROUP_CLOSE) {
                    *error_type = PATTERN_EMPTY;
                    return FALSE;
                }

                /* if end of pattern here then bad pattern */
                if (!*p) {
                    *error_type = PATTERN_CLOSE;
                    return FALSE;
                }
                glvl=0;

BEGIN_OF_GROUP_CHECK_LOOP:
                /* loop to end of (..) construct */
                while (*p != MATCH_GROUP_CLOSE) {
                    if (*p==MATCH_GROUP_OPEN) glvl++;
                    /* check for literal escape */
                    if (*p == MATCH_CHAR_LITERAL) {
                        p++;

                        /* if end of pattern here then bad pattern */
                        if (!*p++) {
                            *error_type = PATTERN_ESC;
                            return FALSE;
                        }
                    }
                    else
                        p++;

                    /* if end of pattern here then bad pattern */
                    if (!*p) {
                        *error_type = PATTERN_CLOSE;
                        return FALSE;
                    }

                    /* if there is more than 1 pattern in group */
                    if (*p == MATCH_GROUP_OR) {

                        /* we must not have an end of group here */
                        if (!*++p || *p == MATCH_GROUP_CLOSE) {
                            *error_type = PATTERN_RANGE;
                            return FALSE;
                        }
                        else {

                            /* check for literal escape */
                            if (*p == MATCH_CHAR_LITERAL)
                                p++;

                            /* if end of pattern here then bad pattern */
                            if (!*p) {
                                *error_type = PATTERN_ESC;
                                return FALSE;
                            }
                        }
                    }
                }
                p++;
                if (glvl>0) {
                    glvl--;
                    goto BEGIN_OF_GROUP_CHECK_LOOP;
                }
                break;
                                   }
                                   /* all other characters are valid pattern elements */
            case MATCH_CHAR_KLEENE_CLOSURE:
            case MATCH_CHAR_SINGLE:
            default:            /* "normal" character */
                p++;
                break;
        }
    }

    return TRUE;
}

int is_null_pattern(char *p)
{
    if (p==NULL) return 1;
    if (*p=='\0') return 1;
#ifdef DEBUG
    printf("testing is_null pattern is: %s\n",p);
#endif
    while (*p) {
        int min=0,max=0,len;
        int retv;
        switch (*p) {
            case MATCH_CHAR_RANGE_OPEN:
                p=strchr(p,MATCH_CHAR_RANGE_CLOSE);
                if (p!=NULL) {
                    p++;
                    if (*p) {
                        consume_repeat(&p,&min,&max,&len);
                        if (min>0) return 0;
                    } else
                        return 0;
                } else return 0;
                break;

            case MATCH_CHAR_SINGLE:
                return 0;
                break;
            case MATCH_CHAR_KLEENE_CLOSURE:
                //return 1;
                p++;
                break;
            case MATCH_CHAR_CLOSURE:
                return 0;
                break;
            case MATCH_GROUP_OPEN:
                {
                    char *op = ++p;
                    p=strchr(p,MATCH_GROUP_CLOSE);
                    if (p!=NULL) {
                        char *tmp = (char*)malloc(p-op+1);
                        if (tmp == NULL) return 0;
                        memset(tmp,0, p-op+1);
                        memcpy(tmp, op, p-op);
                        retv=is_null_pattern(tmp);
                        free(tmp);
                        p++;
                        if (*p) {
                            consume_repeat(&p,&min,&max,&len);
                            if (min>0&&retv==0) return 0;
                        } else return retv;
                    }
                    else return 0;
                }
                break;
            case '|':
                p++;
                break;
            case MATCH_GROUP_CLOSE:
                return 1;
                break;
            case MATCH_CHAR_LITERAL:
                return 0;
                break;
            default:
                return 0;
        }
    }

    return 1;
}

int consume_bracket(char **p,int l)
{
    int glvl;

    glvl=l;
    while (glvl>0) {
        /* bad pattern (Missing MATCH_CHAR_RANGE_CLOSE) */
        if (!**p)
            return MATCH_PATTERN;
        /* skip exact match */
        if (**p == MATCH_CHAR_LITERAL) {
            (*p)++;
            /* if end of text then we have a bad pattern */
            if (!**p)
                return MATCH_PATTERN;
        }

        if (**p==MATCH_GROUP_OPEN) glvl++;
        if (**p==MATCH_GROUP_CLOSE) glvl--;
        /* move to next pattern char */
        (*p)++;
    }
    return 0;
}

int consume_repeat(char **p,int *min,int *max,int *len)
{
    char *rc;
    char cntStr[256];
    char *st,*end;
    int nanako;

    if(p==NULL) return 0;
    if(*p==NULL) return 0;

    switch(**p) {
        case MATCH_CHAR_SINGLE:
            *min=0;
            *max=1;
            break;
        case MATCH_CHAR_KLEENE_CLOSURE:
            *min=0;
            *max=MAXINT;
            break;
        case MATCH_CHAR_CLOSURE:
            *min=1;
            *max=MAXINT;
            break;
        case MATCH_REPEAT_OPEN:
            nanako=0;
            memset(cntStr,0,sizeof(cntStr));
            end=strchr(*p+1,MATCH_REPEAT_CLOSE);
            if(end==NULL) return MATCH_PATTERN;
            nanako=end-*p-1;
            *len=nanako;
            if (nanako<1) return MATCH_PATTERN;
            strncpy(cntStr,*p+1,nanako);
            end=strchr(cntStr,',');
            if (end==NULL)
                *min=*max=atoi(cntStr);
            else {
                *end='\0';
                end++;
                st=cntStr;
                *min=atoi(st);
                *max=atoi(end);
                if (*max<*min)
                    *max=*min;
            }
            rc=strchr(*p,MATCH_REPEAT_CLOSE);
            if(rc==NULL) return 0;
            *p=rc;
            break;
        default:
            *min=1;
            *max=MAXINT;
            return 0;
    }
    (*p)++;
    return 0;
}


/*----------------------------------------------------------------------------
*
* Match the pattern PATTERN against the string TEXT;
*
* returns MATCH_VALID if pattern matches, or an errorcode as follows
* otherwise:
*
*           MATCH_PATTERN  - bad pattern
*           MATCH_LITERAL  - match failure on literal mismatch
*           MATCH_RANGE    - match failure on [..] construct
*           MATCH_ABORT    - premature end of text string
*           MATCH_END      - premature end of pattern string
*           MATCH_VALID    - valid match
*
*
* A match means the entire string TEXT is used up in matching.
*
* In the pattern string:
*      `*' matches any sequence of characters (zero or more)
*      `?' matches any character
*      [SET] matches any character in the specified set,
*      [!SET] or [^SET] matches any character not in the specified set.
*      \ is allowed within a set to escape a character like ']' or '-'
*
* A set is composed of characters or ranges; a range looks like character
* hyphen character (as in 0-9 or A-Z).  [0-9a-zA-Z_] is the minimal set of
* characters allowed in the [..] pattern construct.  Other characters are
* allowed (ie. 8 bit characters) if your system will support them.
*
* To suppress the special syntactic significance of any of `[]*?!^-\', and
* match the character exactly, precede it with a `\'.
*
---------------------------------------------------------------------------*/

int matche(char *p, char **tptr)
{
    register char range_start, range_end;       /* start and end in range */

    int retv;

    int invert;             /* is this [..] or [!..] */
    int member_match;       /* have I matched the [..] construct? */
    int loop;               /* should I terminate? */
    char *t=*tptr;
    char *tend=*tptr;

    for(;*tend;tend++);

    for (; *p; p++, t++) {

#ifdef DEBUG
        printf("pattern is: %s\nliteral is: %s\n",p,t);
#endif

        /* if this is the end of the text then this is the end of the match */
        if (!*t) {
            if (is_null_pattern(p)) {
                *tptr=t;
                return  MATCH_VALID;
            }
            else return MATCH_ABORT;
        }

        /* determine and react to pattern type */
        switch (*p) {

            /* single any character match */
            case MATCH_CHAR_SINGLE:
                break;

                /* multiple any character match */
            case MATCH_CHAR_KLEENE_CLOSURE:
                return matche_after_star(p, t);

                /* [..] construct, single member/exclusion character match */
            case MATCH_CHAR_RANGE_OPEN:{
                char *pp;
                int scan;

                retv=0;
                scan=0;
                pp=p;
BEGIN_OF_RANGE:
                p=pp;
                /* move to beginning of range */
                p++;

                /* check if this is a member match or exclusion match */
                invert = FALSE;
                if (*p == MATCH_CHAR_EXCLAMATION_NEGATE ||
                    *p == MATCH_CHAR_CARAT_NEGATE) {
                        invert = TRUE;
                        p++;
                }

                /* if closing bracket here or at range start then we have
                * a malformed pattern */
                if (*p == MATCH_CHAR_RANGE_CLOSE) {
                    return MATCH_PATTERN;
                }

                member_match = FALSE;
                loop = TRUE;

                while (loop) {

                    /* if end of construct then loop is done */
                    if (*p == MATCH_CHAR_RANGE_CLOSE) {
                        loop = FALSE;
                        continue;
                    }

                    /* matching a '!', '^', '-', '\' or a ']' */
                    if (*p == MATCH_CHAR_LITERAL) {
                        range_start = range_end = *++p;
                    }
                    else {
                        range_start = range_end = *p;
                    }

                    /* if end of pattern then bad pattern (Missing ']') */
                    if (!*p)
                        return MATCH_PATTERN;

                    /* check for range bar */
                    if (*++p == MATCH_CHAR_RANGE) {

                        /* get the range end */
                        range_end = *++p;

                        /* if end of pattern or construct then bad
                        * pattern */
                        if (range_end == MATCH_CHAR_NULL ||
                            range_end == MATCH_CHAR_RANGE_CLOSE)
                            return MATCH_PATTERN;

                        /* special character range end */
                        if (range_end == MATCH_CHAR_LITERAL) {
                            range_end = *++p;

                            /* if end of text then we have a bad pattern */
                            if (!range_end)
                                return MATCH_PATTERN;
                        }

                        /* move just beyond this range */
                        p++;
                    }

                    /* if the text character is in range then match
                    * found. make sure the range letters have the proper
                    * relationship to one another before comparison */
                    if (range_start < range_end) {
                        if (*t >= range_start && *t <= range_end) {
                            member_match = TRUE;
                            loop = FALSE;
                        }
                    }
                    else {
                        if (*t >= range_end && *t <= range_start) {
                            member_match = TRUE;
                            loop = FALSE;
                        }
                    }
                }

                /* if there was a match in an exclusion set then no match */
                /* if there was no match in a member set then no match */
                if ((invert && member_match) || !(invert || member_match))
                    retv=MATCH_RANGE;

                /* if this is not an exclusion then skip the rest of the
                * [...] construct that already matched. */
                if (member_match) {
                    while (*p != MATCH_CHAR_RANGE_CLOSE) {

                        /* bad pattern (Missing MATCH_CHAR_RANGE_CLOSE) */
                        if (!*p)
                            return MATCH_PATTERN;

                        /* skip exact match */
                        if (*p == MATCH_CHAR_LITERAL) {
                            p++;

                            /* if end of text then we have a bad pattern */
                            if (!*p)
                                return MATCH_PATTERN;
                        }

                        /* move to next pattern char */
                        p++;
                    }
                }

                switch (*(p+1)) {
            case MATCH_CHAR_CLOSURE:
                if(retv>0) {
                    if (scan==0) return retv;
                    else {
                        t--;p++;
                        break;
                    }
                }
                else {
                    if (scan>0) {
                        if (match(p+2,t)){
                            *tptr=t;
                            return MATCH_VALID;
                        }
                    }
                    scan=1;
                    t++;
                    goto BEGIN_OF_RANGE;
                }

            case MATCH_CHAR_KLEENE_CLOSURE:
                if(retv>0) {
                    t--;p++;
                    break;
                }
                else {
                    if (match(p+2,t)) {
                        *tptr=t;
                        return MATCH_VALID;
                    }
                    scan=1;
                    t++;
                    goto BEGIN_OF_RANGE;
                }

            case MATCH_CHAR_SINGLE:
                if(retv>0) {
                    t--;p++;
                    break;
                }
                else {
                    if (match(p+2,t)) {
                        *tptr=t;
                        return MATCH_VALID;
                    }
                    p++;
                    break;
                }

            case MATCH_REPEAT_OPEN:
                {
                    char cntStr[256];
                    char *st,*end;
                    int nanako,min,max;

                    nanako=0;
                    memset(cntStr,0,sizeof(cntStr));

                    end=strchr(p+2,MATCH_REPEAT_CLOSE);
                    if(end==NULL)
                        return MATCH_PATTERN;
                    nanako=end-p-2;
                    if (nanako<1)
                        return MATCH_PATTERN;
                    strncpy(cntStr,p+2,nanako);
                    end=strchr(cntStr,',');

                    if (end==NULL)
                        min=max=atoi(cntStr);
                    else {
                        *end='\0';
                        end++;
                        st=cntStr;
                        min=atoi(st);
                        max=atoi(end);

                        if (max<min)
                            max=min;
                    }

                    if(retv>0) {
                        if (scan<min)
                            return retv;
                        else {
                            t--;
                            p+=nanako+2;
                            break;
                        }
                    }
                    else {
                        if (scan>=min) {
                            if (match(p+nanako+2,t)){
                                *tptr=t;
                                return MATCH_VALID;
                            }
                        }
                        scan++;
                        if(scan==max) {
                            p+=nanako+2;
                            break;
                        }
                        t++;
                        goto BEGIN_OF_RANGE;
                    }
                }

            default:
                if(retv)
                    return retv;
                break;
                }
                                       }
                                       break;
                                       /* (..) construct, group pattern match */

            case MATCH_GROUP_OPEN:
                {
                    char *pp,*pr,*pq,*tp;
                    int scan,min,max;
                    int nanako;

                    retv=0;
                    scan=0;
                    pp=pq=p;
                    tp=t;
                    pq++;
                    if (consume_bracket(&pq,1))
                        return MATCH_PATTERN;

                    consume_repeat(&pq,&min,&max,&nanako);
BEGIN_OF_GROUP:
                    if(scan&&(tp==t))
                        return MATCH_PATTERN;
                    else
                        tp=t;
                    p=pp;
                    /* move to beginning of range */
                    p++;

                    if ((*p==MATCH_GROUP_CLOSE) ||(*p==MATCH_GROUP_OR))
                        return MATCH_PATTERN;

                    /* if closing bracket here or a OR seperator then we have a malformed pattern */
                    loop=TRUE;
                    member_match=FALSE;

                    while(loop) {
                        char *tmp;
                        int glvl;

                        pr=p;
                        glvl=0;
                        while(((*pr)&&(*pr!=MATCH_GROUP_OR))||(glvl>0)) {
                            if (*pr==MATCH_GROUP_OPEN)
                                glvl++;
                            if (*pr==MATCH_GROUP_CLOSE){
                                if (glvl>0)
                                    glvl--;
                                else
                                    break;
                            }
                            pr++;
                        }
                        tmp=(char *)malloc((pr-p)+10+strlen(pq));
                        if (tmp==NULL) {
                            //printf("Allocate tmp error!%c",'\n');
                            return MATCH_PATTERN;
                        }
                        memset(tmp,0,(pr-p)+10+strlen(pq));
                        strncat(tmp,p,pr-p);
                        retv=matche(tmp,&t);
                        strcat(tmp,pq);
                        if (scan>=min-1&&match(tmp,tp)) {
                            free(tmp);
                            return MATCH_VALID;
                        }
                        free(tmp);
                        if ((retv==MATCH_VALID)||(retv==MATCH_END&&t!=tp)) {
                            member_match=TRUE;
                            retv=0;
                            loop=FALSE;
                        }
                        if (*pr==MATCH_GROUP_CLOSE) {
                            p=pr;
                            loop=FALSE;
                        }
                        else
                            p=pr+1;
                    }
                    if (consume_bracket(&p,1))
                        return MATCH_PATTERN;

                    /*{
                    int glvl;
                    glvl=1;
                    while (glvl>0) {
                    // bad pattern (Missing MATCH_CHAR_RANGE_CLOSE)
                    if (!*p)
                    return MATCH_PATTERN;
                    // skip exact match
                    if (*p == MATCH_CHAR_LITERAL) {
                    p++;

                    // if end of text then we have a bad pattern
                    if (!*p)
                    return MATCH_PATTERN;
                    }
                    if (*p==MATCH_GROUP_OPEN) glvl++;
                    if (*p==MATCH_GROUP_CLOSE) glvl--;
                    // move to next pattern char
                    p++;
                    }
                    }*/

                    switch (*p) {
            case MATCH_CHAR_CLOSURE:
                if(retv>0) {
                    if (scan==0) return retv;
                    else {
                        t--;
                        break;
                    }
                }
                else {
                    if (scan>0) {
                        if(match(p+1,tp)) {
                            *tptr=tp;
                            return MATCH_VALID;
                        }
                    }
                    scan=1;
                    goto BEGIN_OF_GROUP;
                }

            case MATCH_CHAR_KLEENE_CLOSURE:
                if(retv>0) {
                    t--;
                    break;
                }
                else {
                    if (match(p+1,tp)) {
                        *tptr=tp;
                        return MATCH_VALID;
                    }
                    scan=1;
                    goto BEGIN_OF_GROUP;
                }

            case MATCH_CHAR_SINGLE:
                if(retv>0) {
                    t--;
                    break;
                }
                else {
                    if(match(p+1,tp)) {
                        *tptr=tp;
                        return MATCH_VALID;
                    }
                }

            case MATCH_REPEAT_OPEN:
                if(retv>0) {
                    if (scan<min) return retv;
                    else {
                        t--;p+=nanako+1;
                        break;
                    }
                }
                else {
                    if (scan>=min) {
                        if (match(p+nanako+1,t)){
                            *tptr=t;
                            return MATCH_VALID;
                        }
                    }
                    scan++;
                    if(scan==max) {
                        t--,p+=nanako+1;
                        break;
                    }
                    goto BEGIN_OF_GROUP;
                }


            default:
                if(retv) return retv;
                t--;p--;
                break;
                    }
                    break;
                }

                /* next character is quoted and must match exactly */
            case MATCH_CHAR_LITERAL:

                /* move pattern pointer to quoted char and fall through */
                p++;

                /* if end of text then we have a bad pattern */
                if (!*p)
                    return MATCH_PATTERN;

                /* must match this character exactly */
            default:
                if (*p != *t)
                    return MATCH_LITERAL;
        }
    }

    /* if end of text not reached then the pattern fails */
    *tptr=t;
    if (*t)
        return MATCH_END;
    else
        return MATCH_VALID;
}


/*----------------------------------------------------------------------------
*
* recursively call matche() with final segment of PATTERN and of TEXT.
*
---------------------------------------------------------------------------*/

int matche_after_star(char *p, char *t)
{
    register int match = 0;
    register char nextp;

    /* pass over existing ? and * in pattern */
    while (*p == MATCH_CHAR_SINGLE ||
        *p == MATCH_CHAR_KLEENE_CLOSURE) {

            /* take one char for each ? and + */
            if (*p == MATCH_CHAR_SINGLE) {

                /* if end of text then no match */
                if (!*t++) {
                    return MATCH_ABORT;
                }
            }

            /* move to next char in pattern */
            p++;
    }

    /* if end of pattern we have matched regardless of text left */
    if (!*p) {
        return MATCH_VALID;
    }

    /* get the next character to match which must be a literal or '[' */
    nextp = *p;

    if (nextp == MATCH_CHAR_LITERAL) {
        nextp = p[1];

        /* if end of text then we have a bad pattern */
        if (!nextp)
            return MATCH_PATTERN;
    }

    /* Continue until we run out of text or definite result seen */
    do {

        /* a precondition for matching is that the next character in the
        * pattern match the next character in the text or that the next
        * pattern char is the beginning of a range.  Increment text pointer
        * as we go here */
        if (nextp == *t || nextp == MATCH_CHAR_RANGE_OPEN || nextp == MATCH_GROUP_OPEN) {
            match = matche(p, &t);
        }

        /* if the end of text is reached then no match */
        if (match!=MATCH_VALID && !*t++)
            match = MATCH_ABORT;

    } while (match != MATCH_VALID && match != MATCH_ABORT && match != MATCH_PATTERN);

    /* return result */
    return match;
}


/*----------------------------------------------------------------------------
*
* match() is a shell to matche() to return only BOOLEAN values.
*
---------------------------------------------------------------------------*/

int match(char *p, char *t)
{
    int error_type;

    if (is_pattern(p)) {
        error_type = matche(p, &t);
        return (error_type == MATCH_VALID) ? TRUE : FALSE;
    }
    else
        return (strcmp(p,t)==0)? TRUE: FALSE ;
}

/* Glob-style pattern matching. */
int stringmatchlen(const char *pattern, int patternLen,
        const char *string, int stringLen, int nocase)
{
    while(patternLen) {
        switch(pattern[0]) {
        case '*':
            while (pattern[1] == '*') {
                pattern++;
                patternLen--;
            }
            if (patternLen == 1)
                return 1; /* match */
            while(stringLen) {
                if (stringmatchlen(pattern+1, patternLen-1,
                            string, stringLen, nocase))
                    return 1; /* match */
                string++;
                stringLen--;
            }
            return 0; /* no match */
            break;
        case '?':
            if (stringLen == 0)
                return 0; /* no match */
            string++;
            stringLen--;
            break;
        case '[':
        {
            int nott, match;

            pattern++;
            patternLen--;
            nott = pattern[0] == '^';
            if (nott) {
                pattern++;
                patternLen--;
            }
            match = 0;
            while(1) {
                if (pattern[0] == '\\') {
                    pattern++;
                    patternLen--;
                    if (pattern[0] == string[0])
                        match = 1;
                } else if (pattern[0] == ']') {
                    break;
                } else if (patternLen == 0) {
                    pattern--;
                    patternLen++;
                    break;
                } else if (pattern[1] == '-' && patternLen >= 3) {
                    int start = pattern[0];
                    int end = pattern[2];
                    int c = string[0];
                    if (start > end) {
                        int t = start;
                        start = end;
                        end = t;
                    }
                    if (nocase) {
                        start = tolower(start);
                        end = tolower(end);
                        c = tolower(c);
                    }
                    pattern += 2;
                    patternLen -= 2;
                    if (c >= start && c <= end)
                        match = 1;
                } else {
                    if (!nocase) {
                        if (pattern[0] == string[0])
                            match = 1;
                    } else {
                        if (tolower((int)pattern[0]) == tolower((int)string[0]))
                            match = 1;
                    }
                }
                pattern++;
                patternLen--;
            }
            if (nott)
                match = !match;
            if (!match)
                return 0; /* no match */
            string++;
            stringLen--;
            break;
        }
        case '\\':
            if (patternLen >= 2) {
                pattern++;
                patternLen--;
            }
            /* fall through */
        default:
            if (!nocase) {
                if (pattern[0] != string[0])
                    return 0; /* no match */
            } else {
                if (tolower((int)pattern[0]) != tolower((int)string[0]))
                    return 0; /* no match */
            }
            string++;
            stringLen--;
            break;
        }
        pattern++;
        patternLen--;
        if (stringLen == 0) {
            while(*pattern == '*') {
                pattern++;
                patternLen--;
            }
            break;
        }
    }
    if (patternLen == 0 && stringLen == 0)
        return 1;
    return 0;
}

int stringmatch(const char *pattern, const char *string, int nocase) {
    return stringmatchlen(pattern,strlen(pattern),string,strlen(string),nocase);
}

#ifdef TEST

/*
* This test main expects as first arg the pattern and as second arg
* the match string.  Output is yaeh or nay on match.  If nay on
* match then the error code is parsed and written.
*/

#include <stdio.h>

int main(int argc, char *argv[])
{
    int error, is_valid_error;

    if (argc != 3) {
        printf("Usage:  MATCH Pattern Text\n");
    }
    else {
        printf("Pattern: %s\n", argv[1]);
        printf("Text   : %s\n", argv[2]);

        match(argv[1],argv[2]) ? printf("TRUE") : printf("FALSE");
        error = matche(argv[1], &argv[2]);
        is_valid_pattern(argv[1], &is_valid_error);

        switch (error) {
            case MATCH_VALID:
                printf("    Match Successful");
                if (is_valid_error != PATTERN_VALID)
                    printf(" -- is_valid_pattern() is complaining\n");
                else
                    printf("\n");
                break;

            case MATCH_LITERAL:
                printf("    Match Failed on Literal\n");
                break;

            case MATCH_RANGE:
                printf("    Match Failed on [..]\n");
                break;
            case MATCH_ABORT:
                printf("    Match Failed on Early Text Termination\n");
                break;
            case MATCH_END:
                printf("    Match Failed on Early Pattern Termination\n");
                break;
            case MATCH_PATTERN:
                switch (is_valid_error) {
            case PATTERN_VALID:
                printf("    Internal Disagreement On Pattern\n");
                break;

            case PATTERN_ESC:
                printf("    Literal Escape at End of Pattern\n");
                break;

            case PATTERN_RANGE:
                printf("    No End of Range in [..] Construct\n");
                break;
            case PATTERN_CLOSE:
                printf("    [..] Construct is Open\n");
                break;
            case PATTERN_EMPTY:
                printf("    [..] Construct is Empty\n");
                break;
            default:
                printf("    Internal Error in is_valid_pattern()\n");
                }
                break;
            default:
                printf("    Internal Error in matche()\n");
                break;
        }
    }
    return (0);
}

#endif
