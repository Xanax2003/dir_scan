
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
 * ����: ScanOnly
 * -------------
 * ��ɨ��ָ��Ŀ¼��
 *
 * scan_dir: Ҫɨ���Ŀ¼·����
 *
 * ����: int
 * ɨ������Ľ����ͨ��Ϊ0��ʾ�ɹ���
 *
 * �˺���ɨ��ָ��Ŀ¼������Ŀ¼����Ȼ���ͷ������ڴ档
 */
__declspec(dllexport) int ScanOnly(const char* scan_dir)
{
	// �����ڴ��Դ洢Ŀ¼·��
	TCHAR* directoryPath = (TCHAR*)malloc(MAX_PATH * sizeof(TCHAR));
	// �������Ŀ¼·��ת��Ϊ TCHAR ����
	CharToTChar(scan_dir, directoryPath, MAX_PATH);

	// �����ڴ��Դ洢���ڵ�
	TreeNode* root = (TreeNode*)malloc(sizeof(TreeNode));
	// ����Ŀ¼��
	BuildDirectoryTree(root, directoryPath);

	// �ͷ�Ŀ¼�����ڴ�
	FreeRoot(root);
	// �ͷ�Ŀ¼·���ڴ�
	free(directoryPath);

	return 0; // ���سɹ�
}

/*
 * ����: ScanAndStats
 * -----------------
 * ɨ��ָ��Ŀ¼������ͳ�ơ�
 *
 * scan_dir: Ҫɨ���Ŀ¼·����
 * mystat: ����ͳ����Ϣ���ļ�·����
 *
 * ����: int
 * �����Ľ����ͨ��Ϊ0��ʾ�ɹ���
 *
 * �˺���ɨ��ָ��Ŀ¼������Ŀ¼������ӽڵ���Ϣ�����ӵ����ݿ⣬ɾ�����ݿ⣨������ڣ������������������ݲ������ݲ������ݿ⣬
 * ����ռ�Ŀ¼��Ϣ�������д�뵽���ݿ��С�
 */
__declspec(dllexport) int ScanAndStats(const char* scan_dir, const char* mystat)
{
	// �����ڴ��Դ洢Ŀ¼·��
	TCHAR* directoryPath = (TCHAR*)malloc(MAX_PATH * sizeof(TCHAR));
	// �������Ŀ¼·��ת��Ϊ TCHAR ����
	CharToTChar(scan_dir, directoryPath, MAX_PATH);

	// �����ڴ��Դ洢���ڵ�
	TreeNode* root = (TreeNode*)malloc(sizeof(TreeNode));
	// ����Ŀ¼��
	BuildDirectoryTree(root, directoryPath);
	// ��ӽڵ���Ϣ
	NonRecursiveTraverseAndAdd(root);

	// ���ӵ����ݿ�
	MYSQL* conn = ConnectDatabase();
	// ɾ�����ݿ⣨������ڣ�
	DropDatabase(conn);
	// ���������������ݲ������ݲ������ݿ�
	GenerateBatchInsertFile(root);
	// ʹ�����ݿ�
	UseDatabase(conn);
	// �������ݿ��
	CreateTables(conn);

	// ������������
	BatchInsert(conn);
	// �ռ�Ŀ¼��Ϣ�������д�����ݿ�
	CollectDirsInfo(mystat, conn, root);

	// �ͷ��ڴ�
	free(directoryPath);
	FreeRoot(root);

	return 0; // ���سɹ�
}


/*
 * ����: ScanAndStatsAndDelete
 * --------------------------
 * ɨ��ָ��Ŀ¼������ͳ�ƣ�Ȼ����ݸ�����ɾ���ļ��б����ɾ�����������Ƚϲ��졣
 *
 * scan_dir: Ҫɨ���Ŀ¼·����
 * mystat: ����ͳ����Ϣ���ļ�·����
 * mydir: ����ɾ���ļ��б���ļ�·����
 *
 * ����: int
 * �����Ľ����ͨ��Ϊ0��ʾ�ɹ���
 *
 * �˺���ɨ��ָ��Ŀ¼������Ŀ¼������ӽڵ���Ϣ�����ӵ����ݿ⣬ɾ�����ݿ⣨������ڣ������������������ݲ������ݲ������ݿ⣬
 * ����ռ�Ŀ¼��Ϣ�������д�뵽���ݿ��С�Ȼ����ݸ�����ɾ���ļ��б��ļ������ļ�ɾ�����������Ƚϲ��졣
 */
