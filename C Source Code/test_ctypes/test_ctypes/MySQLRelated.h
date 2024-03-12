#ifndef MYSQLRELATED_H
#define MYSQLRELATED_H

void UseDatabase(MYSQL* conn);

void CreateTables(MYSQL* conn);

MYSQL* ConnectDatabase();

void ExecuteSQLBatch(FILE* file, MYSQL* conn);

void CollectDirsInfo(const TCHAR* path, MYSQL* conn,TreeNode*root);

void BatchInsert(MYSQL* conn);

void ExecuteBatchInsert(FILE* file, MYSQL* conn,BOOL flag);

void DropDatabase(MYSQL* conn);




#endif // MYSQLRELATED_H
