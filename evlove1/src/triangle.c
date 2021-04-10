/*
 * triangle.c
 *
 *  Created on: Apr 3, 2021
 *      Author: Administrator
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include <assert.h>
#include "geometry.h"
#include "triangle.h"


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

    apPSeg[0] = *GetPlaneSegAddr(pParse,apSeg[2]->pLine,apSeg[1]->pLine);

    InsertAnglePair(pParse,apSeg[2],apPSeg[1],apPSeg[0]);
    InsertAnglePair(pParse,apSeg[1],apPSeg[2],apPSeg[0]);
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

//传入的是元素地址，其实a和b是二级指针
int PointCmpfunc(const void *a, const void *b)
{
   int rc = 0;
   PoinData *pA = *((PoinData **)a);
   PoinData *pB = *((PoinData **)b);
   rc = pA->iNum-pB->iNum;
   assert(rc!=0);
   return rc;
}

TrigInfo *GetTriangleObj(AstParse *pParse,TempInfo *pTemp)
{
    PoinData **ppPoint = pTemp->apPoint;
    LineSeg *pSeg;
    LinkNode *p;
    TrigInfo *pTrig;
    TrigInfo *pFind = NULL;
    qsort(ppPoint,3,sizeof(PoinData *),PointCmpfunc);
    pSeg = CreateNewLine(pParse,ppPoint[1],ppPoint[2]);
    p = pSeg->pTriag;
    if(p!=NULL){
        do{
            pTrig = (TrigInfo *)(p->pVal);
            if(pTrig->apPoint[0]==ppPoint[0]){
                pFind = pTrig;
                break;
            }
            p = p->pNext;
        }while(!p->isHead);
    }
    return pFind;
}

TrigInfo *SetTriangleObj(AstParse *pParse,TempInfo *pTemp)
{
    PoinData **ppPoint = pTemp->apPoint;
    LineSeg *pSeg;
    LinkNode *p;
    TrigInfo *pTrig;
    TrigInfo *pFind = NULL;
    qsort(ppPoint,3,sizeof(PoinData *),PointCmpfunc);
    pSeg = CreateNewLine(pParse,ppPoint[1],ppPoint[2]);
    p = pSeg->pTriag;
    if(p!=NULL){
        do{
            pTrig = (TrigInfo *)(p->pVal);
            if(pTrig->apPoint[0]==ppPoint[0]){
                pFind = pTrig;
                break;
            }
            p = p->pNext;
        }while(!p->isHead);
        if(pFind==NULL){
            pFind = (TrigInfo *)Malloc(sizeof(TrigInfo));
            memcpy(pFind->apPoint,ppPoint,sizeof(TrigInfo));
            InsertLinkNode(pParse,pSeg->pTriag,pFind);
        }
    }
    else{
        pFind = (TrigInfo *)Malloc(sizeof(TrigInfo));
        memcpy(pFind->apPoint,ppPoint,sizeof(TrigInfo));
        pSeg->pTriag = NewLinkHead(pFind,sizeof(TrigInfo));
    }
    return pFind;
}
