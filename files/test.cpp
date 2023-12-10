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

int main() {
    TOKEN *p = new Token("s", "type");
    printf("%s %s\n", p->s.c_str(), p->type.c_str());
}
