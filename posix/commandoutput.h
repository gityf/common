#pragma once
#include "exec.h"
#include "fork.h"
#include "pipe.h"

namespace posix {
    /**
     posix::CommandOutput starts a process and the stdout of the process can be read.

     The class is derived from std::istream, so all methods provided from that
     base class can be used to read the output.

     Typical usage example:
     \code
     posix::CommandOutput ls("ls");
     ls.push_back("-l");   // add a parameter
     ls.push_back("/bin"); // and another one
     ls.run();             // start the process

     // read output line by line
     std::string line;
     while (std::getline(ls, line))
     std::cout << line << std::endl;
     \endcode
     */
    class CommandOutput {
        Fork _fork;
        Exec _exec;
        Pipe _pipe;

    public:
        explicit CommandOutput(const std::string& cmd)
            :_fork(false),
            _exec(cmd)
        {
            _pipe.pipe();
        }

        void push_back(const std::string& arg)
        {
            _exec.push_back(arg);
        }

        void run(bool combineStderr = false);

        int wait(int options = 0) {
            return _fork.wait();
        }

        int in() {
            return _pipe.in();
        }

        Pipe& inPipe() {
            return _pipe;
        }

        void close() {
            _pipe.closeReadFd();
        }

    };

}
