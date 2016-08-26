#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <boost/algorithm/string.hpp>

namespace ecom { namespace ad { namespace webframework {
class AuthHelper {
public:
    explicit AuthHelper(const std::string& aksk_file):
        aksk_file_(aksk_file) {}
    virtual ~AuthHelper(){}

    virtual bool initHelper() {
        std::ifstream akskfile(aksk_file_);
        std::string line;

        if (akskfile.is_open()) {
            while (std::getline(akskfile,line)) {
                std::vector<std::string> aksk;
                boost::split(aksk, line, boost::is_any_of(" \t:"), boost::token_compress_on);
                if (aksk.size() == 2) {
                    aksk_pairs_[aksk[0]] = aksk[1];
                }
            }
        }
    }
    virtual std::string getSk(const std::string& ak) noexcept {
        auto ski = aksk_pairs_.find(ak);
        if (ski == aksk_pairs_.end()) {
            return ""; 
        } else {
            return ski->second;
        }
    }
    virtual size_t getSkCount() {
        return aksk_pairs_.size();
    }

private:
    std::unordered_map<std::string, std::string> aksk_pairs_;
    std::string aksk_file_;
};    
}}}
