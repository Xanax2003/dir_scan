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
 * ����: StackInit
 * --------------------
 * ��ʼ��һ�����и���������ջ��
 *
 * s: ָ��Ҫ��ʼ����Stack�ṹ��ָ�롣
 * capacity: ָ��ջ�ĳ�ʼ������������
 *
 * ����: void
 *
 * �˺���ͨ��ΪTreeNodeָ�����飨items������ָ���������ڴ�����ʼ��ջ��
 * ��ջ�Ĵ�С��ʼ����Ϊ0����ʾջΪ�ա�
 * ��ջ����������Ϊ�ṩ��ֵ��
 */
void StackInit(Stack* s, int capacity)
{
	s->items = (TreeNode**)malloc(capacity * sizeof(TreeNode*)); // ΪTreeNodeָ����������ڴ档
	s->size = 0; // ��ջ�Ĵ�С��ʼ��Ϊ0����ʾһ����ջ��
	s->capacity = capacity; // ��ջ����������Ϊ�ṩ��ֵ��
}

/*
 * ����: UseDatabase
 * --------------------
 * ʹ�����ݿ�����������һ����Ϊfile_scan�����ݿ⣨��������ڣ�����������Ϊ��ǰ���ݿ⡣
 *
 * conn: ָ��MYSQL�ṹ��ָ�룬��ʾ���ݿ����ӡ�
 *
 * ����: void
 *
 * �˺���ͨ��ִ��SQL�����������Ϊfile_scan�����ݿ⣨����в����ڣ���Ȼ��������Ϊ��ǰ���ݿ⡣
 * ���ִ��SQL���ʧ�ܣ������������Ϣ����׼�����豸�����˳�����
 */
void UseDatabase(MYSQL* conn)
{
	char pre[MAX_SQL_SIZE]; // ���ڴ洢SQL�����ַ�������
	sprintf_s(pre, MAX_SQL_SIZE, "CREATE DATABASE IF NOT EXISTS file_scan;"); // �������ݿ��SQL���

	// ִ�д������ݿ��SQL���
	if (mysql_query(conn, pre))
	{
		fprintf(stderr, "@@Failed to use database: %s\n", mysql_error(conn)); // ���������Ϣ����׼�����豸
		exit(1); // �˳��������Ӧ�ó����Ҫ�������
	}

	sprintf_s(pre, MAX_SQL_SIZE, "USE file_scan; "); // ���ļ�ɨ�����ݿ�����Ϊ��ǰ���ݿ��SQL���

	// ִ�н��ļ�ɨ�����ݿ�����Ϊ��ǰ���ݿ��SQL���
	if (mysql_query(conn, pre))
	{
		fprintf(stderr, "!!!Failed to use database: %s\n", mysql_error(conn)); // ���������Ϣ����׼�����豸
		exit(1); // �˳��������Ӧ�ó����Ҫ�������
	}
}

/*
 * ����: CreateTables
 * --------------------
 * �ڵ�ǰ���ݿ��д�����Ҫ�ı�񣬰���directories��files���
 *
 * conn: ָ��MYSQL�ṹ��ָ�룬��ʾ���ݿ����ӡ�
 *
 * ����: void
 *
 * �˺���ͨ��ִ��SQL������ڵ�ǰ���ݿ��д�����Ҫ�ı�񣬰���directories��files���
 * ���ִ��SQL���ʧ�ܣ������������Ϣ����׼�����豸�����˳�����
 */
void CreateTables(MYSQL* conn)
{
	// SQL��䣬���ڴ���directories���
	char* create_directory_table = "CREATE TABLE IF NOT EXISTS directories ("
		"id INT AUTO_INCREMENT PRIMARY KEY, "
		"name VARCHAR(255) NOT NULL, "
		"path VARCHAR(255) NOT NULL, "
		"file_size BIGINT UNSIGNED,"
		"creation_time BIGINT UNSIGNED, "
		"file_count INT DEFAULT 0)";

	// SQL��䣬���ڴ���files���
	char* create_file_table = "CREATE TABLE IF NOT EXISTS files ("
		"id INT AUTO_INCREMENT PRIMARY KEY, "
		"name VARCHAR(255) NOT NULL, "
		"path VARCHAR(255) NOT NULL, "
		"file_size BIGINT UNSIGNED,"
		"creation_time BIGINT UNSIGNED)";

	// ִ�д���directories����SQL���
	if (mysql_query(conn, create_directory_table))
	{
		fprintf(stderr, "Failed to create directory table: %s\n", mysql_error(conn)); // ���������Ϣ����׼�����豸
		exit(1); // �˳��������Ӧ�ó����Ҫ�������
	}

	// ִ�д���files����SQL���
	if (mysql_query(conn, create_file_table))
	{
		fprintf(stderr, "Failed to create file table: %s\n", mysql_error(conn)); // ���������Ϣ����׼�����豸
		exit(1); // �˳��������Ӧ�ó����Ҫ�������
	}

	printf("Tables created successfully\n"); // ����ɹ�����������Ϣ
}


/*
 * ����: ConnectDatabase
 * --------------------
 * �������ݿⲢ�������ݿ����Ӷ���
 *
 * ����: ָ��MYSQL�ṹ��ָ�룬��ʾ���ݿ����ӡ�
 *
 * �˺���ͨ������mysql_init()��ʼ��һ��MYSQL����Ȼ�����mysql_real_connect()���ӵ�MySQL��������
 * �������ʧ�ܣ������������Ϣ����׼�����豸���˳�����
 * ������ӳɹ���������ɹ��������ݿ����Ϣ�����������ݿ����Ӷ���
 */
MYSQL* ConnectDatabase()
{
	MYSQL* conn = mysql_init(NULL); // ��ʼ��һ��MYSQL����
	if (conn == NULL)
	{
		fprintf(stderr, "mysql_init() failed\n"); // ���������Ϣ����׼�����豸
		exit(1); // �˳�����
	}

	// �������ӵ�MySQL������
	if (mysql_real_connect(conn, "localhost", "root", "root", NULL, 0, NULL, 0, CLIENT_MULTI_STATEMENTS) == NULL)
	{
		fprintf(stderr, "mysql_real_connect() failed\n"); // ���������Ϣ����׼�����豸
		mysql_close(conn); // �ر����ݿ�����
		exit(1); // �˳�����
	}

	printf("�������ݿ�ɹ�\n"); // ����ɹ��������ݿ����Ϣ
	return conn; // �������ݿ����Ӷ���
}


/*
 * ����: ExecuteSQLBatch
 * --------------------
 * ���ļ��ж�ȡSQL��䲢����ִ����Щ��䡣
 *
 * file: ָ�����SQL�����ļ���ָ�롣
 * conn: ָ��MYSQL�ṹ��ָ�룬��ʾ���ݿ����ӡ�
 *
 * ����: void
 *
 * �˺����Ӹ����ļ������ж�ȡSQL��䣬��������ӵ��������ѯ�ַ����С����������ѯ�ַ����ﵽ200�����ʱ��
 * �����ļ�����ʱ����ִ���������ѯ�����ִ��SQL���ʧ�ܣ������������Ϣ����׼�����豸��
 */
void ExecuteSQLBatch(FILE* file, MYSQL* conn) {
	char query[MAX_SQL_SIZE]; // ���ڴ洢��ȡ��SQL���
	char* batchQuery = (char*)malloc(MAX_SQL_SIZE * 200 * sizeof(char)); // ���ڴ洢�������ѯ�ַ���
	batchQuery[0] = '\0'; // ��ʼ���������ѯ�ַ���Ϊ��
	int count = 0; // ����������¼��ǰ�������ѯ�е��������

	rewind(file); // ���ļ�ָ�����¶�λ���ļ���ͷ

	// ���ж�ȡ���ۻ�SQL���
	while (fgets(query, MAX_SQL_SIZE, file) != NULL) {
		// �Ƴ����з�
		query[strcspn(query, "\n")] = '\0';
		char escaped_query[MAX_SQL_SIZE];
		EscapeSQLString(escaped_query, query, MAX_SQL_SIZE); // ת��SQL����е������ַ�

		// ����ȡ�������ӵ��������ѯ��
		strcat(batchQuery, escaped_query);
		count++;

		// ÿ200�������ļ�����ʱִ���������ѯ
		if (count >= 200) {
			mysql_set_server_option(conn, MYSQL_OPTION_MULTI_STATEMENTS_ON); // ���ö����ִ��ģʽ
			int res = mysql_query(conn, batchQuery); // ִ���������ѯ
			mysql_set_server_option(conn, MYSQL_OPTION_MULTI_STATEMENTS_OFF); // ���ö����ִ��ģʽ
			MYSQL_RES* my_res; // ��ѯ���
			do {
				my_res = mysql_store_result(conn);
				if (my_res) mysql_free_result(my_res);
			} while (!mysql_next_result(conn)); // ����Ƿ��и���Ĳ�ѯ���

			// �����������ѯ�ַ����ͼ�����
			batchQuery[0] = '\0';
			count = 0;
		}
	}

	// ����ʣ�����䣨����У�
	if (batchQuery[0] != '\0') {
		mysql_set_server_option(conn, MYSQL_OPTION_MULTI_STATEMENTS_ON); // ���ö����ִ��ģʽ
		int res = mysql_query(conn, batchQuery); // ִ���������ѯ
		mysql_set_server_option(conn, MYSQL_OPTION_MULTI_STATEMENTS_OFF); // ���ö����ִ��ģʽ

		MYSQL_RES* my_res; // ��ѯ���
		do {
			my_res = mysql_store_result(conn);
			if (my_res) mysql_free_result(my_res);
		} while (!mysql_next_result(conn)); // ����Ƿ��и���Ĳ�ѯ���
	}

	free(batchQuery); // �ͷŶ�̬������ڴ�
	printf("Execute query succeeded\n"); // ����ɹ�ִ�в�ѯ����Ϣ
}


/*
 * ����: extractValues
 * --------------------
 * �Ӹ����ַ�������ȡVALUES�Ӿ�����ݡ�
 *
 * line: ����SQL�����ַ�����
 * output: ���ڴ洢��ȡ��VALUES�Ӿ����ݵ��ַ����顣
 *
 * ����: void
 *
 * �˺������Ҹ����ַ����е�"VALUES "�Ӿ䣬����ȡ�����ݵ�output�����С�
 * ���δ�ҵ���Ч��"VALUES "�Ӿ��������������"Invalid format"���Ƶ�output�����С�
 */
