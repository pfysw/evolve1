/*
 * ast.c
 *
 *  Created on: Aug 26, 2019
 *      Author: Administrator
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include <assert.h>
#include "db.h"
#include "geometry.h"

//#define FREE_TEST

MemInfo g_mem = {0};

void *addr_buf[1000] = {0};
void* Malloc(u32 size){
    void *p = malloc(size);
#ifdef FREE_TEST
    if(g_mem.malloc_cnt==364){
        printf("mal\n");
    }
    addr_buf[g_mem.malloc_cnt] = p;
#endif
    g_mem.malloc_cnt++;
    return p;
}
void Free(void *p){
#ifdef FREE_TEST
    for(int i=0;i<g_mem.malloc_cnt;i++){
        if(addr_buf[i]==p){
            addr_buf[i] = 0;
            break;
        }
    }
#endif
    //log_a("f");
    free(p);
    g_mem.free_cnt++;
}

void CheckFreeNum()
{
    printf("malloc %d free %d\n",g_mem.malloc_cnt,g_mem.free_cnt);
#ifdef FREE_TEST
    for(int i=0;i<g_mem.malloc_cnt;i++){
        if(addr_buf[i]!=0){
            printf("nofree %d\n",i);
        }
    }
#endif
}

void PrintAst(AstParse *pParse,TokenInfo *pAst)
{
    static int cnt = 0;
    static int nPrintSymb = 0;
    cnt++;
    if(cnt==1){
        nPrintSymb = 0;
    }
    assert(pAst!=NULL);
    if( pAst->type==PROP_SYMB )
    {
        if(pAst->bExist){
            log_c("exist ");
        }
        if(pAst->zSymb!=NULL){
            log_c("%s",pAst->zSymb);
        }
        else{
            log_c("%c",pAst->symb);
        }
        nPrintSymb++;
    }
    else if( pAst->type==PROP_NEG )
    {
        log_c("~");
        PrintAst(pParse,pAst->pLeft);
    }
    else
    {
        assert(pAst->type==PROP_IMPL);
        log_c("(");
        PrintAst(pParse,pAst->pLeft);
        //log_c("->");
        if(pAst->zSymb!=NULL){
            log_c("%s",pAst->zSymb);
            if(pAst->nSymbLen>2){
                log_c(" ");
            }
        }
        else{
            log_c("->");
        }
        if(nPrintSymb%10==0){
            log_a("");
        }
        PrintAst(pParse,pAst->pRight);
        log_c(")");

    }
    cnt--;
    if(!cnt)
    {
        log_a("");
        nPrintSymb = 0;
    }
}


void AstToString(AstParse *pParse,TokenInfo *pAst,char *buf)
{
    static int cnt = 0;
    static int nPrintSymb = 0;
    cnt++;
    if(cnt==1){
        nPrintSymb = 0;
        buf[0] = '\0';
    }
    assert(pAst!=NULL);
    if( pAst->type==PROP_SYMB )
    {
        if(pAst->zSymb!=NULL){
            //todo 如果以后这个函数被大量调用，那么strlen可能会影响性能
            sprintf(buf+strlen(buf),"%s",pAst->zSymb);
        }
        else{
            sprintf(buf+strlen(buf),"%c",pAst->symb);
        }
        nPrintSymb++;
    }
    else if( pAst->type==PROP_NEG )
    {
        sprintf(buf+strlen(buf),"~");
        AstToString(pParse,pAst->pLeft,buf);
    }
    else
    {
        assert(pAst->type==PROP_IMPL);
        sprintf(buf+strlen(buf),"(");
        AstToString(pParse,pAst->pLeft,buf);
        if(pAst->zSymb!=NULL){
            sprintf(buf+strlen(buf),"%s",pAst->zSymb);
        }
        else{
            sprintf(buf+strlen(buf),"->");
        }
        if(nPrintSymb%10==0){
            sprintf(buf+strlen(buf),"\n");
        }
        AstToString(pParse,pAst->pRight,buf);
        sprintf(buf+strlen(buf),")");

    }
    cnt--;
    if(!cnt)
    {
        sprintf(buf+strlen(buf),";");
        nPrintSymb = 0;
    }
}

void PrintSubstAst(AstParse *pParse,TokenInfo *pAst)
{
    static int cnt = 0;
    cnt++;
    assert(pAst!=NULL);

    if( pAst->type==PROP_SYMB )
    {
        if( pAst->bSubst )
        {
            if( pAst->pSubst->type==PROP_SYMB &&
                    !pAst->pSubst->bSubst )
            {
                log_c("%c",pAst->pSubst->symb);
            }
            else
            {
                PrintSubstAst(pParse,pAst->pSubst);
            }
        }
        else
        {
            log_c("%c",pAst->symb);
        }
    }
    else if( pAst->type==PROP_NEG )
    {
        log_c("~");
        PrintSubstAst(pParse,pAst->pLeft);
    }
    else
    {
        assert(pAst->type==PROP_IMPL);
        log_c("(");
        PrintSubstAst(pParse,pAst->pLeft);
        log_c("->");
        PrintSubstAst(pParse,pAst->pRight);
        log_c(")");
    }
    cnt--;
    if(!cnt)
    {
        log_a("");
    }
}

TokenInfo *NewNode(AstParse *pParse)
{
    TokenInfo *p;
    p = (TokenInfo *)Malloc(sizeof(TokenInfo));
    memset(p,0,sizeof(TokenInfo));
    return p;
}
void FreeAstNode(AstParse *pParse,TokenInfo *p)
{
    assert(p!=NULL);
    if(p->type==PROP_SYMB || p->type==PROP_IMPL)
    {
        if(p->zSymb!=NULL){
            Free(p->zSymb);
            p->zSymb = NULL;
        }
    }
    Free(p);
}

TokenInfo *NewTempNode(AstParse *pParse)
{
    TokenInfo *p;
    Mem5Global *pMem = pParse->pMem;
    p = (TokenInfo *)memsys5Malloc(pMem,sizeof(TokenInfo));
    memset(p,0,sizeof(TokenInfo));
    pParse->test += sizeof(TokenInfo);
    return p;
}

void FreeAstTree(
        AstParse *pParse,
        TokenInfo **ppAst,
        TokenInfo **ppTemp)
{
    static int cnt = 0;
    cnt++;
    static int n = 0;
    int i;

    assert((*ppAst)!=NULL);

    for(i=0;i<n;i++)
    {
        if( ppTemp[i]==*ppAst )
        {
            goto end;
        }
    }
    if( (*ppAst)->type==PROP_SYMB )
    {
        FreeAstNode(pParse,(*ppAst));
        assert(n<90);
        ppTemp[n++] = *ppAst;
    }
    else if( (*ppAst)->type==PROP_NEG )
    {
        FreeAstTree(pParse,&((*ppAst)->pLeft),ppTemp);
        FreeAstNode(pParse,(*ppAst));
    }
    else
    {
        assert((*ppAst)->type==PROP_IMPL);
        FreeAstTree(pParse,&((*ppAst)->pLeft),ppTemp);
        FreeAstTree(pParse,&((*ppAst)->pRight),ppTemp);
        FreeAstNode(pParse,(*ppAst));
    }

end:
    *ppAst = NULL;
    cnt--;
    if(!cnt) n = 0;
}

void FreeNewImplyNodes(AstParse *pParse,TokenInfo **ppAst)
{
    assert((*ppAst)!=NULL);

    if((*ppAst)->type==PROP_IMPL)
    {
        FreeNewImplyNodes(pParse,&((*ppAst)->pLeft));
        FreeNewImplyNodes(pParse,&((*ppAst)->pRight));
        if((*ppAst)->isNewTemp){
            FreeAstNode(pParse,(*ppAst));
        }
        *ppAst = NULL;
    }

}

void NewSymbString(AstParse *pParse,TokenInfo *p)
{
    char temp[100] = {0};
    assert(p->nSymbLen<10);
    memcpy(temp,p->zSymb,p->nSymbLen);
    p->zSymb = Malloc(p->nSymbLen+1);
    memcpy(p->zSymb,temp,p->nSymbLen+1);
}
void NewSymbStr(AstParse *pParse,TokenInfo *p)
{
    char temp[100] = {0};
    Mem5Global *pMem = pParse->pMem;
    assert(p->nSymbLen<10);
    memcpy(temp,p->zSymb,p->nSymbLen);
    p->zSymb = memsys5Malloc(pMem,p->nSymbLen+1);
    memcpy(p->zSymb,temp,p->nSymbLen+1);
}

void SetSymb(AstParse *pParse, TokenInfo *pB)
{
//分配字符串
    NewSymbString(pParse,pB);
    pB->type = PROP_SYMB;
}

AstParse *CreatAstParse(void){
    AstParse *pParse;
    char aNum[] = "123";
    int i = 0;

    pParse = (AstParse *)Malloc(sizeof(AstParse));
    memset(pParse,0,sizeof(AstParse));
    pParse->pDb = (DbInfo*)Malloc(sizeof(DbInfo));
    pParse->pDb->db = CreatSqliteConn("test.db");
    pParse->pPointSet = CreatPointHash(128);
    pParse->pLineSet = CreatLineHash(128);
    pParse->pPlaneSet = CreatPlaneHash(128);

    for(i=0;i<3;i++){
        pParse->apAxiom[i] = NewNode(pParse);
        pParse->apAxiom[i]->symb = aNum[i];
        pParse->apAxiom[i]->type = PROP_SYMB;
        pParse->apAxiom[i]->zSymb = &aNum[i];
        pParse->apAxiom[i]->nSymbLen = 1;
        NewSymbString(pParse,pParse->apAxiom[i]);
    }
    pParse->ppTemp = (TokenInfo **)Malloc(100*sizeof(TokenInfo *));

    return pParse;
}

void CloseAstParse(AstParse *pParse)
{
    sqlite3_close(pParse->pDb->db);
    //todo close pPointSet
    Free(pParse->pDb);
    Free(pParse->ppTemp);
    Free(pParse);
}

void WritePropStr(
        AstParse *pParse,
        TokenInfo *pA,
        TokenInfo *pB,
        TokenInfo *pC,
        char *op)
{
    char apBuf[4][PROP_STR_LEN] = {0};
    AstToString(pParse,pA,apBuf[0]);
    AstToString(pParse,pB,apBuf[1]);
    strcpy(apBuf[2],op);
    AstToString(pParse,pC,apBuf[3]);
    WritePropToDb(pParse,apBuf);
}

void WriteAxiomStr(AstParse *pParse,TokenInfo *pA)
{
    char buf[PROP_STR_LEN] = {0};
    AstToString(pParse,pA,buf);
    WriteAxiomToDb(pParse,buf);
}

Mem5Global * NewMemPool(AstParse *pParse,int len)
{
    Mem5Global *pMem;
    pMem = memsys5Init(len,16);
    return pMem;
}

void FreeMemPool(AstParse *pParse,Mem5Global **ppMem)
{
    memsys5Shutdown(ppMem);
    //log_a("pool len %d",pParse->test);
    pParse->test = 0;
}

void SetNegExpr(AstParse *pParse,TokenInfo *pA, TokenInfo *pB)
{
    pA->type = PROP_NEG;
    pA->pLeft = pB;
    //log_a("pB %s|%p",pB->zSymb,pB->zSymb);
}

TokenInfo * NewNegNode(AstParse *pParse,TokenInfo *pB)
{
    TokenInfo *pA =  NewTempNode(pParse);
    SetNegExpr(pParse,pA,pB);
    return pA;
}

void SetExprFlag(AstParse *pParse,TokenInfo *pB,TokenInfo *pC)
{
    if(!memcmp(pC->zSymb,"exist",pC->nSymbLen)){
        pB->bExist = 1;
    }
}

void SetImplExpr(
        AstParse *pParse,
        TokenInfo *pA,
        TokenInfo *pB,
        TokenInfo *pC,
        TokenInfo *pD)
{
    if(pD!=NULL){
        //printf("zSymb %s",pD->zSymb);
        if(!strcmp(pD->zSymb,"->")){
            pA->op = OP_IMPL;
        }
        else if(!strcmp(pD->zSymb,">")){
            pA->op = OP_MP;
        }
        else if(!strcmp(pD->zSymb,">>")){
            pA->op = OP_HS;
        }
        else if(!strcmp(pD->zSymb,"+")){
            pA->op = OP_ADD;
            pA->isDeduction = 1;
        }
        else if(!strcmp(pD->zSymb,"-")){
            pA->op = OP_LINE;
        }
        else if(!strcmp(pD->zSymb,"=")){
            pA->op = OP_EQUAL;
        }
        else if(!strcmp(pD->zSymb,"||")){
            pA->op = OP_PARALLEL;
        }
        else if(!strcmp(pD->zSymb,"val")){
            pB->val = atoi(pC->zSymb);
            pA->val = pB->val;
//            FreeAstNode(pParse,pA);
//            FreeAstNode(pParse,pC);
//            printf("val %d\n",pB->val);
//            return pB;
        }
        else{
            assert(0);
        }
        pA->zSymb = pD->zSymb;
        pA->nSymbLen = pD->nSymbLen;
        NewSymbString(pParse,pA);
        //pB->op = pA->op;
        //pC->op = pA->op;
    }

    pA->type = PROP_IMPL;
    pA->pLeft = pB;
    pA->pRight = pC;
    if(pB->isDeduction||pC->isDeduction)
    {
        pA->isDeduction = 1;
    }
}

TokenInfo * NewImplyNode(
        AstParse *pParse,
        TokenInfo *pB,
        TokenInfo *pC,
        char *zSymb)
{
    TokenInfo *pA =  NewTempNode(pParse);
    //printf("new \n");
    SetImplExpr(pParse,pA,pB,pC,NULL);
    pA->zSymb = zSymb;
    pA->nSymbLen = strlen(zSymb);
    if(!strcmp(zSymb,">")){
        pA->op = OP_MP;
    }
    else if(!strcmp(zSymb,">>")){
        pA->op = OP_HS;
    }
    else if(!strcmp(zSymb,"+")){
        pA->op = OP_ADD;
    }
    pA->isNewTemp = 1;
    //printf("end %d\n",pParse->test);
    return pA;
}

TokenInfo * NewSymbNode(AstParse *pParse,char *zSymb)
{
    TokenInfo *pA =  NewTempNode(pParse);
    pA->zSymb = zSymb;
    pA->nSymbLen = strlen(zSymb);
    NewSymbStr(pParse,pA);
    pA->symb = zSymb[0];
    pA->type = PROP_SYMB;
    return pA;
}

TokenInfo * NewNumNode(AstParse *pParse,int num)
{
    TokenInfo *pA;
    char zSymb[10] = {0};
    sprintf(zSymb,"%d",num);
    pA = NewSymbNode(pParse,zSymb);
    return pA;
}

TokenInfo *CopyAstTree(
        AstParse *pParse,
        TokenInfo *pSrc,
        u8 bSubst)
{
    TokenInfo *pDst;

    pDst = NewNode(pParse);

    pDst->type = pSrc->type;
    if( pSrc->type==PROP_SYMB )
    {
        if( bSubst && pSrc->bSubst )
        {
            if( pSrc->pSubst->type==PROP_SYMB  &&
                    !pSrc->pSubst->bSubst )
            {
                pDst->symb = pSrc->pSubst->copy;
               // pDst->symb = pSrc->pSubst->symb;
            }
            else
            {
                //如果不是PROP_SYMB，那么pSrc->pSubst->bSubst不可能是1
                while( pSrc->pSubst->bSubst )
                {
                    assert( pSrc->pSubst->type==PROP_SYMB );
                    pSrc = pSrc->pSubst;
                }
                if( pSrc->pSubst->type==PROP_SYMB )
                {
                    pDst->symb = pSrc->pSubst->copy;
                   //pDst->symb = pSrc->pSubst->symb;
                }
                else
                {
                    pDst->type = pSrc->pSubst->type;
                    pDst->pLeft = CopyAstTree(pParse,
                            pSrc->pSubst->pLeft,bSubst);
                    if( pSrc->pSubst->type==PROP_IMPL )
                    {

                        pDst->pRight = CopyAstTree(pParse,
                                pSrc->pSubst->pRight,bSubst);
                    }
                }
            }
        }
        else
        {
            if( bSubst )
                pDst->symb = pSrc->copy;
            else
                pDst->symb = pSrc->symb;
        }
    }
    else
    {
        pDst->pLeft = CopyAstTree(pParse,pSrc->pLeft,bSubst);
        if( pSrc->type==PROP_IMPL )
            pDst->pRight = CopyAstTree(pParse,pSrc->pRight,bSubst);
    }

    return pDst;
}
