#define _CRT_SECURE_NO_WARNINGS
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
 * 函数: StackInit
 * --------------------
 * 初始化一个具有给定容量的栈。
 *
 * s: 指向要初始化的Stack结构的指针。
 * capacity: 指定栈的初始容量的整数。
 *
 * 返回: void
 *
 * 此函数通过为TreeNode指针数组（items）分配指定容量的内存来初始化栈。
 * 将栈的大小初始设置为0，表示栈为空。
 * 将栈的容量设置为提供的值。
 */
void StackInit(Stack* s, int capacity)
{
	s->items = (TreeNode**)malloc(capacity * sizeof(TreeNode*)); // 为TreeNode指针数组分配内存。
	s->size = 0; // 将栈的大小初始化为0，表示一个空栈。
	s->capacity = capacity; // 将栈的容量设置为提供的值。
}

/*
 * 函数: UseDatabase
 * --------------------
 * 使用数据库连接来创建一个名为file_scan的数据库（如果不存在）并将其设置为当前数据库。
 *
 * conn: 指向MYSQL结构的指针，表示数据库连接。
 *
 * 返回: void
 *
 * 此函数通过执行SQL语句来创建名为file_scan的数据库（如果尚不存在），然后将其设置为当前数据库。
 * 如果执行SQL语句失败，则输出错误消息到标准错误设备，并退出程序。
 */
void UseDatabase(MYSQL* conn)
{
	char pre[MAX_SQL_SIZE]; // 用于存储SQL语句的字符串数组
	sprintf_s(pre, MAX_SQL_SIZE, "CREATE DATABASE IF NOT EXISTS file_scan;"); // 创建数据库的SQL语句

	// 执行创建数据库的SQL语句
	if (mysql_query(conn, pre))
	{
		fprintf(stderr, "@@Failed to use database: %s\n", mysql_error(conn)); // 输出错误消息到标准错误设备
		exit(1); // 退出程序或按照应用程序的要求处理错误
	}

	sprintf_s(pre, MAX_SQL_SIZE, "USE file_scan; "); // 将文件扫描数据库设置为当前数据库的SQL语句

	// 执行将文件扫描数据库设置为当前数据库的SQL语句
	if (mysql_query(conn, pre))
	{
		fprintf(stderr, "!!!Failed to use database: %s\n", mysql_error(conn)); // 输出错误消息到标准错误设备
		exit(1); // 退出程序或按照应用程序的要求处理错误
	}
}

/*
 * 函数: CreateTables
 * --------------------
 * 在当前数据库中创建必要的表格，包括directories和files表格。
 *
 * conn: 指向MYSQL结构的指针，表示数据库连接。
 *
 * 返回: void
 *
 * 此函数通过执行SQL语句来在当前数据库中创建必要的表格，包括directories和files表格。
 * 如果执行SQL语句失败，则输出错误消息到标准错误设备，并退出程序。
 */
void CreateTables(MYSQL* conn)
{
	// SQL语句，用于创建directories表格
	char* create_directory_table = "CREATE TABLE IF NOT EXISTS directories ("
		"id INT AUTO_INCREMENT PRIMARY KEY, "
		"name VARCHAR(255) NOT NULL, "
		"path VARCHAR(255) NOT NULL, "
		"file_size BIGINT UNSIGNED,"
		"creation_time BIGINT UNSIGNED, "
		"file_count INT DEFAULT 0)";

	// SQL语句，用于创建files表格
	char* create_file_table = "CREATE TABLE IF NOT EXISTS files ("
		"id INT AUTO_INCREMENT PRIMARY KEY, "
		"name VARCHAR(255) NOT NULL, "
		"path VARCHAR(255) NOT NULL, "
		"file_size BIGINT UNSIGNED,"
		"creation_time BIGINT UNSIGNED)";

	// 执行创建directories表格的SQL语句
	if (mysql_query(conn, create_directory_table))
	{
		fprintf(stderr, "Failed to create directory table: %s\n", mysql_error(conn)); // 输出错误消息到标准错误设备
		exit(1); // 退出程序或按照应用程序的要求处理错误
	}

	// 执行创建files表格的SQL语句
	if (mysql_query(conn, create_file_table))
	{
		fprintf(stderr, "Failed to create file table: %s\n", mysql_error(conn)); // 输出错误消息到标准错误设备
		exit(1); // 退出程序或按照应用程序的要求处理错误
	}

	printf("Tables created successfully\n"); // 输出成功创建表格的消息
}


/*
 * 函数: ConnectDatabase
 * --------------------
 * 连接数据库并返回数据库连接对象。
 *
 * 返回: 指向MYSQL结构的指针，表示数据库连接。
 *
 * 此函数通过调用mysql_init()初始化一个MYSQL对象，然后调用mysql_real_connect()连接到MySQL服务器。
 * 如果连接失败，则输出错误消息到标准错误设备并退出程序。
 * 如果连接成功，则输出成功连接数据库的消息，并返回数据库连接对象。
 */
MYSQL* ConnectDatabase()
{
	MYSQL* conn = mysql_init(NULL); // 初始化一个MYSQL对象
	if (conn == NULL)
	{
		fprintf(stderr, "mysql_init() failed\n"); // 输出错误消息到标准错误设备
		exit(1); // 退出程序
	}

	// 尝试连接到MySQL服务器
	if (mysql_real_connect(conn, "localhost", "root", "root", NULL, 0, NULL, 0, CLIENT_MULTI_STATEMENTS) == NULL)
	{
		fprintf(stderr, "mysql_real_connect() failed\n"); // 输出错误消息到标准错误设备
		mysql_close(conn); // 关闭数据库连接
		exit(1); // 退出程序
	}

	printf("连接数据库成功\n"); // 输出成功连接数据库的消息
	return conn; // 返回数据库连接对象
}


/*
 * 函数: ExecuteSQLBatch
 * --------------------
 * 从文件中读取SQL语句并批量执行这些语句。
 *
 * file: 指向包含SQL语句的文件的指针。
 * conn: 指向MYSQL结构的指针，表示数据库连接。
 *
 * 返回: void
 *
 * 此函数从给定文件中逐行读取SQL语句，并将其添加到批处理查询字符串中。当批处理查询字符串达到200条语句时，
 * 或者文件结束时，将执行批处理查询。如果执行SQL语句失败，则输出错误消息到标准错误设备。
 */
void ExecuteSQLBatch(FILE* file, MYSQL* conn) {
	char query[MAX_SQL_SIZE]; // 用于存储读取的SQL语句
	char* batchQuery = (char*)malloc(MAX_SQL_SIZE * 200 * sizeof(char)); // 用于存储批处理查询字符串
	batchQuery[0] = '\0'; // 初始化批处理查询字符串为空
	int count = 0; // 计数器，记录当前批处理查询中的语句数量

	rewind(file); // 将文件指针重新定位到文件开头

	// 逐行读取并累积SQL语句
	while (fgets(query, MAX_SQL_SIZE, file) != NULL) {
		// 移除换行符
		query[strcspn(query, "\n")] = '\0';
		char escaped_query[MAX_SQL_SIZE];
		EscapeSQLString(escaped_query, query, MAX_SQL_SIZE); // 转义SQL语句中的特殊字符

		// 将读取的语句添加到批处理查询中
		strcat(batchQuery, escaped_query);
		count++;

		// 每200条语句或文件结束时执行批处理查询
		if (count >= 200) {
			mysql_set_server_option(conn, MYSQL_OPTION_MULTI_STATEMENTS_ON); // 启用多语句执行模式
			int res = mysql_query(conn, batchQuery); // 执行批处理查询
			mysql_set_server_option(conn, MYSQL_OPTION_MULTI_STATEMENTS_OFF); // 禁用多语句执行模式
			MYSQL_RES* my_res; // 查询结果
			do {
				my_res = mysql_store_result(conn);
				if (my_res) mysql_free_result(my_res);
			} while (!mysql_next_result(conn)); // 检查是否有更多的查询结果

			// 重置批处理查询字符串和计数器
			batchQuery[0] = '\0';
			count = 0;
		}
	}

	// 处理剩余的语句（如果有）
	if (batchQuery[0] != '\0') {
		mysql_set_server_option(conn, MYSQL_OPTION_MULTI_STATEMENTS_ON); // 启用多语句执行模式
		int res = mysql_query(conn, batchQuery); // 执行批处理查询
		mysql_set_server_option(conn, MYSQL_OPTION_MULTI_STATEMENTS_OFF); // 禁用多语句执行模式

		MYSQL_RES* my_res; // 查询结果
		do {
			my_res = mysql_store_result(conn);
			if (my_res) mysql_free_result(my_res);
		} while (!mysql_next_result(conn)); // 检查是否有更多的查询结果
	}

	free(batchQuery); // 释放动态分配的内存
	printf("Execute query succeeded\n"); // 输出成功执行查询的消息
}


/*
 * 函数: extractValues
 * --------------------
 * 从给定字符串中提取VALUES子句的内容。
 *
 * line: 包含SQL语句的字符串。
 * output: 用于存储提取的VALUES子句内容的字符数组。
 *
 * 返回: void
 *
 * 此函数查找给定字符串中的"VALUES "子句，并提取其内容到output数组中。
 * 如果未找到有效的"VALUES "子句或发生其他错误，则将"Invalid format"复制到output数组中。
 */
void extractValues(const char* line, char* output) {
	const char* start = strstr(line, "VALUES ") + 6; // 查找"VALUES "子句的起始位置
	const char* end = strrchr(line, ')'); // 查找字符串中最后一个')'的位置

	if (start && end && end > start) { // 如果找到有效的起始和结束位置
		size_t len = end - start + 1; // 计算子句内容的长度
		strncpy(output, start, len); // 复制子句内容到output数组
		output[len] = '\0'; // 确保字符串以null结尾
	}
	else {
		strcpy(output, "Invalid format"); // 处理无效格式的情况，将"Invalid format"复制到output数组中
	}
}


