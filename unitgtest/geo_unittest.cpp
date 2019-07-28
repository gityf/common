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

    Coordinate fromCoord;
    fromCoord.lat = 39.865473;
    fromCoord.lng = 116.378978;

    Coordinate toCoord;
    CoordStructTrans(kCoordTypeWGS84, kCoordTypeGCJ02, fromCoord, toCoord);
    cout << "CoordTrans.from.lat:" << fromCoord.lat << ",lng:" << fromCoord.lng << endl;
    cout << "CoordTrans.to.lat:" << toCoord.lat << ",lng:" << toCoord.lng << endl;

    Coordinate toRawCoord;
    CoordStructTrans(kCoordTypeGCJ02, kCoordTypeWGS84, toCoord, toRawCoord);
    cout << "CoordTrans.from.lat:" << toCoord.lat << ",lng:" << toCoord.lng << endl;
    cout << "CoordTrans.to.lat:" << toRawCoord.lat << ",lng:" << toRawCoord.lng << endl;

    EXPECT_DOUBLE_EQ(fromCoord.lng, toRawCoord.lng);
    EXPECT_DOUBLE_EQ(fromCoord.lat, toRawCoord.lat);
}