//
// Created by 潘鑫 on 2025/11/18.
//

#ifndef TREE_NODE_INDEX_H
#define TREE_NODE_INDEX_H

class v_index {
public:
    int16_t number;

    friend bool operator==(v_index left, v_index right) {
        return left.number == right.number;
    }

    friend bool operator!=(v_index left, v_index right) {
        return left.number != right.number;
    }
};
#endif //TREE_NODE_INDEX_H
