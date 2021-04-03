/*
 * geometry.c
 *
 *  Created on: Jan 8, 2021
 *      Author: Administrator
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "prop.h"
#include <assert.h>
#include "geometry.h"
#include "triangle.h"



PointHash *CreatPointHash(int nSlot)
{
    PointHash *pPointSet;
    int size;
    pPointSet = (PointHash *)Malloc(sizeof(PointHash));
    memset(pPointSet,0,sizeof(PointHash));
    pPointSet->nHash = nSlot*2;
    size = sizeof(PointHash *)*nSlot;
    pPointSet->ppArray = (PoinData **)Malloc(size);
    memset(pPointSet->ppArray,0,size);
    size = sizeof(PointHash *)*pPointSet->nHash;
    pPointSet->ppHash = (PoinData **)Malloc(size);
    memset(pPointSet->ppHash,0,size);
    return pPointSet;
}

LineHash *CreatLineHash(int nSlot)
{
    LineHash *pLineSet;
    int size;
    pLineSet = (LineHash *)Malloc(sizeof(LineHash));
    memset(pLineSet,0,sizeof(LineHash));
    size = sizeof(LineData *)*nSlot;
    pLineSet->nSlot = nSlot;
    pLineSet->ppLine = (LineData **)Malloc(size);
    memset(pLineSet->ppLine,0,size);
    return pLineSet;
}

PlaneHash *CreatPlaneHash(int nSlot)
{
    PlaneHash *pPlaneSet;
    int size;
    size = sizeof(PlaneData *)*nSlot;
    pPlaneSet = (PlaneHash *)Malloc(sizeof(PlaneHash)+size);
    memset(pPlaneSet,0,sizeof(PlaneHash));
    pPlaneSet->nSlot = nSlot;
    pPlaneSet->ppPlane = (PlaneData **)(&pPlaneSet[1]);
    memset(pPlaneSet->ppPlane,0,size);
    return pPlaneSet;
}

void FreeLinePoint(AstParse *pParse,LinePoint *pHead)
{
    LinePoint *p = pHead->pNext;
    LinePoint *pTmp;

//      static int jj = 0;
//      jj++;
//     if(jj==4)
//          printf("jj %d\n",jj);

    if(gDebug.freePrint){
        log_c("line: ");
    }
    while(1)
    {
        if(p->isHead){
            //log_c("%s ",p->pPoint->zSymb);
            Free(p);
            break;
        }
        else{
            if(gDebug.freePrint){
                log_c("%s ",p->pPoint->zSymb);
            }
            pTmp = p;
            p = p->pNext;
            Free(pTmp);
        }
    }
    if(gDebug.freePrint){
        log_a("");
    }
}

void FreeLinkNode(LinkNode *pHead,int isFreeVal)
{
    LinkNode *p;
    LinkNode *pTmp;

    if(pHead==NULL){
        return;
    }
    p = pHead->pNext;
    while(1)
    {
        if(p->isHead){
            if(isFreeVal){
                Free(p->pVal);
            }
            Free(p);
            break;
        }
        else{
            if(isFreeVal){
                Free(p->pVal);
            }
            pTmp = p;
            p = p->pNext;
            Free(pTmp);
        }
    }
}

void FreeLineSeg(PoinData *pArray)
{
    int i = 0;
    for(i=0;i<pArray->iNum+1;i++)
    {
        if(pArray->ppSeg[i] != NULL){
            FreeSamePair(pArray->ppSeg[i]->pSame,ELE_ANGLE);
            FreeLinkNode(pArray->ppSeg[i]->pTriag,1);
            FreeLinkNode(pArray->ppSeg[i]->pTriSame,1);
            Free(pArray->ppSeg[i]);
            pArray->ppSeg[i] = NULL;
        }
    }
    Free(pArray->ppSeg);
    pArray->ppSeg = NULL;
}

void PrintSameLine(void *pVal)
{
    SameLine *pPair = (SameLine *)pVal;
    char *zLeft;
    char *zRight;
    log_c("same:");
    zLeft =  pPair->pSeg1->pLeft->zSymb;
    zRight =  pPair->pSeg1->pRight->zSymb;
    log_c("%s-%s = ",zLeft,zRight);
    zLeft =  pPair->pSeg2->pLeft->zSymb;
    zRight =  pPair->pSeg2->pRight->zSymb;
    log_c("%s-%s\n",zLeft,zRight);
}

void PrintCorner(CornerInfo *pCorner)
{
    log_c("%s%s%s ",pCorner->pLeft->zSymb,
            pCorner->pVertex->zSymb,
            pCorner->pRight->zSymb);
}


void PrintSameAngle(SameAngle *pPair)
{
    log_c("same angle:");
    PrintCorner(pPair->pSeg1->pCorner);
    PrintCorner(pPair->pSeg2->pCorner);
    log_a("");
}

void PrintLine(LineData *pLine)
{
    LinePoint *p;
    int i;
    p = pLine->pHead->pNext;
    for(i=0;i<2;i++){
        log_c("%s",p->pPoint->zSymb);
        p = p->pNext;
    }
    log_c(" ");
}

void PrintPlaneLine(PoinData *pVertex)
{
    PlaneData *pPlane = pVertex->pPlane;
    LinkNode *p;
    log_a("plane center %s:",pVertex->zSymb);
    p=pPlane->pHead;
    do{
        PrintLine(p->pVal);
        p = p->pNext;
    }while(!p->isHead);
    log_a("");

}

void FreeSamePair(LinkNode *pSame,int type)
{
    LinkNode *p;
    LinkNode *pTmp;
    if(pSame!=NULL){
        p = pSame->pNext;
        while(1)
        {
            //printf("addr %p\n",p);
            if(p->isHead){
                if(type==ELE_LINE){
                    if(gDebug.freePrint){
                        printf("head\n");
                        PrintSameLine(p->pVal);
                    }
                }
                Free(p->pVal);
                Free(p);
                break;
            }
            else{
                pTmp = p;
                p = p->pNext;
                if(type==ELE_LINE){
                    if(gDebug.freePrint){
                        PrintSameLine(pTmp->pVal);
                    }
                }
                Free(pTmp->pVal);
                Free(pTmp);
            }
        }
    }
}

void FreePlaneSeg(LineData *pArray)
{
    int i = 0;
    for(i=0;i<pArray->iNum+1;i++)
    {
        if(pArray->ppSeg[i]!= NULL){
            if(pArray->ppSeg[i]->pCorner!=NULL){
                if(gDebug.freePrint){
                    log_a("angle: %s%s%s",pArray->ppSeg[i]->pCorner->pLeft->zSymb,
                            pArray->ppSeg[i]->pCorner->pVertex->zSymb,
                            pArray->ppSeg[i]->pCorner->pRight->zSymb);
                }
            }
            FreeSamePair(pArray->ppSeg[i]->pSame,ELE_LINE);
            if(pArray->ppSeg[i]->pCorner){
                Free(pArray->ppSeg[i]->pCorner);
            }
            Free(pArray->ppSeg[i]);
            pArray->ppSeg[i] = NULL;
        }
    }
    Free(pArray->ppSeg);
    pArray->ppSeg = NULL;
}

void FreePointSet(AstParse *pParse)
{
    PointHash *pPointSet = pParse->pPointSet;
    LineHash *pLineSet = pParse->pLineSet;
    PlaneHash *pPlaneSet = pParse->pPlaneSet;
    int i = 0;
    for(i=0;i<pPlaneSet->nPlane;i++)
    {
        FreeLinkNode(pPlaneSet->ppPlane[i]->pHead,0);
        Free(pPlaneSet->ppPlane[i]);
    }
    for(i=0;i<pLineSet->nLine;i++)
    {
        FreeLinePoint(pParse,pLineSet->ppLine[i]->pHead);
        FreePlaneSeg(pLineSet->ppLine[i]);
        Free(pLineSet->ppLine[i]);
    }
    for(i=0;i<pPointSet->nPoint;i++)
    {
        FreeLineSeg(pPointSet->ppArray[i]);
        if(pPointSet->ppArray[i]->zSymb!=NULL){
            Free(pPointSet->ppArray[i]->zSymb);
        }
        Free(pPointSet->ppArray[i]);
    }
}

void CloseGeomSet(AstParse *pParse)
{
    FreePointSet(pParse);
    Free(pParse->pPointSet->ppArray);
    Free(pParse->pPointSet->ppHash);
    Free(pParse->pPointSet);
    Free(pParse->pLineSet->ppLine);
    Free(pParse->pLineSet);
    Free(pParse->pPlaneSet);
}

PoinData *NewPointObj(AstParse *pParse)
{
    PoinData *pPoint;
    PointHash *pSet = pParse->pPointSet;

    pPoint = (PoinData *)Malloc(sizeof(PoinData));
    memset(pPoint,0,sizeof(PoinData));
    pPoint->iNum = pSet->nPoint++;
    pSet->ppArray[pPoint->iNum] = pPoint;
    pPoint->ppSeg = (LineSeg**)Malloc(sizeof(LineSeg*)*(pPoint->iNum+1));
    memset(pPoint->ppSeg, 0, sizeof(LineSeg*)*(pPoint->iNum+1));
    return pPoint;
}

LinePoint *NewPointNode(PoinData *pPoint)
{
    LinePoint *p;
    p = (LinePoint *)Malloc(sizeof(LinePoint));
    memset(p,0,sizeof(*p));
    p->pPoint = pPoint;
    return p;
}

LinePoint *NewPointHead(PoinData *pPoint)
{
    LinePoint *p;
    p = NewPointNode(pPoint);
    p->pNext = p;
    p->pPre = p;
    p->isHead = 1;
    return p;
}


LinkNode *NewLinkNode(void *pVal,int size)
{
    LinkNode *p;
    p = (LinkNode *)Malloc(size);
    memset(p,0,size);
    p->pVal = pVal;
    return p;
}

LinkNode *NewLinkHead(void *pVal,int size)
{
    LinkNode *p;
    p = NewLinkNode(pVal,size);
    p->pNext = p;
    p->pPre = p;
    p->isHead = 1;
    return p;
}

void InsertPointNode(AstParse *pParse,LinePoint *pPre,PoinData *pNode)
{
    LinePoint *p = (LinePoint*)Malloc(sizeof(LinePoint));
    memset(p,0,sizeof(LinePoint));
    p->pPoint = pNode;

    p->pNext = pPre->pNext;
    p->pPre = pPre;
    p->pNext->pPre = p;
    pPre->pNext = p;
}

void InsertLinkNode(AstParse *pParse,LinkNode *pPre,void *pVal)
{
    LinkNode *p = (LinkNode*)Malloc(sizeof(LinkNode));
    memset(p,0,sizeof(LinkNode));
    p->pVal = pVal;
    p->pNext = pPre->pNext;
    p->pPre = pPre;
    p->pNext->pPre = p;
    pPre->pNext = p;
}

LinePoint *FindPointLoc(LinePoint *pHead,PoinData *pNode)
{
    LinePoint *p;
    assert(pHead->isHead);
    p = pHead;
    do{
        if(p->pPoint==pNode){
            return p;
        }
        p = p->pNext;
    }while(!p->isHead);
    assert(0);
    return NULL;
}

void TravLinePoint(LineData *pLine)
{
    LinePoint *p;
    assert(pLine->pHead->isHead);
    p = pLine->pHead->pNext;
    while(!p->isHead){
        printf("point %s\n",p->pPoint->zSymb);
        p = p->pNext;
    }
}


LineSeg** GetLineSegAddr(PoinData *pLeft,PoinData *pRight)
{
    int iLeft;
    int iRight;
    iLeft = pLeft->iNum;
    iRight = pRight->iNum;
    if(iLeft>iRight){
        return &pLeft->ppSeg[iRight];
    }
    else
    {
        return &pRight->ppSeg[iLeft];
    }
}

PlaneSeg* GetPlaneSeg(LineData *pLeft,LineData *pRight)
{
    int iLeft;
    int iRight;
    iLeft = pLeft->iNum;
    iRight = pRight->iNum;
    if(iLeft>iRight){
        return pLeft->ppSeg[iRight];
    }
    else
    {
        return pRight->ppSeg[iLeft];
    }
}

void ResetLine(AstParse *pParse,GeomType *pDst,LineData *pLine)
{
    LineSeg* pSeg;
    int nLine;
    LineHash *pLineSet = pParse->pLineSet;

    pSeg = CreateNewLine(pParse,pDst->pPoint1,pDst->pPoint2);
    log_c("reset: ");
    nLine = --pParse->pLineSet->nLine;
    //todo 确认pDst->pLine1是否是最后一个
    assert(pLineSet->ppLine[nLine]==pDst->pLine1);
    FreeLinePoint(pParse,pDst->pLine1->pHead);
    Free(pDst->pLine1->ppSeg);
    Free(pDst->pLine1);
    pSeg->pLine = pLine;
}

LineSeg *NewLineObj(AstParse *pParse,PoinData *pLeft,int iRight)
{
    LineData *pNew;
    LineSeg *pSeg;
    PoinData *pInf;
    LineHash *pLineSet = pParse->pLineSet;
    pNew = (LineData*)Malloc(sizeof(LineData));
    memset(pNew,0,sizeof(LineData));
    pSeg = (LineSeg*)NewLinkHead(pNew,sizeof(LineSeg));
    pLeft->ppSeg[iRight] = pSeg;
    pLeft->nArray++;
    pNew->iNum = pLineSet->nLine;
    if(pLineSet->nLine>pLineSet->nSlot){
        //todo resizeHash
        assert(0);
    }
    pLineSet->ppLine[pNew->iNum] = pNew;
    pLineSet->nLine++;
   // pNew->pHead = NewPointHead(NULL);
    pInf = NewPointObj(pParse);
    pNew->pHead = NewPointHead(pInf);
    //两条相同的直线也组成一个seg
    pNew->ppSeg = (PlaneSeg **)Malloc(sizeof(PlaneSeg *)*(pNew->iNum+1));
    memset(pNew->ppSeg,0,sizeof(PlaneSeg *)*(pNew->iNum+1));
    return pSeg;
}


PlaneSeg *NewPlaneObj(AstParse *pParse,LineData *pLeft,int iRight)
{
    PlaneData *pNew;
    PlaneSeg *pSeg;
    PlaneHash *pPlaneSet = pParse->pPlaneSet;
    pNew = (PlaneData*)Malloc(sizeof(PlaneData));
    memset(pNew,0,sizeof(PlaneData));
    pSeg = (PlaneSeg*)NewLinkHead(pNew,sizeof(PlaneSeg));
    pLeft->ppSeg[iRight] = pSeg;
    pLeft->nArray++;
    pNew->iNum = pPlaneSet->nPlane;
    if(pPlaneSet->nPlane>pPlaneSet->nSlot){
        //todo resizeHash
        assert(0);
    }
    pPlaneSet->ppPlane[pNew->iNum] = pNew;
    pPlaneSet->nPlane++;
    pNew->pHead = NewLinkHead(NULL,sizeof(LinkNode));
    return pSeg;
}

GeomType GetPointEle(PoinData *p)
{
    GeomType ele = {0};
    ele.type = ELE_POINT;
    ele.pPoint1 = p;
    return ele;
}

GeomType GetLineEle(LineData *p)
{
    GeomType ele = {0};
    ele.type = ELE_LINE;
    ele.pLine1 = p;
    return ele;
}

void SetSegPoint(LineSeg *pSeg,PoinData *pLeft,PoinData *pRight)
{
    LineData *pLine = pSeg->pLine;
    LinePoint *p;
    p = pLine->pHead->pNext;
    while(!p->isHead){
        if(p->pPoint==pLeft){
            pSeg->pLeft = pLeft;
            pSeg->pRight = pRight;
            break;
        }
        else if(p->pPoint==pRight){
            pSeg->pLeft = pRight;
            pSeg->pRight = pLeft;
            break;
        }
        else{
            p = p->pNext;
        }
    }
}

//如果已经创建了则直接返回
LineSeg *CreateNewLine(AstParse *pParse,PoinData *pPoint1,PoinData *pPoint2)
{
    int iLeft;
    int iRight;
    LineSeg *pSeg;
    LineData *pLine;

    iLeft = pPoint1->iNum;
    iRight = pPoint2->iNum;
    if(iLeft>iRight){
        if(pPoint1->ppSeg[iRight]==NULL){
            pSeg = NewLineObj(pParse,pPoint1,iRight);
            pLine = pSeg->pLine;
            //保持最初始的输入顺序，直线B-C,pLeft是B，pRight是C
            InsertPointNode(pParse,pLine->pHead,pPoint1);
            InsertPointNode(pParse,pLine->pHead->pNext,pPoint2);
            SetSegPoint(pSeg,pPoint1,pPoint2);
        }
        else{
            pSeg = pPoint1->ppSeg[iRight];
        }
    }
    else{
        if(pPoint2->ppSeg[iLeft]==NULL){
            pSeg = NewLineObj(pParse,pPoint2,iLeft);
            pLine = pSeg->pLine;
            //保持最初始的输入顺序，直线B-C,pLeft是B，pRight是C
            InsertPointNode(pParse,pLine->pHead,pPoint1);
            InsertPointNode(pParse,pLine->pHead->pNext,pPoint2);
            SetSegPoint(pSeg,pPoint1,pPoint2);
        }
        else{
            pSeg = pPoint2->ppSeg[iLeft];
        }
    }
    return pSeg;
}

void SetLineSegPoint(AstParse *pParse,
                     LineSeg** ppLs1,
                     LineSeg** ppLs2,
                     GeomType *pLineSeg,//被插入的线段
                     GeomType *pLink,//要插入的点
                     u8 bInsertPre)
{
    LinePoint *pLoc;
    *ppLs1 = (LineSeg*)NewLinkHead(pLineSeg->pLine1,sizeof(LineSeg));
    *ppLs2 = (LineSeg*)NewLinkHead(pLineSeg->pLine1,sizeof(LineSeg));
    pLoc = FindPointLoc(pLineSeg->pLine1->pHead,pLineSeg->pPoint2);
    if(bInsertPre){
        InsertPointNode(pParse,pLoc->pPre,pLineSeg->pPoint1);
    }
    else{
        //pRight->pPoint1插入到pLeft->pLine1后面
        InsertPointNode(pParse,pLoc,pLink->pPoint1);
    }
    SetSegPoint(*ppLs1,pLink->pPoint1,pLineSeg->pPoint1);
    SetSegPoint(*ppLs2,pLink->pPoint1,pLineSeg->pPoint2);
}

void CheckNewSeg(AstParse *pParse,PoinData *pNew,LineData *pLine)
{
    LinePoint *p;
    LineSeg** ppLSeg;

    p = pLine->pHead->pNext;
    while(!p->isHead){
        ppLSeg = GetLineSegAddr(pNew,p->pPoint);
        if(*ppLSeg==NULL){
            *ppLSeg = (LineSeg*)NewLinkHead(pLine,sizeof(LineSeg));
            SetSegPoint(*ppLSeg,pNew,p->pPoint);
        }
        p = p->pNext;
    }
}

GeomType SetLineHash(AstParse *pParse,GeomType *pLeft,GeomType *pRight)
{
    GeomType ele = {0};
    LineData *pLine;
    LinePoint *pLoc;
    LineSeg** ppLs1;
    LineSeg** ppLs2;
    LineSeg *pSeg;
    PoinData *pNew;

    if(pLeft->type==ELE_POINT && pRight->type==ELE_POINT)
    {
        ele.type = ELE_LINE;
        pSeg = CreateNewLine(pParse,pLeft->pPoint1,pRight->pPoint1);
        pLine = pSeg->pLine;
        //保持最初始的输入顺序，直线B-C,ele.pPoint1是B，ele.pPoint2是C
        ele.pPoint1 = pLeft->pPoint1;
        ele.pPoint2 = pRight->pPoint1;
        ele.pLine1 = pLine;
    }
    else if(pLeft->type==ELE_POINT)
    {
        assert(0);//未测试
        assert(pRight->type==ELE_LINE);
        ppLs1 = GetLineSegAddr(pLeft->pPoint1,pRight->pPoint1);
        ppLs2 = GetLineSegAddr(pLeft->pPoint1,pRight->pPoint2);
        assert(*ppLs1==NULL);
        assert(*ppLs2==NULL);
        SetLineSegPoint(pParse,ppLs1,ppLs2,pRight,pLeft,1);
        pNew = pLeft->pPoint1;
        CheckNewSeg(pParse,pNew,(*ppLs1)->pLine);
    }
    else if(pLeft->type==ELE_LINE)
    {
        ppLs1 = GetLineSegAddr(pRight->pPoint1,pLeft->pPoint1);
        ppLs2 = GetLineSegAddr(pRight->pPoint1,pLeft->pPoint2);
        if(*ppLs1!=NULL){
            assert(*ppLs2==NULL);
            pLoc = FindPointLoc((*ppLs1)->pLine->pHead,pLeft->pPoint1);
            ResetLine(pParse,pLeft,(*ppLs1)->pLine);
            *ppLs2 = (LineSeg*)NewLinkHead((*ppLs1)->pLine,sizeof(LineSeg));
            InsertPointNode(pParse,pLoc,pLeft->pPoint2);
            SetSegPoint(*ppLs2,pRight->pPoint1,pLeft->pPoint2);
            pNew = pLeft->pPoint2;
        }
        else if(*ppLs2!=NULL){
            assert(0);
        }
        else{
            SetLineSegPoint(pParse,ppLs1,ppLs2,pLeft,pRight,0);
            pNew = pRight->pPoint1;
        }
        CheckNewSeg(pParse,pNew,(*ppLs1)->pLine);
    }
    else
    {
        assert(0);
    }

    return ele;
}

GeomType GetAngleEle(AstParse *pParse,GeomType *pLeft,GeomType *pRight)
{
    GeomType ele = {0};
    GeomType ele1 = {0};
    GeomType ele2 = {0};

    assert(pLeft->type==ELE_POINT && pRight->type==ELE_LINE);
    ele1 = GetPointEle(pRight->pPoint1);
    ele2 = GetPointEle(pRight->pPoint2);
    ele1 = SetLineHash(pParse,pLeft,&ele1);
    ele2 = SetLineHash(pParse,pLeft,&ele2);
    ele.type = ELE_ANGLE;
    ele.pVertex = pLeft->pPoint1;
    ele.pLine1 = ele1.pLine1;
    ele.pLine2 = ele2.pLine1;
    ele.pPoint1 = pRight->pPoint1;
    ele.pPoint2 = pRight->pPoint2;

    return ele;
}

int GetLinesegDirect(LineData *pLine,PoinData *pPoint1,PoinData *pPoint2)
{
    LinePoint *p;
    int rc = 0;
    p = pLine->pHead->pNext;
    while(!p->isHead){
        if(p->pPoint==pPoint1){
            rc = FORWARD_DIRECT;
            break;
        }
        else if(p->pPoint==pPoint2){
            rc = BACK_DIRECT;
            break;
        }
        p = p->pNext;
    }
    assert(rc!=0);
    return rc;
}

int GetPlanesegDirect(PlaneData *pPlane,LineData *pLine1,LineData *pLine2)
{
    LinkNode *p;
    int rc = 0;
    p = pPlane->pHead->pNext;
    while(!p->isHead){
        if(p->pVal==pLine1){
            rc = FORWARD_DIRECT;
            break;
        }
        else if(p->pVal==pLine2){
            rc = BACK_DIRECT;
            break;
        }
        p = p->pNext;
    }
    assert(rc!=0);
    return rc;
}

PoinData *GetIntersection(AstParse *pParse,LineData *pLine1,LineData *pLine2)
{
    LinePoint *p1,*p2;
    PoinData *pPoint = NULL;
    for(p1=pLine1->pHead->pNext;!p1->isHead;p1=p1->pNext)
    {
        for(p2=pLine2->pHead->pNext;!p2->isHead;p2=p2->pNext)
        {
            if(p1->pPoint==p2->pPoint){
                pPoint = p1->pPoint;
                break;
            }
        }
    }
    return pPoint;
}

LinkNode *GetLinkNode(LinkNode *pHead,void *pVal)
{
    LinkNode *p;
    LinkNode *pFind = NULL;
    p = pHead;
    do{
        if(p->pVal==pVal){
            pFind = p;
            break;
        }
        p = p->pNext;
    }while(!p->isHead);
    return pFind;
}

void InsertPlaneLine(
        AstParse *pParse,
        PoinData *pVertex,
        PoinData *pPoint,
        LineData *pBase)
{
    LinePoint *pNode;
    LinePoint *p;
    LineData *pLine;
    LineSeg *pRSeg;
    LineSeg *pLSeg;
    LinkNode *pLineNode;

    pLine = (*GetLineSegAddr(pPoint,pVertex))->pLine;
    assert(pLine!=NULL);
    if(GetLinkNode(pVertex->pPlane->pHead,pLine))
    {
        return;
    }

    pNode = (LinePoint *)GetLinkNode((LinkNode *)(pBase->pHead),(void*)pPoint);
    if(pNode)
    for(p=pNode->pNext;;p=p->pNext){
        assert(p!=pNode);
        pRSeg = *GetLineSegAddr(p->pPoint,pVertex);
        if(pRSeg!=NULL){
            if(GetLinkNode(pVertex->pPlane->pHead,pRSeg->pLine)){
                break;
            }

        }
    }
    for(p=pNode->pPre;;p=p->pPre){
        assert(p!=pNode);
        pLSeg = *GetLineSegAddr(p->pPoint,pVertex);
        if(pLSeg!=NULL){
            if(GetLinkNode(pVertex->pPlane->pHead,pLSeg->pLine)){
                break;
            }
        }
    }

    pLineNode = GetLinkNode(pVertex->pPlane->pHead,pLSeg->pLine);
    if(pLineNode->pNext->pVal==pRSeg->pLine){
        InsertLinkNode(pParse,pLineNode,pLine);
    }
    else{
        assert(pLineNode->pPre->pVal==pRSeg->pLine);
        InsertLinkNode(pParse,pLineNode->pPre,pLine);
    }
}

void InsertPlaneAngle(AstParse *pParse,GeomType *pAngle)
{
    LineData *pBase;
    pBase = (*GetLineSegAddr(pAngle->pPoint1,pAngle->pPoint2))->pLine;
    InsertPlaneLine(pParse,pAngle->pVertex,pAngle->pPoint1,pBase);
    InsertPlaneLine(pParse,pAngle->pVertex,pAngle->pPoint2,pBase);
    PrintPlaneLine(pAngle->pVertex);
}

PlaneSeg *SetAngleHash(AstParse *pParse,GeomType *pAngle,TokenInfo *pVal)
{
    PlaneSeg *pSeg;
    GeomType ele1;
    GeomType ele2;
    CornerInfo *pCorner;
    u8 bl;
    u8 br;

    ele1 = GetLineEle(pAngle->pLine1);
    ele2 = GetLineEle(pAngle->pLine2);
    if(pAngle->pVertex->pPlane!=NULL){
        InsertPlaneAngle(pParse,pAngle);
    }
    pSeg = SetPlaneHash(pParse,&ele1,&ele2);
    if(pSeg->pCorner==NULL){
        assert(pAngle->pLine1!=pAngle->pLine2);
        bl = GetLinesegDirect(pAngle->pLine1,pAngle->pVertex,pAngle->pPoint1);
        br = GetLinesegDirect(pAngle->pLine2,pAngle->pVertex,pAngle->pPoint2);
        pCorner = (CornerInfo *)Malloc(sizeof(CornerInfo));
        memset(pCorner,0,sizeof(CornerInfo));
        pCorner->pVertex = pAngle->pVertex;
        pCorner->pLeft = pAngle->pPoint1;
        pCorner->pRight = pAngle->pPoint2;
        if(pVal!=NULL){
            pCorner->val = atoi(pVal->zSymb);
        }
        pCorner->pLine1 = pAngle->pLine1;
        pCorner->pLine2 = pAngle->pLine2;
        pCorner->bl = bl;
        pCorner->br = br;
        pSeg->pCorner = pCorner;
        if(pCorner->pVertex->pPlane!=NULL){
            pSeg->pPlane = pCorner->pVertex->pPlane;
            //todo 现在pPlane还放在全局数组里未释放
            //在这里释放后面会导致重复释放
        }
        else{
            pCorner->pVertex->pPlane = pSeg->pPlane;
        }
    }
    return pSeg;
}

//0:没找到相同点 1：同向  2：异向
int GetSegDirect(AstParse *pParse,LineSeg *pIn1,LineSeg *pIn2,GeomType *pOut)
{
    int rc = 0;
    if(pIn1->pLeft==pIn2->pLeft ){
        rc = SAME_DIRECT;
        pOut->pPoint1 = pIn1->pRight;
        pOut->pPoint2 = pIn2->pRight;
    }
    else if(pIn1->pRight==pIn2->pRight){
        rc = SAME_DIRECT;
        pOut->pPoint1 = pIn1->pLeft;
        pOut->pPoint2 = pIn2->pLeft;
    }
    else if(pIn1->pLeft==pIn2->pRight){
        rc = OPPS_DIRECT;
        pOut->pPoint1 = pIn1->pRight;
        pOut->pPoint2 = pIn2->pLeft;
    }
    else if(pIn1->pRight==pIn2->pLeft){
        rc = OPPS_DIRECT;
        pOut->pPoint1 = pIn1->pLeft;
        pOut->pPoint2 = pIn2->pRight;
    }
    return rc;
}


void TrglPointTemp(TempInfo *pTemp,PoinData *p1,PoinData *p2,PoinData *p3)
{
    pTemp->apPoint[0] = p1;
    pTemp->apPoint[1] = p2;
    pTemp->apPoint[2] = p3;
}

int HavePubPoint(LineSeg *pIn1,LineSeg *pIn2,TempInfo *pTemp)
{
    int rc = 0;
    if(pIn1->pLeft==pIn2->pLeft){
        pTemp->a = GetLinesegDirect(pIn1->pLine,pIn1->pLeft,pIn1->pRight);
        pTemp->b = GetLinesegDirect(pIn2->pLine,pIn2->pLeft,pIn2->pRight);
        TrglPointTemp(pTemp,pIn1->pLeft,pIn1->pRight,pIn2->pRight);
        rc = 1;
    }
    else if(pIn1->pLeft==pIn2->pRight){
        pTemp->a = GetLinesegDirect(pIn1->pLine,pIn1->pLeft,pIn1->pRight);
        pTemp->b = GetLinesegDirect(pIn2->pLine,pIn2->pRight,pIn2->pLeft);
        TrglPointTemp(pTemp,pIn1->pLeft,pIn1->pRight,pIn2->pLeft);
        rc = 1;
    }
    else if(pIn1->pRight==pIn2->pLeft){
        pTemp->a = GetLinesegDirect(pIn1->pLine,pIn1->pRight,pIn1->pLeft);
        pTemp->b = GetLinesegDirect(pIn2->pLine,pIn2->pLeft,pIn2->pRight);
        TrglPointTemp(pTemp,pIn1->pLeft,pIn1->pRight,pIn2->pRight);
        rc = 1;
    }
    else if(pIn1->pRight==pIn2->pRight){
        pTemp->a = GetLinesegDirect(pIn1->pLine,pIn1->pRight,pIn1->pLeft);
        pTemp->b = GetLinesegDirect(pIn2->pLine,pIn2->pRight,pIn2->pLeft);
        TrglPointTemp(pTemp,pIn1->pLeft,pIn1->pRight,pIn2->pLeft);
        rc = 1;
    }
    return rc;
}

void CheckNewSame(AstParse *pParse,LinkNode *pSame,SameLine *pIn)
{
    LinkNode *p;
    SameLine *pPair;
    SameLine *pNew;
    GeomType ele1 = {0};
    GeomType ele2 = {0};
    LineSeg **ppSeg1;
    LineSeg **ppSeg2;
    int rc = -1;

    p = pSame;
    do{
        pPair = (SameLine *)p->pVal;
        assert(pIn!=pPair);
        p = p->pNext;//放在前面可以避免遍历新插入的
        rc = GetSegDirect(pParse,pPair->pSeg1,pIn->pSeg1,&ele1);
        if(rc>0){
            if(GetSegDirect(pParse,pPair->pSeg2,pIn->pSeg2,&ele2)==rc)
            {
                ppSeg1 = GetLineSegAddr(ele1.pPoint1,ele1.pPoint2);
                ppSeg2 = GetLineSegAddr(ele2.pPoint1,ele2.pPoint2);
                if( ppSeg1== ppSeg2) {
                    continue;
                }
                pNew = SetSamePair(pParse,ppSeg1,ppSeg2);
               // PrintSameLine(pNew);
                InsertLinkNode(pParse,pSame,pNew);
                CheckOtherPair(pParse,pNew);
            }
        }
    }while(!p->isHead);
}

void LinkLineSeg(AstParse *pParse,LineSeg *pSeg1,LineSeg *pSeg2)
{
    LineSeg *p;
    LineSeg *pHead;
    p = pSeg1;
    while(!p->isHead) p = p->pNext;
    pHead = p;
    p = pSeg2;
    while(!p->isHead) p = p->pNext;
    //判断pSeg2是否已经在pSeg1的链表里了
    if(p!=pHead){
        p->isHead = 0;//清楚掉第二个链表的头结点
        //把第二个链表合到第一个上面
        pSeg1->pPre->pNext = pSeg2->pNext;//从pSeg2和下个结点处切开
        pSeg2->pNext->pPre = pSeg1->pPre;
        pSeg2->pNext = pSeg1;
        pSeg1->pPre = pSeg2;
    }
}

void LinkSegNode(AstParse *pParse,LinkNode *pSeg1,LinkNode *pSeg2)
{
    LinkNode *p;
    LinkNode *pHead;
    p = pSeg1;
    while(!p->isHead) p = p->pNext;
    pHead = p;
    p = pSeg2;
    while(!p->isHead) p = p->pNext;
    //判断pSeg2是否已经在pSeg1的链表里了
    if(p!=pHead){
        p->isHead = 0;//清楚掉第二个链表的头结点
        //把第二个链表合到第一个上面
        pSeg1->pPre->pNext = pSeg2->pNext;//从pSeg2和下个结点处切开
        pSeg2->pNext->pPre = pSeg1->pPre;
        pSeg2->pNext = pSeg1;
        pSeg1->pPre = pSeg2;
    }
}

SameLine *SetSamePair(AstParse *pParse,LineSeg **ppSeg1,LineSeg **ppSeg2)
{
    SameLine *pPair;
    int iNum1,iNum2;

    pPair = (SameLine *)Malloc(sizeof(SameLine));
    iNum1 =  (*ppSeg1)->pLine->iNum;
    iNum2 =  (*ppSeg2)->pLine->iNum;
    if(iNum1<iNum2){
        pPair->pSeg1 = *ppSeg1;
        pPair->pSeg2 = *ppSeg2;
    }
    else{
        pPair->pSeg1 = *ppSeg2;
        pPair->pSeg2 = *ppSeg1;
    }
    LinkLineSeg(pParse,*ppSeg1,*ppSeg2);
    return pPair;
}

SameAngle *SameAnglePair(AstParse *pParse,PlaneSeg *pSeg1,PlaneSeg *pSeg2)
{
    SameAngle *pPair;
    int iNum1,iNum2;

    pPair = (SameAngle *)Malloc(sizeof(SameAngle));
    iNum1 =  pSeg1->pPlane->iNum;
    iNum2 =  pSeg2->pPlane->iNum;
    if(iNum1<iNum2){
        pPair->pSeg1 = pSeg1;
        pPair->pSeg2 = pSeg2;
    }
    else{
        pPair->pSeg1 = pSeg2;
        pPair->pSeg2 = pSeg1;
    }
    if(pSeg1->pCorner->val){
        pSeg2->pCorner->val = pSeg1->pCorner->val;
    }
    else{
        pSeg1->pCorner->val = pSeg2->pCorner->val;
    }
    //printf("angle val：%d %d\n",pSeg1->pCorner->val,pSeg2->pCorner->val);
    LinkSegNode(pParse,(LinkNode*)pSeg1,(LinkNode*)pSeg2);
    return pPair;
}

void SwapTemp(TempInfo *pTemp)
{
    int c;
    c = pTemp->a;
    pTemp->a = pTemp->b;
    pTemp->b = c;
}

int HaveSamePair(LinkNode *pSame,LineSeg *pSeg1,LineSeg *pSeg2)
{
    int rc = 0;
    LinkNode *p;
    SameLine *pPair;

    if(pSame==NULL){
        return 0;
    }
    p = pSame;
    do{
        pPair = (SameLine *)p->pVal;
        if(pSeg1==pPair->pSeg1){
            if(pSeg2==pPair->pSeg2){
                rc = 1;
                break;
            }
        }
        else if(pSeg1==pPair->pSeg2){
            if(pSeg2==pPair->pSeg1){
                rc = 1;
                break;
            }
        }
        p = p->pNext;
    }while(!p->isHead);

    return rc;
}

void InsertSameSeg(AstParse *pParse,LineSeg *p,LineSeg *pSeg)
{
    PlaneSeg *pPSeg;
    GeomType ele1;
    GeomType ele2;
    LineData *pLine1;
    LineData *pLine2;
    int rc = 0;

    pLine1 = pSeg->pLine;
    pLine2 = p->pLine;
    pPSeg = GetPlaneSeg(pLine1,pLine2);
    if(pPSeg==NULL){
        ele1 = GetLineEle(pLine1);
        ele2 = GetLineEle(pLine2);
        pPSeg = SetPlaneHash(pParse,&ele1,&ele2);
        InsertSamePair(pParse,pPSeg,&p,&pSeg);
    }
    else
    {
        rc = HaveSamePair(pPSeg->pSame,p,pSeg);
        if(!rc){
            InsertSamePair(pParse,pPSeg,&p,&pSeg);
        }
    }
}

void CheckOtherPair(AstParse *pParse,SameLine *pPair)
{
    LineSeg *p;

    p = pPair->pSeg1->pNext;
    while(p!=pPair->pSeg1){
        InsertSameSeg(pParse,p,pPair->pSeg1);
        p = p->pNext;
    };
    p = pPair->pSeg2->pNext;
    while(p!=pPair->pSeg2){
        InsertSameSeg(pParse,p,pPair->pSeg2);
        p = p->pNext;
    };
}

void SetEqualAngle(SameAngle *pA,AngleTemp *pTemp,int dir)
{
    AstParse *pParse = pTemp->pParse;
    PlaneSeg *apPSeg[2];
    LineSeg* apPairSeg;
    LineSeg *apSideSeg[2];
    PoinData *pPoint1;
    PoinData *pPoint2;

    pPoint1 = pA->pSeg1->pCorner->pVertex;
    pPoint2 = pA->pSeg2->pCorner->pVertex;
    apSideSeg[0] = CreateNewLine(pParse,pTemp->apSide[0][dir],pPoint1);
    apSideSeg[1] = CreateNewLine(pParse,pTemp->apSide[1][dir],pPoint2);
    apPSeg[0] = GetAndSetAngle(pParse,pTemp->apSide[0][1-dir],apSideSeg[0]);
    apPSeg[1] = GetAndSetAngle(pParse,pTemp->apSide[1][1-dir],apSideSeg[1]);

    apPairSeg = CreateNewLine(pParse,pTemp->apSide[0][1-dir],pTemp->apSide[1][1-dir]);
    InsertAnglePair(pParse,apPairSeg,apPSeg[0],apPSeg[1]);
}

void SetSasEqual(SameAngle *pA,AngleTemp *pTemp)
{
    PoinData *pPoint1;
    PoinData *pPoint2;
    LineSeg* apSeg[2];
    PlaneSeg *apSameSeg;
    AstParse *pParse = pTemp->pParse;
    GeomType ele1;
    GeomType ele2;
    TempInfo tmp;
    PoinData *apLink[2];
    TrigInfo *apTriag[2];

    pPoint1 = pA->pSeg1->pCorner->pVertex;
    pPoint2 = pA->pSeg2->pCorner->pVertex;

    tmp.apPoint[0] = pPoint1;
    tmp.apPoint[1] = pTemp->apSide[0][0];
    tmp.apPoint[2] = pTemp->apSide[0][1];
    apTriag[0] = SetTriangleObj(pParse,&tmp);
    apLink[0] = tmp.apPoint[0];
    tmp.apPoint[0] = pPoint2;
    tmp.apPoint[1] = pTemp->apSide[1][0];
    tmp.apPoint[2] = pTemp->apSide[1][1];
    apTriag[1] = SetTriangleObj(pParse,&tmp);
    apLink[1] = tmp.apPoint[0];
    apSeg[0] = CreateNewLine(pParse,apLink[0],apLink[1]);
    if(InsertCommonPair(pParse,&apSeg[0]->pTriSame,apTriag[0],apTriag[1]))
    {
        log_c("SAS:%s%s%s ",pTemp->apSide[0][0]->zSymb,
                            pPoint1->zSymb
                            ,pTemp->apSide[0][1]->zSymb);
        log_a("%s%s%s ",pTemp->apSide[1][0]->zSymb,
                            pPoint2->zSymb
                            ,pTemp->apSide[1][1]->zSymb);
        apSeg[0] = CreateNewLine(pParse,pTemp->apSide[0][0],pTemp->apSide[0][1]);
        apSeg[1] = CreateNewLine(pParse,pTemp->apSide[1][0],pTemp->apSide[1][1]);
        ele1 = GetLineEle(apSeg[0]->pLine);
        ele2 = GetLineEle(apSeg[1]->pLine);
        apSameSeg = SetPlaneHash(pParse,&ele1,&ele2);
        InsertSamePair(pParse,apSameSeg,&apSeg[0],&apSeg[1]);

        SetEqualAngle(pA,pTemp,0);
        SetEqualAngle(pA,pTemp,1);
    }
}

void CheckSAS(PlaneSeg *pPSeg,SameLine *pS,SameAngle *pA,AngleTemp *pTemp)
{
    PoinData *apLeft[2];
    PoinData *apRight[2];
    LinkNode *p;
    SameLine *pPair;
    PoinData *pPoint1;
    PoinData *pPoint2;
    int i,j;

    pPoint1 = pA->pSeg1->pCorner->pVertex;
    pPoint2 = pA->pSeg2->pCorner->pVertex;
    p = pPSeg->pSame;
    do{
        pPair = (SameLine *)p->pVal;
        apLeft[0] = pPair->pSeg1->pLeft;
        apLeft[1] = pPair->pSeg1->pRight;
        apRight[0] = pPair->pSeg2->pLeft;
        apRight[1] = pPair->pSeg2->pRight;
        for(i=0; i<2; i++){
            for(j=0; j<2; j++){
                if(pPoint1==apLeft[i] && pPoint2==apRight[j]){
                    pTemp->apSide[0][1] = apLeft[1-i];
                    pTemp->apSide[1][1] = apRight[1-j];
                    PrintSameLine(pPair);
                    SetSasEqual(pA,pTemp);
                }
                else if(pPoint2==apLeft[i] && pPoint1==apRight[j]){
                    pTemp->apSide[0][1] = apRight[1-j];
                    pTemp->apSide[1][1] = apLeft[1-i];
                    PrintSameLine(pPair);
                    SetSasEqual(pA,pTemp);
                }
            }
        }
        p = p->pNext;
    }while(!p->isHead);
}

void CheckLineInAngle(AstParse *pParse,SameLine *pS,SameAngle *pA,AngleTemp *pTmp)
{
    LineData *apLine[2][2];
    LineData *pLine1;
    LineData *pLine2;
    PlaneSeg *pPSeg;
    int i,j;

    apLine[0][0] = pA->pSeg1->pCorner->pLine1;
    apLine[0][1] = pA->pSeg1->pCorner->pLine2;
    apLine[1][0] = pA->pSeg2->pCorner->pLine1;
    apLine[1][1] = pA->pSeg2->pCorner->pLine2;
    pLine1 = pS->pSeg1->pLine;
    pLine2 = pS->pSeg2->pLine;
    for(i=0; i<2; i++){
        for(j=0; j<2; j++){
            if((apLine[0][i]==pLine1 && apLine[1][j]==pLine2) ||
               (apLine[0][i]==pLine2 && apLine[1][j]==pLine1))
            {
                pPSeg = GetPlaneSeg(apLine[0][1-i],apLine[1][1-j]);
                if(pPSeg->pSame){
                    CheckSAS(pPSeg,pS,pA,pTmp);
                }
            }
        }
    }
}

void CheckSameAngle(AstParse *pParse,SameLine *pS)
{
    LineSeg *pSeg;
    SameAngle *pPair;
    LinkNode *p;
    PoinData *pPoint1;
    PoinData *pPoint2;
    PoinData *apPoint[2][2];
    AngleTemp tmp = {0};
    int i,j;

    tmp.pParse = pParse;
    apPoint[0][0] = pS->pSeg1->pLeft;
    apPoint[0][1] = pS->pSeg1->pRight;
    apPoint[1][0] = pS->pSeg2->pLeft;
    apPoint[1][1] = pS->pSeg2->pRight;

    for(i=0;i<2;i++){
        for(j=0;j<2;j++){
            pPoint1 = apPoint[0][i];
            pPoint2 = apPoint[1][j];
            pSeg = *(GetLineSegAddr(pPoint1,pPoint2));
            if(pSeg!=NULL && pSeg->pSame!=NULL){
                p = pSeg->pSame;
                do{
                    pPair = (SameAngle *)p->pVal;
                    if(pPoint1==pPair->pSeg1->pCorner->pVertex){
                        tmp.apSide[0][0] = apPoint[0][1-i];
                        tmp.apSide[1][0] = apPoint[1][1-j];
                    }
                    else{
                        tmp.apSide[0][0] = apPoint[1][1-j];
                        tmp.apSide[1][0] = apPoint[0][1-i];
                    }
                    log_a("check same angle:");
                    PrintSameAngle(pPair);
                    CheckLineInAngle(pParse,pS,pPair,&tmp);
                    p = p->pNext;
                }while(!p->isHead);
            }
        }
    }
}

void InsertSamePair(AstParse *pParse,PlaneSeg *pPSeg,
        LineSeg **ppSeg1,
        LineSeg **ppSeg2)
{
    SameLine *pPair;

    if(HaveSamePair(pPSeg->pSame,*ppSeg1,*ppSeg2))
    {
        return;
    }
    else
    {
        pPair = SetSamePair(pParse,ppSeg1,ppSeg2);
    }
    if(pPSeg->pSame==NULL){
        pPSeg->pSame =  NewLinkHead(pPair,sizeof(LinkNode));
//        printf("head %p\n",pPSeg->pSame);
    }
    else{
        CheckNewSame(pParse,pPSeg->pSame,pPair);
        InsertLinkNode(pParse,pPSeg->pSame,pPair);
        CheckOtherPair(pParse,pPair);
//        printf("pSame %p\n",pPSeg->pSame);
//        PrintSameLine(pPSeg->pSame->pNext->pVal);
//        printf("next %p\n",pPSeg->pSame->pNext);
    }
    printf("++++++\n");
    PrintSameLine(pPair);
    CheckSameAngle(pParse,pPair);
    printf("-------\n");
}

void InsertAnglePair(AstParse *pParse,LineSeg *pSeg,
        PlaneSeg *pPSeg1,
        PlaneSeg *pPSeg2)
{
    SameAngle *pAPair;

    if(HaveSamePair(pSeg->pSame,(LineSeg *)pPSeg1,(LineSeg *)pPSeg2))
    {
        return;
    }
    else
    {
        pAPair = SameAnglePair(pParse,pPSeg1,pPSeg2);
    }

    if(pSeg->pSame==NULL){
        pSeg->pSame =  NewLinkHead(pAPair,sizeof(LinkNode));
    }
    else{
        InsertLinkNode(pParse,pSeg->pSame,pAPair);
        //todo CheckAngleSame
    }
    PrintSameAngle(pAPair);
}

int InsertCommonPair(AstParse *pParse,LinkNode **ppSame,
        void *pSeg1,
        void *pSeg2)
{
    CommanPair *pPair;

    if(HaveSamePair(*ppSame,(LineSeg *)pSeg1,(LineSeg *)pSeg2))
    {
        return 0;
    }
    else
    {
        pPair = (CommanPair *)Malloc(sizeof(CommanPair));
        pPair->pSeg1 = pSeg1;
        pPair->pSeg2 = pSeg2;
    }

    if(*ppSame==NULL){
        *ppSame =  NewLinkHead(pPair,sizeof(LinkNode));
    }
    else{
        InsertLinkNode(pParse,*ppSame,pPair);
    }
    return 1;
}

PlaneSeg *GetAndSetAngle(AstParse *pParse,PoinData *pVertex,LineSeg* pSeg)
{
    GeomType ele1;
    GeomType ele2;
    GeomType ele;
    PlaneSeg *pPSeg;
    ele1 = GetPointEle(pVertex);
    ele2 = GetLineEle(pSeg->pLine);
    ele2.pPoint1 = pSeg->pLeft;
    ele2.pPoint2 = pSeg->pRight;
    ele = GetAngleEle(pParse,&ele1,&ele2);
    pPSeg = SetAngleHash(pParse,&ele,NULL);
    return pPSeg;
}

void SetEqualTrgl(AstParse *pParse,TempInfo *pTemp)
{
    LineSeg* apSeg[3];
    PlaneSeg *apPSeg[3];

    apSeg[0] = CreateNewLine(pParse,pTemp->apPoint[1],pTemp->apPoint[2]);
    apSeg[1] = CreateNewLine(pParse,pTemp->apPoint[0],pTemp->apPoint[2]);
    apSeg[2] = CreateNewLine(pParse,pTemp->apPoint[0],pTemp->apPoint[1]);

    apPSeg[1] = GetAndSetAngle(pParse,pTemp->apPoint[1],apSeg[1]);
    InsertSamePair(pParse,apPSeg[1],&apSeg[0],&apSeg[2]);
    apPSeg[2] = GetAndSetAngle(pParse,pTemp->apPoint[2],apSeg[2]);
    InsertSamePair(pParse,apPSeg[2],&apSeg[1],&apSeg[0]);

    apPSeg[0] = GetPlaneSeg(apSeg[2]->pLine,apSeg[1]->pLine);

    InsertAnglePair(pParse,apSeg[2],apPSeg[1],apPSeg[0]);
    InsertAnglePair(pParse,apSeg[1],apPSeg[2],apPSeg[0]);
}

void SetSameSeg(AstParse *pParse,PlaneSeg *pPSeg,GeomType *pLeft,GeomType *pRight)
{
    TempInfo tmp;
    PlaneSeg* pSeg;
    CornerInfo *pCorner;
    int val;//角度
    LineSeg **ppSeg1;
    LineSeg **ppSeg2;

    ppSeg1 = GetLineSegAddr(pLeft->pPoint1,pLeft->pPoint2);
    ppSeg2 = GetLineSegAddr(pRight->pPoint1,pRight->pPoint2);

    InsertSamePair(pParse,pPSeg,ppSeg1,ppSeg2);
    if(HavePubPoint(*ppSeg1,*ppSeg2,&tmp)){
        //tmp.apPoint[0]是顶点
        pSeg = GetPlaneSeg((*ppSeg1)->pLine,(*ppSeg2)->pLine);
        pCorner = pSeg->pCorner;
        if(pCorner->pLine1!=(*ppSeg1)->pLine){
            SwapTemp(&tmp);
        }
        val = pCorner->val;
        assert(val<=180);
        if(tmp.a!=pCorner->bl){
            val = 180-val;
        }
        if(tmp.b!=pCorner->br){
            val = 180-val;
        }
        if(val==DEG_60){
            SetEqualTrgl(pParse,&tmp);
        }
        //printf("corner %d\n",val);
    }
}

PlaneSeg *SetPlaneHash(AstParse *pParse,GeomType *pLeft,GeomType *pRight)
{
    int iLeft;
    int iRight;
    PlaneData *pPlane;
    PlaneSeg *pSeg;

    assert(pLeft->type==ELE_LINE);
    assert(pRight->type==ELE_LINE);

    iLeft =pLeft->pLine1->iNum;
    iRight = pRight->pLine1->iNum;
    if(iLeft>iRight){
        if(pLeft->pLine1->ppSeg[iRight]==NULL){
            pSeg = NewPlaneObj(pParse,pLeft->pLine1,iRight);
            pPlane = pSeg->pPlane;
            pPlane->pHead->pVal = pLeft->pLine1;
            InsertLinkNode(pParse,pPlane->pHead,pRight->pLine1);
//            InsertLinkNode(pParse,pPlane->pHead,pLeft->pLine1);
//            InsertLinkNode(pParse,pPlane->pHead->pNext,pRight->pLine1);
        }
        else{
            pSeg = pLeft->pLine1->ppSeg[iRight];
            pPlane = pSeg->pPlane;
        }
    }
    else{
        if(pRight->pLine1->ppSeg[iLeft]==NULL){
            pSeg = NewPlaneObj(pParse,pRight->pLine1,iLeft);
            pPlane = pSeg->pPlane;
            pPlane->pHead->pVal = pLeft->pLine1;
            InsertLinkNode(pParse,pPlane->pHead,pRight->pLine1);
//            InsertLinkNode(pParse,pPlane->pHead,pLeft->pLine1);
//            InsertLinkNode(pParse,pPlane->pHead->pNext,pRight->pLine1);
        }
        else{
            pSeg = pRight->pLine1->ppSeg[iLeft];
            pPlane = pSeg->pPlane;
        }
    }
    return pSeg;
}

GeomType SetGeomHash(AstParse *pParse,TokenInfo *pAst)
{
    PoinData *pPoint;
    u16 key;
    GeomType ele = {0};
    GeomType ele1 = {0};
    GeomType ele2 = {0};
    PointHash *pSet = pParse->pPointSet;
    PlaneSeg *pSeg;

    if(pAst->type==PROP_SYMB)
    {
        if(pSet->nPoint>pSet->nHash/2-10){
            //todo resizeHash
            assert(0);
        }
        key = ((*((u16 *)pAst->zSymb))*383)&(pSet->nHash-1);
        for(;pSet->ppHash[key];key=((key+1)&(pSet->nHash-1))){
            if(strcmp(pAst->zSymb,pSet->ppHash[key]->zSymb))
            {
                continue;//Collide
            }
            else
            {
                ele = GetPointEle(pSet->ppHash[key]);
                return ele;//hash have the element
            }
        }
        pPoint = (PoinData *)Malloc(sizeof(PoinData));
        memset(pPoint,0,sizeof(PoinData));
        pPoint->iNum = pSet->nPoint++;
        pSet->ppArray[pPoint->iNum] = pPoint;
        pPoint->zSymb = (char*)Malloc(pAst->nSymbLen+1);
        memcpy(pPoint->zSymb,pAst->zSymb,pAst->nSymbLen+1);
        pPoint->ppSeg = (LineSeg**)Malloc(sizeof(LineSeg*)*(pPoint->iNum+1));
        memset(pPoint->ppSeg, 0, sizeof(LineSeg*)*(pPoint->iNum+1));
        pSet->ppHash[key] = pPoint;
        ele = GetPointEle(pSet->ppHash[key]);
        log_a("sym %s key %d num %d",pPoint->zSymb,key,pPoint->iNum);
    }
    else if(pAst->type==PROP_IMPL)
    {
        ele1 = SetGeomHash(pParse,pAst->pLeft);
        if(memcmp(pAst->zSymb,"val",3)!=0){
            ele2 = SetGeomHash(pParse,pAst->pRight);
            switch(pAst->op){
            case OP_LINE:
                ele = SetLineHash(pParse,&ele1,&ele2);
                break;
            case OP_IMPL:
                ele = GetAngleEle(pParse,&ele1,&ele2);
                break;
            case OP_EQUAL:
                pSeg = SetPlaneHash(pParse,&ele1,&ele2);
                SetSameSeg(pParse,pSeg,&ele1,&ele2);
                break;
            default:
                break;
            }

        }
        else if(ele1.type==ELE_ANGLE){
            SetAngleHash(pParse,&ele1,pAst->pRight);
        }
    }
    else
    {
        assert(0);
    }
    return ele;
}

void ParseGeomEle(AstParse *pParse,Vector *pSet)
{
    int i = 0;
    for(i=0;i<pSet->n;i++)
    {
        SetGeomHash(pParse,pSet->data[i]);
    }
}
