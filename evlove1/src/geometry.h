/*
 * geometry.h
 *
 *  Created on: Jan 8, 2021
 *      Author: Administrator
 */

#ifndef GEOMETRY_H_
#define GEOMETRY_H_
#include "ast.h"


typedef struct LineToken LineToken;
typedef struct PointToken PointToken;
typedef struct AngleToken AngleToken;
typedef struct LineSegment LineSegment;

struct PointToken{
    AngleToken *pAngle;
};

struct LineToken{
};

struct LineSegment{
    int length;
    LineToken *pLine;
    PointToken *pPoint1;
    PointToken *pPoint2;
};

struct AngleToken{
    int angle;
    PointToken *pPoint;
    LineToken *pLine1;
    LineToken *pLine2;
};

#endif /* GEOMETRY_H_ */
