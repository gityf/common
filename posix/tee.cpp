#include "tee.h"

namespace common {
    std::streambuf::int_type Teestreambuf::overflow(std::streambuf::int_type ch)
    {
        if (ch != traits_type::eof())
        {
            if (streambuf1 && streambuf1->sputc(ch) == traits_type::eof())
                return traits_type::eof();

            if (streambuf2 && streambuf2->sputc(ch) == traits_type::eof())
                return traits_type::eof();
        }

        return 0;
    }

    std::streambuf::int_type Teestreambuf::underflow()
    {
        return traits_type::eof();
    }

    int Teestreambuf::sync()
    {
        if (streambuf1 && streambuf1->pubsync() == traits_type::eof())
            return traits_type::eof();

        if (streambuf2 && streambuf2->pubsync() == traits_type::eof())
            return traits_type::eof();

        return 0;
    }

    /////////////////////////////////////////////////////////////////////////////
    void Tee::assign(std::ostream& s1, std::ostream& s2)
    {
        Teestreambuf* buf = dynamic_cast<Teestreambuf*>(rdbuf());
        if (buf)
            buf->tie(s1.rdbuf(), s2.rdbuf());
    }

    void Tee::assign_single(std::ostream& s)
    {
        Teestreambuf* buf = dynamic_cast<Teestreambuf*>(rdbuf());
        if (buf)
            buf->tie(s.rdbuf());
    }

}
