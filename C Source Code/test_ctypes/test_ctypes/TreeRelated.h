#ifndef TREERELATED_H
#define TREERELATED_H


void NodeInitialize(TreeNode* node);

int CalculateDepth(TreeNode* root);

void InItilizeRoot(TreeNode* root, TCHAR* path);

int TraverseAndAdd(TreeNode* node);

void BuildDirectoryTree(TreeNode* root, TCHAR* path);

void FreeTree(TreeNode* node);

TreeNode* Find_Node(TreeNode* root, TCHAR* path, Stack* stack, TreeNode** prevSibling);

TreeNode* Find_Node_Only(TreeNode* root, TCHAR* path);

void NonRecursiveTraverseAndAdd(TreeNode* root);

void CasecadeDelete(TreeNode* node);

void ModifyTree_dir(TCHAR* filepath, MYSQL* conn, TreeNode* root);

BOOL DeteleFileFromTree(TCHAR* filepath, TreeNode* root);

BOOL AppandFileIntoTree(TCHAR* path, TreeNode* root, FILETIME creationtime, uint64_t file_size);

void ModifyTree_file(char* filepath, MYSQL* conn, TreeNode* root);

void FreeRoot(TreeNode* root);



#endif // TREERELATED_H