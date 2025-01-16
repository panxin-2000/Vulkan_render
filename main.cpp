#include <iostream>
#include <list>
#include <algorithm>

void Show(int);

const int LIMIT = 10;

int main() {
    using namespace std;
    int ar[LIMIT] = {4, 5, 4, 2, 2, 3, 4, 8, 1, 4};
    list<int> la(ar, ar + LIMIT);
    list<int> lb(la);
    cout << "Original list contents: \n\t";
    for_each(la.begin(), la.end(), Show);
    cout << endl;
    la.remove(4);
    cout << "After using the remove() mothod:\n";
    cout << "la:\t";
    for_each(la.begin(), la.end(), Show);
    cout << endl;
    list<int>::iterator last;
    last = remove(lb.begin(), lb.end(), 4);
    cout << "after using the remove() function:\n";
    cout << "lb: \t";
    for_each(lb.begin(), lb.end(), Show);
    cout << endl;
    lb.erase(last, lb.end());
    cout << " After using the erase() mothod: \n";
    cout << "lb: \t";
    for_each(lb.begin(), lb.end(), Show);
    cout << endl;
    return 0;
}

void Show(int v) {
    std::cout << v << ' ';
}