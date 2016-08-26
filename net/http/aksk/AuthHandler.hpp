#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>

#include <folly/Memory.h>
#include <folly/String.h>
#include <proxygen/httpserver/RequestHandler.h>
#include <boost/algorithm/string.hpp>

#include "AuthHelper.hpp"

namespace ecom { namespace ad { namespace webframework {
class AuthHandler {
public:
    enum class AuthStatus {
        OK,
        INVALID,
        EXPIRED,
        UNAUTH
    };
    struct Auth{
        std::string version;
        std::string accesskey;
        std::string timestamp;
        std::vector<std::string> headers;
        std::string signature; 
        bool parseAuth(const std::string& auth);
    };
    class Signature{
        public:
            void set_method(const std::string& method) {
                method_ = method;
            }
            void set_uri(const std::string& uri) {
                uri_ = uri;
            }
            void add_qs(const std::string& key, const std::string& value) {
                qs_[key.c_str()] =  value.c_str();
            }
            void add_headers(const std::string& key, const std::string& value) {
                headers_[key] = boost::trim_copy(value, default_locale_);
            }
            void set_timestamp(const std::string& timestamp) {
                timestamp_ = timestamp;
            }

            std::string signRequest(const std::string& ak, const std::string& sk);
        private:
            std::locale default_locale_;
            std::string method_;
            std::string uri_;
            std::map<std::string, std::string> qs_;
            std::map<std::string, std::string> headers_;
            std::string timestamp_;
    };
    class HmacUpdater {
        public:
            explicit HmacUpdater() {
                HMAC_CTX_init(&hmac_);
            }
            ~HmacUpdater() {
                HMAC_CTX_cleanup(&hmac_);
            }
            void init(const std::string& key) {
                HMAC_Init_ex(&hmac_, &key[0], key.length(), EVP_sha256(), NULL);
            }
            void update(const char* data, size_t size) {
                HMAC_Update(&hmac_, (const unsigned char*)data, size);
            }
            std::string value() {
                unsigned int len = SHA256_DIGEST_LENGTH;
                unsigned char hash[len];
                HMAC_Final(&hmac_, hash, &len);
                std::stringstream ss;
                ss << std::hex << std::setfill('0');
                for (size_t i = 0; i < len; ++i) {   
                    ss << std::hex << std::setw(2)  << (unsigned int)hash[i];
                }

                return (ss.str());
            }

        private:
            HMAC_CTX hmac_;
    };
    class Sha256Hash {
        public:
            explicit Sha256Hash() {
                SHA256_Init(&sha256_);
            }
            void update(const char* data, size_t size) {
                SHA256_Update(&sha256_, data, size);
            }

            std::string value() {
                unsigned char hash[SHA256_DIGEST_LENGTH];
                SHA256_Final(hash, &sha256_);

                std::stringstream ss;
                ss << std::hex << std::setfill('0');
                for (size_t i = 0; i < SHA256_DIGEST_LENGTH; ++i) {   
                    ss << std::hex << std::setw(2)  << (unsigned int)hash[i];
                }
                return (ss.str());
            }

        private:
            SHA256_CTX sha256_;
    };

    explicit AuthHandler(AuthHelper* helper) : helper_(helper) {}
    virtual ~AuthHandler() {}

    virtual std::string signRequest(const proxygen::HTTPMessage* headers, const folly::IOBuf* body, 
            const std::string& accessky, const std::string& secretkey) noexcept;
    virtual AuthStatus authRequest(const proxygen::HTTPMessage* headers, const folly::IOBuf* body) noexcept;

    std::string digestContent(const folly::IOBuf* body) noexcept;

private:
    AuthHelper* helper_;
};

}}}