/*
 * 函数: ExecuteBatchInsert
 * --------------------
 * 从文件中读取SQL语句并执行批量插入操作。
 *
 * file: 指向包含SQL语句的文件的指针。
 * conn: 指向MYSQL结构的指针，表示数据库连接。
 * flag: 用于指示是向directories表格还是files表格执行插入操作的布尔值。
 *
 * 返回: void
 *
 * 此函数从给定文件中逐行读取SQL语句，并将其添加到批处理查询字符串中。当累计500条插入语句时，将执行一次插入操作。
 * 如果执行SQL语句失败，则输出错误消息到标准错误设备。
 */
void ExecuteBatchInsert(FILE* file, MYSQL* conn, BOOL flag)
{
	char* line = (char*)malloc(MAX_SQL_SIZE * sizeof(char)); // 用于存储读取的文件行
	if (line == NULL)
	{
		fprintf(stderr, "Failed to allocate memories for string:line\n"); // 输出内存分配错误消息到标准错误设备
		return;
	}
	line[0] = '\0';  // 初始化line数组

	char* query = (char*)malloc(500 * MAX_SQL_SIZE * sizeof(char)); // 用于存储插入查询语句的字符串
	if (query == NULL)
	{
		fprintf(stderr, "Failed to allocate memories for string:query\n"); // 输出内存分配错误消息到标准错误设备
		return;
	}
	query[0] = '\0';    // 初始化query数组

	int lineCount = 0; // 计数器，记录当前已读取的行数

	rewind(file); // 将文件指针重新定位到文件开头

	while (fgets(line, MAX_SQL_SIZE, file) != NULL) { // 逐行读取文件中的SQL语句
		if (lineCount == 0) // 如果是第一行
		{
			if (flag == TRUE) // 如果指示向directories表格执行插入操作
			{
				strcat(query, "INSERT INTO directories(name, path, file_size, creation_time, file_count) VALUES");
			}
			else // 如果指示向files表格执行插入操作
			{
				strcat(query, "INSERT INTO files ( name, path, file_size,creation_time) VALUES");
			}
		}

		char sub[MAX_SQL_SIZE]; // 用于存储从SQL语句中提取的VALUES子句
		extractValues(line, sub); // 从SQL语句中提取VALUES子句
		strcat(sub, ","); // 将逗号添加到子句末尾
		char escaped_sub[MAX_SQL_SIZE]; // 用于存储转义后的子句
		EscapeSQLString(escaped_sub, sub, MAX_SQL_SIZE); // 转义VALUES子句中的特殊字符
		strcat(query, escaped_sub); // 将转义后的子句添加到插入查询语句中
		lineCount++;
		if (lineCount == 500) { // 当累计500条插入语句时执行一次插入操作
			*strrchr(query, ',') = ';'; // 将最后一个逗号替换为分号，表示插入查询结束
			if (mysql_query(conn, query)) { // 执行插入查询
				fprintf(stderr, "%s\n", mysql_error(conn)); // 输出错误消息到标准错误设备
			}
			lineCount = 0; // 重置计数器
			strcpy(query, "\0"); // 清空插入查询字符串
		}
	}

	// 执行剩余的插入操作（如果有）
	if (lineCount > 0) {
		*strrchr(query, ',') = ';'; // 将最后一个逗号替换为分号，表示插入查询结束
		if (mysql_query(conn, query)) { // 执行插入查询
			fprintf(stderr, "%s\n", mysql_error(conn)); // 输出错误消息到标准错误设备
		}
	}
	free(query); // 释放动态分配的内存
	free(line); // 释放动态分配的内存
}


/*
 * 函数: StackPush
 * --------------------
 * 将一个元素压入栈中。
 *
 * s: 指向栈结构的指针。
 * item: 指向要压入栈的元素的指针。
 *
 * 返回: void
 *
 * 此函数将给定的元素压入栈中。如果栈已满，则扩展栈的容量为当前容量的两倍，并将元素压入栈顶。
 * 如果内存分配失败，则输出错误消息到标准错误设备并退出程序。
 */
void StackPush(Stack* s, TreeNode* item)
{
	if (s->size >= s->capacity) // 如果栈已满
	{
		s->capacity *= 2; // 扩展栈的容量为当前容量的两倍
		s->items = (TreeNode**)realloc(s->items, (s->capacity) * sizeof(TreeNode*)); // 重新分配栈空间
		if (s->items == NULL) // 如果内存分配失败
		{
			fprintf(stderr, "Failed to push a item\n"); // 输出错误消息到标准错误设备
			exit(-1); // 退出程序
		}
	}
	s->items[s->size++] = item; // 将元素压入栈顶并增加栈的大小
}


/*
 * 函数: StackPop
 * --------------------
 * 从栈中弹出一个元素。
 *
 * s: 指向栈结构的指针。
 *
 * 返回: 指向弹出的元素的指针，如果栈为空，则返回NULL。
 *
 * 此函数从栈顶弹出一个元素，并将栈的大小减小。如果栈为空，则返回NULL。
 */
TreeNode* StackPop(Stack* s)
{
	if (s->size == 0) // 如果栈为空
		return NULL; // 返回NULL
	return s->items[--(s->size)]; // 弹出栈顶元素并返回
}


/*
 * 函数: StackIsEmpty
 * --------------------
 * 检查栈是否为空。
 *
 * s: 指向栈结构的指针。
 *
 * 返回: 如果栈为空，则返回1；否则返回0。
 *
 * 此函数检查给定的栈是否为空。如果栈的大小为0，则返回1；否则返回0。
 */
int StackIsEmpty(Stack* s)
{
	return s->size == 0; // 如果栈的大小为0，则返回1；否则返回0
}


/*
 * 函数: Initialize
 * --------------------
 * 初始化DirectoryStats结构体。
 *
 * stats: 指向DirectoryStats结构的指针。
 *
 * 返回: void
 *
 * 此函数将给定的DirectoryStats结构体初始化为初始状态，即将各成员变量都设为0或默认值。
 */
void Initialize(DirectoryStats* stats)
{
	stats->longestFilePathLength = 0; // 最长文件路径长度设为0
	stats->maxDepth = 1; // 最大深度设为1
	stats->totalDirectories = 0; // 总目录数设为0
	stats->totalFiles = 0; // 总文件数设为0
	stats->maxLenth = 0; // 最大长度设为0
}


/*
 * 函数: PrintStats
 * --------------------
 * 打印DirectoryStats结构体中的统计信息。
 *
 * stats: 指向DirectoryStats结构的指针，包含要打印的统计信息。
 *
 * 返回: void
 *
 * 此函数打印给定的DirectoryStats结构体中的统计信息，包括总目录数、总文件数、最大目录深度、
 * 从根目录到叶节点的最大长度、最长文件路径以及最长文件路径长度。
 */
void PrintStats(const DirectoryStats* stats)
{
	printf("Total directories: %d\n", stats->totalDirectories); // 打印总目录数
	printf("Total files: %d\n", stats->totalFiles); // 打印总文件数
	printf("Max directory depth: %d\n", stats->maxDepth); // 打印最大目录深度
	printf("Max length from root to leaves: %d\n", stats->maxLenth); // 打印从根目录到叶节点的最大长度
	printf("Longest file path: %ls\n", stats->longestFilePath); // 打印最长文件路径
	printf("Longest file path length: %zu\n", stats->longestFilePathLength); // 打印最长文件路径长度
}

/*
 * 函数: StackFree
 * --------------------
 * 释放栈所占用的内存空间。
 *
 * s: 指向栈结构的指针。
 *
 * 返回: void
 *
 * 此函数释放栈结构所占用的内存空间，包括栈中元素所分配的内存空间。
 */
void StackFree(Stack* s)
{
	free(s->items); // 释放栈中元素所占用的内存空间
}


/*
 * 函数: NodeInitialize
 * --------------------
 * 初始化TreeNode结构体。
 *
 * node: 指向TreeNode结构的指针。
 *
 * 返回: void
 *
 * 此函数将给定的TreeNode结构体初始化为初始状态，即将各成员变量都设为NULL或0。
 */
void NodeInitialize(TreeNode* node)
{
	node->child = NULL; // 子节点指针设为NULL
	node->next = NULL; // 下一个兄弟节点指针设为NULL
	node->father = NULL; // 父节点指针设为NULL
	node->fileNum = 0; // 文件数量设为0
}

/*
 * 函数: CalculateDepth
 * --------------------
 * 计算树的深度。
 *
 * root: 指向树根节点的指针。
 *
 * 返回: 树的深度。
 *
 * 此函数计算给定树的深度，即树中从根节点到最远叶节点的最长路径的长度。
 */
int CalculateDepth(TreeNode* root)
{
	if (root == NULL)
	{
		return 0; // 如果根节点为空，则返回0
	}
	int siblingDepth = CalculateDepth(root->next); // 计算兄弟节点的深度
	int childDepth = CalculateDepth(root->child); // 计算子节点的深度
	return (childDepth > siblingDepth) ? childDepth + 1 : siblingDepth + 1; // 返回子节点深度和兄弟节点深度中的较大值加1
}


/*
 * 函数: InItilizeRoot
 * --------------------
 * 初始化根节点的信息。
 *
 * root: 指向根节点的指针。
 * path: 根节点表示的路径。
 *
 * 返回: void
 *
 * 此函数根据给定的路径初始化根节点的各项属性，包括深度、创建时间、路径、文件大小、是否为目录等。
 */
