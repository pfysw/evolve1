/*
 * geometry.h
 *
 *  Created on: Jan 8, 2021
 *      Author: Administrator
 */

#ifndef GEOMETRY_H_
#define GEOMETRY_H_
#include "ast.h"
#include "prop.h"

#define ELE_POINT 0
#define ELE_LINE  1
#define ELE_ANGLE 2

typedef struct PoinData PoinData;
typedef struct LinePoint LinePoint;
struct LinePoint
{
    LinePoint *pNext;
    LinePoint *pPre;
    PoinData *pPoint;
    u8 isHead;
};


typedef struct LineData LineData;
struct LineData
{
    int iNum;
    LinePoint *pHead;
};

struct PoinData
{
    int iNum;
    int nArray;
    char *zSymb;
    LineData **ppLine;
};

typedef struct PointHash PointHash;
struct PointHash
{
    int nHash;
    int nPoint;
    PoinData **ppHash;
    PoinData **ppArray;
};


typedef struct LineHash LineHash;
struct LineHash
{
    int nLine;
    int nSlot;
    LineData **ppLine;
};

typedef struct GeomType GeomType;
struct GeomType
{
    u8 type;
    LineData *pLine1;
    LineData *pLine2;
    PoinData *pPoint1;
    PoinData *pPoint2;
};


PointHash *CreatPointHash(int nSlot);
LineHash *CreatLineHash(int nSlot);
void ParseGeomEle(AstParse *pParse,Vector *pSet);
void CloseGeomSet(AstParse *pParse);

#endif /* GEOMETRY_H_ */
