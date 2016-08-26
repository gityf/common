/*
** Copyright (C) 2014 Wang Yaofu
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: The header file of class PeerFilter.
*/

#ifndef _COMMON_CPEERFILTER_H_
#define _COMMON_CPEERFILTER_H_
#include <algorithm>
#include <functional>
#include <map>
#include <vector>
#include <string>
using std::string;
enum EFilterOption {
    kPeerAddrAllow = 0,
    kPeerAddrDeny
} ;
// ---------------------------------------------------------------------------
// class PeerFilter
//
// this class is a peer host filter class.
//
// ---------------------------------------------------------------------------
//
class PeerFilter {
 public:
    /*
    * peer ip is checked.
    * allow return 0, deny return 1.
    */
    EFilterOption PeerAddrCheck(const string& aPeerAddr) {
        if (mPeerMap.find(aPeerAddr) != mPeerMap.end()) {
            return mPeerMap[aPeerAddr];
        } else {
            return kPeerAddrDeny;
        }
    }

    /*
    * set key-value into map_str_int.
    * aValue = 0 allow, aValue = 1 deny.
    */
    void SetPeerAddr(const string& aKey, EFilterOption aValue) {
        mPeerMap[aKey] = aValue;
    }

    // clear peer map
    void ClearPeerAddrs() {
        mPeerMap.clear();
    }

    // erase peer address from map
    void EraseByAddrs(const string& aKey) {
        mPeerMap.erase(aKey);
    }

    // clear deny or allow peer address from map
    void EraseByValue(EFilterOption aValue) {
        std::vector<string> eraseArr;
        for (auto it = mPeerMap.begin(); it != mPeerMap.end(); ++it) {
            if (it->second == aValue) {
                eraseArr.push_back(it->first);
            }
        }
        for (size_t ii = 0; ii < eraseArr.size(); ++ii) {
            mPeerMap.erase(eraseArr[ii]);
        }
    }

    /*
    * constructor.
    *
    */
    explicit PeerFilter() {
        mPeerMap.clear();
    }

    /*
    * Destructor.
    *
    */
    virtual ~PeerFilter() {
        mPeerMap.clear();
    }

 private:
    /*
    * configuration list for peer addr checked string key, int value.
    *
    */
    std::map<string, EFilterOption > mPeerMap;
};
#endif  // _COMMON_CPEERFILTER_H_
