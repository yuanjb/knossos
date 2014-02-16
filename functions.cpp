#include <math.h>
#include <float.h>
#include <vector>
#include "functions.h"


/** this file contains function which are not dependent from any state */

int roundFloat(float number) {
    if(number >= 0) return (int)(number + 0.5);
    else return (int)(number - 0.5);
}

/**
 * @brief almostEqual checks if two floats are practically equal.
 *        See also: http://www.cygnus-software.com/papers/comparingfloats/comparingfloats.htm
 */
bool almostEqual(float a, float b) {
    if (fabs(a - b) < FLT_MIN) { // absolute difference small enough
        return true;
    }
    float relativeError;
    if(fabs(b) > fabs(a)) {
        relativeError = fabs((a - b) / b);
    }
    else {
        relativeError = fabs((a - b) / a);
    }
    if(relativeError <= 0.01) { // max relative difference of 0.01 was arbitrarily chosen here...
        return true;
    }
    return false;
}

float distance(floatCoordinate a, floatCoordinate b) {
    floatCoordinate distVec;
    SUB_ASSIGN_COORDINATE(distVec, a, b);
    return euclidicNorm(&distVec);
}

float scalarProduct(floatCoordinate *v1, floatCoordinate *v2) {
    return ((v1->x * v2->x) + (v1->y * v2->y) + (v1->z * v2->z));
}

float euclidicNorm(floatCoordinate *v) {
    return ((float)sqrt((double)scalarProduct(v, v)));
}

bool normalizeVector(floatCoordinate *v) {
    float norm = euclidicNorm(v);
    v->x /= norm;
    v->y /= norm;
    v->z /= norm;
    return true;
}

int sgn(float number) {
    if(number > 0.) return 1;
    else if(number == 0.) return 0;
    else return -1;
}


//Some math helper functions
float radToDeg(float rad) {
    return ((180. * rad) / PI);
}

float degToRad(float deg) {
    return ((deg / 180.) * PI);
}

floatCoordinate* crossProduct(floatCoordinate *v1, floatCoordinate *v2) {
    floatCoordinate *result = NULL;
    result = (floatCoordinate*)malloc(sizeof(floatCoordinate));
    result->x = v1->y * v2->z - v1->z * v2->y;
    result->y = v1->z * v2->x - v1->x * v2->z;
    result->z = v1->x * v2->y - v1->y * v2->x;
    return result;
}

floatCoordinate centroidTriangle(Triangle tri) {
    floatCoordinate centroid;
    centroid.x = (float)(tri.a.x + tri.b.x + tri.c.x)/3;
    centroid.y = (float)(tri.a.y + tri.b.y + tri.c.y)/3;
    centroid.z = (float)(tri.a.z + tri.b.z + tri.c.z)/3;
    return centroid;
}

floatCoordinate centroidPolygon(std::vector<floatCoordinate> polygon) {
    float x, y, z;
    x = y = z = 0;
    for(uint i = 0; i < polygon.size(); ++i) {
        x += polygon[i].x;
        y += polygon[i].y;
        z += polygon[i].z;
    }
    floatCoordinate centroid;
    SET_COORDINATE(centroid, x/polygon.size(), y/polygon.size(), z/polygon.size());
    return centroid;
}

float vectorAngle(floatCoordinate *v1, floatCoordinate *v2) {
    return ((float)acos((double)(scalarProduct(v1, v2)) / (euclidicNorm(v1)*euclidicNorm(v2))));
}

float determinant(floatCoordinate v1, floatCoordinate v2, floatCoordinate v3) {
    return v1.x*v2.y*v3.z + v1.y*v2.z*v3.x + v1.z*v2.x*v3.y - v1.z*v2.y*v3.x - v2.z*v3.y*v1.x - v3.z*v1.y*v2.x;
}
