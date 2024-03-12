
#include<windows.h>
#include<stdio.h>
#include<stdlib.h>
#include<tchar.h>
#include<mysql.h>
#include<string.h>


#include"NeededStructure.h"
#include"TreeRelated.h"
#include"AllStack.h"
#include"MySQLRelated.h"
#include"Transformer.h"
#include"DirectoryStats.h"


/*
 * 函数: ScanOnly
 * -------------
 * 仅扫描指定目录。
 *
 * scan_dir: 要扫描的目录路径。
 *
 * 返回: int
 * 扫描操作的结果，通常为0表示成功。
 *
 * 此函数扫描指定目录，构建目录树，然后释放树的内存。
 */
__declspec(dllexport) int ScanOnly(const char* scan_dir)
{
	// 分配内存以存储目录路径
	TCHAR* directoryPath = (TCHAR*)malloc(MAX_PATH * sizeof(TCHAR));
	// 将传入的目录路径转换为 TCHAR 类型
	CharToTChar(scan_dir, directoryPath, MAX_PATH);

	// 分配内存以存储根节点
	TreeNode* root = (TreeNode*)malloc(sizeof(TreeNode));
	// 构建目录树
	BuildDirectoryTree(root, directoryPath);

	// 释放目录树的内存
	FreeRoot(root);
	// 释放目录路径内存
	free(directoryPath);

	return 0; // 返回成功
}

/*
 * 函数: ScanAndStats
 * -----------------
 * 扫描指定目录并进行统计。
 *
 * scan_dir: 要扫描的目录路径。
 * mystat: 包含统计信息的文件路径。
 *
 * 返回: int
 * 操作的结果，通常为0表示成功。
 *
 * 此函数扫描指定目录，构建目录树并添加节点信息，连接到数据库，删除数据库（如果存在），生成批量插入数据并将数据插入数据库，
 * 最后收集目录信息并将结果写入到数据库中。
 */
__declspec(dllexport) int ScanAndStats(const char* scan_dir, const char* mystat)
{
	// 分配内存以存储目录路径
	TCHAR* directoryPath = (TCHAR*)malloc(MAX_PATH * sizeof(TCHAR));
	// 将传入的目录路径转换为 TCHAR 类型
	CharToTChar(scan_dir, directoryPath, MAX_PATH);

	// 分配内存以存储根节点
	TreeNode* root = (TreeNode*)malloc(sizeof(TreeNode));
	// 构建目录树
	BuildDirectoryTree(root, directoryPath);
	// 添加节点信息
	NonRecursiveTraverseAndAdd(root);

	// 连接到数据库
	MYSQL* conn = ConnectDatabase();
	// 删除数据库（如果存在）
	DropDatabase(conn);
	// 生成批量插入数据并将数据插入数据库
	GenerateBatchInsertFile(root);
	// 使用数据库
	UseDatabase(conn);
	// 创建数据库表
	CreateTables(conn);

	// 批量插入数据
	BatchInsert(conn);
	// 收集目录信息并将结果写入数据库
	CollectDirsInfo(mystat, conn, root);

	// 释放内存
	free(directoryPath);
	FreeRoot(root);

	return 0; // 返回成功
}


/*
 * 函数: ScanAndStatsAndDelete
 * --------------------------
 * 扫描指定目录并进行统计，然后根据给定的删除文件列表进行删除操作，并比较差异。
 *
 * scan_dir: 要扫描的目录路径。
 * mystat: 包含统计信息的文件路径。
 * mydir: 包含删除文件列表的文件路径。
 *
 * 返回: int
 * 操作的结果，通常为0表示成功。
 *
 * 此函数扫描指定目录，构建目录树并添加节点信息，连接到数据库，删除数据库（如果存在），生成批量插入数据并将数据插入数据库，
 * 最后收集目录信息并将结果写入到数据库中。然后根据给定的删除文件列表文件进行文件删除操作，并比较差异。
 */
