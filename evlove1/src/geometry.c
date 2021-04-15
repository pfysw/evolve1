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
    pPointSet->ppArray = (PointData **)Malloc(size);
    memset(pPointSet->ppArray,0,size);
    size = sizeof(PointHash *)*pPointSet->nHash;
    pPointSet->ppHash = (PointData **)Malloc(size);
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

void FreeLinePoint(AstParse *pParse,LineData *pLine)
{
    LinePoint *p = pLine->pHead->pNext;
    LinePoint *pTmp;

//      static int jj = 0;
//      jj++;
//     if(jj==4)
//          printf("jj %d\n",jj);

    if(gDebug.freePrint){
        log_c("line %d: ",pLine->iNum);
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

void FreeLineSeg(PointData *pArray)
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

void PrintLine(AstParse *pParse,LineData *pLine)
{
    LinePoint *p;
    int i;
    if(pLine->bImage){
        pLine = pParse->pLineSet->ppLine[pLine->iNum];
    }
    p = pLine->pHead->pNext;
    for(i=0;i<2;i++){
        log_c("%s",p->pPoint->zSymb);
        p = p->pNext;
    }
    log_c(" ");
}


void PrintPlaneLine(PointData *pVertex)
{
    PlaneData *pPlane = pVertex->pPlane;
    LinkNode *p;
    AstParse *pParse = pVertex->pParse;
    log_a("plane center %s:",pVertex->zSymb);
    p=pPlane->pHead;
    do{
        PrintLine(pParse,p->pVal);
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
    CornerInfo *pCorner;

    for(i=0;i<pArray->iNum+1;i++)
    {
        if(pArray->ppSeg[i]!= NULL){
            if(pArray->ppSeg[i]->pCorner!=NULL){
                if(gDebug.freePrint){
                    pCorner = pArray->ppSeg[i]->pCorner;
                    if(!pCorner->pVertex->isInf){
                        log_a("angle: %s%s%s",pCorner->pLeft->zSymb,
                                pCorner->pVertex->zSymb,
                                pCorner->pRight->zSymb);
                    }

                }
            }
            FreeSamePair(pArray->ppSeg[i]->pSame,ELE_LINE);
            if(pArray->ppSeg[i]->pCorner){
                Free(pArray->ppSeg[i]->pCorner);
            }
            assert(pArray->ppSeg[i]->isHead);
            FreeLinkNode((LinkNode *)pArray->ppSeg[i],0);
            //Free(pArray->ppSeg[i]);
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
        if(!pLineSet->ppLine[i]->bImage){
            FreeLinePoint(pParse,pLineSet->ppLine[i]);
            FreePlaneSeg(pLineSet->ppLine[i]);
        }
        else if(pLineSet->ppLine[i]->pHead){
            FreeLinkNode((LinkNode *)pLineSet->ppLine[i]->pHead,0);
            //FreeLinePoint(pParse,pLineSet->ppLine[i]);
        }
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

PointData *NewPointObj(AstParse *pParse)
{
    PointData *pPoint;
    PointHash *pSet = pParse->pPointSet;

    pPoint = (PointData *)Malloc(sizeof(PointData));
    memset(pPoint,0,sizeof(PointData));
    pPoint->iNum = pSet->nPoint++;
    pPoint->pParse = pParse;
    pSet->ppArray[pPoint->iNum] = pPoint;
    pPoint->ppSeg = (LineSeg**)Malloc(sizeof(LineSeg*)*(pPoint->iNum+1));
    memset(pPoint->ppSeg, 0, sizeof(LineSeg*)*(pPoint->iNum+1));
    return pPoint;
}

LinePoint *NewPointNode(PointData *pPoint)
{
    LinePoint *p;
    p = (LinePoint *)Malloc(sizeof(LinePoint));
    memset(p,0,sizeof(*p));
    p->pPoint = pPoint;
    return p;
}

LinePoint *NewPointHead(PointData *pPoint)
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

void InsertPointNode(AstParse *pParse,LinePoint *pPre,PointData *pNode)
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

LinePoint *FindPointLoc(LinePoint *pHead,PointData *pNode)
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


LineSeg** GetLineSegAddr(PointData *pLeft,PointData *pRight)
{
    int iLeft;
    int iRight;
    iLeft = pLeft->iNum;
    iRight = pRight->iNum;
    if(iLeft>iRight){
        if(pLeft->ppSeg[iRight]){
            assert(!pLeft->ppSeg[iRight]->pLine->bImage);
        }
        return &pLeft->ppSeg[iRight];
    }
    else
    {
        if(pRight->ppSeg[iLeft]){
            assert(!pRight->ppSeg[iLeft]->pLine->bImage);
        }
        return &pRight->ppSeg[iLeft];
    }
}

PlaneSeg** GetPlaneSegAddr(AstParse *pParse,LineData *pLeft,LineData *pRight)
{
    int iLeft;
    int iRight;
    LineData **ppLine;

    ppLine = pParse->pLineSet->ppLine;
    iLeft = pLeft->iNum;
    iRight = pRight->iNum;
    if(iLeft>iRight){
        return &ppLine[iLeft]->ppSeg[iRight];
    }
    else
    {
        return &ppLine[iRight]->ppSeg[iLeft];
    }
}

void FreeLine(AstParse *pParse,LineData *pLine)
{
    LineHash *pLineSet = pParse->pLineSet;
    pLineSet->ppLine[pLine->iNum] = NULL;
    //pLineSet->nLine--;//todo 有些线条在中途被释放了怎么办
    //FreeLinePoint(pParse,pLine->pHead);//只释放公共点，其他点链接到新直线上
    FreePlaneSeg(pLine);
    Free(pLine);
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

void LinkLineSame(AstParse *pParse,PlaneSeg** ppSeg1,PlaneSeg** ppSeg2)
{
    if((*ppSeg1)->pSame!=(*ppSeg2)->pSame)
    {
        if((*ppSeg1)->pSame!=NULL&&(*ppSeg2)->pSame!=NULL){
            LinkSegNode(pParse, (LinkNode *)(*ppSeg1)->pSame,
                    (LinkNode *)(*ppSeg2)->pSame);
        }
        else if((*ppSeg2)->pSame!=NULL){
            (*ppSeg1)->pSame = (*ppSeg2)->pSame;
        }
    }
}

void ResetPlaneSeg(AstParse *pParse,LineData *pLine1,LineData *pLine2)
{
    int i = 0;
    LineHash *pLineSet = pParse->pLineSet;
    PlaneSeg** ppSeg1;
    PlaneSeg** ppSeg2;

    assert(pLine1->iNum!=pLine2->iNum);
    for(i=0;i<pLineSet->nLine;i++)
    {
        if(pLineSet->ppLine[i]->bImage) continue;

        ppSeg2 = GetPlaneSegAddr(pParse,pLine2,pLineSet->ppLine[i]);
        if(*ppSeg2!= NULL){
            //pLine2->iNum：5 pLineSet->ppLine[i]->iNum：5  pLine1->iNum:3
            //5-5 不应该转移到5-3，因为直线5已经转移了，应该把5-5转移到3-3
            if(pLine2->iNum==pLineSet->ppLine[i]->iNum){
                ppSeg1 = GetPlaneSegAddr(pParse,pLine1,pLine1);
            }
            else{
                ppSeg1 = GetPlaneSegAddr(pParse,pLine1,pLineSet->ppLine[i]);
            }

            if((*ppSeg1)==NULL){
//                log_a("ppLine null %d:",i);
//                TravLinePoint(pLineSet->ppLine[i]);
                *ppSeg1 = *ppSeg2;
                *ppSeg2 = NULL;
            }
            else{
                assert(*ppSeg1!=*ppSeg2);
//                log_a("ppLine %d:",i);
//                TravLinePoint(pLineSet->ppLine[i]);
                LinkLineSame(pParse,ppSeg1,ppSeg2);
                LinkSegNode(pParse,(LinkNode *)*ppSeg1,(LinkNode *)*ppSeg2);
                if((*ppSeg2)->pCorner!=NULL){
                    //不可能出现2条直线交于2点
                    assert((*ppSeg1)->pCorner==NULL);
                    (*ppSeg1)->pCorner = (*ppSeg2)->pCorner;
                    (*ppSeg2)->pCorner = NULL;
                }
            }
        }
    }
    Free(pLine2->ppSeg);
    pLine2->ppSeg = NULL;
    //-------------------------
    pLine2->iNum = pLine1->iNum;
    pLine2->bImage = 1;
}

PointData *GetIntersection(LineData *pLine1,LineData *pLine2)
{
    LinePoint *p1,*p2;
    PointData *pPoint = NULL;
    if(pLine1->iNum==pLine2->iNum){
        return pPoint;
    }
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

void MergeTwoLinePoint(LineData *pLine1,LineData *pLine2,PointData *pRigth)
{
    LinePoint *p1;
    LinePoint *p2;
    LinePoint *p;
    AstParse *pParse;
    PointData *pInter;

    printf("--1:%d--\n",pLine1->iNum);
    TravLinePoint(pLine1);
    printf("--2:%d--\n",pLine2->iNum);
    TravLinePoint(pLine2);
    pInter = GetIntersection(pLine1,pLine2);
    assert(pInter!=NULL);
    pParse = pInter->pParse;
    p1 = (LinePoint *)GetLinkNode((LinkNode*)pLine1->pHead,pInter);
    p2 = (LinePoint *)GetLinkNode((LinkNode*)pLine2->pHead,pInter);
    assert(!pLine1->pHead->pNext->isHead);
    assert(!pLine2->pHead->pNext->isHead);
    //pLine2头部接在p1前
    if(pLine2->pHead->pNext!=p2){
        for(p=pLine2->pHead->pNext;p!=p2;p=p->pNext){
            CheckNewSeg(pParse,p->pPoint,pLine1);
        }
        p1->pPre->pNext = pLine2->pHead->pNext;
        pLine2->pHead->pNext->pPre = p1->pPre;
        p1->pPre = p2->pPre;
        p2->pPre->pNext = p1;
    }
    //pLine2尾部接在p1后
    if(pLine2->pHead->pPre!=p2){
        for(p=p2->pNext;!p->isHead;p=p->pNext){
            CheckNewSeg(pParse,p->pPoint,pLine1);
        }
        //(A-B)-C  pRigth是B，p1是A，C应该接到B的右边
        if(pRigth){
            p1 = (LinePoint *)GetLinkNode((LinkNode*)pLine1->pHead,pRigth);
        }
        p1->pNext->pPre = pLine2->pHead->pPre;
        pLine2->pHead->pPre->pNext = p1->pNext;
        p1->pNext = p2->pNext;
        p2->pNext->pPre = p1;
    }
    if(pLine2->pHead->pPoint!=NULL){
        assert(pLine1->pHead->pPoint==NULL);
        pLine1->pHead->pPoint = pLine2->pHead->pPoint;
    }
    //Free(pLine2->pHead);
    Free(p2);
    //后面统一释放，有些地方不仅只有head一个结点
    pLine2->pHead->pNext = pLine2->pHead;
    pLine2->pHead->pPre = pLine2->pHead;

   // ResetPlaneSeg(pParse,pLine1,pLine2);
//    printf("--r--\n");
//    TravLinePoint(pLine1);
//    printf("pR %s\n",pRigth->zSymb);
//    printf("p1 %s\n",p1->pPoint->zSymb);
//    printf("p1 next %s\n",p1->pNext->pPoint->zSymb);
}


LineSeg *NewLineObj(AstParse *pParse,PointData *pLeft,int iRight)
{
    LineData *pNew;
    LineSeg *pSeg;
    LineHash *pLineSet = pParse->pLineSet;
    pNew = (LineData*)Malloc(sizeof(LineData));
    memset(pNew,0,sizeof(LineData));
    pSeg = (LineSeg*)NewLinkHead(pNew,sizeof(LineSeg));
    pLeft->ppSeg[iRight] = pSeg;
    pNew->iNum = pLineSet->nLine;
    if(pLineSet->nLine>pLineSet->nSlot){
        //todo resizeHash
        assert(0);
    }
    pLineSet->ppLine[pNew->iNum] = pNew;
    pLineSet->nLine++;
    pNew->pHead = NewPointHead(NULL);
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

PlaneSeg *NewPlaneSeg(LineData *pLeft,int iRight,PlaneData *pPlane)
{
    PlaneSeg *pSeg;
    pSeg = (PlaneSeg*)NewLinkHead(pPlane,sizeof(PlaneSeg));
    pLeft->ppSeg[iRight] = pSeg;
    pLeft->nArray++;
    return pSeg;
}

GeomType GetPointEle(PointData *p)
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

void SetSegPoint(LineSeg *pSeg,PointData *pLeft,PointData *pRight)
{
    pSeg->pLeft = pLeft;
    pSeg->pRight = pRight;
}

//如果已经创建了则直接返回
LineSeg *CreateNewLine(AstParse *pParse,PointData *pPoint1,PointData *pPoint2)
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

void InsertSegPoint(AstParse *pParse,
                     LineSeg *pLineSeg,//被插入的线段
                     PointData *pInsert,
                     u8 bInsertPre)
{
    LinePoint *pLoc;
    if(bInsertPre){
        pLoc = FindPointLoc(pLineSeg->pLine->pHead,pLineSeg->pLeft);
        InsertPointNode(pParse,pLoc->pPre,pInsert);
    }
    else{
        //pRight->pPoint1插入到pLeft->pLine1后面
        pLoc = FindPointLoc(pLineSeg->pLine->pHead,pLineSeg->pRight);
        InsertPointNode(pParse,pLoc,pInsert);
    }
}

LineSeg *SetLineSegPoint(LineSeg *pLineSeg,PointData *pLeft,PointData *pRight)
{
    LineSeg* pSeg;
    pSeg = (LineSeg*)NewLinkHead(pLineSeg->pLine,sizeof(LineSeg));
    SetSegPoint(pSeg,pLeft,pRight);
    return pSeg;
}

void CheckNewSeg(AstParse *pParse,PointData *pNew,LineData *pLine)
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
        else if((*ppLSeg)->pLine != pLine)
        {
            assert((*ppLSeg)->pLine!=NULL);
            assert(pLine!=NULL);
            ResetPlaneSeg(pParse,pLine,(*ppLSeg)->pLine);
            (*ppLSeg)->pLine = pLine;
        }
        p = p->pNext;
    }
}

int GetLinesegDirect(LineData *pLine,PointData *pPoint1,PointData *pPoint2)
{
    LinePoint *p;
    int rc = 0;
    for(p=pLine->pHead;;)
    {
        if(p->pPoint==pPoint1){
            rc = FORWARD_DIRECT;
            break;
        }
        else if(p->pPoint==pPoint2){
            rc = BACK_DIRECT;
            break;
        }
        p = p->pNext;
        if(p->isHead){
            break;
        }
    }
    assert(rc!=0);
    return rc;
}


GeomType SetLineHash(AstParse *pParse,GeomType *pLeft,GeomType *pRight)
{
    GeomType ele = {0};
    LineData *pLine;
    LineSeg** ppLs1;
    LineSeg** ppLs2;
    LineSeg *pSeg;
    int dir1,dir2;

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
        pSeg = CreateNewLine(pParse,pRight->pPoint1,pRight->pPoint2);
        InsertSegPoint(pParse,pSeg,pLeft->pPoint1,0);
        *ppLs1 = SetLineSegPoint(pSeg,pLeft->pPoint1,pRight->pPoint1);
        *ppLs2 = SetLineSegPoint(pSeg,pLeft->pPoint1,pRight->pPoint2);
        CheckNewSeg(pParse,pLeft->pPoint1,(*ppLs1)->pLine);
    }
    else if(pLeft->type==ELE_LINE)
    {
        ppLs1 = GetLineSegAddr(pRight->pPoint1,pLeft->pPoint1);
        ppLs2 = GetLineSegAddr(pRight->pPoint1,pLeft->pPoint2);

        if(*ppLs1!=NULL){
            MergeTwoLinePoint(pLeft->pLine1,(*ppLs1)->pLine,pLeft->pPoint2);
            (*ppLs1)->pLine = pLeft->pLine1;
        }
        else if(*ppLs2!=NULL){
            MergeTwoLinePoint(pLeft->pLine1,(*ppLs2)->pLine,NULL);
            (*ppLs2)->pLine = pLeft->pLine1;
        }
        else
        {
            pSeg = CreateNewLine(pParse,pLeft->pPoint1,pLeft->pPoint2);
            InsertSegPoint(pParse,pSeg,pRight->pPoint1,0);
            *ppLs1 = SetLineSegPoint(pSeg,pLeft->pPoint1,pRight->pPoint1);
            *ppLs2 = SetLineSegPoint(pSeg,pLeft->pPoint2,pRight->pPoint1);
            CheckNewSeg(pParse,pRight->pPoint1,(*ppLs1)->pLine);
        }
        dir1 = GetLinesegDirect(pLeft->pLine1,pLeft->pPoint1,pLeft->pPoint2);
        dir2 = GetLinesegDirect(pLeft->pLine1,pLeft->pPoint2,pRight->pPoint1);
        assert(dir1==dir2);
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

LinePoint *GetLinePoint(LinePoint *pHead,PointData *pVal)
{
    LinkNode *p;
    p = GetLinkNode((LinkNode *)pHead,pVal);
    return (LinePoint *)p;
}


int isSameLine(void *p1,void *p2){
    int rc = 0;
    if(((LineData *)p1)->iNum==((LineData *)p2)->iNum){
        rc = 1;
    }
    return rc;
}
LinkNode *GetLinkSame(
        LinkNode *pHead,
        void *pVal,
        int (*xSame)(void*,void*))
{
    LinkNode *p;
    LinkNode *pFind = NULL;
    p = pHead;
    do{
        if(xSame(p->pVal,pVal)){
            pFind = p;
            break;
        }
        p = p->pNext;
    }while(!p->isHead);
    return pFind;
}

int InsertPlaneLine(
        AstParse *pParse,
        PointData *pVertex,
        PointData *pPoint,
        LineData *pBase)
{
    LinePoint *pNode;
    LinePoint *p;
    LineData *pLine;
    LineSeg *pRSeg;
    LineSeg *pLSeg;
    LinkNode *pLineNode;
    int rc = 0;


    pLine = (*GetLineSegAddr(pPoint,pVertex))->pLine;
    assert(pLine!=NULL);
    if(GetLinkSame(pVertex->pPlane->pHead,pLine,isSameLine))
    {
        rc = 1;
        goto insert_end;
    }

    pNode = (LinePoint *)GetLinkNode((LinkNode *)(pBase->pHead),(void*)pPoint);

//    LinkNode *p1;
//    int iNum;
//    for(p1=pVertex->pPlane->pHead;;)
//    {
//        iNum = ((LineData *)p1->pVal)->iNum;
//        pLine = pParse->pLineSet->ppLine[iNum];
//        PrintLine(pParse,pLine);
//        p1 = p1->pNext;
//        if(p1->isHead)break;
//    }
    for(p=pNode->pNext;;p=p->pNext){
        assert(p!=pNode);
        if(p->pPoint==NULL){
            assert(p->isHead);
            continue;
        }
        pRSeg = *GetLineSegAddr(p->pPoint,pVertex);
        if(pRSeg!=NULL){
            if(GetLinkSame(pVertex->pPlane->pHead,pRSeg->pLine,isSameLine)){
                break;
            }
        }
    }
    for(p=pNode->pPre;;p=p->pPre){
        assert(p!=pNode);
        if(p->pPoint==NULL){
            assert(p->isHead);
            continue;
        }
        pLSeg = *GetLineSegAddr(p->pPoint,pVertex);
        if(pLSeg!=NULL){
            if(GetLinkSame(pVertex->pPlane->pHead,pLSeg->pLine,isSameLine)){
                break;
            }
        }
    }
    //assert(pRSeg!=pLSeg);
    if(pRSeg==pLSeg){
        goto insert_end;
    }
    pLineNode = GetLinkSame(pVertex->pPlane->pHead,pLSeg->pLine,isSameLine);
   // if(pLineNode->pNext->pVal==pRSeg->pLine)
    if(isSameLine(pLineNode->pNext->pVal,pRSeg->pLine))
    {
        InsertLinkNode(pParse,pLineNode,pLine);
    }
    else{
        assert(isSameLine(pLineNode->pPre->pVal,pRSeg->pLine));
        InsertLinkNode(pParse,pLineNode->pPre,pLine);
    }
    rc = 1;
insert_end:
    return rc;
}

/*
 * 三角形ABC，O3是AB的中点，过一点B做CO3的平行线,D在这条平行线上
 * 那么这条平行线必定在角ABC的外侧，因为角ABD=AO3C=ABC+O3CB>ABC
 * 下面这个算法在插入直线BD时无法确定BD与直线AB和BC的关系，设O1是BC中点
 * 这个算法要求D必须事先确定在AO1的延长线上，实际上这个条件不是必须的
 */

void InsertPlaneAngle(AstParse *pParse,GeomType *pAngle)
{
    LineData *pBase;
    int rc1 = 0;
    int rc2 = 0;
    LinePoint *apNode[2];
    LinePoint *p1,*p2;
    LineSeg *pSeg;

    apNode[0] = (LinePoint*)GetLinkNode((LinkNode *)pAngle->pLine1->pHead,pAngle->pVertex);
    apNode[1] = (LinePoint*)GetLinkNode((LinkNode *)pAngle->pLine2->pHead,pAngle->pVertex);
    for(p1=apNode[0]->pNext;p1!=apNode[0];p1=p1->pNext){
        if(p1->pPoint==NULL) continue;
        for(p2=apNode[1]->pNext;p2!=apNode[1];p2=p2->pNext){
            if(p2->pPoint==NULL) continue;
            pSeg = *GetLineSegAddr(p1->pPoint,p2->pPoint);
            if(pSeg==NULL) continue;
            pBase = pSeg->pLine;
            log_a("base %s%s",p1->pPoint->zSymb,p2->pPoint->zSymb);
            TravLinePoint(pBase);
            if(!rc1){
                rc1 = InsertPlaneLine(pParse,pAngle->pVertex,p1->pPoint,pBase);
            }
            if(!rc2){
                rc2 = InsertPlaneLine(pParse,pAngle->pVertex,p2->pPoint,pBase);
            }
            if(rc1+rc2==2){
                goto insert_finish;
            }
        }
    }
insert_finish:
    assert(rc1+rc2==2);
    PrintPlaneLine(pAngle->pVertex);
}

PlaneSeg *SetAngleHash(AstParse *pParse,GeomType *pAngle)
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
        PointData *pInter;
        pInter = GetIntersection(pAngle->pLine1,pAngle->pLine2);
        assert(pInter==pAngle->pVertex);
        //InsertPlaneAngle(pParse,pAngle);
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
        pCorner->pLine1 = pAngle->pLine1;
        pCorner->pLine2 = pAngle->pLine2;
        pCorner->bl = bl;
        pCorner->br = br;
        pSeg->pCorner = pCorner;
        pAngle->pCorner = pCorner;
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


void TrglPointTemp(TempInfo *pTemp,PointData *p1,PointData *p2,PointData *p3)
{
    pTemp->apPoint[0] = p1;
    pTemp->apPoint[1] = p2;
    pTemp->apPoint[2] = p3;
}

int HavePubPoint(LineSeg *pIn1,LineSeg *pIn2,TempInfo *pTemp)
{
    int rc = 0;
    PointData *apPoint[2][2];
    int i,j;

    apPoint[0][0] = pIn1->pLeft;
    apPoint[0][1] = pIn1->pRight;
    apPoint[1][0] = pIn2->pLeft;
    apPoint[1][1] = pIn2->pRight;
    for(i=0;i<2;i++){
        for(j=0;j<2;j++)
        {
            if(apPoint[0][i]==apPoint[1][j]){
                pTemp->a = GetLinesegDirect(pIn1->pLine,apPoint[0][i],apPoint[0][1-i]);
                pTemp->b = GetLinesegDirect(pIn2->pLine,apPoint[1][j],apPoint[1][1-j]);
                TrglPointTemp(pTemp,apPoint[0][i],apPoint[0][1-i],apPoint[1][1-j]);
                rc = 1;
            }
        }
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

SameLine *SetSamePair(AstParse *pParse,LineSeg **ppSeg1,LineSeg **ppSeg2)
{
    SameLine *pPair;
    int iNum1,iNum2;

    pPair = (SameLine *)Malloc(sizeof(SameLine));
    iNum1 =  (*ppSeg1)->pLine->iNum;
    iNum2 =  (*ppSeg2)->pLine->iNum;
    //判断后可以保证GetSegDirect输入的线段在同一条直线上
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
    pPSeg = *GetPlaneSegAddr(pParse,pLine1,pLine2);
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
    PointData *pPoint1;
    PointData *pPoint2;

    pPoint1 = pA->pSeg1->pCorner->pVertex;
    pPoint2 = pA->pSeg2->pCorner->pVertex;
    apSideSeg[0] = CreateNewLine(pParse,pTemp->apSide[0][dir],pPoint1);
    apSideSeg[1] = CreateNewLine(pParse,pTemp->apSide[1][dir],pPoint2);
    apPSeg[0] = GetAndSetAngle(pParse,pTemp->apSide[0][1-dir],apSideSeg[0]);
    apPSeg[1] = GetAndSetAngle(pParse,pTemp->apSide[1][1-dir],apSideSeg[1]);

    apPairSeg = CreateNewLine(pParse,pTemp->apSide[0][1-dir],pTemp->apSide[1][1-dir]);
    InsertAnglePair(pParse,apPairSeg,apPSeg[0],apPSeg[1]);
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
                pPSeg = *GetPlaneSegAddr(pParse,apLine[0][1-i],apLine[1][1-j]);
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
    PointData *pPoint1;
    PointData *pPoint2;
    PointData *apPoint[2][2];
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
                    //todo 还没有检测方向是否与角的方向一致，可能是补角
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

PlaneSeg *GetAndSetAngle(AstParse *pParse,PointData *pVertex,LineSeg* pSeg)
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
    pPSeg = SetAngleHash(pParse,&ele);
    return pPSeg;
}


void SetSameSeg(AstParse *pParse,PlaneSeg *pPSeg,GeomType *pLeft,GeomType *pRight)
{
    TempInfo tmp;
    CornerInfo *pCorner;
    int val;//角度
    LineSeg **ppSeg1;
    LineSeg **ppSeg2;
    LineSeg *pSeg;

    ppSeg1 = GetLineSegAddr(pLeft->pPoint1,pLeft->pPoint2);
    ppSeg2 = GetLineSegAddr(pRight->pPoint1,pRight->pPoint2);

    InsertSamePair(pParse,pPSeg,ppSeg1,ppSeg2);
    if(HavePubPoint(*ppSeg1,*ppSeg2,&tmp)){
        //tmp.apPoint[0]是顶点
        if((*ppSeg1)->pLine==(*ppSeg2)->pLine){
            pSeg = *GetLineSegAddr(tmp.apPoint[1],tmp.apPoint[2]);
            assert(pSeg->pMid==NULL);
            pSeg->pMid = tmp.apPoint[0];
        }
        else{
            if(pPSeg->pCorner==NULL){
                assert(0); //未测试
                pSeg = *GetLineSegAddr(tmp.apPoint[1],tmp.apPoint[2]);
                pPSeg = GetAndSetAngle(pParse,tmp.apPoint[0],pSeg);
            }
            pCorner = pPSeg->pCorner;
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
}

PlaneSeg *SetPlaneHash(AstParse *pParse,GeomType *pLeft,GeomType *pRight)
{
    int iLeft;
    int iRight;
    PlaneData *pPlane;
    PlaneSeg *pSeg;
    LineData **ppLine = pParse->pLineSet->ppLine;
    PointData *pInter;
    GeomType ele = {0};

    assert(pLeft->type==ELE_LINE);
    assert(pRight->type==ELE_LINE);

    iLeft =pLeft->pLine1->iNum;
    iRight = pRight->pLine1->iNum;
    //如果出现直线合并，ppLine[iLeft]与pLeft->pLine1可能并不是同一条直线
    if(iLeft>iRight){
        if(ppLine[iLeft]->ppSeg[iRight]==NULL){
            pInter = GetIntersection(ppLine[iLeft],ppLine[iRight]);
            if(pInter!=NULL&&pInter->pPlane!=NULL){
                pSeg = NewPlaneSeg(ppLine[iLeft],iRight,pInter->pPlane);
                ele.pVertex = pInter;
                ele.pLine1 = ppLine[iLeft];
                ele.pLine2 = ppLine[iRight];
                InsertPlaneAngle(pParse,&ele);
            }
            else{
                pSeg = NewPlaneObj(pParse,ppLine[iLeft],iRight);
                pPlane = pSeg->pPlane;
                pPlane->pHead->pVal = ppLine[iLeft];
                InsertLinkNode(pParse,pPlane->pHead,ppLine[iRight]);
            }

        }
        else{
            pSeg = ppLine[iLeft]->ppSeg[iRight];
            pPlane = pSeg->pPlane;
        }
    }
    else{
        if(ppLine[iRight]->ppSeg[iLeft]==NULL){
            pInter = GetIntersection(ppLine[iLeft],ppLine[iRight]);
            if(pInter!=NULL&&pInter->pPlane!=NULL){
                pSeg = NewPlaneSeg(ppLine[iRight],iLeft,pInter->pPlane);
                ele.pVertex = pInter;
                ele.pLine1 = ppLine[iLeft];
                ele.pLine2 = ppLine[iRight];
                InsertPlaneAngle(pParse,&ele);
            }
            else{
                pSeg = NewPlaneObj(pParse,ppLine[iRight],iLeft);
                pPlane = pSeg->pPlane;
                //right始终在left的next
                pPlane->pHead->pVal = ppLine[iLeft];
                InsertLinkNode(pParse,pPlane->pHead,ppLine[iRight]);
            }
        }
        else{
            pSeg = ppLine[iRight]->ppSeg[iLeft];
            pPlane = pSeg->pPlane;
        }
    }
    return pSeg;
}

GeomType SetGeomHash(AstParse *pParse,TokenInfo *pAst)
{
    PointData *pPoint;
    u16 key;
    GeomType ele = {0};
    GeomType ele1 = {0};
    GeomType ele2 = {0};
    PointHash *pSet = pParse->pPointSet;
    PlaneSeg *pPSeg;
    LineSeg *pLineSeg;

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
        pPoint = (PointData *)Malloc(sizeof(PointData));
        memset(pPoint,0,sizeof(PointData));
        pPoint->iNum = pSet->nPoint++;
        pPoint->pParse = pParse;
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
                SetAngleHash(pParse,&ele);
                pLineSeg = *GetLineSegAddr(ele1.pPoint1,ele2.pPoint1);
                GetAndSetAngle(pParse,ele2.pPoint2,pLineSeg);
                pLineSeg = *GetLineSegAddr(ele1.pPoint1,ele2.pPoint2);
                GetAndSetAngle(pParse,ele2.pPoint1,pLineSeg);
                break;
            case OP_EQUAL:
                pPSeg = SetPlaneHash(pParse,&ele1,&ele2);
                SetSameSeg(pParse,pPSeg,&ele1,&ele2);
                break;
            case OP_PARALLEL:
                SetParallel(pParse,&ele1,&ele2);
                //log_a("parallel");
                break;
            default:
                break;
            }

        }
        else if(ele1.type==ELE_ANGLE){
            ele1.pCorner->val = atoi(pAst->pRight->zSymb);
            //SetAngleHash(pParse,&ele1,pAst->pRight);
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
