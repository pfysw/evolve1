/*
 * main.c
 *
 *  Created on: Aug 9, 2019
 *      Author: Administrator
 */
#include <stdio.h>
#include <assert.h>
#include "lex.yy.h"
#include "ast.h"
#include "prop.h"
#include "db.h"
#include "geometry.h"



void* PropParseAlloc(void* (*allocProc)(size_t));
void* PropParse(void*, int, TokenInfo*,AstParse *pParse);
void* PropParseFree(void*, void(*freeProc)(void*));

FILE *BindScanFd(yyscan_t scanner,char *name)
{
    FILE *fd = NULL;
    if (!(fd = fopen(name, "r")))
    {
        printf("fopen %s error %p\n",name,fd);
        exit(0);
    }
    else
    {
        yyset_in(fd, scanner);
    }
    return fd;
}

FILE *BindMemFd(yyscan_t scanner,char *buf, int len)
{
    FILE *fd = NULL;
    if (!(fd = fmemopen(buf, len,"r")))
    {
        printf("fopen %s error %p\n",buf,fd);
        exit(0);
    }
    else
    {
        yyset_in(fd, scanner);
    }
    return fd;
}


void Token2AstTree(
        AstParse *pParse,
        yyscan_t scanner,
        Vector *pV)
{
    TokenInfo *pToken;
    void* pLemon = PropParseAlloc(malloc);
    char *zSymb;
    int token;
    int idx = 0;

    pToken = NewNode(pParse);
    token = yylex(scanner);
    zSymb = yyget_text(scanner);
    while (token)
    {
        //printf("symb %s token %d\n",zSymb,token);
        pToken->zSymb = zSymb;
        pToken->nSymbLen = yyget_leng(scanner);
        pToken->symb = pToken->zSymb[0];
        PropParse(pLemon, token, pToken,pParse);
        //PropParseTrace(stdout, "");
        if( token==TK_SEM )
        {
            PropParse(pLemon, 0, 0,pParse);
           log_a("----- %d -----",idx);
           PrintAst(pParse,pParse->pRoot);//
            idx++;
            InsertVector(pV,pParse->pRoot);
        }
        token = yylex(scanner);
        if( token )
        {
            zSymb = yyget_text(scanner);
            if(!memcmp(zSymb,"formula",7)){
                 pParse->axiom_num = idx;
                 token = yylex(scanner);//jump formula
                 token = yylex(scanner);//jump ;
                 zSymb = yyget_text(scanner);
            }
            if(!memcmp(zSymb,"end",3)){
                pParse->all_num = idx;
                break;
            }
            pToken = NewNode(pParse);
           continue;
        }
    }
    printf("end %d %s\n",token,yyget_text(scanner));
    PropParseFree(pLemon, free);
}

void GetPropStrParse(AstParse *pParse,char *buf,Vector *pV,int len)
{
    FILE *fd = NULL;
    yyscan_t scanner;
    yylex_init(&scanner);
    fd = BindMemFd(scanner,buf,len);
    Token2AstTree(pParse,scanner,pV);
    yylex_destroy(scanner);
    fclose(fd);
}

int main(int argc, char** argv)
{
   yyscan_t scanner;
   FILE *fd = NULL;
   AstParse *pParse;
   Vector *pSet;

   setbuf(stdout, NULL);
   yylex_init(&scanner);
   fd = BindScanFd(scanner,"in.sh");
   pParse = CreatAstParse();
   pSet = InitTheoremSet(pParse);
   Token2AstTree(pParse,scanner,pSet);
   yylex_destroy(scanner);
   fclose(fd);

   //GenBasicProp(pParse);
  // SubstSingleTest(pParse,theoremset.data);
   //SubstMpTest(pParse,pSet);

   ParseGeomEle(pParse,pSet);
   CloseGeomSet(pParse);
   FreeVector(pParse,pSet);
   for(int i=0;i<3;i++){
       FreeAstNode(pParse,pParse->apAxiom[i]);
   }
   CloseAstParse(pParse);
   printf("%ld\n",sizeof(TokenInfo));

   CheckFreeNum();
   //db_test();
   //mmap_demo();
   return 0;
}



