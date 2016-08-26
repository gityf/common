#include <stdexcept>
#include <memory>
#include <cerrno>
#include <unistd.h>
#include <fcntl.h>
#include "pipe.h"

namespace posix {
    namespace {
        void redirect(int fd, int newFd) {
            if (::dup2(fd, newFd) < 0)
                throw std::runtime_error("dup2");
        }
    }

    void Pipe::pipe() {
        if (-1 == ::pipe(fds_))
            throw std::runtime_error("pipe");
    }
    int Pipe::getReadFd() {
        return in();
    }
    int Pipe::getWriteFd() {
        return out();
    }

    void Pipe::closeReadFd() {
        ::close(out());
    }

    void Pipe::closeWriteFd() {
        ::close(in());
    }

    /// Redirect write-end to stdout.
    /// When the close argument is set, closes the original fd
    void Pipe::redirectStdout() {
        redirect(in(), 1);
    }

    /// Redirect read-end to stdin.
    /// When the close argument is set, closes the original fd
    void Pipe::redirectStdin() {
        redirect(out(), 0);
    }

    /// Redirect write-end to stdout.
    /// When the close argument is set, closes the original fd
    void Pipe::redirectStderr() {
        redirect(in(), 2);
    }

    size_t Pipe::write(const char* buf, size_t count) {
        return ::write(in(), buf, count);
    }

    void Pipe::write(char ch) {
        ::write(in(), &ch, 1);
    }

    size_t Pipe::read(char* buf, size_t count) {
        return ::read(out(), buf, count);
    }

    char Pipe::read() {
        char ch;
        ::read(out(), &ch, 1);
        return ch;
    }
}
