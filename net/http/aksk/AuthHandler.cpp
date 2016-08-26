#include "AuthHandler.hpp"

#include <boost/algorithm/string.hpp>
#include <folly/Conv.h>
#include <ctime>
#include <openssl/hmac.h>
#include <stdlib.h>

#include "log.hpp"

namespace ecom { namespace ad { namespace webframework {
using namespace std;

// a valid auth header pattern is 
// like: auth-version/accesskey/timestamp/headers;headers/signature
static const int VALID_SIGN_TIME       = 86400;
static const int VALID_AUTH_FIELDS_NUM = 5;
static const string AUTH_VERSION       = "kuaidi-auth-v1";
static const string FIELD_SEP          = "/";
static const string HKEY_SEP           = ";";
static const string LINE_SEP           = "\n";
static const string QSKV_SEP           = "=";
static const string QS_SEP             = "&";
static const string HEADERKV_SEP       = ":";


bool AuthHandler::Auth::parseAuth(const std::string& auth) {
    vector<string> tokens;
    boost::split(tokens, auth, boost::is_any_of("/"), boost::token_compress_on);
    
    if (tokens.size() != VALID_AUTH_FIELDS_NUM) {
        // parse failed
        return false;
    }
    version    = tokens[0];
    accesskey  = tokens[1];
    timestamp  = tokens[2];
    
    int64_t timestamp_value = atoll(timestamp.c_str());
    time_t now = time(nullptr);
    if (timestamp_value > now + VALID_SIGN_TIME || timestamp_value < now - VALID_SIGN_TIME) {
        return false;
    }

    boost::split(headers, tokens[3], boost::is_any_of(";"), boost::token_compress_on);
    signature  = tokens[4]; 


    return true;
}
std::string AuthHandler::Signature::signRequest(const std::string& ak, const std::string& sk) {
    // gen signkey
    // hex(hmac_sha256(sk, "kuaidi-auth-v1/ak/140000000/host;x-content-sha256"))
    string header_combine;
    for (auto& h : headers_) {
        if (!header_combine.empty()) {
            header_combine += HKEY_SEP;
        }
        header_combine += h.first;
    }
    string datakey = AUTH_VERSION + FIELD_SEP 
        + ak + FIELD_SEP 
        + timestamp_+ FIELD_SEP 
        + header_combine;

    HmacUpdater hmac;

    // calc signkey
    hmac.init(sk);
    hmac.update(datakey.c_str(), datakey.size());
    string signkey = hmac.value();

    // get str2sign
    string str2sign;
    str2sign.reserve(1024);
    // 1. append HTTP METHOD
    str2sign += method_ + LINE_SEP;
    // 2. append uri
    str2sign += uri_    + LINE_SEP;
    // 3. append query strings
    for (auto& qs: qs_) {
        str2sign += qs.first + QSKV_SEP + qs.second + LINE_SEP;
        LOG_INFO << endl << str2sign << endl;
    }
    // 4. append headers 
    for (auto& h: headers_) {
        str2sign += h.first + HEADERKV_SEP + h.second + LINE_SEP;
    }

    LOG_INFO << str2sign;

    // reuse
    hmac.init(signkey);
    hmac.update(str2sign.c_str(), str2sign.size());
    return hmac.value();
}

std::string AuthHandler::digestContent(const folly::IOBuf* body) noexcept {
    Sha256Hash sha256;
    if (body && body->computeChainDataLength() > 0) {
        sha256.update((const char*)body->data(), body->length());
        for (const folly::IOBuf* current = body->next(); current != body; current = current->next()) {
            sha256.update((const char*)current->data(), current->length());
        }
    } else {
        sha256.update("", 0);
    }
    return sha256.value();
}

std::string AuthHandler::signRequest(const proxygen::HTTPMessage* msg, const folly::IOBuf* body,
        const std::string& accesskey, const std::string& secretkey) noexcept {
    // calc sign
    Signature signer;

    // 1. set METHOD
    signer.set_method(msg->getMethodString());

    // 2. set URI PATH
    signer.set_uri(msg->getPath());

    // 3. set query string
    for (auto& i: msg->getQueryParams()) {
        signer.add_qs(i.first, i.second);
    }

    // 4. set http headers
    // set host
    string host = msg->getHeaders().rawGet("host");
    //signer.add_headers("host", host);
    // set content-digest
    signer.add_headers("x-kuaidi-content-digest", digestContent(body));

    // set request time
    time_t now = time(nullptr);
    string now_str = folly::to<string>(now);
    signer.set_timestamp(now_str);

    // calc signature
    return AUTH_VERSION + FIELD_SEP 
        + accesskey + FIELD_SEP 
        + now_str + FIELD_SEP 
        //+ "host;x-kuaidi-content-digest" + FIELD_SEP 
        + signer.signRequest(accesskey, secretkey);
}

AuthHandler::AuthStatus AuthHandler::authRequest(const proxygen::HTTPMessage* msg, const folly::IOBuf* body) noexcept {
    if (!helper_) {
        // if dont init the auth helper, just pass all the authentication
        return AuthStatus::OK;
    }

    // get auth header
    string auth_string = msg->getHeaders().rawGet("Authorization");
    if (auth_string.empty()) {
        return AuthStatus::INVALID;
    }
    // parse auth header
    Auth auth;
    bool ret = auth.parseAuth(auth_string);
    if (!ret) {
        LOG_INFO << "parse authorization header failed:" << auth_string;
        return AuthStatus::INVALID;
    }

    // calc sign
    Signature signer;

    for (auto& h: auth.headers) {
        if (h == "x-kuaidi-content-digest") {
            signer.add_headers("x-kuaidi-content-digest", digestContent(body));
            continue;
        }
        string v = msg->getHeaders().rawGet(h);
        if (v.empty()) {
            return AuthStatus::INVALID;
        } else {
            signer.add_headers(h, v);
        }
    }
    signer.set_method(msg->getMethodString());
    signer.set_uri(msg->getPath());
    for (auto& i: msg->getQueryParams()) {
        signer.add_qs(i.first, i.second);
    }
    signer.set_timestamp(auth.timestamp);

    signer.add_headers("x-kuaidi-content-digest", digestContent(body));

    // get sk from ak
    string sk = helper_->getSk(auth.accesskey);
    if (sk.empty()) {
        LOG_INFO << "not found sk for " << auth.accesskey;
        return AuthStatus::UNAUTH;
    }
    // check sign
    string signature = signer.signRequest(auth.accesskey, sk);
    if (signature != auth.signature) {
        return AuthStatus::UNAUTH;
    }

    return AuthStatus::OK;
}


}}}
