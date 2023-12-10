// compiler.h: 标准系统包含文件的包含文件
// 或项目特定的包含文件。

#pragma once

#include <iostream>
#include <fstream>
#include <string.h>
#include <string>
#include<stdio.h>
#include<malloc.h>

using namespace std;

typedef struct Token
{
    Token(string _s, string _type) {
        s = _s;
        type = _type;
        next = nullptr;
    }
    string s;
    string type;
    struct Token* next;
}TOKEN;

typedef struct ASTNode
{
    ASTNode(string _s, string _type) {
        s = _s;
        type = _type;
        next = nullptr;
        first_child = nullptr;
        last_child = nullptr;
    }
    string s;
    string type;
    struct ASTNode* next;
    struct ASTNode* first_child;
    struct ASTNode* last_child;
}ASTNODE;

extern void lexAnalysis(ifstream *infile, TOKEN **token, ofstream *outfile=nullptr);

extern void grammarAnalysis(TOKEN **token, string type, ASTNODE *root, ofstream *outfile=nullptr);

extern void genCppCode(ASTNODE *root, string type, ofstream *outfile=nullptr);
