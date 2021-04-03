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
