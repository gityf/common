#include "iconvwrap.h"

namespace common {
    IconvWrap::IconvWrap()
        : cd(iconv_t(-1))
    {
    }

    IconvWrap::IconvWrap(const char *tocode, const char *fromcode) {
        open(tocode, fromcode);
    }

    bool IconvWrap::close() {
        if (cd != iconv_t(-1)) {
            int ret;

            ret = ::iconv_close(cd);
            cd = iconv_t(-1);

            return (ret != -1);
        }

        return true;
    }

    bool IconvWrap::convert(char **inbuf, size_t *inbytesleft,
        char **outbuf, size_t *outbytesleft) {
        return (iconv(cd, inbuf, inbytesleft, outbuf, outbytesleft) != size_t(-1));
    }

    bool IconvWrap::is_open() {
        return (cd != iconv_t(-1));
    }

    bool IconvWrap::open(const char *tocode, const char *fromcode) {
        close();
        cd = ::iconv_open(tocode, fromcode);

        return (cd != iconv_t(-1));
    }

    IconvWrap::~IconvWrap() {
        close();
    }

}