void InItilizeRoot(TreeNode* root, TCHAR* path)
{
	WIN32_FIND_DATA findFileData; // 用于存储文件查找数据的结构体
	HANDLE hFind = INVALID_HANDLE_VALUE; // 文件句柄
	TCHAR subDirPath[MAX_PATH]; // 子目录路径

	_stprintf_s(subDirPath, MAX_PATH, _T("%s\\*"), path); // 格式化子目录路径

	hFind = FindFirstFile(subDirPath, &findFileData); // 查找第一个文件
	root->depth = 1; // 根节点深度为1
	root->CreationTime = findFileData.ftCreationTime; // 设置根节点的创建时间
	_tcscpy_s(root->path, MAX_PATH, path); // 将路径赋给根节点的path属性
	root->FileSize = ((uint64_t)findFileData.nFileSizeHigh << 32) | findFileData.nFileSizeLow; // 设置根节点的文件大小
	root->isdir = TRUE; // 标记根节点为目录
	root->length = 0; // 根节点路径长度为0
	root->father = NULL; // 根节点的父节点为空
}

/*
 * 函数: BuildDirectoryTree
 * --------------------
 * 构建目录树。
 *
 * root: 指向树根节点的指针。
 * path: 根节点表示的路径。
 *
 * 返回: void
 *
 * 此函数根据给定的路径构建目录树，同时统计目录树的相关信息并打印到日志文件中。
 */
void BuildDirectoryTree(TreeNode* root, TCHAR* path)
{
	InItilizeRoot(root, path); // 初始化根节点的信息

	DirectoryStats stats; // 存储目录树统计信息的结构体
	Initialize(&stats); // 初始化统计信息

	WIN32_FIND_DATA findFileData; // 用于存储文件查找数据的结构体
	HANDLE hFind = INVALID_HANDLE_VALUE; // 文件句柄
	TCHAR subDirPath[MAX_PATH]; // 子目录路径

	Stack stack; // 堆栈用于存储目录树节点
	StackInit(&stack, 64); // 初始化堆栈
	StackPush(&stack, root); // 将根节点压入堆栈中
	NodeInitialize(root); // 初始化根节点的子节点指针和兄弟节点指针

	while (!StackIsEmpty(&stack)) // 当堆栈非空时
	{
		TreeNode* currentNode = StackPop(&stack); // 弹出堆栈顶部节点，表示当前节点

		_stprintf_s(subDirPath, MAX_PATH, _T("%s\\*"), currentNode->path); // 格式化当前节点路径下的所有文件和目录的搜索路径
		hFind = FindFirstFile(subDirPath, &findFileData); // 查找第一个文件或目录

		if (hFind == INVALID_HANDLE_VALUE) // 如果查找失败
		{
			currentNode->child = NULL; // 当前节点的子节点置空
			printf("FindFirstFile failed (%d)\n", GetLastError()); // 输出错误信息
			continue;
		}

		do
		{
			// 跳过当前目录和父目录
			if (_tcscmp(findFileData.cFileName, _T(".")) == 0 || _tcscmp(findFileData.cFileName, _T("..")) == 0)
			{
				continue;
			}
			if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				// 发现一个目录
				TreeNode* newNode = (TreeNode*)malloc(sizeof(TreeNode)); // 分配一个新节点
				if (newNode == NULL) // 如果分配失败
				{
					printf("Failed to allocate memory while adding a dirctory node\n"); // 输出错误信息
					StackFree(&stack); // 释放堆栈内存
					return;
				}
				NodeInitialize(newNode); // 初始化新节点

				newNode->depth = currentNode->depth + 1; // 设置新节点的深度
				newNode->CreationTime = findFileData.ftCreationTime; // 设置新节点的创建时间
				newNode->FileSize = ((uint64_t)findFileData.nFileSizeHigh << 32) | findFileData.nFileSizeLow; // 设置新节点的文件大小
				newNode->father = currentNode; // 设置新节点的父节点指针
				newNode->isdir = TRUE; // 标记新节点为目录

				// 将新节点添加为当前节点的子节点或兄弟节点
				if (currentNode->child == NULL)
				{
					currentNode->child = newNode;
					newNode->length = currentNode->length + 1;
				}
				else
				{
					TreeNode* temp = currentNode->child;
					while (temp->next != NULL)
					{
						temp = temp->next;
					}
					temp->next = newNode;
					newNode->length = temp->length + 1;
				}

				_stprintf_s(newNode->path, MAX_PATH, _T("%s\\%s"), currentNode->path, findFileData.cFileName); // 设置新节点的路径
				StackPush(&stack, newNode); // 将新节点压入堆栈中
				stats.totalDirectories++; // 更新目录数统计信息

				// 更新统计信息
				if (_tcslen(newNode->path) > stats.longestFilePathLength)
				{
					_tcscpy_s(stats.longestFilePath, MAX_PATH, newNode->path);
					stats.longestFilePathLength = _tcslen(newNode->path);
				}
				if (newNode->depth > stats.maxDepth)
				{
					stats.maxDepth = newNode->depth;
				}
				if (newNode->length > stats.maxLenth)
				{
					stats.maxLenth = newNode->length;
				}
			}
			else
			{
				// 发现一个文件
				TreeNode* newNode = (TreeNode*)malloc(sizeof(TreeNode)); // 分配一个新节点
				if (newNode == NULL) // 如果分配失败
				{
					printf("Failed to allocate memory while adding a file node\n"); // 输出错误信息
					StackFree(&stack); // 释放堆栈内存
					return;
				}
				NodeInitialize(newNode); // 初始化新节点

				newNode->depth = currentNode->depth + 1; // 设置新节点的深度
				newNode->CreationTime = findFileData.ftCreationTime; // 设置新节点的创建时间
				newNode->FileSize = ((uint64_t)findFileData.nFileSizeHigh << 32) | findFileData.nFileSizeLow; // 设置新节点的文件大小
				newNode->isdir = FALSE; // 标记新节点为文件
				newNode->father = currentNode; // 设置新节点的父节点指针

				// 将新节点添加为当前节点的子节点或兄弟节点
				if (currentNode->child == NULL)
				{
					currentNode->child = newNode;
					newNode->length = currentNode->length + 1;
				}
				else
				{
					TreeNode* temp = currentNode->child;
					while (temp->next != NULL)
					{
						temp = temp->next;
					}
					temp->next = newNode;
					newNode->length = temp->length + 1;
				}

				_stprintf_s(newNode->path, MAX_PATH, _T("%s\\%s"), currentNode->path, findFileData.cFileName); // 设置新节点的路径
				stats.totalFiles++; // 更新文件数统计信息

				// 更新统计信息
				if (_tcslen(newNode->path) > stats.longestFilePathLength)
				{
					_tcscpy_s(stats.longestFilePath, MAX_PATH, newNode->path);
					stats.longestFilePathLength = _tcslen(newNode->path);
				}
				if (newNode->depth > stats.maxDepth)
				{
					stats.maxDepth = newNode->depth;
				}
				if (newNode->length > stats.maxLenth)
				{
					stats.maxLenth = newNode->length;
				}
			}
		} while (FindNextFile(hFind, &findFileData) != FALSE); // 继续查找下一个文件或目录

		FindClose(hFind); // 关闭文件查找句柄
	}

	PrintStats(&stats); // 打印目录树统计信息

	root->fileNum = stats.totalFiles; // 设置根节点文件数属性

	StackFree(&stack); // 释放堆栈内存

	// 写入统计信息到日志文件
	FILE* f = fopen("..\\allfile\\ThisLog.txt", "a");
	if (f == NULL)
	{
		perror("Error opening logfile");
		exit(EXIT_FAILURE);
	}
	fprintf(f, "In directories:%ws\n", path);
	fprintf(f, "Total directories: %d\n", stats.totalDirectories);
	fprintf(f, "Total files: %d\n", stats.totalFiles);
	fprintf(f, "Max directory depth: %d\n", stats.maxDepth);
	fprintf(f, "Max length from root to leves:%d\n", stats.maxLenth);
	fprintf(f, "Longest file path: %ls\n", stats.longestFilePath);
	fprintf(f, "Longest file path length: %zu\n", stats.longestFilePathLength);
	fclose(f);
}


/*
 * 函数: NonRecursiveTraverseAndAdd
 * --------------------
 * 非递归遍历并更新节点的文件数量和父节点的文件数量。
 *
 * root: 指向树根节点的指针。
 *
 * 返回: void
 *
 * 此函数采用非递归方式遍历树中的所有节点，并根据节点的类型更新节点的文件数量和父节点的文件数量。
 */
void NonRecursiveTraverseAndAdd(TreeNode* root)
{
	if (root == NULL) return; // 如果根节点为空，则直接返回

	Stack stack; // 用于存储树节点的堆栈
	StackInit(&stack, 64); // 初始化堆栈
	StackPush(&stack, root); // 将根节点压入堆栈中

	while (!StackIsEmpty(&stack)) // 当堆栈非空时
	{
		TreeNode* currentNode = StackPop(&stack); // 弹出堆栈顶部节点，表示当前节点

		if (currentNode->isdir) // 如果当前节点是目录
		{
			currentNode->fileNum = 0; // 将当前节点的文件数量置为0
			TreeNode* child = currentNode->child; // 获取当前节点的第一个子节点
			while (child != NULL) // 遍历当前节点的所有子节点
			{
				StackPush(&stack, child); // 将子节点压入堆栈中
				child = child->next; // 移动到下一个子节点
			}
		}
		else // 如果当前节点是文件
		{
			currentNode->fileNum = 1; // 将当前节点的文件数量置为1
		}

		// 更新父节点的 fileNum 和 FileSize 属性
		if (currentNode->father != NULL && currentNode->isdir == FALSE) // 如果当前节点有父节点且当前节点是文件
		{
			TreeNode* parent = currentNode->father; // 获取当前节点的父节点
			while (parent != NULL) // 遍历所有父节点
			{
				parent->fileNum += 1; // 将父节点的文件数量加1
				parent->FileSize += currentNode->FileSize; // 将父节点的文件大小增加当前节点的文件大小
				parent = parent->father; // 移动到父节点的父节点
			}
		}
	}

	StackFree(&stack); // 释放堆栈内存
}


