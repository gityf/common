#pragma once
#include <map>
#include <vector>
#include <atomic>
#include <memory>
#include <functional>
#include "codec.h"

enum class ECodecResult {
    kCodecError,
    kCodecCompleted,
    kCodecNotComplete,
    kCodecContining
};
enum class ECallbackOper {
    kSetRead,
    kSetWrite
};
using FinishFuncPtr = std::function<ECodecResult(int&, Buffer&, Slice&)>;
// one buffer event for one thread.
class BufferEvent {
public:
    BufferEvent();
    ~BufferEvent();
    // buffer of reading/writing function for socket io.
    // 1: ok, 0:continue, -1:error.
    int Request(int aSockid, Slice& aOutMsg);
    int Response(int aSockid);
    // set codec method.
    void setCodec(CodecBase* aCodec);
    // complete check function.
    void setFinishCallback(ECallbackOper aOper, FinishFuncPtr&& aCallback) {
        switch (aOper)
        {
        case ECallbackOper::kSetRead:
            read_ = std::move(aCallback);
            break;
        case ECallbackOper::kSetWrite:
            write_ = std::move(aCallback);
            break;
        default:
            break;
        }
    }
    void setFinishCallback(ECallbackOper aOper, const FinishFuncPtr& aCallback) {
        setFinishCallback(aOper, FinishFuncPtr(aCallback));
    }
    Buffer& getRespBuffer(int aSockid);
    Buffer& getRequBuffer(int aSockid);

    // set response buffer here.
    // the encode will be called by default in the function.
    void setRespBuffer(int aSockid, Slice aMsg);
private:
    void setDecodeCallback();
    void setEncodeCallback();
public:
    std::map<int, std::unique_ptr<Buffer>> inputs_;
    std::map<int, std::unique_ptr<Buffer>> outputs_;
private:
    FinishFuncPtr read_;
    FinishFuncPtr write_;
    std::unique_ptr<CodecBase> codec_;
};
// end of local file.
