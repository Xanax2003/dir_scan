#ifndef NEEDEDSTRUCTURE_H
#define NEEDEDSTRUCTURE_H

#define MAX_SQL_SIZE 1024
#define BATCH_SIZE 2000     //用于批量生成插入语句最大值+



typedef struct TreeNode {
	TCHAR path[MAX_PATH];
	FILETIME CreationTime;
	uint64_t FileSize;
	struct TreeNode* next;
	struct TreeNode* child;
	struct TreeNode* father;
	bool isdir;
	int depth; //这个并不是传统意义上的深度
	int fileNum;
	int length;
}TreeNode;


typedef struct {
	int totalDirectories;
	int totalFiles;
	int maxDepth;
	TCHAR longestFilePath[MAX_PATH];      //MAX_PATH是windows中已经定义的宏，为260个字符
	size_t longestFilePathLength;
	int maxLenth;
} DirectoryStats;

typedef struct {
	TreeNode** items;
	int size;
	int capacity;
}Stack;


#endif // NEEDEDSTRUCTURE_H