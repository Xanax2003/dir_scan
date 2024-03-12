#include <windows.h>
#include <stdio.h>
#include <tchar.h>

// 定义用于存储统计信息的结构体
typedef struct {
    int totalDirectories;
    int totalFiles;
    int maxDepth;
    TCHAR longestFilePath[MAX_PATH];      //MAX_PATH是windows中已经定义的宏，为260个字符
    size_t longestFilePathLength;
} DirectoryStats;

void TraverseDirectory(TCHAR* directoryPath, int currentDepth, DirectoryStats* stats) {
    WIN32_FIND_DATA findFileData;
    HANDLE hFind = INVALID_HANDLE_VALUE;         //HANDLE是句柄数据类型
    DWORD dwError;
    TCHAR subDirPath[MAX_PATH];

    // 更新目录深度
    if (currentDepth > stats->maxDepth) {
        stats->maxDepth = currentDepth;
    }

    // 构建用于查找的路径，包括通配符以匹配所有项目
    _stprintf_s(subDirPath, MAX_PATH, TEXT("%s\\*"), directoryPath);       //subDirPath表示内存中的一段

    hFind = FindFirstFile(subDirPath, &findFileData);
    //printf("%ws\n", findFileData.cFileName);

    if (hFind == INVALID_HANDLE_VALUE) {
        printf("FindFirstFile failed (%d)\n", GetLastError());
        return;
    }

    do {
        // 跳过当前目录和上级目录的标识符，这里是比较当前目录和上级目录是否与其一致
        if (_tcscmp(findFileData.cFileName, TEXT(".")) == 0 ||
            _tcscmp(findFileData.cFileName, TEXT("..")) == 0) {
            continue;
        }

        if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            // 发现了一个子目录
            stats->totalDirectories++;
            // 构建子目录的完整路径
            TCHAR nextDirPath[MAX_PATH];
            _stprintf_s(nextDirPath, MAX_PATH, TEXT("%s\\%s"), directoryPath, findFileData.cFileName);
            // 递归遍历子目录
            TraverseDirectory(nextDirPath, currentDepth + 1, stats);
        }
        else {
            // 发现了一个文件
            stats->totalFiles++;
            // 检查并更新最长文件路径
            TCHAR filePath[MAX_PATH];
            _stprintf_s(filePath, MAX_PATH, TEXT("%s\\%s"), directoryPath, findFileData.cFileName);
            size_t pathLength = _tcslen(filePath);
            if (pathLength > stats->longestFilePathLength) {
                _tcscpy_s(stats->longestFilePath, MAX_PATH, filePath);
                stats->longestFilePathLength = pathLength;
            }
        }
    } while (FindNextFile(hFind, &findFileData) != 0);

    dwError = GetLastError();
    if (dwError != ERROR_NO_MORE_FILES) {
        printf("FindNextFile failed (%d)\n", dwError);
    }

    FindClose(hFind);
}

void PrintStats(const DirectoryStats* stats) {
    printf("Total directories: %d\n", stats->totalDirectories);
    printf("Total files: %d\n", stats->totalFiles);
    printf("Max directory depth: %d\n", stats->maxDepth);
    printf("Longest file path: %ls\n", stats->longestFilePath);
    printf("Longest file path length: %zu\n", stats->longestFilePathLength);
}

int main() {
    DirectoryStats stats = { 0, 0, 0, TEXT(""), 0 };
    TCHAR directoryPath[MAX_PATH] = TEXT("C:\\Windows"); // 修改为目标目录路径

    TraverseDirectory(directoryPath, 1, &stats);
    PrintStats(&stats);

    return 0;
}