#include <thread>
#include "bufferevent.h"
#include "base/log.h"

BufferEvent::BufferEvent() {
    read_ = write_ = nullptr;
}
BufferEvent::~BufferEvent() {
}

int BufferEvent::Request(int aSockid, Slice& aOutMsg) {
    if (inputs_.find(aSockid) == inputs_.end()) {
        inputs_.insert(std::make_pair(aSockid,
            std::move(std::unique_ptr<Buffer>(new Buffer()))));
    }
    auto it = inputs_.find(aSockid);
    // wait to implement.
    while (true) {
        Buffer& input_ = *it->second.get();
        input_.makeRoom();
        int rd = ::read(aSockid, input_.end(), input_.space());
        if (rd == -1 && errno == EINTR) {
            continue;
        }
        else if (rd == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            if (read_ && input_.size()) {
                // check codec here, whether completed or not.
                int consumeSize = 0;
                // this is decode function.
                ECodecResult res = read_(consumeSize, input_, aOutMsg);
                if (res == ECodecResult::kCodecCompleted) {
                    // don not delete readevent for good perf.
                    input_.consume(consumeSize);
                    return 1;
                }
                else if (res == ECodecResult::kCodecError) {
                    input_.clear();
                    return -1;
                }
            }
            break;
        }
        else if (rd <= 0) {
            return -1;
        }
        else { //rd > 0
            input_.addSize(rd);
            input_.setEnd();
        }
    }
    return 0;
}

int BufferEvent::Response(int aSockid) {
    if (outputs_.find(aSockid) == outputs_.end()) {
        outputs_.insert(std::make_pair(aSockid,
            std::move(std::unique_ptr<Buffer>(new Buffer()))));
    }
    auto it = outputs_.find(aSockid);
    size_t sended = 0, len = it->second->size();
    while (len > sended) {
        int wd = ::write(aSockid, it->second->data() + sended, len - sended);
        LOG_DEBUG("fd %d write %ld bytes", aSockid, wd);
        if (wd > 0) {
            sended += wd;
            continue;
        }
        else if (wd == -1 && errno == EINTR) {
            continue;
        }
        else if (wd == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            break;
        }
        else {
            LOG_DEBUG("write error: wd %ld %d %s", wd, errno, strerror(errno));
            return -1;
        }
    }
    it->second->consume(sended);
    return sended;
}

Buffer& BufferEvent::getRespBuffer(int aSockid) {
    if (outputs_.find(aSockid) == outputs_.end()) {
        outputs_.insert(std::make_pair(aSockid,
            std::move(std::unique_ptr<Buffer>(new Buffer()))));
    }
    return *(outputs_[aSockid].get());
}

Buffer& BufferEvent::getRequBuffer(int aSockid) {
    if (inputs_.find(aSockid) == inputs_.end()) {
        inputs_.insert(std::make_pair(aSockid,
            std::move(std::unique_ptr<Buffer>(new Buffer()))));
    }
    return *(inputs_[aSockid].get());
}

void BufferEvent::setRespBuffer(int aSockid, Slice aMsg) {
    if (outputs_.find(aSockid) == outputs_.end()) {
        outputs_.insert(std::make_pair(aSockid,
            std::move(std::unique_ptr<Buffer>(new Buffer()))));
    }
    if (write_) {
        int nouse = 0;
        // this is encode function.
        write_(nouse, *(outputs_[aSockid].get()), aMsg);
    }
    else {
        outputs_[aSockid]->append(aMsg.data(), aMsg.size());
    }
}

void BufferEvent::setCodec(CodecBase* aCodec) {
    codec_.reset(aCodec);
    setDecodeCallback();
    setEncodeCallback();
}

void BufferEvent::setDecodeCallback() {
    setFinishCallback(ECallbackOper::kSetRead,
        [&](int& aLen, Buffer& aBuf, Slice& aMsg) {
        int r = 1;
        while (r) {
            r = codec_->tryDecode(aBuf, aMsg);
            if (r < 0) {
                return ECodecResult::kCodecError;
            }
            else if (r > 0) {
                LOG_DEBUG("a msg decoded. origin len %d msg len %ld",
                    r, aMsg.size());
                aLen = r;
                return ECodecResult::kCodecCompleted;
            }
            else {
                return ECodecResult::kCodecContining;
            }
        }
    });
}

void BufferEvent::setEncodeCallback() {
    setFinishCallback(ECallbackOper::kSetWrite,
        [&](int& aLen, Buffer& aBuf, Slice& aMsg) {
        codec_->encode(aMsg, aBuf);
        return ECodecResult::kCodecCompleted;
    });
}