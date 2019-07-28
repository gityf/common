//
// Created by wyf on 2019/7/28.
//

#ifndef __COMMON_GEOUTILS_H__
#define __COMMON_GEOUTILS_H__

#include <vector>
#if defined(__cplusplus)
extern "C" {
#endif

double GeoDistance(double lon1d, double lat1d, double lon2d, double lat2d);
int GeoBoundingBox(double longitude, double latitude, double radius_meters, std::vector<double> &toBBoxs);

#if defined(__cplusplus)
}
#endif

#endif //__COMMON_GEOUTILS_H__
