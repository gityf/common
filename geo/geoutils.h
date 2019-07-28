//
// Created by wyf on 2019/7/28.
//

#ifndef __COMMON_GEOUTILS_H__
#define __COMMON_GEOUTILS_H__

#include <vector>
#if defined(__cplusplus)
extern "C" {
#endif

struct Coordinate {
    double lat;
    double lng;
};

struct GeoHashBits {
    uint64_t bits;
    uint8_t  step;
};

struct GeoHashRange {
    double min;
    double max;
};

double GeoDistance(double lon1d, double lat1d, double lon2d, double lat2d);
int GeoBoundingBox(double longitude, double latitude, double radius_meters, std::vector<double> &toBBoxs);

enum ECoordType {
    kCoordTypeWGS84,
    kCoordTypeGCJ02,
    kCoordTypeBD09ll
};

// Longitude and Latitude 经度 纬度

bool CoordLatLngTrans(ECoordType fromType, ECoordType toType,
               double lat, double lng, double &tolat, double &tolng);

bool CoordStructTrans(ECoordType fromType, ECoordType toType, Coordinate fromCoord, Coordinate &toCoord);

#if defined(__cplusplus)
}
#endif

#endif //__COMMON_GEOUTILS_H__
