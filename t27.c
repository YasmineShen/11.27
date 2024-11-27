#include "t27.h"

static int getIndex_and_invalidChar(char c)
{
    //check the valid character: 26 letters, plus the '
    if (c == '\''){
        return ALPHA - 1 ;
    }
    if (c >= 'a' && c <= 'z'){
        return c - 'a';
    }
    if (c >= 'A' && c <= 'Z'){
        return c - 'A';
    }

    //invalid character
    return -1; 
}

dict* dict_init(void)
{
    dict* node = (dict*)malloc(sizeof(dict));
    //check whether memory allocation is successful
    if (node == NULL){
        return NULL;
    }

    //initialize all child nodes to be empty
    for (int i = 0; i < ALPHA; i++) {
        node->dwn[i] = NULL;
    }

    node->up = NULL;
    node->terminal = false;
    node->freq = 0;
    return node;
}

bool dict_addword(dict* p, const char* wd)
{
    if (p == NULL || wd == NULL){
        return false;
    }

    dict* current = p;
    char ch;
    int index;

    // go through every character of the word
    for (const char* c = wd; *c != '\0'; c++) {
        ch = *c;
        //convert letters to lowercase letters
        if (ch >= 'A' && ch <= 'Z') {
            ch = ch - 'A' + 'a';
        }

        index = getIndex_and_invalidChar(ch);
        //check invalid character
        if (index == -1) {
            return false;
        } 

        //if the child node does not exist
        if (current->dwn[index] == NULL) {
            current->dwn[index] = dict_init();
            if (current->dwn[index] == NULL) {
                return false;
            }
            current->dwn[index]->up = current;
        }
        current = current->dwn[index];
    }

    //if the word already exists
    if (current->terminal) {
        current->freq++;
        return false; 
    } 
    //add new word
    else {
        current->terminal = true;
        current->freq = 1;
        return true;
    }
}



int dict_nodecount(const dict* p) {
    if (p == NULL) {
        return 0;
    }

    //current node is 1
    int count = 1;
    
    //count all child nodes
    for (int i = 0; i < ALPHA; i++) {
        count += dict_nodecount(p->dwn[i]);
    }
    return count;
}

int dict_wordcount(const dict* p) {
     if (p == NULL) {
        return 0;
    }
    
    //count the end points into the word frequency
    int count = 0;
    if (p->terminal) {
        count = p->freq;
    }

    for (int i = 0; i < ALPHA; i++) {
        count += dict_wordcount(p->dwn[i]);
    }
    return count;
}

dict* dict_spell(const dict* p, const char* str) {
    if (p == NULL|| str == NULL) {
        return NULL;
    }

    const dict* current = p;
    for (const char* c = str; *c != '\0'; c++) {
        int index = getIndex_and_invalidChar(*c);
        if (index == -1 || current->dwn[index] == NULL) {
            return NULL;
        }
        current = current->dwn[index];
    }

    if (current->terminal) {
        return (dict*)current;
    }
    else {
        return NULL;
    }
}

void dict_free(dict** p) {
    if (p == NULL || *p == NULL) {
    return;
    }

    for (int i = 0; i < ALPHA; i++) {
        dict_free(&((*p)->dwn[i]));
    }
    free(*p);
    *p = NULL;
}

int dict_mostcommon(const dict* p) {
    if (p == NULL) {
        return 0;
    }

    int highest_freq = 0;
    if (p->terminal) {
        highest_freq = p->freq;
    }

    //get the term frequency of the current node
    for (int i = 0; i < ALPHA; i++) {
        int most_child = dict_mostcommon(p->dwn[i]);
        if (most_child > highest_freq) {
            highest_freq = most_child;
        }
    }
    return highest_freq;
}

unsigned dict_cmp(dict* p1, dict* p2) {
    if (p1 == NULL || p2 == NULL) {
        return 0;
    }

    //find the depth of both nodes
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

    // bring both nodes to the same depth
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

    //find the same parent
    while (p1 != p2) {
        p1 = p1->up;
        p2 = p2->up;
        distance += 2; // One step for each node
    }

    return distance;
}

