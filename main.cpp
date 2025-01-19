#include <iostream>
#include <string>
#include <map>
#include <algorithm>

typedef int KeyType;
typedef std::pair<const KeyType, std::string> Pair;
typedef std::multimap<KeyType, std::string> MapCode;

int main() {
    using namespace std;
    MapCode codes;

    codes.insert(Pair(415, "San francisco"));
    codes.insert(Pair(510, "Oakland"));
    codes.insert(Pair(718, "Brooklyn"));
    codes.insert(Pair(718, "shanghai"));
    // 更改为map后第二次插入是插入不进去的
    codes.insert(Pair(718, "staten Island"));
    codes.insert(Pair(415, "san rafael"));
    codes.insert(Pair(510, "Berkeley"));
    cout << " Number of cities with area code 415: "
         << codes.count(415) << endl;
    cout << " Number of cities with area code 718: "
         << codes.count(718) << endl;
    cout << " Number of cities with area code 510: "
         << codes.count(510) << endl;
    for (auto it = codes.begin(); it != codes.end(); ++it) {
        cout << "    " << (*it).first
             << "    " << (*it).second
             << endl;
    }
    pair<MapCode::iterator, MapCode::iterator> range
            = codes.equal_range(718);
    for (auto it = range.first; it != range.second; ++it) {
        cout << (*it).second << endl;
    }
    // 这里的first和second是什么意思？
    // first  对应于键
    // second 对应于值
    // map 中如何替换已经存在的键对应的值？

    int32_t tem = 12;
    char *tem_char = nullptr;
    cout << tem << endl;
    cout << &tem << endl;
    cout << tem_char << endl;
    cout << (void *) tem_char << endl;

    long ee = 31415926;
    cout.write((char *) &ee, sizeof(ee));
    return 0;
}