void extractValues(const char* line, char* output) {
	const char* start = strstr(line, "VALUES ") + 6; // ����"VALUES "�Ӿ����ʼλ��
	const char* end = strrchr(line, ')'); // �����ַ��������һ��')'��λ��

	if (start && end && end > start) { // ����ҵ���Ч����ʼ�ͽ���λ��
		size_t len = end - start + 1; // �����Ӿ����ݵĳ���
		strncpy(output, start, len); // �����Ӿ����ݵ�output����
		output[len] = '\0'; // ȷ���ַ�����null��β
	}
	else {
		strcpy(output, "Invalid format"); // ������Ч��ʽ���������"Invalid format"���Ƶ�output������
	}
}


/*
 * ����: ExecuteBatchInsert
 * --------------------
 * ���ļ��ж�ȡSQL��䲢ִ���������������
 *
 * file: ָ�����SQL�����ļ���ָ�롣
 * conn: ָ��MYSQL�ṹ��ָ�룬��ʾ���ݿ����ӡ�
 * flag: ����ָʾ����directories�����files���ִ�в�������Ĳ���ֵ��
 *
 * ����: void
 *
 * �˺����Ӹ����ļ������ж�ȡSQL��䣬��������ӵ��������ѯ�ַ����С����ۼ�500���������ʱ����ִ��һ�β��������
 * ���ִ��SQL���ʧ�ܣ������������Ϣ����׼�����豸��
 */
void ExecuteBatchInsert(FILE* file, MYSQL* conn, BOOL flag)
{
	char* line = (char*)malloc(MAX_SQL_SIZE * sizeof(char)); // ���ڴ洢��ȡ���ļ���
	if (line == NULL)
	{
		fprintf(stderr, "Failed to allocate memories for string:line\n"); // ����ڴ���������Ϣ����׼�����豸
		return;
	}
	line[0] = '\0';  // ��ʼ��line����

	char* query = (char*)malloc(500 * MAX_SQL_SIZE * sizeof(char)); // ���ڴ洢�����ѯ�����ַ���
	if (query == NULL)
	{
		fprintf(stderr, "Failed to allocate memories for string:query\n"); // ����ڴ���������Ϣ����׼�����豸
		return;
	}
	query[0] = '\0';    // ��ʼ��query����

	int lineCount = 0; // ����������¼��ǰ�Ѷ�ȡ������

	rewind(file); // ���ļ�ָ�����¶�λ���ļ���ͷ

	while (fgets(line, MAX_SQL_SIZE, file) != NULL) { // ���ж�ȡ�ļ��е�SQL���
		if (lineCount == 0) // ����ǵ�һ��
		{
			if (flag == TRUE) // ���ָʾ��directories���ִ�в������
			{
				strcat(query, "INSERT INTO directories(name, path, file_size, creation_time, file_count) VALUES");
			}
			else // ���ָʾ��files���ִ�в������
			{
				strcat(query, "INSERT INTO files ( name, path, file_size,creation_time) VALUES");
			}
		}

		char sub[MAX_SQL_SIZE]; // ���ڴ洢��SQL�������ȡ��VALUES�Ӿ�
		extractValues(line, sub); // ��SQL�������ȡVALUES�Ӿ�
		strcat(sub, ","); // ��������ӵ��Ӿ�ĩβ
		char escaped_sub[MAX_SQL_SIZE]; // ���ڴ洢ת�����Ӿ�
		EscapeSQLString(escaped_sub, sub, MAX_SQL_SIZE); // ת��VALUES�Ӿ��е������ַ�
		strcat(query, escaped_sub); // ��ת�����Ӿ���ӵ������ѯ�����
		lineCount++;
		if (lineCount == 500) { // ���ۼ�500���������ʱִ��һ�β������
			*strrchr(query, ',') = ';'; // �����һ�������滻Ϊ�ֺţ���ʾ�����ѯ����
			if (mysql_query(conn, query)) { // ִ�в����ѯ
				fprintf(stderr, "%s\n", mysql_error(conn)); // ���������Ϣ����׼�����豸
			}
			lineCount = 0; // ���ü�����
			strcpy(query, "\0"); // ��ղ����ѯ�ַ���
		}
	}

	// ִ��ʣ��Ĳ������������У�
	if (lineCount > 0) {
		*strrchr(query, ',') = ';'; // �����һ�������滻Ϊ�ֺţ���ʾ�����ѯ����
		if (mysql_query(conn, query)) { // ִ�в����ѯ
			fprintf(stderr, "%s\n", mysql_error(conn)); // ���������Ϣ����׼�����豸
		}
	}
	free(query); // �ͷŶ�̬������ڴ�
	free(line); // �ͷŶ�̬������ڴ�
}


/*
 * ����: StackPush
 * --------------------
 * ��һ��Ԫ��ѹ��ջ�С�
 *
 * s: ָ��ջ�ṹ��ָ�롣
 * item: ָ��Ҫѹ��ջ��Ԫ�ص�ָ�롣
 *
 * ����: void
 *
 * �˺�����������Ԫ��ѹ��ջ�С����ջ����������չջ������Ϊ��ǰ����������������Ԫ��ѹ��ջ����
 * ����ڴ����ʧ�ܣ������������Ϣ����׼�����豸���˳�����
 */
void StackPush(Stack* s, TreeNode* item)
{
	if (s->size >= s->capacity) // ���ջ����
	{
		s->capacity *= 2; // ��չջ������Ϊ��ǰ����������
		s->items = (TreeNode**)realloc(s->items, (s->capacity) * sizeof(TreeNode*)); // ���·���ջ�ռ�
		if (s->items == NULL) // ����ڴ����ʧ��
		{
			fprintf(stderr, "Failed to push a item\n"); // ���������Ϣ����׼�����豸
			exit(-1); // �˳�����
		}
	}
	s->items[s->size++] = item; // ��Ԫ��ѹ��ջ��������ջ�Ĵ�С
}


/*
 * ����: StackPop
 * --------------------
 * ��ջ�е���һ��Ԫ�ء�
 *
 * s: ָ��ջ�ṹ��ָ�롣
 *
 * ����: ָ�򵯳���Ԫ�ص�ָ�룬���ջΪ�գ��򷵻�NULL��
 *
 * �˺�����ջ������һ��Ԫ�أ�����ջ�Ĵ�С��С�����ջΪ�գ��򷵻�NULL��
 */
TreeNode* StackPop(Stack* s)
{
	if (s->size == 0) // ���ջΪ��
		return NULL; // ����NULL
	return s->items[--(s->size)]; // ����ջ��Ԫ�ز�����
}


/*
 * ����: StackIsEmpty
 * --------------------
 * ���ջ�Ƿ�Ϊ�ա�
 *
 * s: ָ��ջ�ṹ��ָ�롣
 *
 * ����: ���ջΪ�գ��򷵻�1�����򷵻�0��
 *
 * �˺�����������ջ�Ƿ�Ϊ�ա����ջ�Ĵ�СΪ0���򷵻�1�����򷵻�0��
 */
int StackIsEmpty(Stack* s)
{
	return s->size == 0; // ���ջ�Ĵ�СΪ0���򷵻�1�����򷵻�0
}


/*
 * ����: Initialize
 * --------------------
 * ��ʼ��DirectoryStats�ṹ�塣
 *
 * stats: ָ��DirectoryStats�ṹ��ָ�롣
 *
 * ����: void
 *
 * �˺�����������DirectoryStats�ṹ���ʼ��Ϊ��ʼ״̬����������Ա��������Ϊ0��Ĭ��ֵ��
 */
void Initialize(DirectoryStats* stats)
{
	stats->longestFilePathLength = 0; // ��ļ�·��������Ϊ0
	stats->maxDepth = 1; // ��������Ϊ1
	stats->totalDirectories = 0; // ��Ŀ¼����Ϊ0
	stats->totalFiles = 0; // ���ļ�����Ϊ0
	stats->maxLenth = 0; // ��󳤶���Ϊ0
}


/*
 * ����: PrintStats
 * --------------------
 * ��ӡDirectoryStats�ṹ���е�ͳ����Ϣ��
 *
 * stats: ָ��DirectoryStats�ṹ��ָ�룬����Ҫ��ӡ��ͳ����Ϣ��
 *
 * ����: void
 *
 * �˺�����ӡ������DirectoryStats�ṹ���е�ͳ����Ϣ��������Ŀ¼�������ļ��������Ŀ¼��ȡ�
 * �Ӹ�Ŀ¼��Ҷ�ڵ����󳤶ȡ���ļ�·���Լ���ļ�·�����ȡ�
 */
void PrintStats(const DirectoryStats* stats)
{
	printf("Total directories: %d\n", stats->totalDirectories); // ��ӡ��Ŀ¼��
	printf("Total files: %d\n", stats->totalFiles); // ��ӡ���ļ���
	printf("Max directory depth: %d\n", stats->maxDepth); // ��ӡ���Ŀ¼���
	printf("Max length from root to leaves: %d\n", stats->maxLenth); // ��ӡ�Ӹ�Ŀ¼��Ҷ�ڵ����󳤶�
	printf("Longest file path: %ls\n", stats->longestFilePath); // ��ӡ��ļ�·��
	printf("Longest file path length: %zu\n", stats->longestFilePathLength); // ��ӡ��ļ�·������
}

/*
 * ����: StackFree
 * --------------------
 * �ͷ�ջ��ռ�õ��ڴ�ռ䡣
 *
 * s: ָ��ջ�ṹ��ָ�롣
 *
 * ����: void
 *
 * �˺����ͷ�ջ�ṹ��ռ�õ��ڴ�ռ䣬����ջ��Ԫ����������ڴ�ռ䡣
 */
void StackFree(Stack* s)
{
	free(s->items); // �ͷ�ջ��Ԫ����ռ�õ��ڴ�ռ�
}


/*
 * ����: NodeInitialize
 * --------------------
 * ��ʼ��TreeNode�ṹ�塣
 *
 * node: ָ��TreeNode�ṹ��ָ�롣
 *
 * ����: void
 *
 * �˺�����������TreeNode�ṹ���ʼ��Ϊ��ʼ״̬����������Ա��������ΪNULL��0��
 */
void NodeInitialize(TreeNode* node)
{
	node->child = NULL; // �ӽڵ�ָ����ΪNULL
	node->next = NULL; // ��һ���ֵܽڵ�ָ����ΪNULL
	node->father = NULL; // ���ڵ�ָ����ΪNULL
	node->fileNum = 0; // �ļ�������Ϊ0
}

/*
 * ����: CalculateDepth
 * --------------------
 * ����������ȡ�
 *
 * root: ָ�������ڵ��ָ�롣
 *
 * ����: ������ȡ�
 *
 * �˺����������������ȣ������дӸ��ڵ㵽��ԶҶ�ڵ���·���ĳ��ȡ�
 */
