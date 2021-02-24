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
    size = sizeof(LineHash *)*nSlot;
    pLineSet->nSlot = nSlot;
    pLineSet->ppLine = (LineData **)Malloc(size);
    memset(pLineSet->ppLine,0,size);
    return pLineSet;
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

void FreePointSet(AstParse *pParse)
{
    PointHash *pPointSet = pParse->pPointSet;
    LineHash *pLineSet = pParse->pLineSet;
    int i = 0;
    for(i=0;i<pLineSet->nLine;i++)
    {
        FreeLinePoint(pParse,pLineSet->ppLine[i]->pHead);
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


LinkNode *NewLinkNode(void *pVal)
{
    LinkNode *p;
    p = (LinkNode *)Malloc(sizeof(LinkNode));
    memset(p,0,sizeof(*p));
    p->pVal = pVal;
    return p;
}

LinkNode *NewLinkHead(void *pVal)
{
    LinkNode *p;
    p = NewLinkNode(pVal);
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


void SetLineArray(PoinData *pLeft,LineData *pLine,PoinData *pRight)
{
    int iLeft;
    int iRight;
    assert(0);//待删除
    iLeft = pLeft->iNum;
    iRight = pRight->iNum;
    if(iLeft>iRight){
       // assert(pLeft->ppSeg[iRight]==NULL);
        pLeft->ppSeg[iRight] = (LineSeg*)NewLinkHead(pLine);
    }
    else
    {
        assert(pLeft->ppSeg[iLeft]==NULL);
        pRight->ppSeg[iLeft] = (LineSeg*)NewLinkHead(pLine);
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

LineData *NewLineObj(AstParse *pParse,PoinData *pLeft,int iRight)
{
    LineData *pNew;
    LineSeg *pSeg;
    LineHash *pLineSet = pParse->pLineSet;
    pNew = (LineData*)Malloc(sizeof(LineData));
    memset(pNew,0,sizeof(LineData));
    pSeg = (LineSeg*)NewLinkHead(pNew);
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
    return pNew;
}

GeomType GetPointEle(PoinData *p)
{
    GeomType ele = {0};
    ele.type = ELE_POINT;
    ele.pPoint1 = p;
    return ele;
}

GeomType SetLineHash(AstParse *pParse,GeomType *pLeft,GeomType *pRight,u8 op)
{
    GeomType ele = {0};
    GeomType ele1 = {0};
    GeomType ele2 = {0};
    int iLeft;
    int iRight;
    LineData *pLine;
    LinePoint *pLoc;
    LineSeg** ppLs1;
    LineSeg** ppLs2;
    if(op==OP_LINE){
        if(pLeft->type==ELE_POINT && pRight->type==ELE_POINT)
        {
            ele.type = ELE_LINE;
            iLeft = pLeft->pPoint1->iNum;
            iRight = pRight->pPoint1->iNum;
            if(iLeft>iRight){
                if(pLeft->pPoint1->ppSeg[iRight]==NULL){
                    pLine = NewLineObj(pParse,pLeft->pPoint1,iRight);
                    //保持最初始的输入顺序，直线B-C,pLeft是B，pRight是C
                    InsertPointNode(pParse,pLine->pHead,pLeft->pPoint1);
                    InsertPointNode(pParse,pLine->pHead->pNext,pRight->pPoint1);
                }
                else{
                    pLine = pLeft->pPoint1->ppSeg[iRight]->pLine;
                }
            }
            else{
                if(pRight->pPoint1->ppSeg[iLeft]==NULL){
                    pLine = NewLineObj(pParse,pRight->pPoint1,iLeft);
                    //保持最初始的输入顺序，直线B-C,pLeft是B，pRight是C
                    InsertPointNode(pParse,pLine->pHead,pLeft->pPoint1);
                    InsertPointNode(pParse,pLine->pHead->pNext,pRight->pPoint1);
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
            assert(pRight->type==ELE_LINE);
            //pRight->pPoint2一定在pRight->pPoint1的后面吗
            SetLineArray(pLeft->pPoint1,pRight->pLine1,pRight->pPoint1);
            SetLineArray(pLeft->pPoint1,pRight->pLine1,pRight->pPoint2);
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
                *ppLs1 = (LineSeg*)NewLinkHead(pLeft->pLine1);
                *ppLs2 = (LineSeg*)NewLinkHead(pLeft->pLine1);
                pLoc = FindPointLoc(pLeft->pLine1->pHead,pLeft->pPoint2);
                //pRight->pPoint1插入到pLeft->pLine1后面
                InsertPointNode(pParse,pLoc,pRight->pPoint1);
            }
        }
        else
        {
            assert(0);
        }
    }
    else if(op==OP_IMPL)
    {
        assert(pLeft->type==ELE_POINT && pRight->type==ELE_LINE);
        ele1 = GetPointEle(pRight->pPoint1);
        ele2 = GetPointEle(pRight->pPoint2);
        ele1 = SetLineHash(pParse,pLeft,&ele1,OP_LINE);
        ele2 = SetLineHash(pParse,pLeft,&ele2,OP_LINE);
        ele.type = ELE_ANGLE;
        ele.pPoint1 = pLeft->pPoint1;
        ele.pLine1 = ele1.pLine1;
        ele.pLine2 = ele2.pLine1;
    }
    return ele;
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
            ele = SetLineHash(pParse,&ele1,&ele2,pAst->op);
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
