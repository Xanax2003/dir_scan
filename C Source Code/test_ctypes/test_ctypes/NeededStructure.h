#ifndef NEEDEDSTRUCTURE_H
#define NEEDEDSTRUCTURE_H

#define MAX_SQL_SIZE 1024
#define BATCH_SIZE 2000     //�����������ɲ���������ֵ+



typedef struct TreeNode {
	TCHAR path[MAX_PATH];
	FILETIME CreationTime;
	uint64_t FileSize;
	struct TreeNode* next;
	struct TreeNode* child;
	struct TreeNode* father;
	bool isdir;
	int depth; //��������Ǵ�ͳ�����ϵ����
	int fileNum;
	int length;
}TreeNode;


typedef struct {
	int totalDirectories;
	int totalFiles;
	int maxDepth;
	TCHAR longestFilePath[MAX_PATH];      //MAX_PATH��windows���Ѿ�����ĺ꣬Ϊ260���ַ�
	size_t longestFilePathLength;
	int maxLenth;
} DirectoryStats;

typedef struct {
	TreeNode** items;
	int size;
	int capacity;
}Stack;


#endif // NEEDEDSTRUCTURE_H