int CalculateDepth(TreeNode* root)
{
	if (root == NULL)
	{
		return 0; // ������ڵ�Ϊ�գ��򷵻�0
	}
	int siblingDepth = CalculateDepth(root->next); // �����ֵܽڵ�����
	int childDepth = CalculateDepth(root->child); // �����ӽڵ�����
	return (childDepth > siblingDepth) ? childDepth + 1 : siblingDepth + 1; // �����ӽڵ���Ⱥ��ֵܽڵ�����еĽϴ�ֵ��1
}


/*
 * ����: InItilizeRoot
 * --------------------
 * ��ʼ�����ڵ����Ϣ��
 *
 * root: ָ����ڵ��ָ�롣
 * path: ���ڵ��ʾ��·����
 *
 * ����: void
 *
 * �˺������ݸ�����·����ʼ�����ڵ�ĸ������ԣ�������ȡ�����ʱ�䡢·�����ļ���С���Ƿ�ΪĿ¼�ȡ�
 */
void InItilizeRoot(TreeNode* root, TCHAR* path)
{
	WIN32_FIND_DATA findFileData; // ���ڴ洢�ļ��������ݵĽṹ��
	HANDLE hFind = INVALID_HANDLE_VALUE; // �ļ����
	TCHAR subDirPath[MAX_PATH]; // ��Ŀ¼·��

	_stprintf_s(subDirPath, MAX_PATH, _T("%s\\*"), path); // ��ʽ����Ŀ¼·��

	hFind = FindFirstFile(subDirPath, &findFileData); // ���ҵ�һ���ļ�
	root->depth = 1; // ���ڵ����Ϊ1
	root->CreationTime = findFileData.ftCreationTime; // ���ø��ڵ�Ĵ���ʱ��
	_tcscpy_s(root->path, MAX_PATH, path); // ��·���������ڵ��path����
	root->FileSize = ((uint64_t)findFileData.nFileSizeHigh << 32) | findFileData.nFileSizeLow; // ���ø��ڵ���ļ���С
	root->isdir = TRUE; // ��Ǹ��ڵ�ΪĿ¼
	root->length = 0; // ���ڵ�·������Ϊ0
	root->father = NULL; // ���ڵ�ĸ��ڵ�Ϊ��
}

/*
 * ����: BuildDirectoryTree
 * --------------------
 * ����Ŀ¼����
 *
 * root: ָ�������ڵ��ָ�롣
 * path: ���ڵ��ʾ��·����
 *
 * ����: void
 *
 * �˺������ݸ�����·������Ŀ¼����ͬʱͳ��Ŀ¼���������Ϣ����ӡ����־�ļ��С�
 */
