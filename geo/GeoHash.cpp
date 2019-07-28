/*
** Copyright (C) 2019 Wang Yaofu
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: The source file of GeoHash.
*/
#include "GeoHash.h"

//  26*2 = 52 bits
const static int kGeoStepMax = 26;

// Limits from EPSG:900913 / EPSG:3785 / OSGEO:41001
const static double kGeoLatMin = -85.05112878;
const static double kGeoLatMax = 85.05112878;
const static double kGeoLngMin = -180;
const static double kGeoLngMax = 180;


/**
 * Hashing works like this:
 * Divide the world into 4 buckets.  Label each one as such:
 *  -----------------
 *  |       |       |
 *  |       |       |
 *  | 0,1   | 1,1   |
 *  -----------------
 *  |       |       |
 *  |       |       |
 *  | 0,0   | 1,0   |
 *  -----------------
 */

/* Interleave lower bits of x and y, so the bits of x
 * are in the even positions and bits from y in the odd;
 * x and y must initially be less than 2**32 (65536).
 * From:  https://graphics.stanford.edu/~seander/bithacks.html#InterleaveBMN
 */
static inline uint64_t interleave64(uint32_t xlo, uint32_t ylo) {
    static const uint64_t B[] = {0x5555555555555555ULL, 0x3333333333333333ULL,
                                 0x0F0F0F0F0F0F0F0FULL, 0x00FF00FF00FF00FFULL,
                                 0x0000FFFF0000FFFFULL};
    static const unsigned int S[] = {1, 2, 4, 8, 16};

    uint64_t x = xlo;
    uint64_t y = ylo;

    x = (x | (x << S[4])) & B[4];
    y = (y | (y << S[4])) & B[4];

    x = (x | (x << S[3])) & B[3];
    y = (y | (y << S[3])) & B[3];

    x = (x | (x << S[2])) & B[2];
    y = (y | (y << S[2])) & B[2];

    x = (x | (x << S[1])) & B[1];
    y = (y | (y << S[1])) & B[1];

    x = (x | (x << S[0])) & B[0];
    y = (y | (y << S[0])) & B[0];

    return x | (y << 1);
}

/* reverse the interleave process
 * derived from http://stackoverflow.com/questions/4909263
 */
static inline uint64_t deinterleave64(uint64_t interleaved) {
    static const uint64_t B[] = {0x5555555555555555ULL, 0x3333333333333333ULL,
                                 0x0F0F0F0F0F0F0F0FULL, 0x00FF00FF00FF00FFULL,
                                 0x0000FFFF0000FFFFULL, 0x00000000FFFFFFFFULL};
    static const unsigned int S[] = {0, 1, 2, 4, 8, 16};

    uint64_t x = interleaved;
    uint64_t y = interleaved >> 1;

    x = (x | (x >> S[0])) & B[0];
    y = (y | (y >> S[0])) & B[0];

    x = (x | (x >> S[1])) & B[1];
    y = (y | (y >> S[1])) & B[1];

    x = (x | (x >> S[2])) & B[2];
    y = (y | (y >> S[2])) & B[2];

    x = (x | (x >> S[3])) & B[3];
    y = (y | (y >> S[3])) & B[3];

    x = (x | (x >> S[4])) & B[4];
    y = (y | (y >> S[4])) & B[4];

    x = (x | (x >> S[5])) & B[5];
    y = (y | (y >> S[5])) & B[5];

    return x | (y << 32);
}


bool GeoHash::geoHashEncode(const Coordinate &coordinate, GeoHashBits &toHash) {
    return encode(coordinate.lat, coordinate.lng, toHash);
}

bool GeoHash::encode(double lat, double lng, GeoHashBits &toHash) {
    /* Return an error when trying to index outside the supported
     * constraints. */
    int step = kGeoStepMax;
    if (lng > kGeoLngMax || lng < kGeoLngMin ||
        lat > kGeoLatMax || lat < kGeoLatMin) {
        return false;
    }

    toHash.bits = 0;
    toHash.step = step;

    double lat_offset = (lat - kGeoLatMin) / (kGeoLatMax - kGeoLatMin);
    double long_offset = (lng - kGeoLngMin) / (kGeoLngMax - kGeoLngMin);

    /* convert to fixed point based on the step size */
    lat_offset *= (1ULL << step);
    long_offset *= (1ULL << step);
    toHash.bits = interleave64(lat_offset, long_offset);
    return true;
}

bool GeoHash::geoHashDecode(const GeoHashBits &hash, Coordinate &to) {
    return decode(hash, to.lat, to.lng);
}

bool GeoHash::decode(const GeoHashBits &hash, double &lat, double &lng) {
    GeoHashRange lat_range;
    GeoHashRange lng_range;
    uint8_t step = hash.step;
    uint64_t hash_sep = deinterleave64(hash.bits); /* hash = [LAT][LONG] */

    double lat_scale = kGeoLatMax - kGeoLatMin;
    double long_scale = kGeoLngMax - kGeoLngMin;

    uint32_t ilato = hash_sep;       /* get lat part of deinterleaved hash */
    uint32_t ilono = hash_sep >> 32; /* shift over to get long part of hash */

    /* divide by 2**step.
     * Then, for 0-1 coordinate, multiply times scale and add
       to the min to get the absolute coordinate. */
    lat_range.min =
            kGeoLatMin + (ilato * 1.0 / (1ull << step)) * lat_scale;
    lat_range.max =
            kGeoLatMin + ((ilato + 1) * 1.0 / (1ull << step)) * lat_scale;
    lng_range.min =
            kGeoLngMin + (ilono * 1.0 / (1ull << step)) * long_scale;
    lng_range.max =
            kGeoLngMin + ((ilono + 1) * 1.0 / (1ull << step)) * long_scale;

    lng = (lng_range.min + lng_range.max) / 2.0;
    lat = (lat_range.min + lat_range.max) / 2.0;
    return true;
}