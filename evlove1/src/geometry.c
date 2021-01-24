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
    size = sizeof(PointHash *)*pPointSet->nHash;
    pPointSet->ppArray = (PoinData **)malloc(size);
    memset(pPointSet->ppArray,0,size);
    pPointSet->ppHash = (PoinData **)malloc(size);
    memset(pPointSet->ppHash,0,size);
    return pPointSet;
}


void SetGeomHash(PointHash *pSet,TokenInfo *pAst)
{
    PoinData *pPoint;
    u16 key;
    if(pAst->type==PROP_SYMB)
    {
        key = ((*((u16 *)pAst->zSymb))*383)&(pSet->nHash-1);
        for(;pSet->ppHash[key];key=((key+1)&(pSet->nHash-1))){
            if(strcmp(pAst->zSymb,pSet->ppHash[key]->zSymb))
            {
                continue;//Collide
            }
            else
            {
                return;//hash have the element
            }
        }
        pPoint = (PoinData *)malloc(sizeof(PoinData));
        memset(pPoint,0,sizeof(PoinData));
        pPoint->iNum = pSet->nPoint++;
        pSet->ppArray[pPoint->iNum] = pPoint;
        pPoint->zSymb = (char*)malloc(pAst->nSymbLen+1);
        memcpy(pPoint->zSymb,pAst->zSymb,pAst->nSymbLen+1);
        pSet->ppHash[key] = pPoint;
        log_a("sym %s key %d num %d",pPoint->zSymb,key,pPoint->iNum);
    }
    else if(pAst->type==PROP_IMPL)
    {
        SetGeomHash(pSet,pAst->pLeft);
        if(memcmp(pAst->zSymb,"val",3)){
            SetGeomHash(pSet,pAst->pRight);
        }
    }
    else
    {
        assert(0);
    }
}

void ParseGeomEle(AstParse *pParse,Vector *pSet)
{
    int i = 0;
    for(i=0;i<pSet->n;i++)
    {
        SetGeomHash(pParse->pPointSet,pSet->data[i]);
    }
}
