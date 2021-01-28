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
    pLineSet->nHash = nSlot;
    if(nSlot<128){
        pLineSet->nHash = 128;
    }
    size = sizeof(LineHash *)*pLineSet->nHash/2;
    pLineSet->ppArray = (LineData **)malloc(size);
    memset(pLineSet->ppArray,0,size);
    pLineSet->ppHash = (LineData **)malloc(size);
    memset(pLineSet->ppHash,0,size);
    return pLineSet;
}

GeomType SetLineHash(AstParse *pParse,GeomType *pLeft,GeomType *pRight,u8 op)
{
    GeomType ele = {0};
    if(op==OP_LINE){
        if(pLeft->type==ELE_POINT && pRight->type==ELE_POINT)
        {
            ele.type = ELE_LINE;
            ele.pLeft = pLeft->pLeft;
            ele.pRight = pRight->pLeft;
        }
        else
        {
            assert(0);
        }
    }
    else if(op==OP_IMPL)
    {
        assert(pLeft->type==ELE_POINT && pRight->type==ELE_LINE);
        ele.type = ELE_ANGLE;
        ele.pLeft = pLeft->pLeft;
        //ele.pLine1 = pRight->pLeft;
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
                ele.type = ELE_POINT;
                ele.pLeft = pSet->ppHash[key];
                return ele;//hash have the element
            }
        }
        pPoint = (PoinData *)malloc(sizeof(PoinData));
        memset(pPoint,0,sizeof(PoinData));
        pPoint->iNum = pSet->nPoint++;
        pSet->ppArray[pPoint->iNum] = pPoint;
        pPoint->zSymb = (char*)malloc(pAst->nSymbLen+1);
        memcpy(pPoint->zSymb,pAst->zSymb,pAst->nSymbLen+1);
        pSet->ppHash[key] = pPoint;
        ele.type = ELE_POINT;
        ele.pLeft = pSet->ppHash[key];
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
