#include "commandoutput.h"
#include <iostream>

int main() {
    posix::CommandOutput grep("ls");
    grep.push_back(".");  // add a parameter
    grep.run();             // runs the process
	char buf[100] = {0};
    grep.inPipe().read(buf, sizeof buf);     // this line is printed
	std::cout << "in: " << buf << std::endl;
    return 0;
}