//perform depth-first search for autocomplete
void dfs_Method(const dict* node, char* path, int depth, char* ret, int* highest_freqq) {
    if (node == NULL) {
        return;
    }

    //if the current node is a terminal, consider as a candidate for autocompletion
    if (node->terminal) {
        path[depth] = '\0';

        //check whether results need to be updated
        if (*highest_freqq == -1) {
            *highest_freqq = node->freq;
            strcpy(ret, path);
        } 
        else if (node->freq > *highest_freqq) {
            *highest_freqq = node->freq;
            strcpy(ret, path);
        } 
        else if (node->freq == *highest_freqq) {
            if (ret[0] == '\0' || strcmp(path, ret) < 0) {
                strcpy(ret, path);
            }
        }
    }

    //recursively iterate over all child nodes
    for (int i = 0; i < ALPHA; i++) {
        if (node->dwn[i]) {
            if (i == ALPHA - 1) {
                path[depth] = '\'';
            } 
            else {
                path[depth] = 'a' + i;
            }
            dfs_Method(node->dwn[i], path, depth + 1, ret, highest_freqq);
        }
    }
}


void dict_autocomplete(const dict* p, const char* wd, char* ret) {
    if (p == NULL|| wd == NULL|| ret == NULL) {
        ret[0] = '\0';
        return;
    }

    const dict* current = p;

    //iterate to the node corresponding to the given prefix
    for (const char* c = wd; *c != '\0'; c++) {
        int index = getIndex_and_invalidChar(*c);
        if (index == -1 || !current->dwn[index]) {
            ret[0] = '\0';
            return;
        }
        current = current->dwn[index];
    }

    //starting from the child nodes of the current node depth first search
    char tempArray[100];
    int highest_freq = -1;
    ret[0] = '\0';

    for (int i = 0; i < ALPHA; i++) {
        if (current->dwn[i]) {
            if (i == ALPHA - 1) {
                tempArray[0] = '\'';
            } 
            else {
                tempArray[0] = 'a' + i;
            }
            dfs_Method(current->dwn[i], tempArray, 1, ret, &highest_freq); // 深度优先搜索
        }
    }

    //if no more frequent word is found, an empty string is returned
    if (highest_freq == -1) {
        ret[0] = '\0';
    }
}


void test(void) 
{
    dict* d = NULL;
    char str[100];

    // TEST 1：Initialize the dictionary and check for success
    d = dict_init();
    assert(d != NULL);
    //Only the root node
    assert(dict_nodecount(d) == 1);
    assert(dict_wordcount(d) == 0);


    // TEST 2：Add the word "computer" and check if it exists
    assert(dict_addword(d, "computer") == true);
    assert(dict_spell(d, "computer") != NULL);
    //'c','o','m','p','u','t','e','r' has 8 nodes, plus the root node
    assert(dict_nodecount(d) == 9);
    assert(dict_wordcount(d) == 1);


    // TEST 3：Add the same word "computer" again, checking the frequency and return value
    assert(dict_addword(d, "computer") == false);
    assert(dict_mostcommon(d) == 2);
    //'computer' was added twice
    assert(dict_wordcount(d) == 2); 


    // TEST 4：Add the word "compute" that shares the prefix with "computer"
    assert(dict_addword(d, "compute") == true);
    assert(dict_spell(d, "compute") != NULL);
    //No new node
    assert(dict_nodecount(d) == 9); 
    //A total of 3 words were added
    assert(dict_wordcount(d) == 3);


    // TEST 5：Test case sensitivity and apostrophe handling
    assert(dict_addword(d, "CompuTer's") == true);
    assert(dict_spell(d, "computer's") != NULL);
    assert(dict_spell(d, "CompuTer's") != NULL);
    //Added "'", 's' node
    assert(dict_nodecount(d) == 11);
    assert(dict_wordcount(d) == 4);


    // TEST 6：Test the auto-complete function
    dict_autocomplete(d, "comp", str);
    //'computer' is the most common word beginning with 'comp'
    assert(strcmp(str, "uter") == 0);

    dict_autocomplete(d, "comput", str);
    //'computer' has a higher frequency
    assert(strcmp(str, "er") == 0);

    dict_autocomplete(d, "computer'", str);
    //Complete 'computer' to 's'
    assert(strcmp(str, "s") == 0); 


    // TEST 7：Test the word comparison function dict_cmp
    dict* p1 = dict_spell(d, "computer");
    dict* p2 = dict_spell(d, "compute");
    //'computer' and 'compute' differ only in the last character
    assert(dict_cmp(p1, p2) == 1); 


    // TEST 8：Test adding words that contain illegal characters
    assert(dict_addword(d, "compu123") == false); 
    assert(dict_spell(d, "compu123") == NULL);


    // TEST 9：Release the dictionary and check for success
    dict_free(&d);
    assert(d == NULL);
    assert(dict_nodecount(d) == 0);
    assert(dict_wordcount(d) == 0);
}

