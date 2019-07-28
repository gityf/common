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

const double _GRID_RADIX_ = 3E3;
const double _MAX_dR_ = 2E-5;
const double _MAX_dT_ = 3E-6;
const double _LL2RAD_ = 0.0174532925194;
const double _OFFSET_X_ = 0.0065;
const double _OFFSET_Y_ = 0.0060;
const double pi = 3.14159265358979324;
const double a = 6378245.0;
const double ee = 0.00669342162296594323;

static double get_delta_r(double y0) {
    return sin(y0 * _GRID_RADIX_ * _LL2RAD_) *_MAX_dR_;
}

static double get_delta_t(double x0) {
    return cos(x0 * _GRID_RADIX_ * _LL2RAD_) * _MAX_dT_;
}

static double transformLat(double lng, double lat) {
    double ret = -100.0 + 2.0 * lng + 3.0 * lat + 0.2 * lat * lat + 0.1 * lng * lat + 0.2 * sqrt(lng > 0 ? lng:-lng);
    ret += (20.0 * sin(6.0 * lng * pi) + 20.0 *sin(2.0 * lng * pi)) * 2.0 / 3.0;
    ret += (20.0 * sin(lat * pi) + 40.0 * sin(lat / 3.0 * pi)) * 2.0 / 3.0;
    ret += (160.0 * sin(lat / 12.0 * pi) + 320 * sin(lat * pi / 30.0)) * 2.0 / 3.0;
    return ret;
}

static double transformLon(double lng, double lat) {
    double ret = 300.0 + lng + 2.0 * lat + 0.1 * lng * lng + 0.1 * lng * lat + 0.1 * sqrt(lng > 0 ? lng:-lng);
    ret += (20.0 * sin(6.0 * lng * pi) + 20.0 * sin(2.0 * lng * pi)) * 2.0 / 3.0;
    ret += (20.0 * sin(lng * pi) + 40.0 * sin(lng / 3.0 * pi)) * 2.0 / 3.0;
    ret += (150.0 * sin(lng / 12.0 * pi) + 300.0 * sin(lng / 30.0 * pi)) * 2.0 / 3.0;
    return ret;
}

//GCJ02->BD09ll
static void gcj2bd(Coordinate &coordinate) {
    double y0 = coordinate.lat;
    double x0 = coordinate.lng;
    double r0 = sqrt(x0*x0 + y0*y0);
    double theta0 = atan2(y0, x0);
    double r1 = r0 + get_delta_r(y0);
    double theta1 = theta0 + get_delta_t(x0);
    double x1 = r1 * cos(theta1);
    double y1 = r1 * sin(theta1);
    coordinate.lng = x1 + _OFFSET_X_;
    coordinate.lat = y1 + _OFFSET_Y_;
}

//BD09ll->GCJ02
static void bd2gcj(Coordinate &coordinate) {
    double x0 = coordinate.lng - _OFFSET_X_;
    double y0 = coordinate.lat - _OFFSET_Y_;
    double r0 = sqrt(x0*x0 + y0*y0);
    double theta0 = atan2(y0, x0);
    double r1 = r0 - get_delta_r(y0);
    double theta1 = theta0 - get_delta_t(x0);
    double x1 = r1 * cos(theta1);
    double y1 = r1 * sin(theta1);
    coordinate.lng = x1;
    coordinate.lat = y1;
}

//WGS84->GCJ02
static void wgs2gcj(Coordinate &coordinate) {
    double dLat = transformLat(coordinate.lng - 105.0, coordinate.lat - 35.0);
    double dLon = transformLon(coordinate.lng - 105.0, coordinate.lat - 35.0);
    double radLat = coordinate.lat / 180.0 * pi;
    double magic = sin(radLat);
    magic = 1 - ee * magic * magic;
    double sqrtMagic = sqrt(magic);
    dLat = (dLat * 180.0) / ((a * (1 - ee)) / (magic * sqrtMagic) * pi);
    dLon = (dLon * 180.0) / (a / sqrtMagic * cos(radLat) * pi);

    coordinate.lat += dLat;
    coordinate.lng += dLon;
}

//GCJ02->WGS84
static void gcj2wgs(Coordinate &coordinate) {
    Coordinate wgLoc;
    wgLoc.lat = coordinate.lat;
    wgLoc.lng = coordinate.lng;

    Coordinate dLoc;
    dLoc.lat = 0;
    dLoc.lng = 0;

    Coordinate currGcLoc;
    while (1) {
        currGcLoc.lat = wgLoc.lat;
        currGcLoc.lng = wgLoc.lng;
        wgs2gcj(currGcLoc);
        dLoc.lat = coordinate.lat - currGcLoc.lat;
        dLoc.lng = coordinate.lng - currGcLoc.lng;

        if (fabs(dLoc.lng) < 1e-7 && fabs(dLoc.lat) < 1e-7) {
            break;
        }
        wgLoc.lat += dLoc.lat;
        wgLoc.lng += dLoc.lng;
    }
    coordinate.lat = wgLoc.lat;
    coordinate.lng = wgLoc.lng;
}

bool CoordLatLngTrans(ECoordType fromType, ECoordType toType, double lat, double lng, double &tolat, double &tolng) {
    if (fromType == toType) {
        return false;
    }
    if (lng < 72.004 || lng > 137.8347) {
        return 0;
    }
    if (lat < 0.8293 || lat > 55.8271) {
        return 0;
    }
    Coordinate coordinate;
    coordinate.lat = lat;
    coordinate.lng = lng;

    if (kCoordTypeWGS84 == fromType) {
        //WGS84->GCJ02
        wgs2gcj(coordinate);
    } else if (kCoordTypeBD09ll == fromType) {
        //BD09ll->GCJ02
        bd2gcj(coordinate);
    }

    if (kCoordTypeWGS84 == toType) {
        //GCJ02->WGS84
        gcj2wgs(coordinate);
    } else if (kCoordTypeBD09ll == toType) {
        //GCJ02->BD09ll
        gcj2bd(coordinate);
    }
    tolat = coordinate.lat;
    tolng = coordinate.lng;
    return true;
}

bool CoordStructTrans(ECoordType fromType, ECoordType toType, Coordinate fromCoord, Coordinate &toCoord) {
    return CoordLatLngTrans(fromType, toType, fromCoord.lat, fromCoord.lng, toCoord.lat, toCoord.lng);
}