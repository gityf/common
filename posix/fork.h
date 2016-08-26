#pragma once
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdexcept>
#include <string>

namespace posix {
    /** A simple wrapper for the system function fork(2).
     *
     *  The advantage of using this class instead of fork directly is, easyness,
     *  robustness and readability due to less code. The constructor executes
     *  fork(2) and does error checking. The destructor waits for the child
     *  process, which prevents the creation of zombie processes. The user may
     *  decide to deactivate it or waiting explicitely to receive the return
     *  status, but this has to be done explicitely, which helps robustness.
     *
     *  Example:
     *  \code
     *    {
     *      posix::Fork process;
     *      if (process.child())
     *      {
     *        // we are in the child process here.
     *
     *        exit(0);  // normally the child either exits or execs an other
     *                  // process
     *      }
     *    }
     *  \endcode
     */
    class Fork {
        Fork(const Fork&);
        Fork& operator= (const Fork&);

        pid_t pid;

    public:
        Fork(bool now = true)
            : pid(0) {
            if (now)
                fork();
        }

        ~Fork() {
            if (pid) {
                wait();
            }
        }

        void fork() {
            pid = ::fork();
            if (pid < 0)
                throw std::runtime_error("fork");
        }

        pid_t getPid() const  { return pid; }
        bool parent() const   { return pid > 0; }
        bool child() const    { return !parent(); }
        void setNowait()      { pid = 0; }
        int wait(int options = 0) {
            int status;
            ::waitpid(pid, &status, options);
            pid = 0;
            return status;
        }
    };
}
