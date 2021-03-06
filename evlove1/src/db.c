/*
 * db.c
 *
 *  Created on: Dec 31, 2020
 *      Author: Administrator
 */
#include "db.h"
#include <stdio.h>
#include "ast.h"

//SQLite Backup API

#define DB_DISABLE 0
#define WRITE_DISABLE 1

/*
** This function is used to load the contents of a database file on disk
** into the "main" database of open database connection pInMemory, or
** to save the current contents of the database opened by pInMemory into
** a database file on disk. pInMemory is probably an in-memory database,
** but this function will also work fine if it is not.
**
** Parameter zFilename points to a nul-terminated string containing the
** name of the database file on disk to load from or save to. If parameter
** isSave is non-zero, then the contents of the file zFilename are
** overwritten with the contents of the database opened by pInMemory. If
** parameter isSave is zero, then the contents of the database opened by
** pInMemory are replaced by data loaded from the file zFilename.
**
** If the operation is successful, SQLITE_OK is returned. Otherwise, if
** an error occurs, an SQLite error code is returned.
*/
int loadOrSaveDb(sqlite3 *pInMemory, const char *zFilename, int isSave){
  int rc;                   /* Function return code */
  sqlite3 *pFile;           /* Database connection opened on zFilename */
  sqlite3_backup *pBackup;  /* Backup object used to copy data */
  sqlite3 *pTo;             /* Database to copy to (pFile or pInMemory) */
  sqlite3 *pFrom;           /* Database to copy from (pFile or pInMemory) */

  /* Open the database file identified by zFilename. Exit early if this fails
  ** for any reason. */
  rc = sqlite3_open(zFilename, &pFile);
  if( rc==SQLITE_OK ){

    /* If this is a 'load' operation (isSave==0), then data is copied
    ** from the database file just opened to database pInMemory.
    ** Otherwise, if this is a 'save' operation (isSave==1), then data
    ** is copied from pInMemory to pFile.  Set the variables pFrom and
    ** pTo accordingly. */
    pFrom = (isSave ? pInMemory : pFile);
    pTo   = (isSave ? pFile     : pInMemory);

    /* Set up the backup procedure to copy from the "main" database of
    ** connection pFile to the main database of connection pInMemory.
    ** If something goes wrong, pBackup will be set to NULL and an error
    ** code and message left in connection pTo.
    **
    ** If the backup object is successfully created, call backup_step()
    ** to copy data from pFile to pInMemory. Then call backup_finish()
    ** to release resources associated with the pBackup object.  If an
    ** error occurred, then an error code and message will be left in
    ** connection pTo. If no error occurred, then the error code belonging
    ** to pTo is set to SQLITE_OK.
    */
    pBackup = sqlite3_backup_init(pTo, "main", pFrom, "main");
    if( pBackup ){
      (void)sqlite3_backup_step(pBackup, -1);
      (void)sqlite3_backup_finish(pBackup);
    }
    rc = sqlite3_errcode(pTo);
  }

  /* Close the database connection opened on database file zFilename
  ** and return the result of this function. */
  (void)sqlite3_close(pFile);
  return rc;
}

/*
** Perform an online backup of database pDb to the database file named
** by zFilename. This function copies 5 database pages from pDb to
** zFilename, then unlocks pDb and sleeps for 250 ms, then repeats the
** process until the entire database is backed up.
**
** The third argument passed to this function must be a pointer to a progress
** function. After each set of 5 pages is backed up, the progress function
** is invoked with two integer parameters: the number of pages left to
** copy, and the total number of pages in the source file. This information
** may be used, for example, to update a GUI progress bar.
**
** While this function is running, another thread may use the database pDb, or
** another process may access the underlying database file via a separate
** connection.
**
** If the backup process is successfully completed, SQLITE_OK is returned.
** Otherwise, if an error occurs, an SQLite error code is returned.
*/
int backupDb(
  sqlite3 *pDb,               /* Database to back up */
  const char *zFilename,      /* Name of file to back up to */
  void(*xProgress)(int, int)  /* Progress function to invoke */
){
  int rc;                     /* Function return code */
  sqlite3 *pFile;             /* Database connection opened on zFilename */
  sqlite3_backup *pBackup;    /* Backup handle used to copy data */

  /* Open the database file identified by zFilename. */
  rc = sqlite3_open(zFilename, &pFile);
  if( rc==SQLITE_OK ){

    /* Open the sqlite3_backup object used to accomplish the transfer */
    pBackup = sqlite3_backup_init(pFile, "main", pDb, "main");
    if( pBackup ){

      /* Each iteration of this loop copies 5 database pages from database
      ** pDb to the backup database. If the return value of backup_step()
      ** indicates that there are still further pages to copy, sleep for
      ** 250 ms before repeating. */
      do {
        rc = sqlite3_backup_step(pBackup, 5);
        xProgress(
            //显示进度
            sqlite3_backup_remaining(pBackup),
            sqlite3_backup_pagecount(pBackup)
        );
        if( rc==SQLITE_OK || rc==SQLITE_BUSY || rc==SQLITE_LOCKED ){
          sqlite3_sleep(250);
        }
      } while( rc==SQLITE_OK || rc==SQLITE_BUSY || rc==SQLITE_LOCKED );

      /* Release resources allocated by backup_init(). */
      (void)sqlite3_backup_finish(pBackup);
    }
    rc = sqlite3_errcode(pFile);
  }

  /* Close the database connection opened on database file zFilename
  ** and return the result of this function. */
  (void)sqlite3_close(pFile);
  return rc;
}

