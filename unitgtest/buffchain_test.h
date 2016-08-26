#include "bufchain.h"
#include <iostream>
#include <string>
using namespace std;

int main() {

    BufferChain bc;
    while (1) {
        string str;
        cin >> str;
        bc.add(str.data(), str.length());
        cout << "size of bufchain: " << bc.size();
        char buf[1024] = {0};
        bc.fetch(buf, 1023);
        cout << "buf is: " << buf << endl;
        bc.comsume(1);
    }
    return 0;
}