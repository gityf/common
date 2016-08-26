#pragma once
#include <stdexcept>
#include <string>
#include <unistd.h>
#include "base/uncopyable.h"

namespace posix {
    /** cxxtools::posix::Exec is a wrapper around the exec?? functions of posix.

        Usage is like this:
        \code
        posix::Exec e("ls");
        e.push_back("-l");
        e.exec();
        \endcode

        This replaces the current process with the unix command "ls -l".
        */
    template <unsigned dataSize, unsigned maxArgs>
    class BasicExec : private Uncopyable {
        char data[dataSize];
        char* args[maxArgs + 2];
        unsigned argc;

    public:
        typedef const std::string& const_reference;
        typedef std::string& reference;

        explicit BasicExec(const std::string& cmd)
            : argc(0) {
            if (cmd.size() >= dataSize - 1)
                throw std::out_of_range("command <" + cmd + "> too large");
            args[0] = &data[0];
            cmd.copy(args[0], cmd.size());
            args[0][cmd.size()] = '\0';
            args[1] = args[0] + cmd.size() + 1;
        }

        BasicExec& push_back(const std::string& arg) {
            if (static_cast<unsigned>(args[argc + 1] + arg.size() - data) >= dataSize)
                throw std::out_of_range("argument list too long");
            if (argc >= maxArgs)
                throw std::out_of_range("too many arguments");

            ++argc;
            arg.copy(args[argc], arg.size());
            args[argc][arg.size()] = '\0';
            args[argc + 1] = args[argc] + arg.size() + 1;

            return *this;
        }

        // nice alias of push_back
        BasicExec& arg(const std::string& arg) {
            return push_back(arg);
        }

        void exec() {
            args[argc + 1] = 0;
            ::execvp(args[0], args);
            throw std::runtime_error("execvp");
        }

    };

    typedef BasicExec<0x8000, 256> Exec;
}
