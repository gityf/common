#pragma once

namespace posix {

    class Pipe {
    public:
        /** @brief Creates the pipe with two fd

            The default constructor will create the pipe and the appropriate
            IODevices to read and write to the pipe.
            */
        explicit Pipe();

        /** @brief Endpoint of the pipe to read from

            @return An IODevice used to read from the pipe
            */
        int getReadFd() const;

        int getWriteFd() const;

        void closeReadFd();

        void closeWriteFd();

        /// Redirect write-end to stdout.
        /// When the close argument is set, closes the original fd
        void redirectStdout(bool close = true, bool inherit = true);

        /// Redirect read-end to stdin.
        /// When the close argument is set, closes the original fd
        void redirectStdin(bool close = true, bool inherit = true);

        /// Redirect write-end to stdout.
        /// When the close argument is set, closes the original fd
        void redirectStderr(bool close = true, bool inherit = true);

        size_t write(const char* buf, size_t count);

        void write(char ch);

        size_t read(char* buf, size_t count);

        char read();

        int in() {
            return fds_[1];
        }

        int out() {
            return fds_[0];
        }
    private:
        int fds_[2];
    };
}
