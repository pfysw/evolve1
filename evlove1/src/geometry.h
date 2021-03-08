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

typedef struct LinkNode LinkNode;
struct LinkNode
{
    LinkNode *pNext;
    LinkNode *pPre;
    void *pVal;
    u8 isHead;
};


typedef struct PlaneData PlaneData;
struct PlaneData
{
    int iNum;
    LinkNode *pHead;
};


typedef struct PlaneSeg PlaneSeg;
struct PlaneSeg
{
    PlaneSeg *pNext;
    PlaneSeg *pPre;
    PlaneData *pPlane;
    u8 isHead;
    LinkNode *pSame;
};


typedef struct LineData LineData;
struct LineData
{
    int iNum;
    int nArray;
    LinePoint *pHead;
    PlaneSeg **ppSeg;
};

typedef struct LineSeg LineSeg;
struct LineSeg
{
    LineSeg *pNext;
    LineSeg *pPre;
    LineData *pLine;
    u8 isHead;
    PoinData *pLeft;
    PoinData *pRight;
};

typedef struct SameLine SameLine;
struct SameLine
{
    LineSeg *pSeg1;
    LineSeg *pSeg2;
};


struct PoinData
{
    int iNum;
    int nArray;
    char *zSymb;
    LineSeg **ppSeg;
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

typedef struct PlaneHash PlaneHash;
struct PlaneHash
{
    int nPlane;
    int nSlot;
    PlaneData **ppPlane;
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
PlaneHash *CreatPlaneHash(int nSlot);
void ParseGeomEle(AstParse *pParse,Vector *pSet);
void CloseGeomSet(AstParse *pParse);

#endif /* GEOMETRY_H_ */