int select_callback(void * data, int col_count, char ** col_values, char ** col_Name)
{
  // 每条记录回调一次该函数,有多少条就回调多少次
  printf( "%s = %s\n",col_Name[0],col_values[0] );
//  int x = atoi(col_values[0]);
//  printf("0x%x\n",x);
  return 1;
}

sqlite3 * CreatSqliteConn(char *db_name)
{
    sqlite3 *db = 0;
    sqlite3_config(SQLITE_CONFIG_URI,1);
    int rc = 0;
    char sSql[SQL_LEN] = {0};
    char * pErrMsg = 0;
#if DB_DISABLE
    return NULL;
#endif
    if ( !access(db_name,0) )
    {
        //printf("存在 退出\n");
       // exit(0);
#if !WRITE_DISABLE
        printf("存在 删除\n");
        remove(db_name);
        sleep(1);
#endif
    }
    rc = sqlite3_open(db_name, &db);
    if( rc != SQLITE_OK ) {
      fprintf(stderr, "open file: %s", sqlite3_errmsg(db));
      return NULL;
    }
    sprintf(sSql,"CREATE TABLE TheoremSet(gen TEXT,left TEXT,op TEXT,"
                 "right TEXT,primary key(gen,left))");
    rc = sqlite3_exec( db, sSql, 0, 0, &pErrMsg );
    if(rc!= SQLITE_OK ){
      fprintf(stderr, "CREATE TABLE error[%d]: %s\n",rc, pErrMsg);
      sqlite3_free(pErrMsg);
    }
    return db;
}

void SqliteReadDemo(sqlite3 *db,char *zSql);

void db_test()
{
    sqlite3 *db = 0;
    int rc = 0;
    rc = sqlite3_open(":memory:", &db);
    char sSql[SQL_LEN] = {0};
    char * pErrMsg = 0;
    sprintf(sSql,"CREATE TABLE TheoremSet(a INTEGER)");
    rc = sqlite3_exec( db, sSql, 0, 0, &pErrMsg );
    if(rc!= SQLITE_OK ){
      fprintf(stderr, "CREATE TABLE error[%d]: %s\n",rc, pErrMsg);
      sqlite3_free(pErrMsg);
    }
    sprintf(sSql,"INSERT INTO TheoremSet VALUES(%p)",&db);
    rc = sqlite3_exec( db, sSql, 0, 0, &pErrMsg );
    if(rc!= SQLITE_OK ){
      fprintf(stderr, "insert error[%d]: %s\n",rc, pErrMsg);
      sqlite3_free(pErrMsg);
    }
//    SqliteReadDemo(db,"SELECT * FROM TheoremSet");
//    int ss;
//    u32 s;
//    printf("db %p ss %d\n",&db,sizeof(ss));

       sqlite3_int64 addr;
       sqlite3_stmt *pstmt;
       const char *sql = "SELECT * FROM TheoremSet";
       rc = sqlite3_prepare_v2(db, sql, strlen(sql), &pstmt, 0);

       while(sqlite3_step( pstmt ) == SQLITE_ROW){
           addr = sqlite3_column_int64(pstmt, 0);
           printf("0x%llx %p\n", addr,&db);
       }
       sqlite3_finalize(pstmt);
       printf("a %p sql %s\n", &pstmt,sql);
       memcpy(&db,&addr,8);
       printf("b %p sql %s\n", &pstmt,sql);
       rc = sqlite3_prepare_v2(db, sql, strlen(sql), &pstmt, 0);
       printf("rc %d\n",rc);
       while(sqlite3_step( pstmt ) == SQLITE_ROW){
           addr = sqlite3_column_int64(pstmt, 0);
           printf("0x%llx %p\n", addr,&db);
       }
       sqlite3_finalize(pstmt);
//       extern int sqlite3OsShmMap(sqlite3_file *,int,int,int,void volatile **);
//       sqlite3OsShmMap(0,0,0,0,0);
      // dll_test();
//       sqlite3OsOpen(0,0,0,0,0);
//       extern int sqlite3OsShmMap(
//         sqlite3_file *id,               /* Database file handle */
//         int iPage,
//         int pgsz,
//         int bExtend,                    /* True to extend file if necessary */
//         void volatile **pp              /* OUT: Pointer to mapping */
//       );
//       sqlite3OsShmMap(0,0,0,0,0);
}


