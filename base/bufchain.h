#pragma  once
#include <assert.h>
#include <string.h>
#include <list>
#include <memory>

// ---------------------------------------------------------------------------
// Chunk of code lifted to manage buffers of
// data to be written to an fd.
//
struct ChainNode {
    ChainNode() {
        data_ = pos_ = NULL;
        length_ = 0;
    }
    ~ChainNode() {
        if (data_) {
            delete[] data_;
        }
    }
    char *data_;
    char *pos_;
    int length_;
};

static const int chainNodeSize = 5;
class BufferChain {
public:
    BufferChain() {
        bufferSize_ = 0;
    }
    ~BufferChain() {
        chains_.clear();
    }

    int size() {
        return bufferSize_;
    }

    void add(const char* aData, int aLen) {
        if (chains_.empty()) {
            createNode();
        }
        int tailLeft = chainNodeSize - chains_.back()->length_;
        if (tailLeft >= aLen) {
            appendData(aData, aLen);
        }
        else {
            appendData(aData, tailLeft);
            aLen -= tailLeft;
            int nodeSize = aLen / chainNodeSize;
            int leftSize = aLen % chainNodeSize;
            int pos = tailLeft;
            while (nodeSize-- > 0) {
                createNode();
                appendData(aData + pos, chainNodeSize);
                pos += chainNodeSize;
            }
            if (leftSize > 0) {
                createNode();
                appendData(aData + pos, leftSize);
            }
        }
    }

    int fetch(char* aOutData, int aLen) {
        if (aLen > bufferSize_) {
            aLen = bufferSize_;
        }
        int pos = 0;
        for (auto it = chains_.begin();
            it != chains_.end() && pos < aLen; ++it) {
            memcpy(aOutData + pos, it->get()->pos_, it->get()->length_);
            pos += it->get()->length_;
        }
        return pos;
    }

    void comsume(int aLen) {
        if (aLen > bufferSize_) {
            aLen = bufferSize_;
        }

        if (chains_.empty()) {
            return;
        }
        int nodeSize = aLen / chainNodeSize;
        int leftSize = aLen % chainNodeSize;
        while (nodeSize-- > 0) {
            chains_.pop_front();
        }
        if (leftSize > 0) {
            if (leftSize >= chains_.front()->length_) {
                leftSize -= chains_.front()->length_;
                chains_.pop_front();
            }
            chains_.front()->pos_ += leftSize;
            chains_.front()->length_ -= leftSize;
        }
        bufferSize_ -= aLen;
    }
private:
    void createNode() {
        ChainNode* node = new ChainNode;
        node->data_ = new char[chainNodeSize];
        node->length_ = 0;
        node->pos_ = node->data_;
        chains_.push_back(std::move(std::unique_ptr<ChainNode>(node)));
    }

    void appendData(const char* aData, int aLen) {
        memcpy(chains_.back()->data_ + chains_.back()->length_, aData, aLen);
        chains_.back()->length_ += aLen;
        bufferSize_ += aLen;
    }
private:
    int bufferSize_;
    std::list<std::unique_ptr<ChainNode>> chains_;
};
