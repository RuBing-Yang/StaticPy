#include <iostream>
#include <fstream>
#include <string.h>
#include <string>
#include<stdio.h>
#include<malloc.h>
#include <map>

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
    std::map<int, int> minitlist {
        {1, 10},
        {2, 20},
        {3, 30},
        {4, 40},
    };
    for (auto it = minitlist.begin(); it != minitlist.end(); it++) {
        printf("map[%d] = %d\n", it->first, it->second);
    }
}
