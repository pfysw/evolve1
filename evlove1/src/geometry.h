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

#define SAME_DIRECT 1
#define OPPS_DIRECT 2

#define FORWARD_DIRECT 1
#define BACK_DIRECT 2

#define DEG_60  60

typedef struct LineData LineData;
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


typedef struct CornerInfo CornerInfo;
struct CornerInfo
{
    PoinData *pVertex;
    PoinData *pLeft;
    PoinData *pRight;
    LineData *pLine1;
    LineData *pLine2;
    int val;
    u8 bl;
    u8 br;
};

typedef struct TempInfo TempInfo;
struct TempInfo{
    int a;
    int b;
    PoinData *apPoint[3];
};

typedef struct AngleTemp AngleTemp;
struct AngleTemp{
    //第一个下标表示哪个角，第二个下标表示哪条边
    PoinData *apSide[2][2];
};

typedef struct PlaneSeg PlaneSeg;
struct PlaneSeg
{
    //距离相等的两条平行线
    PlaneSeg *pNext;
    PlaneSeg *pPre;
    PlaneData *pPlane;
    u8 isHead;
    CornerInfo *pCorner;
    LinkNode *pSame;
};


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
    LinkNode *pSame;//2个端点上还有的相等的角
};

typedef struct SameLine SameLine;
struct SameLine
{
    LineSeg *pSeg1;
    LineSeg *pSeg2;
};

typedef struct SameAngle SameAngle;
struct SameAngle
{
    PlaneSeg *pSeg1;
    PlaneSeg *pSeg2;
};

struct PoinData
{
    int iNum;
    int nArray;
    char *zSymb;
    LineSeg **ppSeg;
    PlaneData *pPlane;
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
    PoinData *pVertex;
};


PointHash *CreatPointHash(int nSlot);
LineHash *CreatLineHash(int nSlot);
PlaneHash *CreatPlaneHash(int nSlot);
void ParseGeomEle(AstParse *pParse,Vector *pSet);
void CloseGeomSet(AstParse *pParse);
SameLine *SetSamePair(AstParse *pParse,LineSeg **ppSeg1,LineSeg **ppSeg2);
PlaneSeg *SetPlaneHash(AstParse *pParse,GeomType *pLeft,GeomType *pRight);
LineSeg *CreateNewLine(AstParse *pParse,PoinData *pPoint1,PoinData *pPoint2);
void FreeSamePair(LinkNode *pSame,int type);
void InsertSamePair(AstParse *pParse,PlaneSeg *pPSeg,SameLine *pPair);
void CheckOtherPair(AstParse *pParse,SameLine *pPair);

#endif /* GEOMETRY_H_ */
