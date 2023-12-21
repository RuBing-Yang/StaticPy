#include <iostream>
#include <fstream>
#include <string.h>
#include <string>
#include <stdio.h>
#include <malloc.h>
#include <map>
#include <vector>

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

void testSTL(vector<int> v) {
    v.push_back(3);
}

int main() {
    TOKEN *p = new Token("s", "type");
    printf("%s %s\n", p->s.c_str(), p->type.c_str());
    map<int, int> minitlist {
        {1, 10},
        {2, 20},
        {3, 30},
        {4, 40},
    };
    for (auto it = minitlist.begin(); it != minitlist.end(); it++) {
        printf("map[%d] = %d\n", it->first, it->second);
    }
    vector<int> vinitlist {1, 2, 3, 4, 5};
    vector<vector<int>> vvinitlist {{1, 2}, {3, 4, 5}};

    bool b = 3;
    cout << "bool b = 3: " << b << endl;
    if (b) cout << "b is true" << endl;
    else cout << "b is false" << endl;
    int i = b + 1;
    cout << "bool b + 1: " << i << endl;
    float f = b;
    cout << "float f = b: " << f << endl;

    vector<int> v = {1, 2};
    testSTL(v);
    cout << "v.size(): " << v.size() << endl;
}
