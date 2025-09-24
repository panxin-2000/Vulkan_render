//
// Created by 潘鑫 on 2025/9/23.
//
#include  <gtest/gtest.h>


struct bracket_symbol_position {
    int index;
    int position_in_string;
    int length;
};


struct tree_node_bracket {
    struct tree_node_bracket *parent;
    struct tree_node_bracket *left;
    struct tree_node_bracket *right;
    int start_position;
    int length;
};
const char bracket_symbol[] = {
    {'{'},
    {'}'},
    {'['},
    {']'},
    {'('},
    {')'},
};

int get_if_bracket_symbol(char input_char) {
    for (int i = 0; i < sizeof(bracket_symbol) / sizeof(char); i++) {
        if (bracket_symbol[i] == input_char) {
            return i;
        }
    }
    return -1;
}

TEST(bracket, string) {
    char *calculate_string = {
        "(80+(30+50)/20)-1*2+3*4"
    };
    int str_length = strlen(calculate_string);
    std::vector<bracket_symbol_position> bracket_symbol_positions = {};
    for (int i = 0; i < str_length; i++) {
        int reslut = get_if_bracket_symbol(calculate_string[i]);
        if (reslut >= 0) {
            bracket_symbol_position auto_para = {};
            auto_para.index = reslut;
            auto_para.position_in_string = i;
            bracket_symbol_positions.push_back(auto_para);
        }
    }
    // 有一个操作是找到上一个为空的，这个操作比较重要
    // 先不管那些内容，先说这个操作应该分为几步？
    // 第一步只管符号的识别，以及确定位置
    // 第二步，已经拿到这个vector了，那么就可以开始考虑匹配括号的事情了
    // 匹配的括号需要添加长度这个操作
    bracket_symbol_position *last_one = nullptr;
    std::stack<bracket_symbol_position *> stack;

    for (int i = 0; i < bracket_symbol_positions.size(); i++) {
        // 这里是需要实现一个栈的，每次进入的时候将指针压栈，然后比较新加入的和上一个是否相同，
        // 相同就把这两个同时出栈
        stack.push(&bracket_symbol_positions[i]);
        bracket_symbol_position *new_one = stack.top();
        if (last_one != nullptr && new_one != nullptr) {
            if (last_one->index % 2 == 0 && (new_one->index - 1) == last_one->index) {
                // 括号匹配的情况
                last_one->length = new_one->position_in_string - last_one->position_in_string;
                new_one->length = last_one->position_in_string - new_one->position_in_string;
                stack.pop();
                stack.pop();
                if (!stack.empty())
                    new_one = stack.top();
            }
        }
        last_one = new_one;
    }
    EXPECT_EQ(0, stack.size());
    last_one = nullptr;

    // 匹配完长度呢？之后需要做什么呢？
    // 创建称一个向量，可以包含字符串的向量

}
