#pragma  once
class StackBuffer {
public:
    StackBuffer(const char* buf, size_t c) : buffer(buf), count(c) {}

    const char* buffer;
    size_t count;
};
// end of local file.