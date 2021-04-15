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
    PointData *pPoint1;
    PointData *pPoint2;
    LineSeg* apSeg[2];
    PlaneSeg *apSameSeg;
    AstParse *pParse = pTemp->pParse;
    GeomType ele1;
    GeomType ele2;
    TempInfo tmp;
    PointData *apLink[2];
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
    PointData *apLeft[2];
    PointData *apRight[2];
    LinkNode *p;
    SameLine *pPair;
    PointData *pPoint1;
    PointData *pPoint2;
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
   PointData *pA = *((PointData **)a);
   PointData *pB = *((PointData **)b);
   rc = pA->iNum-pB->iNum;
   assert(rc!=0);
   return rc;
}

TrigInfo *GetTriangleObj(AstParse *pParse,TempInfo *pTemp)
{
    PointData **ppPoint = pTemp->apPoint;
    LineSeg *pSeg;
    LinkNode *p;
    TrigInfo *pTrig;
    TrigInfo *pFind = NULL;
    qsort(ppPoint,3,sizeof(PointData *),PointCmpfunc);
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
    PointData **ppPoint = pTemp->apPoint;
    LineSeg *pSeg;
    LinkNode *p;
    TrigInfo *pTrig;
    TrigInfo *pFind = NULL;
    qsort(ppPoint,3,sizeof(PointData *),PointCmpfunc);
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


//---------------------------

void PrintParall(CornerInfo *pCorner){
    LineSeg* apSeg[2];
    AstParse *pParse = pCorner->pVertex->pParse;
    apSeg[0] = *GetLineSegAddr(pCorner->pLeft,pCorner->pVertex);
    apSeg[1] = *GetLineSegAddr(pCorner->pRight,pCorner->pVertex);
    log_c("parallel:");
    PrintLine(pParse,apSeg[0]->pLine);
    log_c("// ");
    PrintLine(pParse,apSeg[1]->pLine);
    log_c("\n");
}

void SetParallAngle(CornerInfo *pCorner){
    LineSeg* apSeg[2];
    LinePoint *p1,*p2;
    PlaneSeg *apPSeg[2];
    AstParse *pParse = pCorner->pLeft->pParse;
    LineSeg* apBottom[2];
    PointData *apPoint[2];
    LinePoint *pNode;
    LineSeg* pCross;
    PointData *pDir;
    int dir;
    int bl,br;
    int aDir[2][2];
    LineSeg* apPairSeg;

    apSeg[0] = *GetLineSegAddr(pCorner->pLeft,pCorner->pVertex);
    apSeg[1] = *GetLineSegAddr(pCorner->pRight,pCorner->pVertex);
    for(p1=apSeg[0]->pLine->pHead->pNext;!p1->isHead;p1=p1->pNext)
    {
        for(p2=apSeg[1]->pLine->pHead->pNext;!p2->isHead;p2=p2->pNext)
        {
            pCross = CreateNewLine(pParse,p1->pPoint,p2->pPoint);
            dir = GetLinesegDirect(pCross->pLine,p1->pPoint,p2->pPoint);
            printf("dir %s%s:%d\n",p1->pPoint->zSymb,p2->pPoint->zSymb,dir);
            apPoint[0] = p1->pPoint;
            apPoint[1] = p2->pPoint;
            for(int i=0;i<2;i++){
                pNode = GetLinePoint(apSeg[i]->pLine->pHead,apPoint[i]);
                if(i==0){
                    pDir = pNode->pNext->pPoint;
                }
                else{
                    pDir = pNode->pPre->pPoint;
                }
                assert(pDir!=NULL);
                apBottom[i] = CreateNewLine(pParse,apPoint[1-i],pDir);
                apPSeg[i] = GetAndSetAngle(pParse,apPoint[i],apBottom[i]);
                bl = GetLinesegDirect(pCross->pLine,apPoint[i],apPoint[1-i]);
                br = GetLinesegDirect(apSeg[i]->pLine,apPoint[i],pDir);
                if(pDir->isInf&&i==0){
                    if(apPSeg[i]->pCorner->pLine1!=pCross->pLine){
                        assert(br==BACK_DIRECT);
                        apPSeg[i]->pCorner->bl = FORWARD_DIRECT;
                    }
                    else{
                        assert(br==BACK_DIRECT);
                        apPSeg[i]->pCorner->br = FORWARD_DIRECT;
                    }
                    br = FORWARD_DIRECT;
                }
                aDir[i][0] = bl;
                aDir[i][1] = br;
            }
            assert(aDir[0][0]!=aDir[1][0]);
            assert(aDir[0][1]!=aDir[1][1]);
            apPairSeg = CreateNewLine(pParse,apPoint[0],apPoint[1]);
            InsertAnglePair(pParse,apPairSeg,apPSeg[0],apPSeg[1]);
        }
    }
}


void InsertInfnite(AstParse *pParse,PointData *pPoint,LineSeg* pSeg)
{
    LineSeg** ppLs1;
    LineSeg** ppLs2;

    ppLs1 = GetLineSegAddr(pPoint,pSeg->pLeft);
    ppLs2 = GetLineSegAddr(pPoint,pSeg->pRight);
    assert(*ppLs1==NULL);
    assert(*ppLs2==NULL);
    *ppLs1 = (LineSeg*)NewLinkHead(pSeg->pLine,sizeof(LineSeg));
    *ppLs2 = (LineSeg*)NewLinkHead(pSeg->pLine,sizeof(LineSeg));
    SetSegPoint(*ppLs1,pPoint,pSeg->pLeft);
    SetSegPoint(*ppLs2,pPoint,pSeg->pRight);
    CheckNewSeg(pParse,pPoint,(*ppLs1)->pLine);
}

void SetParallel(AstParse *pParse,GeomType *pLeft,GeomType *pRight)
{
    LinePoint *apHead[2];
    PointData *pInf;
    LineSeg* apSide[2];
    GeomType ele;
    PlaneSeg *pPSeg;
    int bl;
    int br;

    apHead[0] = pLeft->pLine1->pHead;
    apHead[1] = pRight->pLine1->pHead;
    apSide[0] = *GetLineSegAddr(pLeft->pPoint1,pLeft->pPoint2);
    apSide[1] = *GetLineSegAddr(pRight->pPoint1,pRight->pPoint2);
    bl = GetLinesegDirect(pLeft->pLine1,pLeft->pPoint1,pLeft->pPoint2);
    br = GetLinesegDirect(pRight->pLine1,pRight->pPoint1,pRight->pPoint2);
    assert(bl==br);
    if(apHead[0]->pPoint==NULL&&apHead[1]->pPoint==NULL){
        pInf = NewPointObj(pParse);
        pInf->zSymb = Malloc(10);
        sprintf(pInf->zSymb,"_%d_",pInf->iNum);
        pInf->isInf = 1;
        apHead[0]->pPoint = pInf;
        apHead[1]->pPoint = pInf;
        InsertInfnite(pParse,pInf,apSide[0]);
        InsertInfnite(pParse,pInf,apSide[1]);

        ele.type = ELE_ANGLE;
        ele.pVertex = pInf;
        ele.pLine1 = apSide[0]->pLine;
        ele.pLine2 = apSide[1]->pLine;
        ele.pPoint1 = pLeft->pPoint1;
        ele.pPoint2 = pRight->pPoint1;
        pPSeg = SetAngleHash(pParse,&ele);
        PrintParall(pPSeg->pCorner);
        SetParallAngle(pPSeg->pCorner);
    }
    else{
        assert(0);
        //InsertPlaneLine
    }
}