/*
 * 函数: TraverseAndAdd
 * --------------------
 * 递归遍历树并添加节点的文件数量。
 *
 * node: 指向要遍历的节点的指针。
 *
 * 返回: int
 *     返回节点的文件数量。
 *
 * 此函数递归遍历树中的所有节点，并根据节点的类型计算节点的文件数量。
 */
int TraverseAndAdd(TreeNode* node)
{
	if (node == NULL) return 0; // 如果节点为空，则返回0

	int fileCount = 0; // 文件数量初始化为0
	if (node->isdir) // 如果节点是目录
	{
		TreeNode* child = node->child; // 获取节点的第一个子节点
		while (child != NULL) // 遍历所有子节点
		{
			fileCount += TraverseAndAdd(child); // 递归调用 TraverseAndAdd 函数，并累加返回的文件数量
			child = child->next; // 移动到下一个子节点
		}
		node->fileNum = fileCount; // 将节点的文件数量设置为累加的文件数量
	}
	else // 如果节点是文件
	{
		// 插入文件信息到数据库
		node->fileNum = 1; // 将节点的文件数量设置为1
		fileCount = 1; // 文件数量设置为1
	}
	return fileCount; // 返回节点的文件数量
}



/*
 * 函数: GenerateBatchInsertFile
 * --------------------
 * 生成批量插入文件信息的SQL语句。
 *
 * root: 指向树根节点的指针。
 *
 * 返回: void
 *
 * 此函数调用 NewTraverseGenerate 函数来生成批量插入文件信息的SQL语句。
 */
void GenerateBatchInsertFile(TreeNode* root)
{
	if (root == NULL)
	{
		fprintf(stderr, "The root node is NULL. An error occurred in file insertion\n");
		exit(1); // 如果根节点为空，则输出错误消息并退出程序
	}
	NewTraverseGenerate(root); // 调用 NewTraverseGenerate 函数生成批量插入文件信息的SQL语句
}


/*
 * 函数: GetFileNameFromPath
 * --------------------
 * 从路径中获取文件名。
 *
 * path: 要提取文件名的路径。
 *
 * 返回: const TCHAR*
 *     返回指向提取的文件名的指针。
 *
 * 此函数从给定路径中提取文件名，并返回指向文件名的指针。
 */
const TCHAR* GetFileNameFromPath(const TCHAR* path)
{
	const TCHAR* lastSlash = _tcsrchr(path, '\\'); // 获取路径中最后一个反斜杠字符的位置
	if (lastSlash != NULL) {
		return lastSlash + 1; // 返回指向反斜杠后面字符的指针，即文件名的起始位置
	}
	else {
		return path; // 如果路径中没有反斜杠，则直接返回路径本身，因为路径本身就是文件名
	}
}


/*
 * 函数: PrintFileTime
 * --------------------
 * 将文件时间转换为字符串格式。
 *
 * ft: 要打印的文件时间。
 * dateTimeStr: 存储文件时间字符串的缓冲区。
 *
 * 返回: void
 *
 * 此函数将给定的文件时间转换为字符串格式，并存储在指定的缓冲区中。
 */
