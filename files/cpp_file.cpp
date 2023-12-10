#include <iostream>
#include <map>
#include <vector>
using namespace std;

void func1()
{
    int i = 1 + 1 * 1 - 1;
    if (i == 1)
    {
        cout << "i is " << i << endl;
    }
}

vector<int> func2(vector<int> l)
{
    l[1] = 5;
    return l;
}

int main()
{
    func1();
    vector<int> l = {1, 2, 3};
    vector<int> l2 = func2(l);
    int i = 0;
    while (i < 3)
    {
        cout << "list [" << i << "] = " << l2[i] << endl;
        i = i + 1;
    }
    map<int, int> d = {{1, 11}, {2, 22}};
    i = 1;
    while (i < 10)
    {
        cout << "dict [" << i << "] = " << d[i] << endl;
        i = i + 1;
        if (i >= 3)
        {
            break;
        }
    }
    return 0;
}