__declspec(dllexport) int ScanAndStatsAndDelete(const char* scan_dir, const char* mystat, const char* mydir)
{
	// 分配内存以存储目录路径
	TCHAR* directoryPath = (TCHAR*)malloc(MAX_PATH * sizeof(TCHAR));
	// 将传入的目录路径转换为 TCHAR 类型
	CharToTChar(scan_dir, directoryPath, MAX_PATH);

	// 分配内存以存储根节点
	TreeNode* root = (TreeNode*)malloc(sizeof(TreeNode));
	// 构建目录树
	BuildDirectoryTree(root, directoryPath);
	// 添加节点信息
	NonRecursiveTraverseAndAdd(root);

	// 连接到数据库
	MYSQL* conn = ConnectDatabase();
	// 删除数据库（如果存在）
	DropDatabase(conn);
	// 生成批量插入数据并将数据插入数据库
	GenerateBatchInsertFile(root);
	// 使用数据库
	UseDatabase(conn);
	// 创建数据库表
	CreateTables(conn);
	// 批量插入数据
	BatchInsert(conn);
	// 收集目录信息并将结果写入数据库
	CollectDirsInfo(mystat, conn, root);
	// 根据给定的删除文件列表文件进行文件删除操作
	ModifyTree_dir(mydir, conn, root);
	// 比较差异
	CompareDifference(mystat, conn, root);

	// 释放内存
	free(directoryPath);
	FreeRoot(root);

	return 0; // 返回成功
}


/*
 * 函数: ScanAndStatsAndDeleteFile
 * -------------------------------
 * 扫描指定目录并进行统计，然后根据给定的删除文件列表和修改文件列表进行删除和修改操作，并比较差异。
 *
 * scan_dir: 要扫描的目录路径。
 * mystat: 包含统计信息的文件路径。
 * mydir: 包含删除目录列表的文件路径。
 * myfile: 包含修改文件列表的文件路径。
 *
 * 返回: int
 * 操作的结果，通常为0表示成功。
 *
 * 此函数扫描指定目录，构建目录树并添加节点信息，连接到数据库，删除数据库（如果存在），生成批量插入数据并将数据插入数据库，
 * 最后收集目录信息并将结果写入到数据库中。然后根据给定的删除目录列表文件进行目录删除操作，再根据给定的修改文件列表文件进行文件修改操作，
 * 并比较差异。
 */
__declspec(dllexport) int ScanAndStatsAndDeleteFile(const char* scan_dir, const char* mystat, const char* mydir, const char* myfile)
{
	// 分配内存以存储目录路径
	TCHAR* directoryPath = (TCHAR*)malloc(MAX_PATH * sizeof(TCHAR));
	// 将传入的目录路径转换为 TCHAR 类型
	CharToTChar(scan_dir, directoryPath, MAX_PATH);

	// 分配内存以存储根节点
	TreeNode* root = (TreeNode*)malloc(sizeof(TreeNode));
	// 构建目录树
	BuildDirectoryTree(root, directoryPath);
	// 添加节点信息
	NonRecursiveTraverseAndAdd(root);

	// 连接到数据库
	MYSQL* conn = ConnectDatabase();
	// 删除数据库（如果存在）
	DropDatabase(conn);
	// 生成批量插入数据并将数据插入数据库
	GenerateBatchInsertFile(root);
	// 使用数据库
	UseDatabase(conn);
	// 创建数据库表
	CreateTables(conn);
	// 批量插入数据
	BatchInsert(conn);
	// 收集目录信息并将结果写入数据库
	CollectDirsInfo(mystat, conn, root);
	// 根据给定的删除目录列表文件进行目录删除操作
	ModifyTree_dir(mydir, conn, root);
	// 比较差异
	CompareDifference(mystat, conn, root);
	// 根据给定的修改文件列表文件进行文件修改操作
	ModifyTree_file(myfile, conn, root);
	// 再次比较差异
	CompareDifference(mystat, conn, root);

	// 释放内存
	free(directoryPath);
	FreeRoot(root);

	return 0; // 返回成功
}

