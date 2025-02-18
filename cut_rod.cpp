//
// Created by 潘鑫 on 2025/2/5.
//

/**
 * 从0开始还是从1开始确实是一个问题，我刚刚因为从零开始还是从1开始又把程序搞挂了
 * 现在还没有存其中的任何信息
 * @param price 举例 11，当出现11的时候就出现问题了，
 * @param n
 * @return
 */
int cut_rod(const int price[], int price_length, int n) {
    if (n == 0)
        return 0;
    int q = 0;
    for (int i = 1; i <= n; ++i) {
        int tem;
        if (i <= price_length)
            tem = cut_rod(price, price_length, n - i) + price[i];
        else
            tem = cut_rod(price, price_length, n % price_length - i) + price[price_length] * (i / price_length);
        q = q >= tem ? q : tem;
    }
    return q;
}


int memorized_cut_rod_aux(const int price[], int n, int r[]);

int memorized_cut_rod(const int price[], int n) {
    int r[n + 1];
    for (int i = 0; i < n + 1; ++i) {
        r[i] = -1;
    }
    return memorized_cut_rod_aux(price, n, r);
}

int memorized_cut_rod_aux(const int price[], int n, int r[]) {
    if (r[n] >= 0) {
        return r[n];
    }
    int q;
    if (n == 0) {
        q = 0;
    } else {
        q = -1;
        for (int i = 1; i <= n; ++i) {
            const int tem = memorized_cut_rod_aux(price, n - i, r) + price[i];
            q = q >= tem ? q : tem;
        }
    }
    r[n] = q;
    return q;
}