#include <iostream>
#include <string>
#include <vector>
#include <set>
#include <map>
#include <iterator>
#include <algorithm>
#include <cctype>

using namespace std;

int main() {
    const int N = 6;
    string s1[N] = {"buffoon", "thinkers", "for", "heavy", "can", "for"};
    string s2[N] = {"metal", "any", "food", "elegant", "deliver", "for"};

    set<string> A{s1, s1 + N};
    set<string> B{s2, s2 + N};

    ostream_iterator<string, char> out(cout, " ");
    cout << "Set A : ";
    copy(A.begin(), A.end(), out);
    cout << endl;
    cout << "Set B : ";
    copy(B.begin(), B.end(), out);
    cout << endl;

    cout << "Union of A and B:\n";
    set_union(A.begin(), A.end(), B.begin(), B.end(), out);
    cout << endl;

    cout << "Intersection of A and B: \n";
    set_intersection(A.begin(), A.end(), B.begin(), B.end(), out);
    cout << endl;

    cout << "Difference of A and B :\n";
    set_difference(A.begin(), A.end(), B.begin(), B.end(), out);
    cout << endl;

    cout << "Difference of B and A :\n";
    set_difference(B.begin(), B.end(), A.begin(), A.end(), out);
    cout << endl;

    set<string> C;
    cout << "Set c:\n";
    set_union(A.begin(), A.end(), B.begin(), B.end(),
              insert_iterator<set<string>>(C, C.begin()));
    copy(C.begin(), C.end(), out);
    cout << endl;

    string s3{"grungy"};
    C.insert(s3);
    cout << "Set C after insertion:\n";
    copy(C.begin(), C.end(), out);
    cout << endl;

    cout << "showing a range :\n";
    copy(C.lower_bound("ghost"), C.upper_bound("spook"), out);
    cout << endl;
    pair
    return 0;
}
// 这里的主要的问题是使用的less<>
// template<_Key, _Compare = less<_Key> >
// 省略了第二个模版参数，只输入了第一个key,应该是可以理解为键的意思
// 对于最简单的关联容器 set ，其 值和键相同，键是唯一的。
// map 中值和键的类型不同，其实是可以相同的，只是一般情况下不同。 键唯一。
// map 可以用来加速搜索吗？