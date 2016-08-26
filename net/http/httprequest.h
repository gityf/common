#pragma  once
#include <assert.h>
#include <stdio.h>
#include <map>
#include <string>

using std::string;

class HttpRequest {
public:
    enum Method {
        kInvalid, kGet, kPost, kHead, kPut, kDelete
    };
    enum Version {
        kUnknown, kHttp10, kHttp11
    };

    HttpRequest()
        : method_(kInvalid),
        version_(kUnknown)
    {
    }

    void setVersion(Version v) {
        version_ = v;
    }

    Version getVersion() const {
        return version_;
    }

    bool setMethod(const char* start, const char* end) {
        assert(method_ == kInvalid);
        string m(start, end);
        if (m == "GET") {
            method_ = kGet;
        }
        else if (m == "POST") {
            method_ = kPost;
        }
        else if (m == "HEAD") {
            method_ = kHead;
        }
        else if (m == "PUT") {
            method_ = kPut;
        }
        else if (m == "DELETE") {
            method_ = kDelete;
        }
        else {
            method_ = kInvalid;
        }
        return method_ != kInvalid;
    }

    Method method() const {
        return method_;
    }

    const char* methodString() const {
        const char* result = "UNKNOWN";
        switch (method_)
        {
        case kGet:
            result = "GET";
            break;
        case kPost:
            result = "POST";
            break;
        case kHead:
            result = "HEAD";
            break;
        case kPut:
            result = "PUT";
            break;
        case kDelete:
            result = "DELETE";
            break;
        default:
            break;
        }
        return result;
    }

    void setPath(const char* start, const char* end) {
        path_.assign(start, end);
    }

    const string& path() const {
        return path_;
    }

    void setQuery(const char* start, const char* end) {
        query_.assign(start, end);
    }

    const string& query() const {
        return query_;
    }

    void addParam(const string& field, const string& value) {
        queryParams_[field] = value;
    }

    std::map<string, string>& mutable_params() {
        return queryParams_;
    }

    void addHeader(const string& field, const string& value) {
        headers_[field] = value;
    }

    std::map<string, string>& mutable_headers() {
        return headers_;
    }

    string getHeader(const string& field) const {
        auto it = headers_.find(field);
        if (it != headers_.end()) {
            return it->second;
        }
        return string("");
    }

    const std::map<string, string>& headers() const {
        return headers_;
    }

    void setBody(const char* start, const char* end) {
        if (start < end) {
            body_.assign(start, end);
        }
    }

    const string& body() const {
        return body_;
    }

    void setBodyLength(int length) {
        bodyLength_ = length;
    }

    int bodyLength() const {
        return bodyLength_;
    }

    void swap(HttpRequest& that) {
        std::swap(method_, that.method_);
        path_.swap(that.path_);
        query_.swap(that.query_);
        headers_.swap(that.headers_);
    }

private:
    Method method_;
    Version version_;
    string path_;
    string query_;
    string body_;
    int bodyLength_;
    std::map<string, string> headers_;
    std::map<string, string> queryParams_;
};

