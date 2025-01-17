#include <iostream>
#include <list>
#include <algorithm>
#include <vector>

void Show(int);

const int LIMIT = 10;

int main() {
    using namespace std;

    string letters;
    cout << "Enter the letter grouping (quit to quit):";
    while (cin >> letters && letters != "quit") {
        cout << "Permutations of " << letters << endl;
        sort(letters.begin(), letters.end());
        cout << letters << endl;
        while (next_permutation(letters.begin(), letters.end())) {
            cout << letters << endl;
        }
        cout << "Enter next sequence (quit to quit) :";
    }
    cout << "Done .\\n";
    return 0;
}
//我今天才发现字符串原来是能够这么搞的，
//以前我一只以为的都是尽量以单词为一组，比如在一个句子中查找其中一个单词
//现在发现，居然能够对其中的每个字符都进行排序。