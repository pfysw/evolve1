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

PointHash *CreatPointHash(int nSlot);
void ParseGeomEle(AstParse *pParse,Vector *pSet);

#endif /* GEOMETRY_H_ */