__declspec(dllexport) int ScanAndStatsAndDelete(const char* scan_dir, const char* mystat, const char* mydir)
{
	// �����ڴ��Դ洢Ŀ¼·��
	TCHAR* directoryPath = (TCHAR*)malloc(MAX_PATH * sizeof(TCHAR));
	// �������Ŀ¼·��ת��Ϊ TCHAR ����
	CharToTChar(scan_dir, directoryPath, MAX_PATH);

	// �����ڴ��Դ洢���ڵ�
	TreeNode* root = (TreeNode*)malloc(sizeof(TreeNode));
	// ����Ŀ¼��
	BuildDirectoryTree(root, directoryPath);
	// ��ӽڵ���Ϣ
	NonRecursiveTraverseAndAdd(root);

	// ���ӵ����ݿ�
	MYSQL* conn = ConnectDatabase();
	// ɾ�����ݿ⣨������ڣ�
	DropDatabase(conn);
	// ���������������ݲ������ݲ������ݿ�
	GenerateBatchInsertFile(root);
	// ʹ�����ݿ�
	UseDatabase(conn);
	// �������ݿ��
	CreateTables(conn);
	// ������������
	BatchInsert(conn);
	// �ռ�Ŀ¼��Ϣ�������д�����ݿ�
	CollectDirsInfo(mystat, conn, root);
	// ���ݸ�����ɾ���ļ��б��ļ������ļ�ɾ������
	ModifyTree_dir(mydir, conn, root);
	// �Ƚϲ���
	CompareDifference(mystat, conn, root);

	// �ͷ��ڴ�
	free(directoryPath);
	FreeRoot(root);

	return 0; // ���سɹ�
}


/*
 * ����: ScanAndStatsAndDeleteFile
 * -------------------------------
 * ɨ��ָ��Ŀ¼������ͳ�ƣ�Ȼ����ݸ�����ɾ���ļ��б���޸��ļ��б����ɾ�����޸Ĳ��������Ƚϲ��졣
 *
 * scan_dir: Ҫɨ���Ŀ¼·����
 * mystat: ����ͳ����Ϣ���ļ�·����
 * mydir: ����ɾ��Ŀ¼�б���ļ�·����
 * myfile: �����޸��ļ��б���ļ�·����
 *
 * ����: int
 * �����Ľ����ͨ��Ϊ0��ʾ�ɹ���
 *
 * �˺���ɨ��ָ��Ŀ¼������Ŀ¼������ӽڵ���Ϣ�����ӵ����ݿ⣬ɾ�����ݿ⣨������ڣ������������������ݲ������ݲ������ݿ⣬
 * ����ռ�Ŀ¼��Ϣ�������д�뵽���ݿ��С�Ȼ����ݸ�����ɾ��Ŀ¼�б��ļ�����Ŀ¼ɾ���������ٸ��ݸ������޸��ļ��б��ļ������ļ��޸Ĳ�����
 * ���Ƚϲ��졣
 */
__declspec(dllexport) int ScanAndStatsAndDeleteFile(const char* scan_dir, const char* mystat, const char* mydir, const char* myfile)
{
	// �����ڴ��Դ洢Ŀ¼·��
	TCHAR* directoryPath = (TCHAR*)malloc(MAX_PATH * sizeof(TCHAR));
	// �������Ŀ¼·��ת��Ϊ TCHAR ����
	CharToTChar(scan_dir, directoryPath, MAX_PATH);

	// �����ڴ��Դ洢���ڵ�
	TreeNode* root = (TreeNode*)malloc(sizeof(TreeNode));
	// ����Ŀ¼��
	BuildDirectoryTree(root, directoryPath);
	// ��ӽڵ���Ϣ
	NonRecursiveTraverseAndAdd(root);

	// ���ӵ����ݿ�
	MYSQL* conn = ConnectDatabase();
	// ɾ�����ݿ⣨������ڣ�
	DropDatabase(conn);
	// ���������������ݲ������ݲ������ݿ�
	GenerateBatchInsertFile(root);
	// ʹ�����ݿ�
	UseDatabase(conn);
	// �������ݿ��
	CreateTables(conn);
	// ������������
	BatchInsert(conn);
	// �ռ�Ŀ¼��Ϣ�������д�����ݿ�
	CollectDirsInfo(mystat, conn, root);
	// ���ݸ�����ɾ��Ŀ¼�б��ļ�����Ŀ¼ɾ������
	ModifyTree_dir(mydir, conn, root);
	// �Ƚϲ���
	CompareDifference(mystat, conn, root);
	// ���ݸ������޸��ļ��б��ļ������ļ��޸Ĳ���
	ModifyTree_file(myfile, conn, root);
	// �ٴαȽϲ���
	CompareDifference(mystat, conn, root);

	// �ͷ��ڴ�
	free(directoryPath);
	FreeRoot(root);

	return 0; // ���سɹ�
}

