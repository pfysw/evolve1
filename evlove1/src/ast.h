/*
 * ast.h
 *
 *  Created on: Dec 13, 2020
 *      Author: Administrator
 */

#ifndef AST_H_
#define AST_H_
#include "token.h"
#include "mem5.h"

#define PROP_STR_LEN 200
typedef struct DbInfo DbInfo;
typedef struct PointHash PointHash;
typedef struct LineHash LineHash;
typedef struct PlaneHash PlaneHash;

typedef struct AstParse AstParse;
struct AstParse
{
    TokenInfo *pRoot;
    TokenInfo * apAxiom[3];
    TokenInfo **ppTemp;//存在递归时的共享变量
    Mem5Global *pMem;
    DbInfo *pDb;
    PointHash *pPointSet;
    LineHash *pLineSet;
    PlaneHash *pPlaneSet;
    u8 bDiscard;
    u8 usePool;
    int n;
    int cnt;
    int axiom_num;
    int all_num;
    int test;
};

typedef struct MemInfo MemInfo;
struct MemInfo{
    int malloc_cnt;
    int free_cnt;
};

void PrintAst(AstParse *pParse,TokenInfo *pAst);
void SetSymb(AstParse *pParse,TokenInfo *pB);
TokenInfo *NewNode(AstParse *pParse);
void SetNegExpr(AstParse *pParse,TokenInfo *pA, TokenInfo *pB);
void SetImplExpr(
        AstParse *pParse,
        TokenInfo *pA,
        TokenInfo *pB,
        TokenInfo *pC,
        TokenInfo *pD);
void FreeAstNode(AstParse *pParse,TokenInfo *p);
void FreeAstTree( AstParse *pParse,TokenInfo **ppAst,TokenInfo **ppTemp);
void PrintAstAddr(AstParse *pParse,TokenInfo *pAst);
void PrintSubstAst(AstParse *pParse,TokenInfo *pAst);
void AstToString(AstParse *pParse,TokenInfo *pAst,char *buf);
TokenInfo *CopyAstTree(
        AstParse *pParse,
        TokenInfo *pSrc,
        u8 bSubst);
AstParse *CreatAstParse(void);
TokenInfo * NewImplyNode(
        AstParse *pParse,
        TokenInfo *pB,
        TokenInfo *pC,
        char *zSymb);
void FreeNewImplyNodes(AstParse *pParse,TokenInfo **ppAst);
TokenInfo * NewNegNode(AstParse *pParse,TokenInfo *pB);
TokenInfo * NewSymbNode(AstParse *pParse,char *zSymb);
TokenInfo * NewNumNode(AstParse *pParse,int num);

Mem5Global * NewMemPool(AstParse *pParse,int len);
void FreeMemPool(AstParse *pParse,Mem5Global **ppMem);
void CloseAstParse(AstParse *pParse);
void WritePropStr(
        AstParse *pParse,
        TokenInfo *pA,
        TokenInfo *pB,
        TokenInfo *pC,
        char *op);
void WriteAxiomStr(AstParse *pParse,TokenInfo *pA);
void SetExprFlag(AstParse *pParse,TokenInfo *pB,TokenInfo *pC);
void* Malloc(u32 size);
void Free(void *p);
void CheckFreeNum();

#endif /* AST_H_ */
