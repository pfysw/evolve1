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

#ifdef FREE_TEST
extern u8 testbuf[];
#endif

PointHash *CreatPointHash(int nSlot)
{
    PointHash *pPointSet;
    int size;
    pPointSet = (PointHash *)malloc(sizeof(PointHash));
    memset(pPointSet,0,sizeof(PointHash));
    pPointSet->nHash = nSlot*2;
    size = sizeof(PointHash *)*nSlot;
    pPointSet->ppArray = (PoinData **)malloc(size);
    memset(pPointSet->ppArray,0,size);
    size = sizeof(PointHash *)*pPointSet->nHash;
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
    pLineSet->nSlot = nSlot;
    pLineSet->ppLine = (LineData **)malloc(size);
    memset(pLineSet->ppLine,0,size);
    return pLineSet;
}


void FreeLinePoint(AstParse *pParse,LinePoint *pHead)
{
    LinePoint *p = pHead->pNext;
    LinePoint *pTmp;

      static int jj = 0;
      jj++;
     if(jj==4)
          printf("jj %d\n",jj);

    while(1)
    {
        if(p->isHead){
#ifdef FREE_TEST
        testbuf[p->malloc_flag] = 0;
#endif
            free(p);
            pParse->free_cnt++;
            break;
        }
        else{
#ifdef FREE_TEST
    testbuf[p->malloc_flag] = 0;
#endif
            pTmp = p;
            p = p->pNext;
            free(pTmp);
            pParse->free_cnt++;
        }
    }
}


void FreeLineData(AstParse *pParse,PoinData *pPoint)
{
    int i;
    for(i=0;i<pPoint->iNum;i++){
        if(pPoint->ppLine[i]!=NULL){
            FreeLinePoint(pParse,pPoint->ppLine[i]->pHead);
        }
    }
}

void FreePointSet(AstParse *pParse)
{
    PointHash *pPointSet = pParse->pPointSet;
    LineHash *pLineSet = pParse->pLineSet;
    int i = 0;
    for(i=0;i<pPointSet->nPoint;i++)
    {
        //FreeLineData(pParse,pPointSet->ppArray[i]);
        free(pPointSet->ppArray[i]->ppLine);
        pPointSet->ppArray[i]->ppLine = NULL;
        pParse->free_cnt++;
        free(pPointSet->ppArray[i]->zSymb);
        pParse->free_cnt++;
        free(pPointSet->ppArray[i]);
        pParse->free_cnt++;
    }
    for(i=0;i<pLineSet->nLine;i++)
    {
        FreeLinePoint(pParse,pLineSet->ppLine[i]->pHead);
        free(pLineSet->ppLine[i]);
        pParse->free_cnt++;
    }
}

void CloseGeomSet(AstParse *pParse)
{
    FreePointSet(pParse);
    free(pParse->pPointSet->ppArray);
    free(pParse->pPointSet->ppHash);
    free(pParse->pPointSet);
    free(pParse->pLineSet->ppLine);
    free(pParse->pLineSet);
    pParse->free_cnt += 5;
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
    p->pNext->pPre = p;
    pPre->pNext = p;
#ifdef FREE_TEST
    p->malloc_flag = pParse->malloc_cnt;
    testbuf[pParse->malloc_cnt] = 1;
#endif
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



void SetLineArray(PoinData *pLeft,LineData *pLine,PoinData *pRight)
{
    int iLeft;
    int iRight;
    iLeft = pLeft->iNum;
    iRight = pRight->iNum;
    if(iLeft>iRight){
        pLeft->ppLine[iRight] = pLine;
    }
    else
    {
        pRight->ppLine[iLeft] = pLine;
    }
}

LineData *NewLineObj(AstParse *pParse,PoinData *pLeft,int iRight)
{
    LineData *pNew;
    LineHash *pLineSet = pParse->pLineSet;
    pNew = (LineData*)malloc(sizeof(LineData));
    pParse->malloc_cnt++;
    memset(pNew,0,sizeof(LineData));
    pLeft->ppLine[iRight] = pNew;
    pLeft->nArray++;
    pNew->iNum = pLineSet->nLine;
    if(pLineSet->nLine>pLineSet->nSlot){
        //todo resizeHash
        assert(0);
    }
    pLineSet->ppLine[pNew->iNum] = pNew;
    pLineSet->nLine++;
    pNew->pHead = NewPointHead(pLeft);
    pParse->malloc_cnt++;
#ifdef FREE_TEST
    pNew->pHead->malloc_flag = pParse->malloc_cnt;
    testbuf[pParse->malloc_cnt] = 1;
#endif
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
                if(pLeft->pPoint1->ppLine[iRight]==NULL){
                    pLine = NewLineObj(pParse,pLeft->pPoint1,iRight);
                    InsertPointNode(pParse,pLine->pHead,pRight->pPoint1);
                }
                else{
                    pLine = pLeft->pPoint1->ppLine[iRight];
                }
                //在前面
                ele.pPoint1 = pLeft->pPoint1;
                //在后面
                ele.pPoint2 = pRight->pPoint1;
            }
            else{
                if(pRight->pPoint1->ppLine[iLeft]==NULL){
                    pLine = NewLineObj(pParse,pRight->pPoint1,iLeft);
                    InsertPointNode(pParse,pLine->pHead,pLeft->pPoint1);
                }
                else{
                    pLine = pRight->pPoint1->ppLine[iLeft];
                }
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
            //点pLeft->pPoint1插入到线段pRight的前面
            InsertPointNode(pParse,pLoc->pPre,pLeft->pPoint1);
        }
        else if(pLeft->type==ELE_LINE)
        {
            assert(pRight->type==ELE_POINT);
            SetLineArray(pRight->pPoint1,pLeft->pLine1,pLeft->pPoint1);
            SetLineArray(pRight->pPoint1,pLeft->pLine1,pLeft->pPoint2);
            pLoc = FindPointLoc(pLeft->pLine1->pHead,pLeft->pPoint2);
            //pRight->pPoint1插入到pLeft->pLine1后面
            InsertPointNode(pParse,pLoc,pRight->pPoint1);
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
        pPoint = (PoinData *)malloc(sizeof(PoinData));
        pParse->malloc_cnt++;
        memset(pPoint,0,sizeof(PoinData));
        pPoint->iNum = pSet->nPoint++;
        pSet->ppArray[pPoint->iNum] = pPoint;
        pPoint->zSymb = (char*)malloc(pAst->nSymbLen+1);
        pParse->malloc_cnt++;
        memcpy(pPoint->zSymb,pAst->zSymb,pAst->nSymbLen+1);
        pPoint->ppLine = (LineData**)malloc(sizeof(LineData*)*pPoint->iNum);
        pParse->malloc_cnt++;
        memset(pPoint->ppLine, 0, sizeof(LineData*)*pPoint->iNum);
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
