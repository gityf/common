#include "commandinput.h"

int main() {
    posix::CommandInput grep("cat");
    grep.push_back("foo");  // add a parameter
    grep.run();             // runs the process
    grep.outPipe().write("foo", 3);     // this line is printed
    grep.outPipe().write("bar", 3);     // this line is not printed
    grep.outPipe().write("foobar", 6);    // this line is printed
    return 0;
}
