//
// Created by 潘鑫 on 2025/4/24.
//

#ifndef SAMPLE3_H
#define SAMPLE3_H

#include <stddef.h>

template<typename E>
class Queue; // 这一行是为了后面的友元做准备

template<typename E>
class QueueNode {
    friend class Queue<E>;

public:
    const E &element() const { return element_; }

    QueueNode *next() { return next_; }
    const QueueNode *next() const{ return next_; } // 这一行也是必须有的

private:
    //  explicit 的声明都有什么做用记不得了
    explicit QueueNode(const E &an_element) //这个我知道是构造函数
        : element_(an_element), next_(nullptr) {
    }


    // we disable the default assignment operator and copy c'tor
    // c'tor 是什么意思，但是下面的大概是移动构造函数和复制构造函数
    const QueueNode &operator=(const QueueNode &);

    QueueNode(const QueueNode &); // 这个是构造函数？

    E element_;
    QueueNode *next_;
};


template<typename E>
class Queue {
public:
    Queue(): head_(nullptr), tail_(nullptr), size_(0) {
    }

    ~Queue() { Clear(); }

    void Clear() {
        if (size_ > 0) {
            QueueNode<E> *node = head_;
            QueueNode<E>* next = node->next();
            // 原本的代码是上一行，如果是size_ > 0 是没有问题的，不会出现node == nullptr的情况
            // QueueNode<E> next;  // 我这里写的稍微有点重复了，但是不影响任何性能
            // if (node != nullptr)
            //     next = node->next();
            // for (; node != nullptr && next = node->next(); node = next) {  //需要在这一行加上后面的才是正确的 next = node->next()
            //     delete node;
            // }
            // 原本的循环代码是下面这个, 我上面的代码会触发一个问题，assignable 的问题，
            for (;;) {
                delete node;
                node = next;
                if (node == nullptr)break;
                next = node->next();
            }

            // 将私有成员变量全部清空
            head_ = nullptr;
            tail_ = nullptr;
            size_ = 0;
        }
    }


    size_t size() const { return size_; }


    // 我个人感觉下面的两行的内容应该是一样的，有必要重复吗？
    QueueNode<E>*Head() { return head_; }
    const QueueNode<E>* Head() const { return head_; }
    QueueNode<E>*Tail() { return tail_; }
    const QueueNode<E>* Tail() const { return tail_; }

    void Enqueue(const E &element) {
        QueueNode<E> *new_node = new QueueNode<E>(element);

        if (size_ == 0) {
            head_ = new_node;
            tail_ = new_node;
            size_ = 1;
        }else {
            tail_->next_ = new_node;
            tail_ = new_node;
            size_++;
        }
    }

    E *Dequeue() {
        if (size_ == 0) {
            return nullptr;
        }
        const QueueNode<E> * const old_head = head_;
        head_ = head_->next();
        size_--;
        if (size_ == 0) {
            tail_ = nullptr;
        }
        // 这里内容太多，我都不知道到底是在做什么了
        E *element = new E(old_head->element());
        delete old_head;
        return element;
    }

    template <typename F>
    Queue *Map(F function)const {
        Queue *new_queue = new Queue();
        for (QueueNode<E> *node = head_; node != nullptr;
            node = node->next()) {
            new_queue->Enqueue(function(node->element()));
        }
    }// 抄完了，还是不清楚这个函数在做什么？


private:
    QueueNode<E> *head_;
    QueueNode<E> *tail_;
    size_t size_;

    // we disallow copying a queue
    // 这里的注释的大概意思是禁止复制queue
    // 让我想到了两个构造函数，复制构造函数和移动构造函数
    Queue(const Queue &);

    const Queue &operator=(const Queue &);
};

#endif //SAMPLE3_H