void PrintFileTime(FILETIME ft, char* dateTimeStr)
{
	SYSTEMTIME st;
	FileTimeToSystemTime(&ft, &st); // 将文件时间转换为系统时间格式

	// 将系统时间格式转换为指定的日期时间字符串格式
	sprintf_s(dateTimeStr, 40, "%04d-%02d-%02d %02d:%02d:%02d",
		st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
}


//这个函数已经被舍弃
/*
 * 函数: TraverseGenerate
 * --------------------
 * 遍历树并生成批量插入SQL语句文件。
 *
 * node: 指向当前节点的指针。
 * filecount: 指向文件数量计数器的指针。
 * dircount: 指向目录数量计数器的指针。
 *
 * 返回: void
 *
 * 此函数递归遍历树中的所有节点，并根据节点的类型生成批量插入SQL语句文件。
 */
void TraverseGenerate(TreeNode* node, int* filecount, int* dircount)
{
	if (node == NULL) return; // 如果当前节点为空，则直接返回

	char filename[15]; // 用于存储文件名的缓冲区
	if (node->isdir) // 如果当前节点是目录
	{
		sprintf_s(filename, 15, "dirdata%d", ((*dircount)++ / BATCH_SIZE) + 1); // 生成目录数据文件名
	}
	else // 如果当前节点是文件
	{
		sprintf_s(filename, 15, "filedata%d", ((*filecount)++ / BATCH_SIZE) + 1); // 生成文件数据文件名
	}

	FILE* file = fopen(filename, "a"); // 打开文件以追加模式写入数据
	if (file == NULL) // 如果文件打开失败
	{
		perror("Unable to open the file");
		exit(EXIT_FAILURE); // 输出错误消息并退出程序
	}

	char time[40]; // 用于存储时间字符串的缓冲区
	PrintFileTime(node->CreationTime, time); // 将文件时间转换为字符串格式

	// 根据节点类型生成相应的SQL语句，并写入文件
	if (node->isdir) // 如果当前节点是目录
	{
		char char_path[MAX_PATH];
		char char_name[MAX_PATH];
		TCharToChar(GetFileNameFromPath(node->path), char_name, MAX_PATH);
		TCharToChar(node->path, char_path, MAX_PATH);
		fprintf(file, "INSERT INTO directories ( name, path, file_size, creation_time, file_count) VALUES (%s, '%s', %llu, '%s', %d);\n",
			char_name, char_path, node->FileSize, time, node->fileNum);
	}
	else // 如果当前节点是文件
	{
		char char_path[MAX_PATH];
		char char_name[MAX_PATH];
		TCharToChar(GetFileNameFromPath(node->path), char_name, MAX_PATH);
		TCharToChar(node->path, char_path, MAX_PATH);
		fprintf(file, "INSERT INTO directories ( name, path, file_size, creation_time, file_count) VALUES (%s, '%s', %llu, '%s');\n",
			char_name, char_path, node->FileSize, time);
	}

	fclose(file); // 关闭文件

	// 递归遍历当前节点的子节点和兄弟节点
	TraverseGenerate(node->child, filecount, dircount); // 遍历子节点
	TraverseGenerate(node->next, filecount, dircount); // 遍历兄弟节点
}


/*
 * 函数: NewTraverseGenerate
 * -------------------------
 * 使用新的非递归遍历方法生成批量插入SQL语句文件。
 *
 * root: 指向树的根节点的指针。
 *
 * 返回: void
 *
 * 此函数使用新的非递归遍历方法遍历树中的所有节点，并根据节点类型生成批量插入SQL语句文件。
 */
void NewTraverseGenerate(TreeNode* root)
{
	Stack stack;
	StackInit(&stack, 64);
	StackPush(&stack, root); // 将根节点压入栈中

	// 打开目录数据文件和文件数据文件以追加模式写入数据
	FILE* file_dir = fopen("..\\allfile\\dirdata.txt", "a");
	FILE* file_file = fopen("..\\allfile\\filedata.txt", "a");
	if (file_dir == NULL || file_file == NULL) // 如果文件打开失败
	{
		perror("Unable to open the file");
		exit(EXIT_FAILURE); // 输出错误消息并退出程序
	}

	while (!StackIsEmpty(&stack)) // 当栈非空时循环遍历节点
	{
		TreeNode* currentNode = StackPop(&stack); // 弹出栈顶节点

		// 遍历当前节点及其所有兄弟节点
		do {
			if (currentNode->isdir) // 如果当前节点是目录
			{
				char char_path[MAX_PATH];
				char char_name[MAX_PATH];
				TCharToChar(GetFileNameFromPath(currentNode->path), char_name, MAX_PATH);
				TCharToChar(currentNode->path, char_path, MAX_PATH);
				// 将目录数据插入目录数据文件
				fprintf(file_dir, "INSERT INTO directories ( name, path, file_size, creation_time, file_count) VALUES (\"%s\", \"%s\", %llu, %llu, %d);\n",
					char_name, char_path, currentNode->FileSize, FileTimetoUlonglong(currentNode->CreationTime), currentNode->fileNum);
			}
			else // 如果当前节点是文件
			{
				char char_path[MAX_PATH];
				char char_name[MAX_PATH];
				TCharToChar(GetFileNameFromPath(currentNode->path), char_name, MAX_PATH);
				TCharToChar(currentNode->path, char_path, MAX_PATH);
				// 将文件数据插入文件数据文件
				fprintf(file_file, "INSERT INTO files ( name, path, file_size, creation_time) VALUES (\"%s\", \"%s\", %llu, %llu);\n",
					char_name, char_path, currentNode->FileSize, FileTimetoUlonglong(currentNode->CreationTime));
			}

			if (currentNode->child != NULL) // 如果当前节点有子节点，则将子节点压入栈中
				StackPush(&stack, currentNode->child);
			currentNode = currentNode->next; // 遍历下一个兄弟节点
		} while (currentNode != NULL); // 当前节点的所有兄弟节点遍历完毕

	}

	fclose(file_dir); // 关闭目录数据文件
	fclose(file_file); // 关闭文件数据文件
	StackFree(&stack); // 释放栈内存
}


/*
 * 函数: FreeTree
 * -------------
 * 递归释放树中的所有节点。
 *
 * node: 指向当前节点的指针。
 *
 * 返回: void
 *
 * 此函数递归地释放树中的所有节点。首先释放所有孩子节点，然后释放所有兄弟节点，最后释放节点本身。
 */
void FreeTree(TreeNode* node) {
	if (node == NULL) return; // 如果节点为空，直接返回

	// 递归释放所有孩子节点
	if (node->child != NULL) {
		FreeTree(node->child);
	}

	// 递归释放所有兄弟节点
	if (node->next != NULL) {
		FreeTree(node->next);
	}

	free(node); // 释放节点本身
}



/*
 * 函数: BatchInsert
 * ----------------
 * 执行批量插入操作，将文件中的数据批量插入到数据库中。
 *
 * conn: 指向已连接的数据库的指针。
 *
 * 返回: void
 *
 * 此函数打开文件，分别读取文件中的目录和文件数据，并调用 ExecuteBatchInsert 函数执行批量插入操作。
 * 最后关闭文件。
 */
void BatchInsert(MYSQL* conn) {
	FILE* file_file = fopen("..\\allfile\\filedata.txt", "r");
	if (file_file == NULL) {
		perror("Error opening file");
		exit(EXIT_FAILURE);
	}
	ExecuteBatchInsert(file_file, conn, FALSE); // 插入文件数据
	fclose(file_file);

	FILE* file_dir = fopen("..\\allfile\\dirdata.txt", "r");
	if (file_dir == NULL) {
		perror("Error opening file");
		exit(EXIT_FAILURE);
	}
	ExecuteBatchInsert(file_dir, conn, TRUE); // 插入目录数据
	fclose(file_dir);
}



/*
 * 函数: EscapeSQLString
 * ---------------------
 * 将源字符串中的特殊字符进行转义，并将结果存储在目标字符串中。
 *
 * dest: 目标字符串的指针，用于存储转义后的结果。
 * src: 源字符串的指针，需要转义的原始字符串。
 * destSize: 目标字符串的大小，包括结尾的 null 字符。
 *
 * 返回: void
 *
 * 此函数将源字符串中的特殊字符进行转义，并将转义后的结果存储在目标字符串中。
 * 转义规则：将源字符串中的 '\' 转义为 '\\'。
 * 如果目标字符串空间不足以存储转义后的结果，则停止转义操作。
 */
void EscapeSQLString(char* dest, const char* src, size_t destSize) {
	const char* srcPtr = src; // 源字符串指针
	char* destPtr = dest; // 目标字符串指针
	size_t remainingSize = destSize; // 目标字符串剩余空间大小

	// 循环处理源字符串中的每个字符，直到遇到空字符或目标字符串空间不足
	while (*srcPtr && remainingSize > 1) {
		// 如果当前字符是 '\'，需要转义
		if (*srcPtr == '\\') {
			// 如果目标字符串空间足够，则进行转义操作
			if (remainingSize > 2) {
				*destPtr++ = '\\'; // 将 '\' 转义为 '\\'
				*destPtr++ = *srcPtr++; // 复制原始字符到目标字符串中
				remainingSize -= 2; // 更新剩余空间大小
			}
			else {
				// 目标字符串空间不足，无法继续转义
				break;
			}
		}
		else {
			// 非特殊字符，直接复制到目标字符串中
			*destPtr++ = *srcPtr++;
			--remainingSize; // 更新剩余空间大小
		}
	}
	*destPtr = '\0'; // 添加结尾的 null 字符
}

//已经废弃的函数
void DoubleEscapeSQLString(char* dest, const char* src, size_t destSize) {
	const char* srcPtr = src;
	char* destPtr = dest;
	size_t remainingSize = destSize;

	while (*srcPtr && remainingSize > 1) {
		if (*srcPtr == '\\') { //也是转义   代表'\'
			if (remainingSize > 4) {
				*destPtr++ = '\\';
				*destPtr++ = *srcPtr++;
				remainingSize -= 4;
			}
			else {
				// No space left for escaping
				break;
			}
		}
		else {
			*destPtr++ = *srcPtr++;
			--remainingSize;
		}
	}
	*destPtr = '\0';
}

/*
 * 函数: CollectDirsInfo
 * ---------------------
 * 从给定文件中读取文件目录路径，并在数据库中收集目录信息。
 *
 * path: 包含文件目录路径的文件路径。
 * conn: MySQL 连接句柄，用于执行数据库查询。
 * root: 树的根节点指针，用于查找目录节点。
 *
 * 返回: void
 *
 * 此函数从给定文件中读取文件目录路径，逐个路径执行 SQL 查询以获取目录信息，并将结果写入日志文件中。
 * 在写入日志文件时，函数会根据目录信息查找树中对应的节点，并获取该目录下文件的最早和最晚创建时间。
 */
void CollectDirsInfo(const char* path, MYSQL* conn, TreeNode* root) {     //从mystats中读取文件目录，收集信息
	// 打开给定路径的文件，用于读取文件目录路径
	FILE* file = fopen(path, "r");
	if (file == NULL) {
		perror("Error opening file");
		exit(EXIT_FAILURE);
	}

	// 打开日志文件，用于记录目录信息
	FILE* log_file = fopen("..\\allfile\\ThisLog.txt", "a");
	if (log_file == NULL) {
		fclose(file);
		perror("Error opening logfile");
		exit(EXIT_FAILURE);
	}

	// 写入日志文件的文件信息头部
	fprintf(log_file, "\n^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^文件信息^^^^^^^^^^^^^^^^^^^^^^^^\n");
	rewind(file); // 重置文件指针到文件开始
	char dir_path[MAX_PATH];
	// 逐行读取文件路径并执行 SQL 查询
	while (fgets(dir_path, MAX_PATH, file) != NULL) {
		dir_path[strcspn(dir_path, "\n")] = '\0'; // 移除换行符
		if (!strcmp(dir_path, "stat dirs")) continue;
		if (!strcmp(dir_path, "end of dirs")) break;

		uint64_t file_size = 0;
		int file_count = 0;
		char query[MAX_SQL_SIZE];
		char escapedPath[MAX_PATH];
		*strrchr(dir_path, '\\') = '\0'; // 从路径中移除文件名部分
		EscapeSQLString(escapedPath, dir_path, MAX_PATH); // 转义路径字符串

		// 构建 SQL 查询语句
		sprintf_s(query, MAX_SQL_SIZE,
			"SELECT file_size, file_count "
			"FROM directories "
			"WHERE path = '%s'",
			escapedPath);

		// 执行 SQL 查询并处理结果
		if (mysql_query(conn, query)) {
			fclose(log_file);
			fclose(file);
			fprintf(stderr, "%s\n", mysql_error(conn));
			exit(EXIT_FAILURE);
		}

		MYSQL_RES* result = mysql_store_result(conn);
		if (result == NULL) {
			fclose(log_file);
			fclose(file);
			fprintf(stderr, "%s\n", mysql_error(conn));
			exit(EXIT_FAILURE);
		}

		MYSQL_ROW row = mysql_fetch_row(result);
		if (row) {
			file_size = atoll(row[0]);
			file_count = atoi(row[1]);
		}
		else {
			fprintf(log_file, "No data found for directory: %s\n", dir_path);
		}
		mysql_free_result(result);

		// 获取目录下文件的最早和最晚创建时间
		FILETIME earliest, latest;
		TCHAR _tcs_escapedpath[MAX_PATH];
		CharToTChar(dir_path, _tcs_escapedpath, MAX_PATH);
		TreeNode* this = Find_Node_Only(root, _tcs_escapedpath); // 查找树中对应的节点
		if (this == NULL) {
			fprintf(log_file, "在路径：%s中数据损坏\n", dir_path);
			continue;
		}
		FindExtretimeFromTree(this, &earliest, &latest);

		// 将文件时间转换为字符串格式
		char* earliest_time = (char*)malloc(40 * sizeof(char));
		char* latest_time = (char*)malloc(40 * sizeof(char));
		PrintFileTime(earliest, earliest_time);
		PrintFileTime(latest, latest_time);

		// 将目录信息写入日志文件
		fprintf(log_file, "在路径：%s中 文件大小：%lld B 文件数量：%d 最早文件创建时间：%s 最晚创建时间：%s \n",
			dir_path, file_size, file_count, earliest_time, latest_time);

		// 释放动态分配的内存
		free(earliest_time);
		free(latest_time);
	}

	// 关闭日志文件和文件
	fclose(log_file);
	fclose(file);
}


/**
 * 从给定的字符串中提取路径信息。
 *
 * @param line 输入字符串，包含路径信息和其他数据。
 * @param path 存储提取的路径信息的缓冲区。
 */
void extractPath(TCHAR* line, TCHAR* path) {
	TCHAR* token = _tcstok(line, ",");
	if (token != NULL) {
		_tcsncpy(path, token, MAX_PATH);
		path[MAX_PATH - 1] = '\0'; // 确保字符串以NULL结尾
	}
	else {
		path[0] = '\0'; // 如果没有找到分隔符，则返回空字符串
	}
}


/*
 * 函数: Find_Node
 * ---------------------
 * 在树中查找指定路径的节点，并返回该节点。
 *
 * root: 树的根节点指针。
 * path: 目标节点的路径。
 * stack: 用于存储节点的栈。
 * prevSibling: 指向前一个兄弟节点的指针。
 *
 * 返回: 指向目标节点的指针，如果未找到目标节点，则返回 NULL。
 *
 * 此函数在树中查找指定路径的节点，并返回该节点。如果找到目标节点，则将当前节点的所有父节点保存到栈中，并将指向前一个兄弟节点的指针设置为相应的值。
 */
TreeNode* Find_Node(TreeNode* root, TCHAR* path, Stack* stack, TreeNode** prevSibling) {
	TreeNode* current = root; // 当前节点指针，从根节点开始搜索
	TreeNode* prev = NULL; // 用于保存当前节点的父节点
	// 初始化前一个兄弟节点为 NULL

	// 循环遍历树中的每个节点，直到找到目标节点或遍历完整个树
	while (current != NULL) {
		// 如果找到目标节点
		if (_tcscmp(current->path, path) == 0) {
			// 将当前节点的所有父节点保存到栈中
			*prevSibling = prev; // 设置前一个兄弟节点
			return current; // 返回目标节点指针
		}

		// 如果当前节点有孩子节点，则将当前节点推入栈中，然后前往第一个孩子节点
		if (current->child != NULL) {
			StackPush(&stack, current); // 将当前节点推入栈中
			prev = NULL; // 进入新的层级，重置前一个兄弟节点
			current = current->child; // 移动到当前节点的第一个孩子节点
		}
		else {
			// 没有孩子节点，尝试移动到下一个兄弟节点
			if (current->next != NULL) {
				prev = current; // 更新前一个兄弟节点
				current = current->next; // 移动到当前节点的下一个兄弟节点
			}
			else {
				// 没有下一个兄弟节点，回溯到父节点，然后尝试父节点的下一个兄弟节点
				while (!StackIsEmpty(stack) && current->next == NULL) {
					current = StackPop(&stack); // 回溯到父节点
					prev = current; // 更新前一个兄弟节点
				}
				if (current != NULL) {
					prev = current;
					current = current->next; // 因为是父节点的兄弟节点，移动到父节点的下一个兄弟节点
					// 重置前一个兄弟节点
				}
			}
		}
	}
	return NULL; // 未找到目标节点，返回 NULL
}


/*
 * 函数: Find_Node_Only
 * ---------------------
 * 在树中查找指定路径的节点，并返回该节点。
 * 与 Find_Node 函数不同的是，此函数不保存节点的父节点到栈中。
 *
 * root: 树的根节点指针。
 * path: 目标节点的路径。
 *
 * 返回: 指向目标节点的指针，如果未找到目标节点，则返回 NULL。
 *
 * 此函数在树中查找指定路径的节点，并返回该节点。与 Find_Node 函数不同的是，此函数不保存节点的父节点到栈中。
 */
TreeNode* Find_Node_Only(TreeNode* root, TCHAR* path) {
	Stack stack; // 用于存储节点的栈
	StackInit(&stack, 20); // 初始化栈
	TreeNode* current = root; // 当前节点指针，从根节点开始搜索

	// 循环遍历树中的每个节点，直到找到目标节点或遍历完整个树
	while (current != NULL) {
		// 如果找到目标节点
		if (_tcscmp(current->path, path) == 0) {
			StackFree(&stack); // 释放栈内存
			return current; // 返回目标节点指针
		}

		// 如果当前节点有孩子节点，则将当前节点推入栈中，然后前往第一个孩子节点
		if (current->child != NULL) {
			StackPush(&stack, current); // 将当前节点推入栈中
			current = current->child; // 移动到当前节点的第一个孩子节点
		}
		else {
			// 没有孩子节点，尝试移动到下一个兄弟节点
			if (current->next != NULL) {
				current = current->next; // 移动到当前节点的下一个兄弟节点
			}
			else {
				// 没有下一个兄弟节点，回溯到父节点，然后尝试父节点的下一个兄弟节点
				while (!StackIsEmpty(&stack) && current->next == NULL) {
					current = StackPop(&stack); // 回溯到父节点
				}
				if (current != NULL) {
					current = current->next; 
					// 因为是父节点的兄弟节点，移动到父节点的下一个兄弟节点
				}
			}
		}
	}

	StackFree(&stack); // 释放栈内存
	return NULL; // 未找到目标节点，返回 NULL
}


/*
 * 函数: CasecadeDelete
 * --------------------
 * 递归删除以给定节点为根的子树，并释放节点内存。
 *
 * node: 待删除子树的根节点指针。
 *
 * 返回: void
 *
 * 此函数递归删除以给定节点为根的子树，并释放节点内存。
 */
void CasecadeDelete(TreeNode* node) {
	if (node == NULL) {
		return; // 如果节点为空，直接返回
	}
	FreeTree(node->child); // 递归释放子树
	free(node); // 释放当前节点内存
}


/*
 * 函数: ModifyTree_dir
 * --------------------
 * 根据文件（mydir）对树进行剪枝，移除指定路径下的节点及其子节点，并更新树的统计信息。
 *
 * filepath: 文件路径，包含要剪枝的目录信息。
 * conn: MySQL 连接句柄，用于查询目录信息并更新统计数据。
 * root: 树的根节点指针。
 *
 * 返回: void
 *
 * 此函数根据文件（mydir）对树进行剪枝，移除指定路径下的节点及其子节点，并更新树的统计信息。
 */
void ModifyTree_dir(char* filepath, MYSQL* conn, TreeNode* root) {
	FILE* file = fopen(filepath, "r"); // 打开文件以读取目录信息
	if (file == NULL) { // 检查文件是否成功打开
		perror("Error opening file"); // 打印错误信息
		exit(EXIT_FAILURE); // 退出程序
	}
	rewind(file); // 重置文件指针到文件开头

	char line[MAX_PATH]; // 用于存储读取的每一行数据
	while (fgets(line, MAX_PATH, file) != NULL) { // 逐行读取文件内容
		TCHAR dir_path[MAX_PATH]; // 存储目录路径的缓冲区
		line[strcspn(line, "\n")] = 0; // 移除换行符
		if (!strcmp(line, "selected dirs")) continue; // 如果是特殊标记，则继续下一行
		if (!strcmp(line, "end of dirs")) break; // 如果是特殊标记，则跳出循环结束读取
		// 将读取的路径字符串转换为 TCHAR 格式
		TCHAR _tline[MAX_PATH];
		CharToTChar(line, _tline, MAX_PATH);
		// 从路径中提取目录部分，并将最后一个路径分隔符设置为字符串结尾
		extractPath(_tline, dir_path);
		*_tcsrchr(dir_path, _T('\\')) = '\0';
		// 在树中查找指定路径对应的节点
		TreeNode* node = Find_Node_Only(root, dir_path);
		if (node != NULL) { // 如果找到了目标节点
			BOOL flag = TRUE; // 标记变量，用于标记是否已经处理了父节点的子节点关系
			TreeNode* parent = node; // 记录当前节点的父节点
			do {
				parent = parent->father; // 移动到父节点
				if (flag) { // 如果尚未处理父节点的子节点关系
					if (parent->child == node) { // 如果目标节点是父节点的第一个子节点
						parent->child = node->next; // 更新父节点的子节点指针
					}
					else { // 如果目标节点是父节点的兄弟节点
						// 找到目标节点的前一个兄弟节点
						TreeNode* pre = parent->child;
						while (pre->next != node) {
							pre = pre->next;
						}
						// 更新前一个兄弟节点的 next 指针
						pre->next = node->next;
					}
					flag = FALSE; // 标记为已处理父节点的子节点关系
				}
				// 更新父节点的文件数量和文件总大小信息
				parent->fileNum--;
				parent->FileSize -= node->FileSize;
			} while (parent->father != NULL); // 循环直到到达根节点
			// 完成节点剪枝后，释放节点及其子节点的内存资源
			CasecadeDelete(node); // 层级删除该节点和所有子节点，同时释放资源
		}
		else {
			_tprintf(_T("%s的文件夹不存在"), dir_path); // 如果目标节点不存在，则打印错误信息
		}
	}
	fclose(file); // 关闭文件
}

/*
 * 函数: UnixTimeToFileTime
 * ------------------------
 * 将 Unix 时间戳转换为 Windows FILETIME 结构。
 *
 * unixTime: Unix 时间戳，表示自 1970 年 1 月 1 日以来经过的秒数。
 * fileTime: 指向 FILETIME 结构的指针，用于存储转换后的 Windows 文件时间。
 *
 * 返回: void
 *
 * 此函数将 Unix 时间戳转换为 Windows FILETIME 结构。
 * Unix 时间戳是从 1970 年 1 月 1 日开始计算的秒数，而 FILETIME 结构是 Windows 中表示时间的一种方式。
 * FILETIME 结构存储的是自 1601 年 1 月 1 日以来的 100 纳秒间隔数。
 * 因此，需要将 Unix 时间戳转换为 FILETIME 结构中的间隔数。
 */
void UnixTimeToFileTime(time_t unixTime, FILETIME* fileTime)
{
	// Unix 纪元和 FILETIME 纪元之间的 100 纳秒间隔数
	const int64_t EPOCH_DIFFERENCE = 116444736000000000LL;

	// 将 Unix 时间戳转换为 100 纳秒间隔数
	int64_t intervals = ((int64_t)unixTime * 10000000) + EPOCH_DIFFERENCE;

	// 设置 FILETIME 结构
	fileTime->dwLowDateTime = (DWORD)(intervals);
	fileTime->dwHighDateTime = (DWORD)(intervals >> 32);
}


/*
 * 函数: ExtractInfo
 * -----------------
 * 从一行文本中提取文件信息，并将其存储在指定的变量中。
 *
 * line: 包含文件信息的一行文本。
 * path: 用于存储文件路径的缓冲区。
 * flag: 用于存储文件标志的缓冲区，只取第一个字符。
 * creationtime: 用于存储文件创建时间的 FILETIME 结构的指针。
 * file_size: 用于存储文件大小的变量的指针。
 *
 * 返回: void
 *
 * 此函数从一行文本中提取文件信息，并将其存储在指定的变量中。
 * 该函数假设输入文本采用 UTF-8 编码。
 * 输入文本的格式为：“路径,标志,创建时间,文件大小”，其中各个字段由逗号分隔。
 */
void ExtractInfo(TCHAR* line, TCHAR* path, TCHAR* flag, FILETIME* creationtime, uint64_t* file_size)
{
	_tcscpy(path, _tcstok(line, _T(",")));
	// strtok() 函数会修改原始字符串，将分隔符位置替换为 null 终止符，并返回下一个非空的子字符串的指针。
	*flag = _tcstok(NULL, _T(","))[0];
	UnixTimeToFileTime(atol(_tcstok(NULL, _T(","))), creationtime);
	*file_size = atoll(_tcstok(NULL, _T(",")));
}



/*
 * 函数: DeteleFileFromTree
 * ------------------------
 * 从树中删除指定路径的文件节点，并更新其父节点的文件数和文件大小。
 *
 * filepath: 指定要删除的文件路径。
 * root: 树的根节点。
 *
 * 返回: 若成功删除文件节点，则返回 TRUE，否则返回 FALSE。
 *
 * 此函数从树中删除指定路径的文件节点，并更新其父节点的文件数和文件大小。
 * 如果文件节点不存在或已被删除，则打印错误消息并返回 FALSE。
 */
BOOL DeteleFileFromTree(TCHAR* filepath, TreeNode* root)
{
	// 查找指定路径的文件节点
	TreeNode* node = Find_Node_Only(root, filepath);
	if (node != NULL)
	{
		TreeNode* parent = node->father;
		uint64_t file_size = node->FileSize;
		// 如果父节点的子节点不是要删除的节点
		if (parent->child != node)
		{
			TreeNode* pre = parent->child;
			// 寻找要删除节点的前一个兄弟节点
			while (pre->next != node)
			{
				pre = pre->next;
			}
			pre->next = node->next;
		}
		else
		{
			parent->child = node->next;
		}
		// 更新父节点及其祖先节点的文件数和文件大小
		while (parent != NULL)
		{
			parent->fileNum--;
			parent->FileSize -= file_size;
			parent = parent->father;
		}
		// 删除节点及其子节点
		CasecadeDelete(node);
		return TRUE; // 返回成功删除文件节点
	}
	else
	{
		_tprintf(_T("%s的文件夹不存在，或已被删除\n"), filepath);
		return FALSE; // 返回未找到文件节点或文件已被删除
	}
}


/*
 * 函数: ModifyFileFromFree
 * ------------------------
 * 修改树中指定路径的文件节点的创建时间和文件大小，并更新其父节点的文件大小。
 *
 * path: 指定要修改的文件路径。
 * root: 树的根节点。
 * creationtime: 新的创建时间。
 * modified_size: 修改后的文件大小。
 *
 * 返回: 若成功修改文件节点，则返回 TRUE，否则返回 FALSE。
 *
 * 此函数修改树中指定路径的文件节点的创建时间和文件大小，并更新其父节点的文件大小。
 * 如果未找到指定路径的文件节点，则打印错误消息并返回 FALSE。
 */
BOOL ModifyFileFromFree(TCHAR* path, TreeNode* root, FILETIME creationtime, uint64_t modified_size)
{
	// 查找指定路径的文件节点
	TreeNode* node = Find_Node_Only(root, path);
	if (node == NULL) {
		// 处理未找到节点的情况
		_tprintf(_T("Failed to find %s dir\n"), path);
		return FALSE;
	}
	uint64_t size_diff = modified_size - node->FileSize;
	TreeNode* parent = node->father;
	// 更新父节点及其祖先节点的文件大小
	while (parent != NULL)
	{
		parent->FileSize += size_diff;
		parent = parent->father;
	}
	// 更新文件节点的创建时间
	node->CreationTime = creationtime;

	return TRUE; // 返回成功修改文件节点
}



/*
 * 函数: SplitPath
 * --------------
 * 将给定的完整路径拆分为父目录路径和文件名。
 *
 * fullPath: 完整的文件路径。
 * directory: 用于存储父目录路径的缓冲区。
 * fileName: 用于存储文件名的缓冲区。
 *
 * 返回: void
 *
 * 此函数将给定的完整路径拆分为父目录路径和文件名，并将结果存储在相应的缓冲区中。
 * 如果路径中存在斜线，则将路径拆分为父目录路径和文件名；如果没有斜线，则整个路径被视为文件名。
 */
void SplitPath(const TCHAR* fullPath, TCHAR* directory, TCHAR* fileName) {
	const TCHAR* lastSlash = _tcsrchr(fullPath, _T('\\')); // 查找最后一个斜线
	if (lastSlash != NULL)
	{
		// 提取父目录路径
		_tcsncpy_s(directory, MAX_PATH, fullPath, lastSlash - fullPath);
		directory[lastSlash - fullPath] = _T('\0');
		// 提取文件名
		_tcscpy_s(fileName, MAX_PATH, lastSlash + 1);
	}
	else
	{
		// 如果没有斜线，整个路径就是文件名
		_tcscpy_s(directory, MAX_PATH, _T(""));
		_tcscpy_s(fileName, MAX_PATH, fullPath);
	}
}



/*
 * 函数: AppandFileIntoTree
 * ------------------------
 * 将指定路径的文件添加到树中，并更新文件节点的相关属性以及父节点的文件数量和文件大小。
 *
 * path: 要添加的文件的路径。
 * root: 树的根节点。
 * creationtime: 文件的创建时间。
 * file_size: 文件的大小。
 *
 * 返回: 若成功添加文件节点，则返回 TRUE，否则返回 FALSE。
 *
 * 此函数将指定路径的文件添加到树中，并更新文件节点的相关属性以及父节点的文件数量和文件大小。
 * 如果无法找到指定路径的父目录节点，则打印错误消息并返回 FALSE。
 * 如果内存分配失败，则打印错误消息并返回 FALSE。
 */
BOOL AppandFileIntoTree(TCHAR* path, TreeNode* root, FILETIME creationtime, uint64_t file_size)
{
	TCHAR dir_path[MAX_PATH];
	TCHAR file_name[MAX_PATH];
	SplitPath(path, dir_path, file_name); // 分割路径为父目录和文件名

	// 查找父目录节点
	TreeNode* node = Find_Node_Only(root, dir_path);
	if (node == NULL)
	{
		_tprintf(_T("Failed to find %s dir\n"), dir_path);
		return FALSE; // 未找到父目录节点，返回 FALSE
	}

	// 分配新的文件节点
	TreeNode* newnode = (TreeNode*)malloc(sizeof(TreeNode));
	if (newnode == NULL) {
		// 处理内存分配失败的情况
		fprintf(stderr, "Failed to allocate memory while adding a file node\n");
		return FALSE;
	}
	NodeInitialize(newnode);
	newnode->fileNum = 1;        // 设置文件的 filenum 属性为 1
	newnode->CreationTime = creationtime;
	newnode->FileSize = file_size;

	// 将新节点添加到父目录节点的子节点链表中
	if (node->child == NULL)
	{
		node->child = newnode; // 若父目录节点无子节点，则将新节点设为其子节点
	}
	else
	{
		TreeNode* pre = node->child;
		while (pre->next != NULL)
		{
			pre = pre->next;
		}
		pre->next = newnode; // 否则将新节点添加到父目录节点的子节点链表末尾
	}

	// 更新父目录节点及其祖先节点的文件数量和文件大小
	TreeNode* parent = node;
	while (parent->father != NULL)
	{
		parent->fileNum++;
		parent->FileSize += newnode->FileSize;
		parent = parent->father;
	}

	return TRUE; // 返回成功添加文件节点
}


/*
 * 函数: ModifyTree_file
 * ---------------------
 * 根据文件中的操作指令，修改树的文件节点。
 *
 * filepath: 操作指令文件的路径。
 * conn: MySQL 连接句柄，未使用。
 * root: 树的根节点。
 *
 * 返回: void
 *
 * 此函数根据文件中的操作指令，对树的文件节点进行添加、修改或删除操作。
 * 操作指令文件包含一系列指令，每条指令占一行，格式为：路径,标志,创建时间,文件大小。
 * 其中，路径为要操作的文件路径，标志为操作类型，包括 'A' (添加)、'M' (修改)、'D' (删除)。
 * 创建时间和文件大小用于添加或修改文件节点的属性。
 * 如果文件打开失败，则打印错误消息并退出程序。
 */
void ModifyTree_file(char* filepath, MYSQL* conn, TreeNode* root)
{
	FILE* file = fopen(filepath, "r");
	if (file == NULL) {
		perror("Error opening file");
		exit(EXIT_FAILURE);
	}
	rewind(file);
	char line[MAX_PATH];
	while (fgets(line, MAX_PATH, file) != NULL)
	{
		line[strcspn(line, "\n")] = 0;
		if (!strcmp(line, "selected files")) continue;
		if (!strcmp(line, "end of files")) break;
		TCHAR _tline[MAX_PATH];
		CharToTChar(line, _tline, MAX_PATH);
		// 移除换行符
		TCHAR path[MAX_PATH];
		TCHAR flag;
		FILETIME creationtime;
		uint64_t file_size;
		ExtractInfo(_tline, path, &flag, &creationtime, &file_size);
		if (flag == _T('A'))
		{
			AppandFileIntoTree(path, root, creationtime, file_size);
		}
		else if (flag == _T('M'))
		{
			ModifyFileFromFree(path, root, creationtime, file_size);
		}
		else if (flag == _T('D'))
		{
			DeteleFileFromTree(path, root);
		}
	}
	fclose(file);
}


/*
 * 函数: FileTimetoUlonglong
 * ------------------------
 * 将 FILETIME 结构转换为 ULONGLONG 类型的整数值。
 *
 * time: 待转换的 FILETIME 结构。
 *
 * 返回: ULONGLONG 类型的整数值，表示时间。
 *
 * 此函数将 FILETIME 结构表示的时间转换为 ULONGLONG 类型的整数值，
 * 以便于进行比较或其他操作。
 */
ULONGLONG FileTimetoUlonglong(FILETIME time)
{
	return (((ULONGLONG)time.dwHighDateTime) << 32) + time.dwLowDateTime;
}


/*
 * 函数: FindExtretimeFromTree
 * ---------------------------
 * 在树中查找最早和最晚的文件创建时间。
 *
 * node: 树的根节点。
 * earliest: 用于存储最早的文件创建时间。
 * latest: 用于存储最晚的文件创建时间。
 *
 * 返回: void
 *
 * 此函数遍历给定树中的所有节点，查找最早和最晚的文件创建时间，并将结果存储在 earliest 和 latest 中。
 */
void FindExtretimeFromTree(TreeNode* node, FILETIME* earliest, FILETIME* latest)
{
	Stack stack;
	StackInit(&stack, 64); // 初始化栈
	*earliest = node->CreationTime; // 初始化最早时间为根节点的创建时间
	*latest = *earliest; // 初始化最晚时间为最早时间

	// 将根节点推入栈
	StackPush(&stack, node);

	// 循环处理栈中的节点
	while (!StackIsEmpty(&stack))
	{
		TreeNode* current = StackPop(&stack); // 弹出栈顶节点

		// 检查当前节点是否为空
		while (current != NULL) {
			// 处理子节点
			if (current->child != NULL) {
				StackPush(&stack, current->child); // 将子节点推入栈
			}

			// 更新最早和最晚时间
			if (FileTimetoUlonglong(*earliest) > FileTimetoUlonglong(current->CreationTime)) {
				*earliest = current->CreationTime; // 更新最早时间
			}
			if (FileTimetoUlonglong(*latest) < FileTimetoUlonglong(current->CreationTime)) {
				*latest = current->CreationTime; // 更新最晚时间
			}

			// 移动到下一个兄弟节点
			current = current->next;
		}
	}

	StackFree(&stack); // 释放栈内存
}

/*
 * 函数: CharToTChar
 * -----------------
 * 将多字节字符串转换为宽字符字符串。
 *
 * src: 源多字节字符串。
 * dest: 目标宽字符字符串的指针，用于存储转换后的结果。
 * destSize: 目标字符串的大小，包括结尾的 null 字符。
 *
 * 返回: void
 *
 * 如果定义了 UNICODE，使用 mbstowcs 函数将多字节字符串转换为宽字符字符串；
 * 如果没有定义 UNICODE，则直接复制多字节字符串到目标字符串中。
 * 最后，确保目标字符串以空字符结束。
 */
void CharToTChar(const char* src, TCHAR* dest, size_t destSize)
{
#ifdef UNICODE
	// 如果定义了 UNICODE，使用多字节到宽字符的转换
	mbstowcs(dest, src, destSize);
#else
	// 如果没有定义 UNICODE，直接复制字符串
	strncpy(dest, src, destSize);
#endif
	dest[destSize - 1] = _T('\0'); // 确保字符串以空字符结束
}


/*
 * 函数: TCharToChar
 * ----------------
 * 将宽字符字符串转换为多字节字符串。
 *
 * src: 源宽字符字符串。
 * dest: 目标多字节字符串的指针，用于存储转换后的结果。
 * destSize: 目标字符串的大小，包括结尾的 null 字符。
 *
 * 返回: void
 *
 * 如果定义了 _UNICODE，使用 wcstombs_s 函数将宽字符字符串转换为多字节字符串；
 * 如果没有定义 _UNICODE，则直接复制宽字符字符串到目标字符串中。
 */
void TCharToChar(const TCHAR* src, char* dest, size_t destSize)
{
#ifdef _UNICODE
	// 宽字符转换为多字节字符
	size_t convertedChars = 0;
	wcstombs_s(&convertedChars, dest, destSize, src, _TRUNCATE);
#else
	// 直接复制
	strncpy_s(dest, destSize, src, _TRUNCATE);
#endif
}

/*
 * 函数: CompareDifference
 * ----------------------
 * 比较数据库中的目录与树中目录的差异，并将结果记录在日志文件中。
 *
 * path: 包含文件夹路径的文件的路径。
 * conn: MySQL 连接。
 * root: 树的根节点。
 *
 * 返回: void
 *
 * 此函数打开包含文件夹路径的文件，逐行读取文件夹路径，并针对每个路径执行比较操作。
 * 比较的结果包括文件夹大小差异、文件数量差异、最早文件创建时间和最晚文件创建时间。
 * 结果记录在日志文件中。
 */
void CompareDifference(const char* path, MYSQL* conn, TreeNode* root)
{
	FILE* file = fopen(path, "r"); // 打开包含文件夹路径的文件
	if (file == NULL) {
		perror("Error opening file");
		exit(EXIT_FAILURE);
	}
	char dir_path[MAX_PATH];

	FILE* log_file = fopen("..\\allfile\\log.txt", "a"); // 打开日志文件
	if (log_file == NULL)
	{
		fclose(file);
		perror("Error opening logfile");
		exit(EXIT_FAILURE);
	}

	fprintf(log_file, "\n^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^文件差异^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");

	rewind(file); // 重置文件指针到文件开始

	// 逐条读取并执行SQL语句
	while (fgets(dir_path, MAX_PATH, file) != NULL)
	{
		dir_path[strcspn(dir_path, "\n")] = 0;
		if (!strcmp(dir_path, "stat dirs")) continue;
		if (!strcmp(dir_path, "end of dirs")) break;
		TCHAR tcs_dir_path[MAX_PATH];
		*strrchr(dir_path, '\\') = '\0'; // 移除路径中的文件名部分
		CharToTChar(dir_path, tcs_dir_path, MAX_PATH); // 将路径转换为宽字符
		TreeNode* this = Find_Node_Only(root, tcs_dir_path); // 查找树中对应节点
		if (!this)
		{
			fprintf(log_file, "无法找到 %s 文件夹\n", dir_path);
			continue;
		}
		FILETIME earliest;
		FILETIME latest;
		FindExtretimeFromTree(this, &earliest, &latest); // 查找树中节点的最早和最晚时间
		int tree_file_num = this->fileNum; // 树中节点的文件数量
		uint64_t tree_dir_size = this->FileSize; // 树中节点的文件夹大小
		int database_file_num = 0;
		uint64_t database_dir_size = 0;
		char query[MAX_SQL_SIZE];
		char escaped_dir_path[MAX_PATH];
		EscapeSQLString(escaped_dir_path, dir_path, MAX_PATH); // 转义路径中的特殊字符
		sprintf_s(query, MAX_SQL_SIZE,
			"SELECT file_size, file_count "
			"FROM directories "
			"WHERE path = '%s'"
			, escaped_dir_path); // 构建查询语句
		if (mysql_query(conn, query)) // 执行查询
		{
			fclose(log_file);
			fclose(file);
			fprintf(stderr, "%s\n", mysql_error(conn));
			exit(1);
		}
		MYSQL_RES* result = mysql_store_result(conn); // 获取查询结果
		if (result == NULL) {
			fclose(log_file);
			fclose(file);
			fprintf(stderr, "%s\n", mysql_error(conn));
			exit(1);
		}

		MYSQL_ROW row = mysql_fetch_row(result);
		if (row) {
			database_dir_size = atoll(row[0]); // 数据库中节点的文件夹大小
			database_file_num = atoi(row[1]); // 数据库中节点的文件数量
		}
		else {
			fprintf(log_file, "未找到路径为：%s 的数据\n", dir_path);
		}
		mysql_free_result(result); // 释放结果集
		char earliesttime_str[40];
		char latesttime_str[40];
		PrintFileTime(earliest, earliesttime_str); // 将文件时间转换为字符串格式
		PrintFileTime(latest, latesttime_str); // 将文件时间转换为字符串格式
		fprintf(log_file, "\n在路径：%s 中，文件大小差异：%lld B，文件数量差异：%d，最早文件创建时间：%s，最晚创建时间：%s\n",
			dir_path, database_dir_size - tree_dir_size,
			database_file_num - tree_file_num, earliesttime_str, latesttime_str);

	}
	fclose(log_file); // 关闭日志文件
	fclose(file); // 关闭文件
}

/*
 * 函数: FreeRoot
 * -------------
 * 释放树的所有节点及其内存。
 *
 * root: 树的根节点。
 *
 * 返回: void
 *
 * 此函数使用深度优先搜索算法遍历树，并释放每个节点的内存。
 */
void FreeRoot(TreeNode* root)
{
	if (root == NULL) return;

	Stack stack;
	StackInit(&stack, 64);
	StackPush(&stack, root);

	while (!StackIsEmpty(&stack)) {
		TreeNode* current = StackPop(&stack);

		// 将当前节点的孩子和兄弟节点压入栈
		if (current->child != NULL) {
			StackPush(&stack, current->child);
		}
		if (current->next != NULL) {
			StackPush(&stack, current->next);
		}

		// 释放当前节点
		free(current);
	}

	StackFree(&stack);
}


/*
 * 函数: DropDatabase
 * -----------------
 * 删除数据库（如果存在）。
 *
 * conn: MySQL 连接句柄。
 *
 * 返回: void
 *
 * 此函数用于执行 SQL 语句以删除指定的数据库。
 */
void DropDatabase(MYSQL* conn)
{
	char pre[MAX_SQL_SIZE];
	sprintf_s(pre, MAX_SQL_SIZE, "DROP DATABASE IF EXISTS file_scan;");
	// 执行 SQL 语句来删除数据库
	if (mysql_query(conn, pre))
	{
		fprintf(stderr, "@@Failed to use database: %s\n", mysql_error(conn));
		// 程序的错误消息将被输出到标准错误设备（通常是屏幕或控制台），而不是标准输出设备
		exit(1); // 或者根据应用程序的要求处理错误
	}
}


