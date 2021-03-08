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
#include <assert.h>
#include "geometry.h"


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

    log_c("line: ");
    while(1)
    {
        if(p->isHead){
            //log_c("%s ",p->pPoint->zSymb);
            Free(p);
            break;
        }
        else{
            log_c("%s ",p->pPoint->zSymb);
            pTmp = p;
            p = p->pNext;
            Free(pTmp);
        }
    }
    log_a("");
}

void FreeLinkNode(AstParse *pParse,LinkNode *pHead)
{
    LinkNode *p = pHead->pNext;
    LinkNode *pTmp;

    while(1)
    {
        if(p->isHead){
            Free(p);
            break;
        }
        else{
            pTmp = p;
            p = p->pNext;
            Free(pTmp);
        }
    }
}

void FreeLineSeg(PoinData *pArray)
{
    int i = 0;
    for(i=0;i<pArray->iNum;i++)
    {
        if(pArray->ppSeg[i] != NULL){
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

void FreeSameLine(PlaneSeg *pSeg)
{
    LinkNode *p;
    LinkNode *pTmp;
    if(pSeg->pSame!=NULL){
        p = pSeg->pSame->pNext;
        while(1)
        {
            if(p->isHead){
                PrintSameLine(p->pVal);
                Free(p->pVal);
                Free(p);
                break;
            }
            else{
                pTmp = p;
                p = p->pNext;
                PrintSameLine(pTmp->pVal);
                Free(pTmp->pVal);
                Free(pTmp);
            }
        }
    }
}

void FreePlaneSeg(LineData *pArray)
{
    int i = 0;
    for(i=0;i<pArray->iNum;i++)
    {
        if(pArray->ppSeg[i] != NULL){
            FreeSameLine(pArray->ppSeg[i]);
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
        FreeLinkNode(pParse,pPlaneSet->ppPlane[i]->pHead);
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
        Free(pPointSet->ppArray[i]->zSymb);
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
    memset(p,0,sizeof(*p));
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


LineSeg** GetLineSeg(PoinData *pLeft,PoinData *pRight)
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

void ResetLine(AstParse *pParse,GeomType *pDst,LineData *pLine)
{
    LineSeg** ppSeg;
    int nLine;
    LineHash *pLineSet = pParse->pLineSet;

    ppSeg = GetLineSeg(pDst->pPoint1,pDst->pPoint2);

    log_c("reset: ");
    nLine = --pParse->pLineSet->nLine;
    //todo 确认pDst->pLine1是否是最后一个
    assert(pLineSet->ppLine[nLine]==pDst->pLine1);
    FreeLinePoint(pParse,pDst->pLine1->pHead);
    Free(pDst->pLine1);
    (*ppSeg)->pLine = pLine;
}

LineSeg *NewLineObj(AstParse *pParse,PoinData *pLeft,int iRight)
{
    LineData *pNew;
    LineSeg *pSeg;
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
    pNew->pHead = NewPointHead(NULL);
    pNew->ppSeg = (PlaneSeg **)Malloc(sizeof(PlaneSeg *)*pNew->iNum);
    memset(pNew->ppSeg,0,sizeof(PlaneSeg *)*pNew->iNum);
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

void SetSegPoint(LineSeg *pSeg,PoinData *pLeft,PoinData *pRight)
{
   // LineData *pLine = pSeg->pLine;
   // InsertPointNode(pParse,pLine->pHead,pLeft->pPoint1);
   // InsertPointNode(pParse,pLine->pHead->pNext,pRight->pPoint1);
    pSeg->pLeft = pLeft;
    pSeg->pRight = pRight;
}


GeomType SetLineHash(AstParse *pParse,GeomType *pLeft,GeomType *pRight)
{
    GeomType ele = {0};
    int iLeft;
    int iRight;
    LineData *pLine;
    LinePoint *pLoc;
    LineSeg** ppLs1;
    LineSeg** ppLs2;
    LineSeg *pSeg;

    if(pLeft->type==ELE_POINT && pRight->type==ELE_POINT)
    {
        ele.type = ELE_LINE;
        iLeft = pLeft->pPoint1->iNum;
        iRight = pRight->pPoint1->iNum;
        if(iLeft>iRight){
            if(pLeft->pPoint1->ppSeg[iRight]==NULL){
                pSeg = NewLineObj(pParse,pLeft->pPoint1,iRight);
                pLine = pSeg->pLine;
                //保持最初始的输入顺序，直线B-C,pLeft是B，pRight是C
                InsertPointNode(pParse,pLine->pHead,pLeft->pPoint1);
                InsertPointNode(pParse,pLine->pHead->pNext,pRight->pPoint1);
                SetSegPoint(pSeg,pLeft->pPoint1,pRight->pPoint1);
            }
            else{
                pLine = pLeft->pPoint1->ppSeg[iRight]->pLine;
            }
        }
        else{
            if(pRight->pPoint1->ppSeg[iLeft]==NULL){
                pSeg = NewLineObj(pParse,pRight->pPoint1,iLeft);
                pLine = pSeg->pLine;
                //保持最初始的输入顺序，直线B-C,pLeft是B，pRight是C
                InsertPointNode(pParse,pLine->pHead,pLeft->pPoint1);
                InsertPointNode(pParse,pLine->pHead->pNext,pRight->pPoint1);
                SetSegPoint(pSeg,pLeft->pPoint1,pRight->pPoint1);
            }
            else{
                pLine = pRight->pPoint1->ppSeg[iLeft]->pLine;
            }
        }
        //保持最初始的输入顺序，直线B-C,ele.pPoint1是B，ele.pPoint2是C
        ele.pPoint1 = pLeft->pPoint1;
        ele.pPoint2 = pRight->pPoint1;
        ele.pLine1 = pLine;
    }
    else if(pLeft->type==ELE_POINT)
    {
        assert(0);//未测试
        assert(pRight->type==ELE_LINE);
        ppLs1 = GetLineSeg(pLeft->pPoint1,pRight->pPoint1);
        ppLs2 = GetLineSeg(pLeft->pPoint1,pRight->pPoint2);
        assert(*ppLs1==NULL);
        assert(*ppLs2==NULL);
        *ppLs1 = (LineSeg*)NewLinkHead(pRight->pLine1,sizeof(LineSeg));
        SetSegPoint(*ppLs1,pLeft->pPoint1,pRight->pPoint1);
        *ppLs2 = (LineSeg*)NewLinkHead(pRight->pLine1,sizeof(LineSeg));
        SetSegPoint(*ppLs2,pLeft->pPoint1,pRight->pPoint2);
        pLoc = FindPointLoc(pRight->pLine1->pHead,pRight->pPoint1);
        //点pLeft->pPoint1插入到线段pRight的前面
        InsertPointNode(pParse,pLoc->pPre,pLeft->pPoint1);
    }
    else if(pLeft->type==ELE_LINE)
    {
        ppLs1 = GetLineSeg(pRight->pPoint1,pLeft->pPoint1);
        ppLs2 = GetLineSeg(pRight->pPoint1,pLeft->pPoint2);
        if(*ppLs1!=NULL){
            assert(*ppLs2==NULL);
            pLoc = FindPointLoc((*ppLs1)->pLine->pHead,pLeft->pPoint1);
            ResetLine(pParse,pLeft,(*ppLs1)->pLine);
            InsertPointNode(pParse,pLoc,pLeft->pPoint2);
        }
        else if(*ppLs2!=NULL){
            assert(0);
        }
        else{
            *ppLs1 = (LineSeg*)NewLinkHead(pLeft->pLine1,sizeof(LineSeg));
            *ppLs2 = (LineSeg*)NewLinkHead(pLeft->pLine1,sizeof(LineSeg));
            pLoc = FindPointLoc(pLeft->pLine1->pHead,pLeft->pPoint2);
            //pRight->pPoint1插入到pLeft->pLine1后面
            InsertPointNode(pParse,pLoc,pRight->pPoint1);
        }
        SetSegPoint(*ppLs1,pRight->pPoint1,pLeft->pPoint1);
        SetSegPoint(*ppLs2,pRight->pPoint1,pLeft->pPoint2);
    }
    else
    {
        assert(0);
    }

    return ele;
}

GeomType SetAngleHash(AstParse *pParse,GeomType *pLeft,GeomType *pRight)
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
    ele.pPoint1 = pLeft->pPoint1;
    ele.pLine1 = ele1.pLine1;
    ele.pLine2 = ele2.pLine1;

    return ele;
}

void SetSameSeg(AstParse *pParse,PlaneSeg *pPSeg,GeomType *pLeft,GeomType *pRight)
{
    SameLine *pPair;
    LineSeg **ppSeg1;
    LineSeg **ppSeg2;

    pPair = (SameLine *)Malloc(sizeof(SameLine));
    ppSeg1 = GetLineSeg(pLeft->pPoint1,pLeft->pPoint2);
    ppSeg2 = GetLineSeg(pRight->pPoint1,pRight->pPoint2);
    pPair->pSeg1 = *ppSeg1;
    pPair->pSeg2 = *ppSeg2;
    //PrintSameLine(pPair);
    if(pPSeg->pSame==NULL){
        pPSeg->pSame =  NewLinkHead(pPair,sizeof(LinkNode));
    }
    else{
        InsertLinkNode(pParse,pPSeg->pSame,pPair);
    }
}

void SetPlaneHash(AstParse *pParse,GeomType *pLeft,GeomType *pRight)
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
            InsertLinkNode(pParse,pPlane->pHead,pLeft->pLine1);
            InsertLinkNode(pParse,pPlane->pHead->pNext,pRight->pLine1);
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
            InsertLinkNode(pParse,pPlane->pHead,pLeft->pLine1);
            InsertLinkNode(pParse,pPlane->pHead->pNext,pRight->pLine1);
        }
        else{
            pSeg = pRight->pLine1->ppSeg[iLeft];
            pPlane = pSeg->pPlane;
        }
    }
    SetSameSeg(pParse,pSeg,pLeft,pRight);
}

GeomType SetGeomHash(AstParse *pParse,TokenInfo *pAst)
{
    PoinData *pPoint;
    u16 key;
    GeomType ele = {0};
    GeomType ele1 = {0};
    GeomType ele2 = {0};
    PointHash *pSet = pParse->pPointSet;
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
        pPoint->ppSeg = (LineSeg**)Malloc(sizeof(LineSeg*)*pPoint->iNum);
        memset(pPoint->ppSeg, 0, sizeof(LineSeg*)*pPoint->iNum);
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
                ele = SetAngleHash(pParse,&ele1,&ele2);
                break;
            case OP_EQUAL:
                SetPlaneHash(pParse,&ele1,&ele2);
                break;
            default:
                break;
            }

        }
        else{
            assert(ele1.type==ELE_ANGLE);
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
