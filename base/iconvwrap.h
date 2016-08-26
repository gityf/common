#pragma once
#include <iconv.h>

namespace common {

    /** Wraps iconv. */
    class IconvWrap {
    public:

        /** Create iconvwrap object. */
        IconvWrap();

        /** Create iconvwrap object and initializes it.
         *
         * @param tocode destination encoding name
         * @param fromcode source encoding name
         */
        IconvWrap(const char *tocode, const char *fromcode);

        /** Close iconvwrap object, release resouces.
         *
         * @return true on succes, otherwise return false and set errno. */
        bool close();

        /** Recode input string into output buffer.
         *
         * @param inbuf
         * @param inbytesleft
         * @param outbuf
         * @param outbytesleft
         * @return true if all succesfully converted, on error returns false
         *      and se errno
         */
        bool convert(char **inbuf, size_t *inbytesleft,
            char **outbuf, size_t *outbytesleft);

        /** Return true if IConv is open, false otherwise. */
        bool is_open();

        /** (Re)initializes iconvwrap object.
         *
         * @param tocode target encoding name
         * @param fromcode source encoding name
         * @return true on succes, on error return false and set errno
         */
        bool open(const char *tocode, const char *fromcode);

        /** Destroy iconvwrap object. */
        ~IconvWrap();

    protected:
        iconv_t cd;
    };

}

