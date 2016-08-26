#pragma once
#include <iostream>

namespace common {

    class Teestreambuf : public std::streambuf
    {
    public:
        Teestreambuf(std::streambuf* buf1 = 0, std::streambuf* buf2 = 0)
            : streambuf1(buf1),
            streambuf2(buf2)
        {
            setp(0, 0);
        }

        void tie(std::streambuf* buf1, std::streambuf* buf2 = 0)
        {
            streambuf1 = buf1;
            streambuf2 = buf2;
        }

    private:
        std::streambuf::int_type overflow(std::streambuf::int_type ch);
        std::streambuf::int_type underflow();
        int sync();

        std::streambuf* streambuf1;
        std::streambuf* streambuf2;
    };

    /////////////////////////////////////////////////////////////////////////////

    class Tee : public std::ostream
    {
        typedef std::ostream base_class;
        Teestreambuf streambuf;

    public:
        Tee()
            : std::ostream(0),
            streambuf(std::cout.rdbuf())
        {
                init(&streambuf);
            }
        Tee(std::ostream& s1, std::ostream& s2)
            : std::ostream(0),
            streambuf(s1.rdbuf(), s2.rdbuf())
        {
                init(&streambuf);
            }
        Tee(std::ostream& s)
            : std::ostream(0),
            streambuf(s.rdbuf(), std::cout.rdbuf())
        {
                init(&streambuf);
            }

        void assign(std::ostream& s1, std::ostream& s2);
        void assign(std::ostream& s)
        {
            assign(s, std::cout);
        }
        void assign_single(std::ostream& s);
    };

}