void BuildDirectoryTree(TreeNode* root, TCHAR* path)
{
	InItilizeRoot(root, path); // ��ʼ�����ڵ����Ϣ

	DirectoryStats stats; // �洢Ŀ¼��ͳ����Ϣ�Ľṹ��
	Initialize(&stats); // ��ʼ��ͳ����Ϣ

	WIN32_FIND_DATA findFileData; // ���ڴ洢�ļ��������ݵĽṹ��
	HANDLE hFind = INVALID_HANDLE_VALUE; // �ļ����
	TCHAR subDirPath[MAX_PATH]; // ��Ŀ¼·��

	Stack stack; // ��ջ���ڴ洢Ŀ¼���ڵ�
	StackInit(&stack, 64); // ��ʼ����ջ
	StackPush(&stack, root); // �����ڵ�ѹ���ջ��
	NodeInitialize(root); // ��ʼ�����ڵ���ӽڵ�ָ����ֵܽڵ�ָ��

	while (!StackIsEmpty(&stack)) // ����ջ�ǿ�ʱ
	{
		TreeNode* currentNode = StackPop(&stack); // ������ջ�����ڵ㣬��ʾ��ǰ�ڵ�

		_stprintf_s(subDirPath, MAX_PATH, _T("%s\\*"), currentNode->path); // ��ʽ����ǰ�ڵ�·���µ������ļ���Ŀ¼������·��
		hFind = FindFirstFile(subDirPath, &findFileData); // ���ҵ�һ���ļ���Ŀ¼

		if (hFind == INVALID_HANDLE_VALUE) // �������ʧ��
		{
			currentNode->child = NULL; // ��ǰ�ڵ���ӽڵ��ÿ�
			printf("FindFirstFile failed (%d)\n", GetLastError()); // ���������Ϣ
			continue;
		}

		do
		{
			// ������ǰĿ¼�͸�Ŀ¼
			if (_tcscmp(findFileData.cFileName, _T(".")) == 0 || _tcscmp(findFileData.cFileName, _T("..")) == 0)
			{
				continue;
			}
			if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				// ����һ��Ŀ¼
				TreeNode* newNode = (TreeNode*)malloc(sizeof(TreeNode)); // ����һ���½ڵ�
				if (newNode == NULL) // �������ʧ��
				{
					printf("Failed to allocate memory while adding a dirctory node\n"); // ���������Ϣ
					StackFree(&stack); // �ͷŶ�ջ�ڴ�
					return;
				}
				NodeInitialize(newNode); // ��ʼ���½ڵ�

				newNode->depth = currentNode->depth + 1; // �����½ڵ�����
				newNode->CreationTime = findFileData.ftCreationTime; // �����½ڵ�Ĵ���ʱ��
				newNode->FileSize = ((uint64_t)findFileData.nFileSizeHigh << 32) | findFileData.nFileSizeLow; // �����½ڵ���ļ���С
				newNode->father = currentNode; // �����½ڵ�ĸ��ڵ�ָ��
				newNode->isdir = TRUE; // ����½ڵ�ΪĿ¼

				// ���½ڵ����Ϊ��ǰ�ڵ���ӽڵ���ֵܽڵ�
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

				_stprintf_s(newNode->path, MAX_PATH, _T("%s\\%s"), currentNode->path, findFileData.cFileName); // �����½ڵ��·��
				StackPush(&stack, newNode); // ���½ڵ�ѹ���ջ��
				stats.totalDirectories++; // ����Ŀ¼��ͳ����Ϣ

				// ����ͳ����Ϣ
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
				// ����һ���ļ�
				TreeNode* newNode = (TreeNode*)malloc(sizeof(TreeNode)); // ����һ���½ڵ�
				if (newNode == NULL) // �������ʧ��
				{
					printf("Failed to allocate memory while adding a file node\n"); // ���������Ϣ
					StackFree(&stack); // �ͷŶ�ջ�ڴ�
					return;
				}
				NodeInitialize(newNode); // ��ʼ���½ڵ�

				newNode->depth = currentNode->depth + 1; // �����½ڵ�����
				newNode->CreationTime = findFileData.ftCreationTime; // �����½ڵ�Ĵ���ʱ��
				newNode->FileSize = ((uint64_t)findFileData.nFileSizeHigh << 32) | findFileData.nFileSizeLow; // �����½ڵ���ļ���С
				newNode->isdir = FALSE; // ����½ڵ�Ϊ�ļ�
				newNode->father = currentNode; // �����½ڵ�ĸ��ڵ�ָ��

				// ���½ڵ����Ϊ��ǰ�ڵ���ӽڵ���ֵܽڵ�
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

				_stprintf_s(newNode->path, MAX_PATH, _T("%s\\%s"), currentNode->path, findFileData.cFileName); // �����½ڵ��·��
				stats.totalFiles++; // �����ļ���ͳ����Ϣ

				// ����ͳ����Ϣ
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
		} while (FindNextFile(hFind, &findFileData) != FALSE); // ����������һ���ļ���Ŀ¼

		FindClose(hFind); // �ر��ļ����Ҿ��
	}

	PrintStats(&stats); // ��ӡĿ¼��ͳ����Ϣ

	root->fileNum = stats.totalFiles; // ���ø��ڵ��ļ�������

	StackFree(&stack); // �ͷŶ�ջ�ڴ�

	// д��ͳ����Ϣ����־�ļ�
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
 * ����: NonRecursiveTraverseAndAdd
 * --------------------
 * �ǵݹ���������½ڵ���ļ������͸��ڵ���ļ�������
 *
 * root: ָ�������ڵ��ָ�롣
 *
 * ����: void
 *
 * �˺������÷ǵݹ鷽ʽ�������е����нڵ㣬�����ݽڵ�����͸��½ڵ���ļ������͸��ڵ���ļ�������
 */
void NonRecursiveTraverseAndAdd(TreeNode* root)
{
	if (root == NULL) return; // ������ڵ�Ϊ�գ���ֱ�ӷ���

	Stack stack; // ���ڴ洢���ڵ�Ķ�ջ
	StackInit(&stack, 64); // ��ʼ����ջ
	StackPush(&stack, root); // �����ڵ�ѹ���ջ��

	while (!StackIsEmpty(&stack)) // ����ջ�ǿ�ʱ
	{
		TreeNode* currentNode = StackPop(&stack); // ������ջ�����ڵ㣬��ʾ��ǰ�ڵ�

		if (currentNode->isdir) // �����ǰ�ڵ���Ŀ¼
		{
			currentNode->fileNum = 0; // ����ǰ�ڵ���ļ�������Ϊ0
			TreeNode* child = currentNode->child; // ��ȡ��ǰ�ڵ�ĵ�һ���ӽڵ�
			while (child != NULL) // ������ǰ�ڵ�������ӽڵ�
			{
				StackPush(&stack, child); // ���ӽڵ�ѹ���ջ��
				child = child->next; // �ƶ�����һ���ӽڵ�
			}
		}
		else // �����ǰ�ڵ����ļ�
		{
			currentNode->fileNum = 1; // ����ǰ�ڵ���ļ�������Ϊ1
		}

		// ���¸��ڵ�� fileNum �� FileSize ����
		if (currentNode->father != NULL && currentNode->isdir == FALSE) // �����ǰ�ڵ��и��ڵ��ҵ�ǰ�ڵ����ļ�
		{
			TreeNode* parent = currentNode->father; // ��ȡ��ǰ�ڵ�ĸ��ڵ�
			while (parent != NULL) // �������и��ڵ�
			{
				parent->fileNum += 1; // �����ڵ���ļ�������1
				parent->FileSize += currentNode->FileSize; // �����ڵ���ļ���С���ӵ�ǰ�ڵ���ļ���С
				parent = parent->father; // �ƶ������ڵ�ĸ��ڵ�
			}
		}
	}

	StackFree(&stack); // �ͷŶ�ջ�ڴ�
}


/*
 * ����: TraverseAndAdd
 * --------------------
 * �ݹ����������ӽڵ���ļ�������
 *
 * node: ָ��Ҫ�����Ľڵ��ָ�롣
 *
 * ����: int
 *     ���ؽڵ���ļ�������
 *
 * �˺����ݹ�������е����нڵ㣬�����ݽڵ�����ͼ���ڵ���ļ�������
 */
int TraverseAndAdd(TreeNode* node)
{
	if (node == NULL) return 0; // ����ڵ�Ϊ�գ��򷵻�0

	int fileCount = 0; // �ļ�������ʼ��Ϊ0
	if (node->isdir) // ����ڵ���Ŀ¼
	{
		TreeNode* child = node->child; // ��ȡ�ڵ�ĵ�һ���ӽڵ�
		while (child != NULL) // ���������ӽڵ�
		{
			fileCount += TraverseAndAdd(child); // �ݹ���� TraverseAndAdd ���������ۼӷ��ص��ļ�����
			child = child->next; // �ƶ�����һ���ӽڵ�
		}
		node->fileNum = fileCount; // ���ڵ���ļ���������Ϊ�ۼӵ��ļ�����
	}
	else // ����ڵ����ļ�
	{
		// �����ļ���Ϣ�����ݿ�
		node->fileNum = 1; // ���ڵ���ļ���������Ϊ1
		fileCount = 1; // �ļ���������Ϊ1
	}
	return fileCount; // ���ؽڵ���ļ�����
}



/*
 * ����: GenerateBatchInsertFile
 * --------------------
 * �������������ļ���Ϣ��SQL��䡣
 *
 * root: ָ�������ڵ��ָ�롣
 *
 * ����: void
 *
 * �˺������� NewTraverseGenerate �������������������ļ���Ϣ��SQL��䡣
 */
void GenerateBatchInsertFile(TreeNode* root)
{
	if (root == NULL)
	{
		fprintf(stderr, "The root node is NULL. An error occurred in file insertion\n");
		exit(1); // ������ڵ�Ϊ�գ������������Ϣ���˳�����
	}
	NewTraverseGenerate(root); // ���� NewTraverseGenerate �����������������ļ���Ϣ��SQL���
}


/*
 * ����: GetFileNameFromPath
 * --------------------
 * ��·���л�ȡ�ļ�����
 *
 * path: Ҫ��ȡ�ļ�����·����
 *
 * ����: const TCHAR*
 *     ����ָ����ȡ���ļ�����ָ�롣
 *
 * �˺����Ӹ���·������ȡ�ļ�����������ָ���ļ�����ָ�롣
 */
const TCHAR* GetFileNameFromPath(const TCHAR* path)
{
	const TCHAR* lastSlash = _tcsrchr(path, '\\'); // ��ȡ·�������һ����б���ַ���λ��
	if (lastSlash != NULL) {
		return lastSlash + 1; // ����ָ��б�ܺ����ַ���ָ�룬���ļ�������ʼλ��
	}
	else {
		return path; // ���·����û�з�б�ܣ���ֱ�ӷ���·��������Ϊ·����������ļ���
	}
}


/*
 * ����: PrintFileTime
 * --------------------
 * ���ļ�ʱ��ת��Ϊ�ַ�����ʽ��
 *
 * ft: Ҫ��ӡ���ļ�ʱ�䡣
 * dateTimeStr: �洢�ļ�ʱ���ַ����Ļ�������
 *
 * ����: void
 *
 * �˺������������ļ�ʱ��ת��Ϊ�ַ�����ʽ�����洢��ָ���Ļ������С�
 */
void PrintFileTime(FILETIME ft, char* dateTimeStr)
{
	SYSTEMTIME st;
	FileTimeToSystemTime(&ft, &st); // ���ļ�ʱ��ת��Ϊϵͳʱ���ʽ

	// ��ϵͳʱ���ʽת��Ϊָ��������ʱ���ַ�����ʽ
	sprintf_s(dateTimeStr, 40, "%04d-%02d-%02d %02d:%02d:%02d",
		st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
}


//��������Ѿ�������
/*
 * ����: TraverseGenerate
 * --------------------
 * ��������������������SQL����ļ���
 *
 * node: ָ��ǰ�ڵ��ָ�롣
 * filecount: ָ���ļ�������������ָ�롣
 * dircount: ָ��Ŀ¼������������ָ�롣
 *
 * ����: void
 *
 * �˺����ݹ�������е����нڵ㣬�����ݽڵ������������������SQL����ļ���
 */
void TraverseGenerate(TreeNode* node, int* filecount, int* dircount)
{
	if (node == NULL) return; // �����ǰ�ڵ�Ϊ�գ���ֱ�ӷ���

	char filename[15]; // ���ڴ洢�ļ����Ļ�����
	if (node->isdir) // �����ǰ�ڵ���Ŀ¼
	{
		sprintf_s(filename, 15, "dirdata%d", ((*dircount)++ / BATCH_SIZE) + 1); // ����Ŀ¼�����ļ���
	}
	else // �����ǰ�ڵ����ļ�
	{
		sprintf_s(filename, 15, "filedata%d", ((*filecount)++ / BATCH_SIZE) + 1); // �����ļ������ļ���
	}

	FILE* file = fopen(filename, "a"); // ���ļ���׷��ģʽд������
	if (file == NULL) // ����ļ���ʧ��
	{
		perror("Unable to open the file");
		exit(EXIT_FAILURE); // ���������Ϣ���˳�����
	}

	char time[40]; // ���ڴ洢ʱ���ַ����Ļ�����
	PrintFileTime(node->CreationTime, time); // ���ļ�ʱ��ת��Ϊ�ַ�����ʽ

	// ���ݽڵ�����������Ӧ��SQL��䣬��д���ļ�
	if (node->isdir) // �����ǰ�ڵ���Ŀ¼
	{
		char char_path[MAX_PATH];
		char char_name[MAX_PATH];
		TCharToChar(GetFileNameFromPath(node->path), char_name, MAX_PATH);
		TCharToChar(node->path, char_path, MAX_PATH);
		fprintf(file, "INSERT INTO directories ( name, path, file_size, creation_time, file_count) VALUES (%s, '%s', %llu, '%s', %d);\n",
			char_name, char_path, node->FileSize, time, node->fileNum);
	}
	else // �����ǰ�ڵ����ļ�
	{
		char char_path[MAX_PATH];
		char char_name[MAX_PATH];
		TCharToChar(GetFileNameFromPath(node->path), char_name, MAX_PATH);
		TCharToChar(node->path, char_path, MAX_PATH);
		fprintf(file, "INSERT INTO directories ( name, path, file_size, creation_time, file_count) VALUES (%s, '%s', %llu, '%s');\n",
			char_name, char_path, node->FileSize, time);
	}

	fclose(file); // �ر��ļ�

	// �ݹ������ǰ�ڵ���ӽڵ���ֵܽڵ�
	TraverseGenerate(node->child, filecount, dircount); // �����ӽڵ�
	TraverseGenerate(node->next, filecount, dircount); // �����ֵܽڵ�
}


/*
 * ����: NewTraverseGenerate
 * -------------------------
 * ʹ���µķǵݹ��������������������SQL����ļ���
 *
 * root: ָ�����ĸ��ڵ��ָ�롣
 *
 * ����: void
 *
 * �˺���ʹ���µķǵݹ���������������е����нڵ㣬�����ݽڵ�����������������SQL����ļ���
 */
void NewTraverseGenerate(TreeNode* root)
{
	Stack stack;
	StackInit(&stack, 64);
	StackPush(&stack, root); // �����ڵ�ѹ��ջ��

	// ��Ŀ¼�����ļ����ļ������ļ���׷��ģʽд������
	FILE* file_dir = fopen("..\\allfile\\dirdata.txt", "a");
	FILE* file_file = fopen("..\\allfile\\filedata.txt", "a");
	if (file_dir == NULL || file_file == NULL) // ����ļ���ʧ��
	{
		perror("Unable to open the file");
		exit(EXIT_FAILURE); // ���������Ϣ���˳�����
	}

	while (!StackIsEmpty(&stack)) // ��ջ�ǿ�ʱѭ�������ڵ�
	{
		TreeNode* currentNode = StackPop(&stack); // ����ջ���ڵ�

		// ������ǰ�ڵ㼰�������ֵܽڵ�
		do {
			if (currentNode->isdir) // �����ǰ�ڵ���Ŀ¼
			{
				char char_path[MAX_PATH];
				char char_name[MAX_PATH];
				TCharToChar(GetFileNameFromPath(currentNode->path), char_name, MAX_PATH);
				TCharToChar(currentNode->path, char_path, MAX_PATH);
				// ��Ŀ¼���ݲ���Ŀ¼�����ļ�
				fprintf(file_dir, "INSERT INTO directories ( name, path, file_size, creation_time, file_count) VALUES (\"%s\", \"%s\", %llu, %llu, %d);\n",
					char_name, char_path, currentNode->FileSize, FileTimetoUlonglong(currentNode->CreationTime), currentNode->fileNum);
			}
			else // �����ǰ�ڵ����ļ�
			{
				char char_path[MAX_PATH];
				char char_name[MAX_PATH];
				TCharToChar(GetFileNameFromPath(currentNode->path), char_name, MAX_PATH);
				TCharToChar(currentNode->path, char_path, MAX_PATH);
				// ���ļ����ݲ����ļ������ļ�
				fprintf(file_file, "INSERT INTO files ( name, path, file_size, creation_time) VALUES (\"%s\", \"%s\", %llu, %llu);\n",
					char_name, char_path, currentNode->FileSize, FileTimetoUlonglong(currentNode->CreationTime));
			}

			if (currentNode->child != NULL) // �����ǰ�ڵ����ӽڵ㣬���ӽڵ�ѹ��ջ��
				StackPush(&stack, currentNode->child);
			currentNode = currentNode->next; // ������һ���ֵܽڵ�
		} while (currentNode != NULL); // ��ǰ�ڵ�������ֵܽڵ�������

	}

	fclose(file_dir); // �ر�Ŀ¼�����ļ�
	fclose(file_file); // �ر��ļ������ļ�
	StackFree(&stack); // �ͷ�ջ�ڴ�
}


/*
 * ����: FreeTree
 * -------------
 * �ݹ��ͷ����е����нڵ㡣
 *
 * node: ָ��ǰ�ڵ��ָ�롣
 *
 * ����: void
 *
 * �˺����ݹ���ͷ����е����нڵ㡣�����ͷ����к��ӽڵ㣬Ȼ���ͷ������ֵܽڵ㣬����ͷŽڵ㱾��
 */
void FreeTree(TreeNode* node) {
	if (node == NULL) return; // ����ڵ�Ϊ�գ�ֱ�ӷ���

	// �ݹ��ͷ����к��ӽڵ�
	if (node->child != NULL) {
		FreeTree(node->child);
	}

	// �ݹ��ͷ������ֵܽڵ�
	if (node->next != NULL) {
		FreeTree(node->next);
	}

	free(node); // �ͷŽڵ㱾��
}



/*
 * ����: BatchInsert
 * ----------------
 * ִ������������������ļ��е������������뵽���ݿ��С�
 *
 * conn: ָ�������ӵ����ݿ��ָ�롣
 *
 * ����: void
 *
 * �˺������ļ����ֱ��ȡ�ļ��е�Ŀ¼���ļ����ݣ������� ExecuteBatchInsert ����ִ���������������
 * ���ر��ļ���
 */
void BatchInsert(MYSQL* conn) {
	FILE* file_file = fopen("..\\allfile\\filedata.txt", "r");
	if (file_file == NULL) {
		perror("Error opening file");
		exit(EXIT_FAILURE);
	}
	ExecuteBatchInsert(file_file, conn, FALSE); // �����ļ�����
	fclose(file_file);

	FILE* file_dir = fopen("..\\allfile\\dirdata.txt", "r");
	if (file_dir == NULL) {
		perror("Error opening file");
		exit(EXIT_FAILURE);
	}
	ExecuteBatchInsert(file_dir, conn, TRUE); // ����Ŀ¼����
	fclose(file_dir);
}



/*
 * ����: EscapeSQLString
 * ---------------------
 * ��Դ�ַ����е������ַ�����ת�壬��������洢��Ŀ���ַ����С�
 *
 * dest: Ŀ���ַ�����ָ�룬���ڴ洢ת���Ľ����
 * src: Դ�ַ�����ָ�룬��Ҫת���ԭʼ�ַ�����
 * destSize: Ŀ���ַ����Ĵ�С��������β�� null �ַ���
 *
 * ����: void
 *
 * �˺�����Դ�ַ����е������ַ�����ת�壬����ת���Ľ���洢��Ŀ���ַ����С�
 * ת����򣺽�Դ�ַ����е� '\' ת��Ϊ '\\'��
 * ���Ŀ���ַ����ռ䲻���Դ洢ת���Ľ������ֹͣת�������
 */
void EscapeSQLString(char* dest, const char* src, size_t destSize) {
	const char* srcPtr = src; // Դ�ַ���ָ��
	char* destPtr = dest; // Ŀ���ַ���ָ��
	size_t remainingSize = destSize; // Ŀ���ַ���ʣ��ռ��С

	// ѭ������Դ�ַ����е�ÿ���ַ���ֱ���������ַ���Ŀ���ַ����ռ䲻��
	while (*srcPtr && remainingSize > 1) {
		// �����ǰ�ַ��� '\'����Ҫת��
		if (*srcPtr == '\\') {
			// ���Ŀ���ַ����ռ��㹻�������ת�����
			if (remainingSize > 2) {
				*destPtr++ = '\\'; // �� '\' ת��Ϊ '\\'
				*destPtr++ = *srcPtr++; // ����ԭʼ�ַ���Ŀ���ַ�����
				remainingSize -= 2; // ����ʣ��ռ��С
			}
			else {
				// Ŀ���ַ����ռ䲻�㣬�޷�����ת��
				break;
			}
		}
		else {
			// �������ַ���ֱ�Ӹ��Ƶ�Ŀ���ַ�����
			*destPtr++ = *srcPtr++;
			--remainingSize; // ����ʣ��ռ��С
		}
	}
	*destPtr = '\0'; // ��ӽ�β�� null �ַ�
}

//�Ѿ������ĺ���
void DoubleEscapeSQLString(char* dest, const char* src, size_t destSize) {
	const char* srcPtr = src;
	char* destPtr = dest;
	size_t remainingSize = destSize;

	while (*srcPtr && remainingSize > 1) {
		if (*srcPtr == '\\') { //Ҳ��ת��   ����'\'
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
 * ����: CollectDirsInfo
 * ---------------------
 * �Ӹ����ļ��ж�ȡ�ļ�Ŀ¼·�����������ݿ����ռ�Ŀ¼��Ϣ��
 *
 * path: �����ļ�Ŀ¼·�����ļ�·����
 * conn: MySQL ���Ӿ��������ִ�����ݿ��ѯ��
 * root: ���ĸ��ڵ�ָ�룬���ڲ���Ŀ¼�ڵ㡣
 *
 * ����: void
 *
 * �˺����Ӹ����ļ��ж�ȡ�ļ�Ŀ¼·�������·��ִ�� SQL ��ѯ�Ի�ȡĿ¼��Ϣ���������д����־�ļ��С�
 * ��д����־�ļ�ʱ�����������Ŀ¼��Ϣ�������ж�Ӧ�Ľڵ㣬����ȡ��Ŀ¼���ļ��������������ʱ�䡣
 */
void CollectDirsInfo(const char* path, MYSQL* conn, TreeNode* root) {     //��mystats�ж�ȡ�ļ�Ŀ¼���ռ���Ϣ
	// �򿪸���·�����ļ������ڶ�ȡ�ļ�Ŀ¼·��
	FILE* file = fopen(path, "r");
	if (file == NULL) {
		perror("Error opening file");
		exit(EXIT_FAILURE);
	}

	// ����־�ļ������ڼ�¼Ŀ¼��Ϣ
	FILE* log_file = fopen("..\\allfile\\ThisLog.txt", "a");
	if (log_file == NULL) {
		fclose(file);
		perror("Error opening logfile");
		exit(EXIT_FAILURE);
	}

	// д����־�ļ����ļ���Ϣͷ��
	fprintf(log_file, "\n^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^�ļ���Ϣ^^^^^^^^^^^^^^^^^^^^^^^^\n");
	rewind(file); // �����ļ�ָ�뵽�ļ���ʼ
	char dir_path[MAX_PATH];
	// ���ж�ȡ�ļ�·����ִ�� SQL ��ѯ
	while (fgets(dir_path, MAX_PATH, file) != NULL) {
		dir_path[strcspn(dir_path, "\n")] = '\0'; // �Ƴ����з�
		if (!strcmp(dir_path, "stat dirs")) continue;
		if (!strcmp(dir_path, "end of dirs")) break;

		uint64_t file_size = 0;
		int file_count = 0;
		char query[MAX_SQL_SIZE];
		char escapedPath[MAX_PATH];
		*strrchr(dir_path, '\\') = '\0'; // ��·�����Ƴ��ļ�������
		EscapeSQLString(escapedPath, dir_path, MAX_PATH); // ת��·���ַ���

		// ���� SQL ��ѯ���
		sprintf_s(query, MAX_SQL_SIZE,
			"SELECT file_size, file_count "
			"FROM directories "
			"WHERE path = '%s'",
			escapedPath);

		// ִ�� SQL ��ѯ��������
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

		// ��ȡĿ¼���ļ��������������ʱ��
		FILETIME earliest, latest;
		TCHAR _tcs_escapedpath[MAX_PATH];
		CharToTChar(dir_path, _tcs_escapedpath, MAX_PATH);
		TreeNode* this = Find_Node_Only(root, _tcs_escapedpath); // �������ж�Ӧ�Ľڵ�
		if (this == NULL) {
			fprintf(log_file, "��·����%s��������\n", dir_path);
			continue;
		}
		FindExtretimeFromTree(this, &earliest, &latest);

		// ���ļ�ʱ��ת��Ϊ�ַ�����ʽ
		char* earliest_time = (char*)malloc(40 * sizeof(char));
		char* latest_time = (char*)malloc(40 * sizeof(char));
		PrintFileTime(earliest, earliest_time);
		PrintFileTime(latest, latest_time);

		// ��Ŀ¼��Ϣд����־�ļ�
		fprintf(log_file, "��·����%s�� �ļ���С��%lld B �ļ�������%d �����ļ�����ʱ�䣺%s ������ʱ�䣺%s \n",
			dir_path, file_size, file_count, earliest_time, latest_time);

		// �ͷŶ�̬������ڴ�
		free(earliest_time);
		free(latest_time);
	}

	// �ر���־�ļ����ļ�
	fclose(log_file);
	fclose(file);
}


/**
 * �Ӹ������ַ�������ȡ·����Ϣ��
 *
 * @param line �����ַ���������·����Ϣ���������ݡ�
 * @param path �洢��ȡ��·����Ϣ�Ļ�������
 */
void extractPath(TCHAR* line, TCHAR* path) {
	TCHAR* token = _tcstok(line, ",");
	if (token != NULL) {
		_tcsncpy(path, token, MAX_PATH);
		path[MAX_PATH - 1] = '\0'; // ȷ���ַ�����NULL��β
	}
	else {
		path[0] = '\0'; // ���û���ҵ��ָ������򷵻ؿ��ַ���
	}
}


/*
 * ����: Find_Node
 * ---------------------
 * �����в���ָ��·���Ľڵ㣬�����ظýڵ㡣
 *
 * root: ���ĸ��ڵ�ָ�롣
 * path: Ŀ��ڵ��·����
 * stack: ���ڴ洢�ڵ��ջ��
 * prevSibling: ָ��ǰһ���ֵܽڵ��ָ�롣
 *
 * ����: ָ��Ŀ��ڵ��ָ�룬���δ�ҵ�Ŀ��ڵ㣬�򷵻� NULL��
 *
 * �˺��������в���ָ��·���Ľڵ㣬�����ظýڵ㡣����ҵ�Ŀ��ڵ㣬�򽫵�ǰ�ڵ�����и��ڵ㱣�浽ջ�У�����ָ��ǰһ���ֵܽڵ��ָ������Ϊ��Ӧ��ֵ��
 */
TreeNode* Find_Node(TreeNode* root, TCHAR* path, Stack* stack, TreeNode** prevSibling) {
	TreeNode* current = root; // ��ǰ�ڵ�ָ�룬�Ӹ��ڵ㿪ʼ����
	TreeNode* prev = NULL; // ���ڱ��浱ǰ�ڵ�ĸ��ڵ�
	// ��ʼ��ǰһ���ֵܽڵ�Ϊ NULL

	// ѭ���������е�ÿ���ڵ㣬ֱ���ҵ�Ŀ��ڵ�������������
	while (current != NULL) {
		// ����ҵ�Ŀ��ڵ�
		if (_tcscmp(current->path, path) == 0) {
			// ����ǰ�ڵ�����и��ڵ㱣�浽ջ��
			*prevSibling = prev; // ����ǰһ���ֵܽڵ�
			return current; // ����Ŀ��ڵ�ָ��
		}

		// �����ǰ�ڵ��к��ӽڵ㣬�򽫵�ǰ�ڵ�����ջ�У�Ȼ��ǰ����һ�����ӽڵ�
		if (current->child != NULL) {
			StackPush(&stack, current); // ����ǰ�ڵ�����ջ��
			prev = NULL; // �����µĲ㼶������ǰһ���ֵܽڵ�
			current = current->child; // �ƶ�����ǰ�ڵ�ĵ�һ�����ӽڵ�
		}
		else {
			// û�к��ӽڵ㣬�����ƶ�����һ���ֵܽڵ�
			if (current->next != NULL) {
				prev = current; // ����ǰһ���ֵܽڵ�
				current = current->next; // �ƶ�����ǰ�ڵ����һ���ֵܽڵ�
			}
			else {
				// û����һ���ֵܽڵ㣬���ݵ����ڵ㣬Ȼ���Ը��ڵ����һ���ֵܽڵ�
				while (!StackIsEmpty(stack) && current->next == NULL) {
					current = StackPop(&stack); // ���ݵ����ڵ�
					prev = current; // ����ǰһ���ֵܽڵ�
				}
				if (current != NULL) {
					prev = current;
					current = current->next; // ��Ϊ�Ǹ��ڵ���ֵܽڵ㣬�ƶ������ڵ����һ���ֵܽڵ�
					// ����ǰһ���ֵܽڵ�
				}
			}
		}
	}
	return NULL; // δ�ҵ�Ŀ��ڵ㣬���� NULL
}


/*
 * ����: Find_Node_Only
 * ---------------------
 * �����в���ָ��·���Ľڵ㣬�����ظýڵ㡣
 * �� Find_Node ������ͬ���ǣ��˺���������ڵ�ĸ��ڵ㵽ջ�С�
 *
 * root: ���ĸ��ڵ�ָ�롣
 * path: Ŀ��ڵ��·����
 *
 * ����: ָ��Ŀ��ڵ��ָ�룬���δ�ҵ�Ŀ��ڵ㣬�򷵻� NULL��
 *
 * �˺��������в���ָ��·���Ľڵ㣬�����ظýڵ㡣�� Find_Node ������ͬ���ǣ��˺���������ڵ�ĸ��ڵ㵽ջ�С�
 */
TreeNode* Find_Node_Only(TreeNode* root, TCHAR* path) {
	Stack stack; // ���ڴ洢�ڵ��ջ
	StackInit(&stack, 20); // ��ʼ��ջ
	TreeNode* current = root; // ��ǰ�ڵ�ָ�룬�Ӹ��ڵ㿪ʼ����

	// ѭ���������е�ÿ���ڵ㣬ֱ���ҵ�Ŀ��ڵ�������������
	while (current != NULL) {
		// ����ҵ�Ŀ��ڵ�
		if (_tcscmp(current->path, path) == 0) {
			StackFree(&stack); // �ͷ�ջ�ڴ�
			return current; // ����Ŀ��ڵ�ָ��
		}

		// �����ǰ�ڵ��к��ӽڵ㣬�򽫵�ǰ�ڵ�����ջ�У�Ȼ��ǰ����һ�����ӽڵ�
		if (current->child != NULL) {
			StackPush(&stack, current); // ����ǰ�ڵ�����ջ��
			current = current->child; // �ƶ�����ǰ�ڵ�ĵ�һ�����ӽڵ�
		}
		else {
			// û�к��ӽڵ㣬�����ƶ�����һ���ֵܽڵ�
			if (current->next != NULL) {
				current = current->next; // �ƶ�����ǰ�ڵ����һ���ֵܽڵ�
			}
			else {
				// û����һ���ֵܽڵ㣬���ݵ����ڵ㣬Ȼ���Ը��ڵ����һ���ֵܽڵ�
				while (!StackIsEmpty(&stack) && current->next == NULL) {
					current = StackPop(&stack); // ���ݵ����ڵ�
				}
				if (current != NULL) {
					current = current->next; 
					// ��Ϊ�Ǹ��ڵ���ֵܽڵ㣬�ƶ������ڵ����һ���ֵܽڵ�
				}
			}
		}
	}

	StackFree(&stack); // �ͷ�ջ�ڴ�
	return NULL; // δ�ҵ�Ŀ��ڵ㣬���� NULL
}


/*
 * ����: CasecadeDelete
 * --------------------
 * �ݹ�ɾ���Ը����ڵ�Ϊ�������������ͷŽڵ��ڴ档
 *
 * node: ��ɾ�������ĸ��ڵ�ָ�롣
 *
 * ����: void
 *
 * �˺����ݹ�ɾ���Ը����ڵ�Ϊ�������������ͷŽڵ��ڴ档
 */
void CasecadeDelete(TreeNode* node) {
	if (node == NULL) {
		return; // ����ڵ�Ϊ�գ�ֱ�ӷ���
	}
	FreeTree(node->child); // �ݹ��ͷ�����
	free(node); // �ͷŵ�ǰ�ڵ��ڴ�
}


/*
 * ����: ModifyTree_dir
 * --------------------
 * �����ļ���mydir���������м�֦���Ƴ�ָ��·���µĽڵ㼰���ӽڵ㣬����������ͳ����Ϣ��
 *
 * filepath: �ļ�·��������Ҫ��֦��Ŀ¼��Ϣ��
 * conn: MySQL ���Ӿ�������ڲ�ѯĿ¼��Ϣ������ͳ�����ݡ�
 * root: ���ĸ��ڵ�ָ�롣
 *
 * ����: void
 *
 * �˺��������ļ���mydir���������м�֦���Ƴ�ָ��·���µĽڵ㼰���ӽڵ㣬����������ͳ����Ϣ��
 */
void ModifyTree_dir(char* filepath, MYSQL* conn, TreeNode* root) {
	FILE* file = fopen(filepath, "r"); // ���ļ��Զ�ȡĿ¼��Ϣ
	if (file == NULL) { // ����ļ��Ƿ�ɹ���
		perror("Error opening file"); // ��ӡ������Ϣ
		exit(EXIT_FAILURE); // �˳�����
	}
	rewind(file); // �����ļ�ָ�뵽�ļ���ͷ

	char line[MAX_PATH]; // ���ڴ洢��ȡ��ÿһ������
	while (fgets(line, MAX_PATH, file) != NULL) { // ���ж�ȡ�ļ�����
		TCHAR dir_path[MAX_PATH]; // �洢Ŀ¼·���Ļ�����
		line[strcspn(line, "\n")] = 0; // �Ƴ����з�
		if (!strcmp(line, "selected dirs")) continue; // ����������ǣ��������һ��
		if (!strcmp(line, "end of dirs")) break; // ����������ǣ�������ѭ��������ȡ
		// ����ȡ��·���ַ���ת��Ϊ TCHAR ��ʽ
		TCHAR _tline[MAX_PATH];
		CharToTChar(line, _tline, MAX_PATH);
		// ��·������ȡĿ¼���֣��������һ��·���ָ�������Ϊ�ַ�����β
		extractPath(_tline, dir_path);
		*_tcsrchr(dir_path, _T('\\')) = '\0';
		// �����в���ָ��·����Ӧ�Ľڵ�
		TreeNode* node = Find_Node_Only(root, dir_path);
		if (node != NULL) { // ����ҵ���Ŀ��ڵ�
			BOOL flag = TRUE; // ��Ǳ��������ڱ���Ƿ��Ѿ������˸��ڵ���ӽڵ��ϵ
			TreeNode* parent = node; // ��¼��ǰ�ڵ�ĸ��ڵ�
			do {
				parent = parent->father; // �ƶ������ڵ�
				if (flag) { // �����δ�����ڵ���ӽڵ��ϵ
					if (parent->child == node) { // ���Ŀ��ڵ��Ǹ��ڵ�ĵ�һ���ӽڵ�
						parent->child = node->next; // ���¸��ڵ���ӽڵ�ָ��
					}
					else { // ���Ŀ��ڵ��Ǹ��ڵ���ֵܽڵ�
						// �ҵ�Ŀ��ڵ��ǰһ���ֵܽڵ�
						TreeNode* pre = parent->child;
						while (pre->next != node) {
							pre = pre->next;
						}
						// ����ǰһ���ֵܽڵ�� next ָ��
						pre->next = node->next;
					}
					flag = FALSE; // ���Ϊ�Ѵ����ڵ���ӽڵ��ϵ
				}
				// ���¸��ڵ���ļ��������ļ��ܴ�С��Ϣ
				parent->fileNum--;
				parent->FileSize -= node->FileSize;
			} while (parent->father != NULL); // ѭ��ֱ��������ڵ�
			// ��ɽڵ��֦���ͷŽڵ㼰���ӽڵ���ڴ���Դ
			CasecadeDelete(node); // �㼶ɾ���ýڵ�������ӽڵ㣬ͬʱ�ͷ���Դ
		}
		else {
			_tprintf(_T("%s���ļ��в�����"), dir_path); // ���Ŀ��ڵ㲻���ڣ����ӡ������Ϣ
		}
	}
	fclose(file); // �ر��ļ�
}

/*
 * ����: UnixTimeToFileTime
 * ------------------------
 * �� Unix ʱ���ת��Ϊ Windows FILETIME �ṹ��
 *
 * unixTime: Unix ʱ�������ʾ�� 1970 �� 1 �� 1 ������������������
 * fileTime: ָ�� FILETIME �ṹ��ָ�룬���ڴ洢ת����� Windows �ļ�ʱ�䡣
 *
 * ����: void
 *
 * �˺����� Unix ʱ���ת��Ϊ Windows FILETIME �ṹ��
 * Unix ʱ����Ǵ� 1970 �� 1 �� 1 �տ�ʼ������������� FILETIME �ṹ�� Windows �б�ʾʱ���һ�ַ�ʽ��
 * FILETIME �ṹ�洢������ 1601 �� 1 �� 1 �������� 100 ����������
 * ��ˣ���Ҫ�� Unix ʱ���ת��Ϊ FILETIME �ṹ�еļ������
 */
void UnixTimeToFileTime(time_t unixTime, FILETIME* fileTime)
{
	// Unix ��Ԫ�� FILETIME ��Ԫ֮��� 100 ��������
	const int64_t EPOCH_DIFFERENCE = 116444736000000000LL;

	// �� Unix ʱ���ת��Ϊ 100 ��������
	int64_t intervals = ((int64_t)unixTime * 10000000) + EPOCH_DIFFERENCE;

	// ���� FILETIME �ṹ
	fileTime->dwLowDateTime = (DWORD)(intervals);
	fileTime->dwHighDateTime = (DWORD)(intervals >> 32);
}


/*
 * ����: ExtractInfo
 * -----------------
 * ��һ���ı�����ȡ�ļ���Ϣ��������洢��ָ���ı����С�
 *
 * line: �����ļ���Ϣ��һ���ı���
 * path: ���ڴ洢�ļ�·���Ļ�������
 * flag: ���ڴ洢�ļ���־�Ļ�������ֻȡ��һ���ַ���
 * creationtime: ���ڴ洢�ļ�����ʱ��� FILETIME �ṹ��ָ�롣
 * file_size: ���ڴ洢�ļ���С�ı�����ָ�롣
 *
 * ����: void
 *
 * �˺�����һ���ı�����ȡ�ļ���Ϣ��������洢��ָ���ı����С�
 * �ú������������ı����� UTF-8 ���롣
 * �����ı��ĸ�ʽΪ����·��,��־,����ʱ��,�ļ���С�������и����ֶ��ɶ��ŷָ���
 */
void ExtractInfo(TCHAR* line, TCHAR* path, TCHAR* flag, FILETIME* creationtime, uint64_t* file_size)
{
	_tcscpy(path, _tcstok(line, _T(",")));
	// strtok() �������޸�ԭʼ�ַ��������ָ���λ���滻Ϊ null ��ֹ������������һ���ǿյ����ַ�����ָ�롣
	*flag = _tcstok(NULL, _T(","))[0];
	UnixTimeToFileTime(atol(_tcstok(NULL, _T(","))), creationtime);
	*file_size = atoll(_tcstok(NULL, _T(",")));
}



/*
 * ����: DeteleFileFromTree
 * ------------------------
 * ������ɾ��ָ��·�����ļ��ڵ㣬�������丸�ڵ���ļ������ļ���С��
 *
 * filepath: ָ��Ҫɾ�����ļ�·����
 * root: ���ĸ��ڵ㡣
 *
 * ����: ���ɹ�ɾ���ļ��ڵ㣬�򷵻� TRUE�����򷵻� FALSE��
 *
 * �˺���������ɾ��ָ��·�����ļ��ڵ㣬�������丸�ڵ���ļ������ļ���С��
 * ����ļ��ڵ㲻���ڻ��ѱ�ɾ�������ӡ������Ϣ������ FALSE��
 */
BOOL DeteleFileFromTree(TCHAR* filepath, TreeNode* root)
{
	// ����ָ��·�����ļ��ڵ�
	TreeNode* node = Find_Node_Only(root, filepath);
	if (node != NULL)
	{
		TreeNode* parent = node->father;
		uint64_t file_size = node->FileSize;
		// ������ڵ���ӽڵ㲻��Ҫɾ���Ľڵ�
		if (parent->child != node)
		{
			TreeNode* pre = parent->child;
			// Ѱ��Ҫɾ���ڵ��ǰһ���ֵܽڵ�
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
		// ���¸��ڵ㼰�����Ƚڵ���ļ������ļ���С
		while (parent != NULL)
		{
			parent->fileNum--;
			parent->FileSize -= file_size;
			parent = parent->father;
		}
		// ɾ���ڵ㼰���ӽڵ�
		CasecadeDelete(node);
		return TRUE; // ���سɹ�ɾ���ļ��ڵ�
	}
	else
	{
		_tprintf(_T("%s���ļ��в����ڣ����ѱ�ɾ��\n"), filepath);
		return FALSE; // ����δ�ҵ��ļ��ڵ���ļ��ѱ�ɾ��
	}
}


/*
 * ����: ModifyFileFromFree
 * ------------------------
 * �޸�����ָ��·�����ļ��ڵ�Ĵ���ʱ����ļ���С���������丸�ڵ���ļ���С��
 *
 * path: ָ��Ҫ�޸ĵ��ļ�·����
 * root: ���ĸ��ڵ㡣
 * creationtime: �µĴ���ʱ�䡣
 * modified_size: �޸ĺ���ļ���С��
 *
 * ����: ���ɹ��޸��ļ��ڵ㣬�򷵻� TRUE�����򷵻� FALSE��
 *
 * �˺����޸�����ָ��·�����ļ��ڵ�Ĵ���ʱ����ļ���С���������丸�ڵ���ļ���С��
 * ���δ�ҵ�ָ��·�����ļ��ڵ㣬���ӡ������Ϣ������ FALSE��
 */
BOOL ModifyFileFromFree(TCHAR* path, TreeNode* root, FILETIME creationtime, uint64_t modified_size)
{
	// ����ָ��·�����ļ��ڵ�
	TreeNode* node = Find_Node_Only(root, path);
	if (node == NULL) {
		// ����δ�ҵ��ڵ�����
		_tprintf(_T("Failed to find %s dir\n"), path);
		return FALSE;
	}
	uint64_t size_diff = modified_size - node->FileSize;
	TreeNode* parent = node->father;
	// ���¸��ڵ㼰�����Ƚڵ���ļ���С
	while (parent != NULL)
	{
		parent->FileSize += size_diff;
		parent = parent->father;
	}
	// �����ļ��ڵ�Ĵ���ʱ��
	node->CreationTime = creationtime;

	return TRUE; // ���سɹ��޸��ļ��ڵ�
}



/*
 * ����: SplitPath
 * --------------
 * ������������·�����Ϊ��Ŀ¼·�����ļ�����
 *
 * fullPath: �������ļ�·����
 * directory: ���ڴ洢��Ŀ¼·���Ļ�������
 * fileName: ���ڴ洢�ļ����Ļ�������
 *
 * ����: void
 *
 * �˺���������������·�����Ϊ��Ŀ¼·�����ļ�������������洢����Ӧ�Ļ������С�
 * ���·���д���б�ߣ���·�����Ϊ��Ŀ¼·�����ļ��������û��б�ߣ�������·������Ϊ�ļ�����
 */
void SplitPath(const TCHAR* fullPath, TCHAR* directory, TCHAR* fileName) {
	const TCHAR* lastSlash = _tcsrchr(fullPath, _T('\\')); // �������һ��б��
	if (lastSlash != NULL)
	{
		// ��ȡ��Ŀ¼·��
		_tcsncpy_s(directory, MAX_PATH, fullPath, lastSlash - fullPath);
		directory[lastSlash - fullPath] = _T('\0');
		// ��ȡ�ļ���
		_tcscpy_s(fileName, MAX_PATH, lastSlash + 1);
	}
	else
	{
		// ���û��б�ߣ�����·�������ļ���
		_tcscpy_s(directory, MAX_PATH, _T(""));
		_tcscpy_s(fileName, MAX_PATH, fullPath);
	}
}



/*
 * ����: AppandFileIntoTree
 * ------------------------
 * ��ָ��·�����ļ���ӵ����У��������ļ��ڵ����������Լ����ڵ���ļ��������ļ���С��
 *
 * path: Ҫ��ӵ��ļ���·����
 * root: ���ĸ��ڵ㡣
 * creationtime: �ļ��Ĵ���ʱ�䡣
 * file_size: �ļ��Ĵ�С��
 *
 * ����: ���ɹ�����ļ��ڵ㣬�򷵻� TRUE�����򷵻� FALSE��
 *
 * �˺�����ָ��·�����ļ���ӵ����У��������ļ��ڵ����������Լ����ڵ���ļ��������ļ���С��
 * ����޷��ҵ�ָ��·���ĸ�Ŀ¼�ڵ㣬���ӡ������Ϣ������ FALSE��
 * ����ڴ����ʧ�ܣ����ӡ������Ϣ������ FALSE��
 */
BOOL AppandFileIntoTree(TCHAR* path, TreeNode* root, FILETIME creationtime, uint64_t file_size)
{
	TCHAR dir_path[MAX_PATH];
	TCHAR file_name[MAX_PATH];
	SplitPath(path, dir_path, file_name); // �ָ�·��Ϊ��Ŀ¼���ļ���

	// ���Ҹ�Ŀ¼�ڵ�
	TreeNode* node = Find_Node_Only(root, dir_path);
	if (node == NULL)
	{
		_tprintf(_T("Failed to find %s dir\n"), dir_path);
		return FALSE; // δ�ҵ���Ŀ¼�ڵ㣬���� FALSE
	}

	// �����µ��ļ��ڵ�
	TreeNode* newnode = (TreeNode*)malloc(sizeof(TreeNode));
	if (newnode == NULL) {
		// �����ڴ����ʧ�ܵ����
		fprintf(stderr, "Failed to allocate memory while adding a file node\n");
		return FALSE;
	}
	NodeInitialize(newnode);
	newnode->fileNum = 1;        // �����ļ��� filenum ����Ϊ 1
	newnode->CreationTime = creationtime;
	newnode->FileSize = file_size;

	// ���½ڵ���ӵ���Ŀ¼�ڵ���ӽڵ�������
	if (node->child == NULL)
	{
		node->child = newnode; // ����Ŀ¼�ڵ����ӽڵ㣬���½ڵ���Ϊ���ӽڵ�
	}
	else
	{
		TreeNode* pre = node->child;
		while (pre->next != NULL)
		{
			pre = pre->next;
		}
		pre->next = newnode; // �����½ڵ���ӵ���Ŀ¼�ڵ���ӽڵ�����ĩβ
	}

	// ���¸�Ŀ¼�ڵ㼰�����Ƚڵ���ļ��������ļ���С
	TreeNode* parent = node;
	while (parent->father != NULL)
	{
		parent->fileNum++;
		parent->FileSize += newnode->FileSize;
		parent = parent->father;
	}

	return TRUE; // ���سɹ�����ļ��ڵ�
}


/*
 * ����: ModifyTree_file
 * ---------------------
 * �����ļ��еĲ���ָ��޸������ļ��ڵ㡣
 *
 * filepath: ����ָ���ļ���·����
 * conn: MySQL ���Ӿ����δʹ�á�
 * root: ���ĸ��ڵ㡣
 *
 * ����: void
 *
 * �˺��������ļ��еĲ���ָ��������ļ��ڵ������ӡ��޸Ļ�ɾ��������
 * ����ָ���ļ�����һϵ��ָ�ÿ��ָ��ռһ�У���ʽΪ��·��,��־,����ʱ��,�ļ���С��
 * ���У�·��ΪҪ�������ļ�·������־Ϊ�������ͣ����� 'A' (���)��'M' (�޸�)��'D' (ɾ��)��
 * ����ʱ����ļ���С������ӻ��޸��ļ��ڵ�����ԡ�
 * ����ļ���ʧ�ܣ����ӡ������Ϣ���˳�����
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
		// �Ƴ����з�
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
 * ����: FileTimetoUlonglong
 * ------------------------
 * �� FILETIME �ṹת��Ϊ ULONGLONG ���͵�����ֵ��
 *
 * time: ��ת���� FILETIME �ṹ��
 *
 * ����: ULONGLONG ���͵�����ֵ����ʾʱ�䡣
 *
 * �˺����� FILETIME �ṹ��ʾ��ʱ��ת��Ϊ ULONGLONG ���͵�����ֵ��
 * �Ա��ڽ��бȽϻ�����������
 */
ULONGLONG FileTimetoUlonglong(FILETIME time)
{
	return (((ULONGLONG)time.dwHighDateTime) << 32) + time.dwLowDateTime;
}


/*
 * ����: FindExtretimeFromTree
 * ---------------------------
 * �����в��������������ļ�����ʱ�䡣
 *
 * node: ���ĸ��ڵ㡣
 * earliest: ���ڴ洢������ļ�����ʱ�䡣
 * latest: ���ڴ洢������ļ�����ʱ�䡣
 *
 * ����: void
 *
 * �˺��������������е����нڵ㣬���������������ļ�����ʱ�䣬��������洢�� earliest �� latest �С�
 */
void FindExtretimeFromTree(TreeNode* node, FILETIME* earliest, FILETIME* latest)
{
	Stack stack;
	StackInit(&stack, 64); // ��ʼ��ջ
	*earliest = node->CreationTime; // ��ʼ������ʱ��Ϊ���ڵ�Ĵ���ʱ��
	*latest = *earliest; // ��ʼ������ʱ��Ϊ����ʱ��

	// �����ڵ�����ջ
	StackPush(&stack, node);

	// ѭ������ջ�еĽڵ�
	while (!StackIsEmpty(&stack))
	{
		TreeNode* current = StackPop(&stack); // ����ջ���ڵ�

		// ��鵱ǰ�ڵ��Ƿ�Ϊ��
		while (current != NULL) {
			// �����ӽڵ�
			if (current->child != NULL) {
				StackPush(&stack, current->child); // ���ӽڵ�����ջ
			}

			// �������������ʱ��
			if (FileTimetoUlonglong(*earliest) > FileTimetoUlonglong(current->CreationTime)) {
				*earliest = current->CreationTime; // ��������ʱ��
			}
			if (FileTimetoUlonglong(*latest) < FileTimetoUlonglong(current->CreationTime)) {
				*latest = current->CreationTime; // ��������ʱ��
			}

			// �ƶ�����һ���ֵܽڵ�
			current = current->next;
		}
	}

	StackFree(&stack); // �ͷ�ջ�ڴ�
}

/*
 * ����: CharToTChar
 * -----------------
 * �����ֽ��ַ���ת��Ϊ���ַ��ַ�����
 *
 * src: Դ���ֽ��ַ�����
 * dest: Ŀ����ַ��ַ�����ָ�룬���ڴ洢ת����Ľ����
 * destSize: Ŀ���ַ����Ĵ�С��������β�� null �ַ���
 *
 * ����: void
 *
 * ��������� UNICODE��ʹ�� mbstowcs ���������ֽ��ַ���ת��Ϊ���ַ��ַ�����
 * ���û�ж��� UNICODE����ֱ�Ӹ��ƶ��ֽ��ַ�����Ŀ���ַ����С�
 * ���ȷ��Ŀ���ַ����Կ��ַ�������
 */
void CharToTChar(const char* src, TCHAR* dest, size_t destSize)
{
#ifdef UNICODE
	// ��������� UNICODE��ʹ�ö��ֽڵ����ַ���ת��
	mbstowcs(dest, src, destSize);
#else
	// ���û�ж��� UNICODE��ֱ�Ӹ����ַ���
	strncpy(dest, src, destSize);
#endif
	dest[destSize - 1] = _T('\0'); // ȷ���ַ����Կ��ַ�����
}


/*
 * ����: TCharToChar
 * ----------------
 * �����ַ��ַ���ת��Ϊ���ֽ��ַ�����
 *
 * src: Դ���ַ��ַ�����
 * dest: Ŀ����ֽ��ַ�����ָ�룬���ڴ洢ת����Ľ����
 * destSize: Ŀ���ַ����Ĵ�С��������β�� null �ַ���
 *
 * ����: void
 *
 * ��������� _UNICODE��ʹ�� wcstombs_s ���������ַ��ַ���ת��Ϊ���ֽ��ַ�����
 * ���û�ж��� _UNICODE����ֱ�Ӹ��ƿ��ַ��ַ�����Ŀ���ַ����С�
 */
void TCharToChar(const TCHAR* src, char* dest, size_t destSize)
{
#ifdef _UNICODE
	// ���ַ�ת��Ϊ���ֽ��ַ�
	size_t convertedChars = 0;
	wcstombs_s(&convertedChars, dest, destSize, src, _TRUNCATE);
#else
	// ֱ�Ӹ���
	strncpy_s(dest, destSize, src, _TRUNCATE);
#endif
}

/*
 * ����: CompareDifference
 * ----------------------
 * �Ƚ����ݿ��е�Ŀ¼������Ŀ¼�Ĳ��죬���������¼����־�ļ��С�
 *
 * path: �����ļ���·�����ļ���·����
 * conn: MySQL ���ӡ�
 * root: ���ĸ��ڵ㡣
 *
 * ����: void
 *
 * �˺����򿪰����ļ���·�����ļ������ж�ȡ�ļ���·���������ÿ��·��ִ�бȽϲ�����
 * �ȽϵĽ�������ļ��д�С���졢�ļ��������졢�����ļ�����ʱ��������ļ�����ʱ�䡣
 * �����¼����־�ļ��С�
 */
void CompareDifference(const char* path, MYSQL* conn, TreeNode* root)
{
	FILE* file = fopen(path, "r"); // �򿪰����ļ���·�����ļ�
	if (file == NULL) {
		perror("Error opening file");
		exit(EXIT_FAILURE);
	}
	char dir_path[MAX_PATH];

	FILE* log_file = fopen("..\\allfile\\log.txt", "a"); // ����־�ļ�
	if (log_file == NULL)
	{
		fclose(file);
		perror("Error opening logfile");
		exit(EXIT_FAILURE);
	}

	fprintf(log_file, "\n^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^�ļ�����^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");

	rewind(file); // �����ļ�ָ�뵽�ļ���ʼ

	// ������ȡ��ִ��SQL���
	while (fgets(dir_path, MAX_PATH, file) != NULL)
	{
		dir_path[strcspn(dir_path, "\n")] = 0;
		if (!strcmp(dir_path, "stat dirs")) continue;
		if (!strcmp(dir_path, "end of dirs")) break;
		TCHAR tcs_dir_path[MAX_PATH];
		*strrchr(dir_path, '\\') = '\0'; // �Ƴ�·���е��ļ�������
		CharToTChar(dir_path, tcs_dir_path, MAX_PATH); // ��·��ת��Ϊ���ַ�
		TreeNode* this = Find_Node_Only(root, tcs_dir_path); // �������ж�Ӧ�ڵ�
		if (!this)
		{
			fprintf(log_file, "�޷��ҵ� %s �ļ���\n", dir_path);
			continue;
		}
		FILETIME earliest;
		FILETIME latest;
		FindExtretimeFromTree(this, &earliest, &latest); // �������нڵ�����������ʱ��
		int tree_file_num = this->fileNum; // ���нڵ���ļ�����
		uint64_t tree_dir_size = this->FileSize; // ���нڵ���ļ��д�С
		int database_file_num = 0;
		uint64_t database_dir_size = 0;
		char query[MAX_SQL_SIZE];
		char escaped_dir_path[MAX_PATH];
		EscapeSQLString(escaped_dir_path, dir_path, MAX_PATH); // ת��·���е������ַ�
		sprintf_s(query, MAX_SQL_SIZE,
			"SELECT file_size, file_count "
			"FROM directories "
			"WHERE path = '%s'"
			, escaped_dir_path); // ������ѯ���
		if (mysql_query(conn, query)) // ִ�в�ѯ
		{
			fclose(log_file);
			fclose(file);
			fprintf(stderr, "%s\n", mysql_error(conn));
			exit(1);
		}
		MYSQL_RES* result = mysql_store_result(conn); // ��ȡ��ѯ���
		if (result == NULL) {
			fclose(log_file);
			fclose(file);
			fprintf(stderr, "%s\n", mysql_error(conn));
			exit(1);
		}

		MYSQL_ROW row = mysql_fetch_row(result);
		if (row) {
			database_dir_size = atoll(row[0]); // ���ݿ��нڵ���ļ��д�С
			database_file_num = atoi(row[1]); // ���ݿ��нڵ���ļ�����
		}
		else {
			fprintf(log_file, "δ�ҵ�·��Ϊ��%s ������\n", dir_path);
		}
		mysql_free_result(result); // �ͷŽ����
		char earliesttime_str[40];
		char latesttime_str[40];
		PrintFileTime(earliest, earliesttime_str); // ���ļ�ʱ��ת��Ϊ�ַ�����ʽ
		PrintFileTime(latest, latesttime_str); // ���ļ�ʱ��ת��Ϊ�ַ�����ʽ
		fprintf(log_file, "\n��·����%s �У��ļ���С���죺%lld B���ļ��������죺%d�������ļ�����ʱ�䣺%s��������ʱ�䣺%s\n",
			dir_path, database_dir_size - tree_dir_size,
			database_file_num - tree_file_num, earliesttime_str, latesttime_str);

	}
	fclose(log_file); // �ر���־�ļ�
	fclose(file); // �ر��ļ�
}

/*
 * ����: FreeRoot
 * -------------
 * �ͷ��������нڵ㼰���ڴ档
 *
 * root: ���ĸ��ڵ㡣
 *
 * ����: void
 *
 * �˺���ʹ��������������㷨�����������ͷ�ÿ���ڵ���ڴ档
 */
void FreeRoot(TreeNode* root)
{
	if (root == NULL) return;

	Stack stack;
	StackInit(&stack, 64);
	StackPush(&stack, root);

	while (!StackIsEmpty(&stack)) {
		TreeNode* current = StackPop(&stack);

		// ����ǰ�ڵ�ĺ��Ӻ��ֵܽڵ�ѹ��ջ
		if (current->child != NULL) {
			StackPush(&stack, current->child);
		}
		if (current->next != NULL) {
			StackPush(&stack, current->next);
		}

		// �ͷŵ�ǰ�ڵ�
		free(current);
	}

	StackFree(&stack);
}


/*
 * ����: DropDatabase
 * -----------------
 * ɾ�����ݿ⣨������ڣ���
 *
 * conn: MySQL ���Ӿ����
 *
 * ����: void
 *
 * �˺�������ִ�� SQL �����ɾ��ָ�������ݿ⡣
 */
void DropDatabase(MYSQL* conn)
{
	char pre[MAX_SQL_SIZE];
	sprintf_s(pre, MAX_SQL_SIZE, "DROP DATABASE IF EXISTS file_scan;");
	// ִ�� SQL �����ɾ�����ݿ�
	if (mysql_query(conn, pre))
	{
		fprintf(stderr, "@@Failed to use database: %s\n", mysql_error(conn));
		// ����Ĵ�����Ϣ�����������׼�����豸��ͨ������Ļ�����̨���������Ǳ�׼����豸
		exit(1); // ���߸���Ӧ�ó����Ҫ�������
	}
}


