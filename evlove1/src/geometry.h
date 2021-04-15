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
typedef struct PointData PointData;
typedef struct LinePoint LinePoint;
struct LinePoint
{
    LinePoint *pNext;
    LinePoint *pPre;
    PointData *pPoint;
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
    PointData *pVertex;
    PointData *pLeft;
    PointData *pRight;
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
    PointData *apPoint[3];
};

typedef struct AngleTemp AngleTemp;
struct AngleTemp{
    AstParse *pParse;
    //第一个下标表示哪个角，第二个下标表示哪条边
    PointData *apSide[2][2];
};

typedef struct PlaneSeg PlaneSeg;
struct PlaneSeg
{
    //距离相等的两条平行线
    PlaneSeg*pNext;
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
    u8 bImage;
};

typedef struct LineSeg LineSeg;
struct LineSeg
{
    LineSeg *pNext;
    LineSeg *pPre;
    LineData *pLine;
    u8 isHead;
    PointData *pLeft;
    PointData *pRight;
    PointData *pMid;
    LinkNode *pSame;//2个端点上还有的相等的角
    LinkNode *pTriag;
    LinkNode *pTriSame;//全等三角形
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

typedef struct CommanPair CommanPair;
struct CommanPair
{
    void *pSeg1;
    void *pSeg2;
};

struct PointData
{
    int iNum;
    char *zSymb;
    AstParse *pParse;
    LineSeg **ppSeg;
    PlaneData *pPlane;
    u8 isInf;
};

typedef struct PointHash PointHash;
struct PointHash
{
    int nHash;
    int nPoint;
    PointData **ppHash;
    PointData **ppArray;
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
    PointData *pPoint1;
    PointData *pPoint2;
    PointData *pVertex;
    CornerInfo *pCorner;
};


PointHash *CreatPointHash(int nSlot);
LineHash *CreatLineHash(int nSlot);
PlaneHash *CreatPlaneHash(int nSlot);
void ParseGeomEle(AstParse *pParse,Vector *pSet);
void CloseGeomSet(AstParse *pParse);
SameLine *SetSamePair(AstParse *pParse,LineSeg **ppSeg1,LineSeg **ppSeg2);
PlaneSeg *SetPlaneHash(AstParse *pParse,GeomType *pLeft,GeomType *pRight);
LineSeg *CreateNewLine(AstParse *pParse,PointData *pPoint1,PointData *pPoint2);
void FreeSamePair(LinkNode *pSame,int type);
void InsertSamePair(AstParse *pParse,PlaneSeg *pPSeg,
        LineSeg **ppSeg1,
        LineSeg **ppSeg2);
void CheckOtherPair(AstParse *pParse,SameLine *pPair);
PlaneSeg *GetAndSetAngle(AstParse *pParse,PointData *pVertex,LineSeg* pSeg);
void InsertAnglePair(AstParse *pParse,LineSeg *pSeg,
        PlaneSeg *pPSeg1,
        PlaneSeg *pPSeg2);
void InsertLinkNode(AstParse *pParse,LinkNode *pPre,void *pVal);
LinkNode *NewLinkHead(void *pVal,int size);
int InsertCommonPair(AstParse *pParse,LinkNode **ppSame,
        void *pSeg1,
        void *pSeg2);
PlaneSeg *SetAngleHash(AstParse *pParse,GeomType *pAngle);
LineSeg** GetLineSegAddr(PointData *pLeft,PointData *pRight);
LinkNode *GetLinkNode(LinkNode *pHead,void *pVal);
void CheckNewSeg(AstParse *pParse,PointData *pNew,LineData *pLine);
GeomType GetLineEle(LineData *p);
PlaneSeg** GetPlaneSegAddr(AstParse *pParse,LineData *pLeft,LineData *pRight);
int GetLinesegDirect(LineData *pLine,PointData *pPoint1,PointData *pPoint2);
PointData *NewPointObj(AstParse *pParse);
void SetSegPoint(LineSeg *pSeg,PointData *pLeft,PointData *pRight);
void PrintLine(AstParse *pParse,LineData *pLine);
void SetEqualAngle(SameAngle *pA,AngleTemp *pTemp,int dir);
void PrintSameLine(void *pVal);
LineSeg *NewLineObj(AstParse *pParse,PointData *pLeft,int iRight);
LinePoint *GetLinePoint(LinePoint *pHead,PointData *pVal);

#endif /* GEOMETRY_H_ */
