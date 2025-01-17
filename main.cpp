#include <iostream>
#include <string>
#include <vector>
#include <set>
#include <map>
#include <iterator>
#include <algorithm>
#include <cctype>

using namespace std;

char toLower(char ch) { return tolower(ch); }

string &ToLower(string &st);

void display(const string &s);

int main() {
    vector<string> words;
    cout << "Enter words (enter quit to quit) : \n";
    string input;
    while (cin >> input && input != "quit")
        words.push_back(input);
    cout << "You entered the following words:\n";
    for_each(words.begin(), words.end(), display);
    cout << endl;

    // place words in set, converting to lowercase
    set<string> word_set;
    transform(words.begin(), words.end(),
              insert_iterator<set<string>>(word_set, word_set.begin()),
              ToLower);
    cout << "\n Alphabetic list of words: \n";
    for_each(word_set.begin(), word_set.end(), display);
    cout << endl;

    // place word and frequency in map
    map<string, int> word_map;
    for (auto si = word_set.begin(); si != word_set.end(); ++si)
        word_map[*si] = count(words.begin(),words.end(),*si);

    //display map contents
    cout << "\nWord frequency :\n";
    for (auto si = word_set.begin(); si != word_set.end(); ++si)
        cout << *si << ": " << word_map[*si] << endl;

    return 0;
}

string & ToLower (string & st){
    transform(st.begin(),st.end(),st.begin(), toLower);
    return st;
}

void display(const string &s){
    cout << s << " ";
}

// 简答的抄完了，但是不理解