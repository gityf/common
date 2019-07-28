//
// Created by wyf on 2019/7/28.
//

#include "ut/test_harness.h"
#include "../geo/GeoHash.h"
#include <iostream>

using namespace std;

TEST(GeoHash_test, testHash) {
    GeoHash geoHash;
    GeoHashBits geoHashBits;
    geoHash.encode(39.865473, 116.378978,geoHashBits);

    cout << geoHashBits.bits << endl;
    double lat = 0.0, lng = 0.0;
    geoHash.decode(geoHashBits, lat, lng);

    cout << "lat:" << lat << ",lng:" << lng << endl;
}