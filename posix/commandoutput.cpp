#include <stdexcept>
#include <string>
#include "commandoutput.h"

namespace posix {
    void CommandOutput::run(bool combineStderr) {
        _fork.fork();
        if (_fork.child()) {
            _pipe.redirectStdout();
            if (combineStderr)
                _pipe.redirectStderr();

            _pipe.closeReadFd();
            try {
                _exec.exec();
            } catch (const std::string&) {
                ::_exit(-1);
            }
        }

        _pipe.closeWriteFd();
    }

}
