//
// Created by wyf on 2019/7/28.
//

#include "geoutils.h"
#include <math.h>
#include <vector>

#define D_R (M_PI / 180.0)
static inline double deg_rad(double ang) { return ang * D_R; }
static inline double rad_deg(double ang) { return ang / D_R; }
/// @brief Earth's quatratic mean radius for WGS-84
const double EARTH_RADIUS_IN_METERS = 6372797.560856;

double GeoDistance(double lon1d, double lat1d, double lon2d, double lat2d) {
    double lat1r, lon1r, lat2r, lon2r, u, v;
    lat1r = deg_rad(lat1d);
    lon1r = deg_rad(lon1d);
    lat2r = deg_rad(lat2d);
    lon2r = deg_rad(lon2d);
    u = sin((lat2r - lat1r) / 2);
    v = sin((lon2r - lon1r) / 2);
    return 2.0 * EARTH_RADIUS_IN_METERS *
           asin(sqrt(u * u + cos(lat1r) * cos(lat2r) * v * v));
}

/* Return the bounding box of the search area centered at latitude,longitude
 * having a radius of radius_meter. bounds[0] - bounds[2] is the minimum
 * and maxium longitude, while bounds[1] - bounds[3] is the minimum and
 * maximum latitude.
 *
 * This function does not behave correctly with very large radius values, for
 * instance for the coordinates 81.634948934258375 30.561509253718668 and a
 * radius of 7083 kilometers, it reports as bounding boxes:
 *
 * min_lon 7.680495, min_lat -33.119473, max_lon 155.589402, max_lat 94.242491
 *
 * However, for instance, a min_lon of 7.680495 is not correct, because the
 * point -1.27579540014266968 61.33421815228281559 is at less than 7000
 * kilometers away.
 *
 * Since this function is currently only used as an optimization, the
 * optimization is not used for very big radiuses, however the function
 * should be fixed. */
int GeoBoundingBox(double longitude, double latitude, double radius_meters,
                       std::vector<double> &toBBoxs) {
    double val = longitude - rad_deg(radius_meters/EARTH_RADIUS_IN_METERS/cos(deg_rad(latitude)));
    toBBoxs.push_back(val);
    val = latitude - rad_deg(radius_meters/EARTH_RADIUS_IN_METERS);
    toBBoxs.push_back(val);
    val = longitude + rad_deg(radius_meters/EARTH_RADIUS_IN_METERS/cos(deg_rad(latitude)));
    toBBoxs.push_back(val);
    val = latitude + rad_deg(radius_meters/EARTH_RADIUS_IN_METERS);
    toBBoxs.push_back(val);
    return 1;
}