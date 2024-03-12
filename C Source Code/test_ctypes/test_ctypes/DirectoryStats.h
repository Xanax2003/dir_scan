#ifndef DIRECTORYSTATS_H
#define DIRECTORYSTATS_H




void Initialize(DirectoryStats* stats);

void PrintStats(const DirectoryStats* stats);

void GenerateBatchInsertFile(TreeNode* root);

void CompareDifference(const char* path, MYSQL* conn, TreeNode* root);

void TraverseGenerate(TreeNode* node, int* filecount, int* dircount);

void NewTraverseGenerate(TreeNode* root);


#endif // !DIRECTORYSTATS_H