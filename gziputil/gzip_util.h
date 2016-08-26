#pragma once
#include <string>

namespace common {
class GZipUtil {
    public:
       //return 0:success -1:failed
       static int GZip(const std::string& str, std::string& out);
       //return 0:success -1:failed
       static int GUnZip(const std::string& str, std::string& out);
  
};
}
