#pragma once
#include "fork.h"
#include "exec.h"
#include "pipe.h"

namespace posix {
    /**
     posix::CommandInput starts a process and the stdin is connected to the current process.

     The class is derived from std::ostream. You can use all methods provided
     from that class to send data to the stdin of the process.

     Typical usage example:
     \code
       posix::CommandInput grep("grep");
       grep.push_back("foo");  // add a parameter
       grep.run();             // runs the process
       grep << "foo" << std::endl;     // this line is printed
       grep << "bar" << std::endl;     // this line is not printed
       grep << "foobar" << std::endl;     // this line is printed
     \endcode
     */
    class CommandInput {
        Fork _fork;
        Exec _exec;
        Pipe _pipe;

      public:
        explicit CommandInput(const std::string& cmd)
          : _fork(false),
            _exec(cmd)
        {
            _pipe.pipe();
        }

        void push_back(const std::string& arg)
        { _exec.push_back(arg); }

        void run();

        int wait() { return _fork.wait(); }

        int out() { return _pipe.out(); }

        Pipe& outPipe() {
            return _pipe;
        }

        void close() {
            _pipe.closeWriteFd();
        }

    };

  }
