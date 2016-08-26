#pragma  once
#include <map>
#include <string>
#include <stdio.h>
#include <string.h>
#include "net/buffer.h"

using std::string;

class HttpResponse {
public:
    enum HttpStatusCode {
        kUnknown,
        k200Ok = 200,
        k301MovedPermanently = 301,
        k400BadRequest = 400,
        k404NotFound = 404,
    };

    explicit HttpResponse(bool close)
        : statusCode_(kUnknown), closeConnection_(close) {}

    void setStatusCode(HttpStatusCode code) {
        statusCode_ = code;
    }

    void setStatusMessage(const string& message) {
        statusMessage_ = message;
    }

    void setCloseConnection(bool on) {
        closeConnection_ = on;
    }

    bool closeConnection() const {
        return closeConnection_;
    }

    void setContentType(const string& contentType) {
        addHeader("Content-Type", contentType);
    }

    // FIXME: replace string with StringPiece
    void addHeader(const string& key, const string& value) {
        headers_[key] = value;
    }

    void setBody(const string& body) {
        body_ = body;
    }

    void appendToBuffer(Buffer& output) const {
        char buf[32];
        sprintf(buf, "HTTP/1.1 %d ", statusCode_);
        output.append(buf).append(statusMessage_).append("\r\n");

        if (closeConnection_) {
            output.append("Connection: close\r\n");
        }
        else {
            output.append("Connection: Keep-Alive\r\n");
        }
        sprintf(buf, "Content-Length: %zd\r\n", body_.size());
        output.append(buf);

        for (auto it = headers_.begin(); it != headers_.end(); ++it) {
            output.append(it->first).append(": ").append(it->second).append("\r\n");
        }

        output.append("\r\n").append(body_);
    }

private:
    std::map<string, string> headers_;
    HttpStatusCode statusCode_;
    string statusMessage_;
    bool closeConnection_;
    string body_;
};

