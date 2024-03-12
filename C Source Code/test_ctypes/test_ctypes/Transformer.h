#ifndef TRANSFORMER_H
#define TRANSFORMER_H


const TCHAR* GetFileNameFromPath(const TCHAR* path);

void extractPath(TCHAR* line, TCHAR* path);

void UnixTimeToFileTime(time_t unixTime, FILETIME* fileTime);

void ExtractInfo(TCHAR* line, TCHAR* path, TCHAR* flag, FILETIME* creationtime, uint64_t* file_size);

void SplitPath(const TCHAR* fullPath, TCHAR* directory, TCHAR* fileName);

void CharToTChar(const char* src, TCHAR* dest, size_t destSize);

ULONGLONG FileTimetoUlonglong(FILETIME time);

void TCharToChar(const TCHAR* src, char* dest, size_t destSize);

void EscapeSQLString(char* dest, const char* src, size_t destSize);

void extractValues(const char* line, char* output);

void DoubleEscapeSQLString(char* dest, const char* src, size_t destSize);

void FindExtretimeFromTree(TreeNode* node, FILETIME* earliest, FILETIME* latest);

#endif // TRANSFORMER_H