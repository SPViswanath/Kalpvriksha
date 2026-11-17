#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BLOCK_SIZE 512
#define NUM_BLOCKS 1024      
#define MAX_NAME   50
#define MAX_INPUT  2048

typedef struct FreeBlock {
    int index;
    struct FreeBlock *next;
    struct FreeBlock *prev;
} FreeBlock;

typedef struct FileNode {
    char name[MAX_NAME + 1];
    int isDirectory;
    struct FileNode *parent;
    struct FileNode *next;       
    struct FileNode *child;      
    int *blockPointers;          
    int blockCount;
    int contentSize;
} FileNode;

unsigned char virtualDisk[NUM_BLOCKS][BLOCK_SIZE];
FreeBlock *freeHead = NULL;
FreeBlock *freeTail = NULL;
int freeBlockCount = 0;

FileNode *root = NULL;
FileNode *cwd = NULL;

void appendFreeBlock(int index) {
    FreeBlock *block = (FreeBlock *)malloc(sizeof(FreeBlock));
    block->index = index;
    block->next = NULL;
    block->prev = freeTail;
    if (freeTail) freeTail->next = block;
    else freeHead = block;
    freeTail = block;
    freeBlockCount++;
}

int allocateBlock(void) {
    if (!freeHead) return -1;
    FreeBlock *block = freeHead;
    int index = block->index;
    freeHead = block->next;
    if (freeHead) freeHead->prev = NULL;
    else freeTail = NULL;
    free(block);
    freeBlockCount--;
    return index;
}

void initVirtualDisk(void) {
    memset(virtualDisk, 0, sizeof(virtualDisk));
    for (int i = 0; i < NUM_BLOCKS; i++) appendFreeBlock(i);
}

void freeAllFreeBlocks(void) {
    while (freeHead) {
        FreeBlock *block = freeHead;
        freeHead = freeHead->next;
        free(block);
    }
    freeTail = NULL;
    freeBlockCount = 0;
}

FileNode *createNode(const char *name, int isDirectory, FileNode *parent) {
    FileNode *node = (FileNode *)calloc(1, sizeof(FileNode));
    strncpy(node->name, name, MAX_NAME);
    node->isDirectory = isDirectory;
    node->parent = parent;
    return node;
}

FileNode *findChild(FileNode *dir, const char *name) {
    if (!dir->child) return NULL;
    FileNode *cur = dir->child;
    do {
        if (strcmp(cur->name, name) == 0) return cur;
        cur = cur->next;
    } while (cur != dir->child);
    return NULL;
}

void insertChild(FileNode *dir, FileNode *node) {
    if (!dir->child) {
        node->next = node;               
        dir->child = node;
    } else {
        FileNode *last = dir->child;
        while (last->next != dir->child) last = last->next;
        last->next = node;               
        node->next = dir->child;
    }
}

void removeChild(FileNode *dir, FileNode *node) {
    if (node->next == node) {
        dir->child = NULL;              
    } else {
        FileNode *prev = dir->child;
        while (prev->next != node) prev = prev->next;
        prev->next = node->next;        
        if (dir->child == node) dir->child = node->next;
    }
    node->next = NULL;
}

void releaseFileBlocks(FileNode *file) {
    for (int i = 0; i < file->blockCount; i++)
        appendFreeBlock(file->blockPointers[i]);  
    free(file->blockPointers);
    file->blockPointers = NULL;
    file->blockCount = 0;
    file->contentSize = 0;
}

void freeTree(FileNode *node) {
    if (node->isDirectory) {
        while (node->child) {
            FileNode *c = node->child;
            removeChild(node, c);
            freeTree(c);
        }
    } else {
        free(node->blockPointers);
    }
    free(node);
}


int validName(const char *name) {
    if (strlen(name) == 0 || strlen(name) > MAX_NAME) {
        printf("Error: name must be 1-%d characters.\n", MAX_NAME);
        return 0;
    }
    if (findChild(cwd, name)) {
        printf("Error: '%s' already exists.\n", name);
        return 0;
    }
    return 1;
}

void cmdMkdir(const char *name) {
    if (!validName(name)) return;
    insertChild(cwd, createNode(name, 1, cwd));
    printf("Directory '%s' created successfully.\n", name);
}

void cmdCreate(const char *name) {
    if (!validName(name)) return;
    insertChild(cwd, createNode(name, 0, cwd));
    printf("File '%s' created successfully.\n", name);
}

void cmdWrite(const char *name, const char *content) {
    FileNode *file = findChild(cwd, name);
    if (!file || file->isDirectory) {
        printf("Error: file '%s' not found.\n", name);
        return;
    }
    int size = (int)strlen(content);
    int needed = (size + BLOCK_SIZE - 1) / BLOCK_SIZE;

    if (needed > freeBlockCount + file->blockCount) {
        printf("Error: disk full. Not enough free blocks.\n");
        return;
    }

    releaseFileBlocks(file);            

    if (needed > 0) file->blockPointers = (int *)malloc(needed * sizeof(int));
    for (int i = 0; i < needed; i++) {
        int index = allocateBlock();
        file->blockPointers[i] = index;
        int offset = i * BLOCK_SIZE;
        int chunk = size - offset < BLOCK_SIZE ? size - offset : BLOCK_SIZE;
        memset(virtualDisk[index], 0, BLOCK_SIZE);
        memcpy(virtualDisk[index], content + offset, chunk);
    }
    file->blockCount = needed;
    file->contentSize = size;
    printf("Data written successfully (size=%d bytes).\n", size);
}

