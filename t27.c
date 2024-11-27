#include "t27.h"

static int char_to_index(char c) {
    if (c == '\''){
    return ALPHA-1 ;
    } ; // Apostrophe
    if (c >= 'a' && c <= 'z') return c - 'a';
    if (c >= 'A' && c <= 'Z') return c - 'A';
    return -1; // Invalid character
}

dict* dict_init(void) {
    dict* node = (dict*)malloc(sizeof(dict));
    if (!node) return NULL;
    for (int i = 0; i < ALPHA; i++) {
        node->dwn[i] = NULL;
    }
    node->up = NULL;
    node->terminal = false;
    node->freq = 0;
    return node;
}

bool dict_addword(dict* p, const char* wd) {
    if (!p || !wd) return false;

    dict* current = p;
    char ch;
    int index;

    for (const char* c = wd; *c != '\0'; c++) {
        ch = *c;
        if (ch >= 'A' && ch <= 'Z') {
            ch = ch - 'A' + 'a'; // 转换为小写
        }

        index = char_to_index(ch);
        if (index == -1) return false;

        if (!current->dwn[index]) {
            current->dwn[index] = dict_init();
            if (!current->dwn[index]) return false;
            current->dwn[index]->up = current;
        }
        current = current->dwn[index];
    }

    if (current->terminal) {
        current->freq++;
        return false; // Word already exists
    } else {
        current->terminal = true;
        current->freq = 1;
        return true; // New word added
    }
}



int dict_nodecount(const dict* p) {
    if (!p) return 0;
    int count = 1; // Count current node
    for (int i = 0; i < ALPHA; i++) {
        count += dict_nodecount(p->dwn[i]);
    }
    return count;
}

int dict_wordcount(const dict* p) {
    if (!p) return 0;
    int count = p->terminal ? p->freq : 0;
    for (int i = 0; i < ALPHA; i++) {
        count += dict_wordcount(p->dwn[i]);
    }
    return count;
}

dict* dict_spell(const dict* p, const char* str) {
    if (!p || !str) return NULL;

    const dict* current = p;
    for (const char* c = str; *c != '\0'; c++) {
        int index = char_to_index(*c);
        if (index == -1 || !current->dwn[index]) return NULL;
        current = current->dwn[index];
    }

    return current->terminal ? (dict*)current : NULL;
}

void dict_free(dict** p) {
    if (!p || !*p) return;

    for (int i = 0; i < ALPHA; i++) {
        dict_free(&((*p)->dwn[i]));
    }
    free(*p);
    *p = NULL;
}

int dict_mostcommon(const dict* p) {
    if (!p) return 0;

    int max_freq = p->terminal ? p->freq : 0;
    for (int i = 0; i < ALPHA; i++) {
        int child_max = dict_mostcommon(p->dwn[i]);
        if (child_max > max_freq) {
            max_freq = child_max;
        }
    }
    return max_freq;
}

unsigned dict_cmp(dict* p1, dict* p2) {
    if (!p1 || !p2) return 0;

    // Step 1: Find the depth of both nodes
    unsigned depth1 = 0, depth2 = 0;
    dict* temp1 = p1;
    dict* temp2 = p2;

    while (temp1) {
        depth1++;
        temp1 = temp1->up;
    }

    while (temp2) {
        depth2++;
        temp2 = temp2->up;
    }

    // Step 2: Bring both nodes to the same depth
    unsigned distance = 0;
    while (depth1 > depth2) {
        p1 = p1->up;
        depth1--;
        distance++;
    }

    while (depth2 > depth1) {
        p2 = p2->up;
        depth2--;
        distance++;
    }

    // Step 3: Find the common ancestor
    while (p1 != p2) {
        p1 = p1->up;
        p2 = p2->up;
        distance += 2; // One step for each node
    }

    return distance;
}

// Helper function to perform depth-first search for autocomplete
void dfs(const dict* node, char* path, int depth, char* ret, int* max_freq) {
    if (!node) return;

    // 如果当前节点是终结点，考虑作为自动补全的候选项
    if (node->terminal) {
        path[depth] = '\0';
        if (*max_freq == -1 || node->freq > *max_freq ||
            (node->freq == *max_freq && (ret[0] == '\0' || strcmp(path, ret) < 0))) {
            *max_freq = node->freq;
            strcpy(ret, path);
        }
    }

    // 递归遍历所有子节点
    for (int i = 0; i < ALPHA; i++) {
        if (node->dwn[i]) {
            path[depth] = (i == ALPHA - 1) ? '\'' : 'a' + i;
            dfs(node->dwn[i], path, depth + 1, ret, max_freq);
        }
    }
}


void dict_autocomplete(const dict* p, const char* wd, char* ret) {
    if (!p || !wd || !ret) {
        ret[0] = '\0';
        return;
    }

    const dict* current = p;

    // 遍历到给定前缀对应的节点
    for (const char* c = wd; *c != '\0'; c++) {
        int index = char_to_index(*c);
        if (index == -1 || !current->dwn[index]) {
            ret[0] = '\0';
            return;
        }
        current = current->dwn[index];
    }

    // 从当前节点的子节点开始深度优先搜索
    char buffer[100];
    int max_freq = -1;
    ret[0] = '\0';
    for (int i = 0; i < ALPHA; i++) {
        if (current->dwn[i]) {
            buffer[0] = (i == ALPHA - 1) ? '\'' : 'a' + i;
            dfs(current->dwn[i], buffer, 1, ret, &max_freq);
        }
    }

    // 如果没有找到更频繁的单词，返回空字符串
    if (max_freq == -1) {
        ret[0] = '\0';
    }
}
