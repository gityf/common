/*
** Copyright (C) 2019 Wang Yaofu
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: The header file of GeoHash.
*/

#ifndef __COMMON_GEOHASH_H__
#define __COMMON_GEOHASH_H__

#include <cstdint>
#include "geoutils.h"

class GeoHash {
public:
    GeoHash() {}
    ~GeoHash() {}

    // encode from Coordinate lat/lng to geohashbits
    bool geoHashEncode(const Coordinate &coordinate, GeoHashBits &toHash);
    bool encode(double lat, double lng, GeoHashBits &toHash);

    // decode from GeoHashBits to Coordinate
    bool geoHashDecode(const GeoHashBits &hash, Coordinate &to);
    bool decode(const GeoHashBits &hash, double &lat, double &lng);
};


#endif //__COMMON_GEOHASH_H__
