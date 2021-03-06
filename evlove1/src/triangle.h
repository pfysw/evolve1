/*
 * triangle.h
 *
 *  Created on: Apr 3, 2021
 *      Author: Administrator
 */

#ifndef TRIANGLE_H_
#define TRIANGLE_H_

typedef struct TrigInfo TrigInfo;
struct TrigInfo{
    PointData *apPoint[3];
};

TrigInfo *SetTriangleObj(AstParse *pParse,TempInfo *pTemp);
void CheckSAS(PlaneSeg *pPSeg,SameLine *pS,SameAngle *pA,AngleTemp *pTemp);
void SetEqualTrgl(AstParse *pParse,TempInfo *pTemp);
void SetParallel(AstParse *pParse,GeomType *pLeft,GeomType *pRight);

#endif /* TRIANGLE_H_ */
