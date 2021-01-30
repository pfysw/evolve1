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
    pPointSet = (PointHash *)malloc(sizeof(PointHash));
    memset(pPointSet,0,sizeof(PointHash));
    pPointSet->nHash = nSlot;
    if(nSlot<128){
        pPointSet->nHash = 128;
    }
    size = sizeof(PointHash *)*pPointSet->nHash/2;
    pPointSet->ppArray = (PoinData **)malloc(size);
    memset(pPointSet->ppArray,0,size);
    pPointSet->ppHash = (PoinData **)malloc(size);
    memset(pPointSet->ppHash,0,size);
    return pPointSet;
}

LineHash *CreatLineHash(int nSlot)
{
    LineHash *pLineSet;
    int size;
    pLineSet = (LineHash *)malloc(sizeof(LineHash));
    memset(pLineSet,0,sizeof(LineHash));
    size = sizeof(LineHash *)*nSlot;
    pLineSet->nSlot;
    pLineSet->ppArray = (LineData **)malloc(size);
    memset(pLineSet->ppArray,0,size);
    return pLineSet;
}

LinePoint *NewPointNode(PoinData *pPoint)
{
    LinePoint *p;
    p = (LinePoint *)malloc(sizeof(LinePoint));
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

void InsertPointNode(AstParse *pParse,LinePoint *pPre,PoinData *pNode)
{
    LinePoint *p = (LinePoint*)malloc(sizeof(LinePoint));
    pParse->malloc_cnt++;
    memset(p,0,sizeof(LinePoint));
    p->pPoint = pNode;

    p->pNext = pPre->pNext;
    p->pPre = pPre;
    pPre->pPre = p;
    pPre->pNext = p;
}

LinePoint *FindPointLoc(LinePoint *pHead,PoinData *pNode)
{
    LinePoint *p;
    assert(pHead->isHead);
    do{
        p = pHead;
        if(p->pPoint==pNode){
            return p;
        }
    }while(!p->isHead);
    assert(0);
    return NULL;
}

void InsertLinePoint(
        AstParse *pParse,
        LineData *pLine,
        PoinData *pLoc,
        PoinData *pNode)
{
    LinePoint *pPre;
    pPre = FindPointLoc(pLine->pHead,pLoc);
}


void SetLineArray(PoinData *pLeft,LineData *pLine,PoinData *pRight)
{
    int iLeft;
    int iRight;
    iLeft = pLeft->iNum;
    iRight = pRight->iNum;
    if(iLeft>iRight){
        pLeft->ppArray[iRight] = pLine;
    }
    else
    {
        pRight->ppArray[iLeft] = pLine;
    }
}

LineData *NewLineObj(AstParse *pParse,PoinData *pLeft,int iRight)
{
    LineData *pNew;
    LineHash *pLineSet = pParse->pLineSet;
    pNew = (LineData*)malloc(sizeof(LineData));
    pParse->malloc_cnt++;
    memset(pNew,0,sizeof(LineData));
    pLeft->ppArray[iRight] = pNew;
    pLeft->nArray++;//use for debug
    pNew->iNum = pLineSet->nLine;
    if(pLineSet->nLine>pLineSet->nSlot){
        //todo resizeHash
        assert(0);
    }
    pLineSet->ppArray[pNew->iNum] = pNew;
    pLineSet->nLine++;
    pNew->pHead = NewPointHead(pLeft);
    pParse->malloc_cnt++;
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
    if(op==OP_LINE){
        if(pLeft->type==ELE_POINT && pRight->type==ELE_POINT)
        {
            ele.type = ELE_LINE;
            iLeft = pLeft->pPoint1->iNum;
            iRight = pRight->pPoint1->iNum;
            if(iLeft>iRight){
                assert(pLeft->pPoint1->ppArray[iRight]==NULL);
                pLine = NewLineObj(pParse,pLeft->pPoint1,iRight);
                InsertPointNode(pParse,pLine->pHead,pRight->pPoint1);
                ele.pPoint1 = pLeft->pPoint1;
                ele.pPoint2 = pRight->pPoint1;
            }
            else{
                assert(pRight->pPoint1->ppArray[iLeft]==NULL);
                pLine = NewLineObj(pParse,pRight->pPoint1,iLeft);
                InsertPointNode(pParse,pLine->pHead,pLeft->pPoint1);
                ele.pPoint1 = pRight->pPoint1;
                ele.pPoint2 = pLeft->pPoint1;
            }
            ele.pLine1 = pLine;
        }
        else if(pLeft->type==ELE_POINT)
        {
            assert(pRight->type==ELE_LINE);
            //pRight->pPoint2一定在pRight->pPoint1的后面吗
            SetLineArray(pLeft->pPoint1,pRight->pLine1,pRight->pPoint1);
            SetLineArray(pLeft->pPoint1,pRight->pLine1,pRight->pPoint2);
            pLoc = FindPointLoc(pRight->pLine1->pHead,pRight->pPoint1);
            InsertPointNode(pParse,pLoc->pPre,pLeft->pPoint1);
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
        ele1 = SetLineHash(pParse,pLeft,ele1,OP_LINE);
        ele2 = SetLineHash(pParse,pLeft,ele2,OP_LINE);
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
        pPoint = (PoinData *)malloc(sizeof(PoinData));
        pParse->malloc_cnt++;
        memset(pPoint,0,sizeof(PoinData));
        pPoint->iNum = pSet->nPoint++;
        pSet->ppArray[pPoint->iNum] = pPoint;
        pPoint->zSymb = (char*)malloc(pAst->nSymbLen+1);
        pParse->malloc_cnt++;
        memcpy(pPoint->zSymb,pAst->zSymb,pAst->nSymbLen+1);
        pPoint->ppArray = (LineData**)malloc(sizeof(LineData*)*pPoint->iNum);
        pParse->malloc_cnt++;
        memset(pPoint->ppArray, 0, sizeof(LineData*)*pPoint->iNum);
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