void BeginSqliteWrite(AstParse *pParse)
{
    char sql[SQL_LEN] = {0};
    int rc;
    sqlite3 *db = pParse->pDb->db;
    sqlite3_stmt *stmt;

    if(db==NULL){
        return;
    }

    sqlite3_exec(db,"begin;",0,0,0);
    sprintf(sql,"insert into TheoremSet values(?,?,?,?)");
    rc = sqlite3_prepare_v2(db,sql,strlen(sql),&stmt,0);
    if(rc!=SQLITE_OK ){
        printf("sqlite3_prepare_v2 error %d\n",rc);
    }
    pParse->pDb->stmt = stmt;
}

void EndSqliteWrite(AstParse *pParse)
{
    int rc;
    sqlite3 *db = pParse->pDb->db;
    sqlite3_stmt *stmt = pParse->pDb->stmt;

    if(db==NULL){
        return;
    }
    sqlite3_finalize(stmt);
    pParse->pDb->stmt = NULL;
    rc = sqlite3_exec(db,"commit;",0,0,0);
    if(rc!=SQLITE_OK ){
        printf("commit error %d\n",rc);
    }
}

void WritePropToDb(AstParse *pParse,char azProp[][PROP_STR_LEN])
{
    int rc;
    sqlite3_stmt *stmt;

#if WRITE_DISABLE
    return;
#endif
    if(pParse->pDb->db==NULL){
        return;
    }
    stmt = pParse->pDb->stmt;
    for(int i=0;i<4;i++){
        rc = sqlite3_bind_text(stmt, i+1, azProp[i], strlen(azProp[i]), NULL);
        if(rc!=SQLITE_OK ){
            printf("sqlite3_bind_text error %d\n",rc);
        }
    }

    rc = sqlite3_step(stmt);
    if(rc!=SQLITE_DONE && rc!=SQLITE_CONSTRAINT ){
        printf("sqlite3_step error %d\n",rc);
    }
    else if(rc==SQLITE_DONE){
        printf("sqlite3_step insert success\n");
    }
    sqlite3_reset(stmt);
}

void WriteAxiomToDb(AstParse *pParse,char *zProp)
{
    int rc;
    sqlite3_stmt *stmt;

    if(pParse->pDb->db==NULL){
        return;
    }
    stmt = pParse->pDb->stmt;

    rc = sqlite3_bind_text(stmt, 1, zProp, strlen(zProp), NULL);
    if(rc!=SQLITE_OK ){
        printf("sqlite3_bind_text error %d\n",rc);
    }
    for(int i=1;i<4;i++){
        rc = sqlite3_bind_text(stmt, i+1, "no", strlen("no"), NULL);
        if(rc!=SQLITE_OK ){
            printf("sqlite3_bind_null error %d\n",rc);
        }
    }

    rc = sqlite3_step(stmt);
    if(rc!=SQLITE_DONE && rc!=SQLITE_CONSTRAINT){
        printf("sqlite3_step error %d\n",rc);
    }
    sqlite3_reset(stmt);
}

void SqliteReadDemo(sqlite3 *db,char *zSql)
{
    char * pErrMsg = 0;
    sqlite3_exec( db, zSql, select_callback, 0, &pErrMsg);
}

void SqliteReadTable(AstParse *pParse,sqlite3 *db,char *table,Vector *pV)
{
    char * pErrMsg = 0;
    char sql[SQL_LEN] = {0};
    int nRow,nCol;
    char **dbResult;
    int rc;
    char *pMemFile;
    int i;
    int offset = 0;
    int nLen;
    int nFile = 0;

    sprintf(sql,"select rowid,gen from %s ORDER BY rowid",table);
    rc = sqlite3_get_table(db, sql, &dbResult, &nRow, &nCol, &pErrMsg);
    if(rc!= SQLITE_OK ){
      fprintf(stderr, "SELECT error[%d]: %s\n",rc, pErrMsg);
      sqlite3_free(pErrMsg);
    }
    printf("row %d\n",nRow);
    nFile = nRow*PROP_STR_LEN;
    pMemFile = (char *)Malloc(nFile);
    memset(pMemFile,0,nFile);
    for(i=1;i<nRow+1;i++){
        printf("%s = %s\n",dbResult[i*2],dbResult[i*2+1]);
        nLen = strlen(dbResult[i*2+1]);
        memcpy(pMemFile+offset,dbResult[i*2+1],nLen);
        offset += nLen;
    }
    GetPropStrParse(pParse,pMemFile,pV,nFile);
    sqlite3_free_table(dbResult);
    Free(pMemFile);
}
