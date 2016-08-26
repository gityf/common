#include <stdexcept>
#include "commandinput.h"

namespace posix {
    void CommandInput::run() {
        _fork.fork();
        if (_fork.child()) {
            _pipe.redirectStdin();
            _pipe.closeWriteFd();
            try {
                _exec.exec();
            } catch (const std::string&) {
                ::_exit(-1);
            }
        }
        _pipe.closeReadFd();
    }

}