void cmdRead(const char *name) {
    FileNode *file = findChild(cwd, name);
    if (!file || file->isDirectory) {
        printf("Error: file '%s' not found.\n", name);
        return;
    }
    int remaining = file->contentSize;
    for (int i = 0; i < file->blockCount; i++) {
        int chunk = remaining < BLOCK_SIZE ? remaining : BLOCK_SIZE;
        fwrite(virtualDisk[file->blockPointers[i]], 1, chunk, stdout);
        remaining -= chunk;
    }
    printf("\n");
}

void cmdDelete(const char *name) {
    FileNode *file = findChild(cwd, name);
    if (!file || file->isDirectory) {
        printf("Error: file '%s' not found.\n", name);
        return;
    }
    removeChild(cwd, file);
    releaseFileBlocks(file);
    free(file);
    printf("File deleted successfully.\n");
}

void cmdRmdir(const char *name) {
    FileNode *dir = findChild(cwd, name);
    if (!dir || !dir->isDirectory) {
        printf("Error: directory '%s' not found.\n", name);
        return;
    }
    if (dir->child) {
        printf("Error: directory '%s' is not empty.\n", name);
        return;
    }
    removeChild(cwd, dir);
    free(dir);
    printf("Directory removed successfully.\n");
}

void cmdLs(void) {
    if (!cwd->child) {
        printf("(empty)\n");
        return;
    }
    FileNode *cur = cwd->child;
    do {
        printf("%s%s\n", cur->name, cur->isDirectory ? "/" : "");
        cur = cur->next;
    } while (cur != cwd->child);
}

void buildPath(FileNode *node, char *path) {
    if (node == root) return;
    buildPath(node->parent, path);
    strcat(path, "/");
    strcat(path, node->name);
}

void getPath(char *path) {
    path[0] = '\0';
    if (cwd == root) strcpy(path, "/");
    else buildPath(cwd, path);
}

void cmdCd(const char *name) {
    if (strcmp(name, "..") == 0) {
        if (cwd != root) cwd = cwd->parent;
    } else if (strcmp(name, "/") == 0) {
        cwd = root;
    } else {
        FileNode *dir = findChild(cwd, name);
        if (!dir || !dir->isDirectory) {
            printf("Error: directory '%s' not found.\n", name);
            return;
        }
        cwd = dir;
    }
    char path[1024];
    getPath(path);
    printf("Moved to %s\n", path);
}

void cmdPwd(void) {
    char path[1024];
    getPath(path);
    printf("%s\n", path);
}

void cmdDf(void) {
    int used = NUM_BLOCKS - freeBlockCount;
    printf("Total Blocks: %d\n", NUM_BLOCKS);
    printf("Used Blocks: %d\n", used);
    printf("Free Blocks: %d\n", freeBlockCount);
    printf("Disk Usage: %.2f%%\n", 100.0 * used / NUM_BLOCKS);
}

void parseWriteContent(const char *line, const char *name, char *content) {
    const char *p = line;
    while (*p == ' ' || *p == '\t') p++;      
    p += strlen("write");                     
    p = strstr(p, name);                      
    p += strlen(name);
    while (*p == ' ' || *p == '\t') p++;
    strcpy(content, p);
    int len = (int)strlen(content);
    if (len >= 2 && (content[0] == '"' || content[0] == '\'') &&
        content[len - 1] == content[0]) {
        memmove(content, content + 1, len - 2);   
        content[len - 2] = '\0';
    }
}

int main(void) {
    char line[MAX_INPUT], cmd[MAX_INPUT], arg[MAX_INPUT], content[MAX_INPUT];

    initVirtualDisk();
    root = createNode("/", 1, NULL);
    cwd = root;

    printf("Compact VFS - ready. Type 'exit' to quit.\n");

    while (1) {
        char path[1024];
        getPath(path);
        printf("%s > ", path);

        if (!fgets(line, sizeof(line), stdin)) break;
        line[strcspn(line, "\r\n")] = '\0';

        cmd[0] = arg[0] = '\0';
        int n = sscanf(line, "%s %s", cmd, arg);
        if (n < 1) continue;

        if (strcmp(cmd, "exit") == 0) {
            break;
        } else if (strcmp(cmd, "ls") == 0) {
            cmdLs();
        } else if (strcmp(cmd, "pwd") == 0) {
            cmdPwd();
        } else if (strcmp(cmd, "df") == 0) {
            cmdDf();
        } else if (n < 2) {
            printf("Error: missing argument for '%s'.\n", cmd);
        } else if (strcmp(cmd, "mkdir") == 0) {
            cmdMkdir(arg);
        } else if (strcmp(cmd, "create") == 0) {
            cmdCreate(arg);
        } else if (strcmp(cmd, "write") == 0) {
            parseWriteContent(line, arg, content);
            cmdWrite(arg, content);
        } else if (strcmp(cmd, "read") == 0) {
            cmdRead(arg);
        } else if (strcmp(cmd, "delete") == 0) {
            cmdDelete(arg);
        } else if (strcmp(cmd, "rmdir") == 0) {
            cmdRmdir(arg);
        } else if (strcmp(cmd, "cd") == 0) {
            cmdCd(arg);
        } else {
            printf("Error: unknown command '%s'.\n", cmd);
        }
    }

    freeTree(root);
    freeAllFreeBlocks();
    printf("Memory released. Exiting program...\n");
    return 0;
}