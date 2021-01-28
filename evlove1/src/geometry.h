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
    LinePoint *pLeft;
    LinePoint *pRigth;
};


typedef struct LineHash LineData;
struct LineData
{
    TokenInfo *pPoint;
};

struct PoinData
{
    int iNum;
    int nHash;
    char *zSymb;
    LineData **ppHash;
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
    int nHash;
    int nLine;
    LineData **ppHash;
    LineData **ppArray;
};

typedef struct GeomType GeomType;
struct GeomType
{
    u8 type;
    LineData *pLine1;
    LineData *pLine2;
    PoinData *pLeft;
    PoinData *pRight;
};


PointHash *CreatPointHash(int nSlot);
LineHash *CreatLineHash(int nSlot);
void ParseGeomEle(AstParse *pParse,Vector *pSet);

#endif /* GEOMETRY_H_ */
