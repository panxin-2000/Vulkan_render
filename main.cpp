#include <iostream>
#include <Eigen/Dense>

using Eigen::MatrixXd;

int main() {
//如果只是想单纯的进行矩阵计算，我觉得还是octave是最方便的。因为对于octave的输入和输出我还是比较熟悉的，主要看对什么熟悉。
//然后就是我知道了关于图形学，其实内容是很多很多的。不止有我看到的这些内容。

//    glm::vec4 vec(1.0f, 0.0f, 0.0f, 1.0f);
//    std::cout << glm::to_string(vec) << std::endl;
//    glm::mat4 trans = glm::mat4(1.0f);
//    trans = glm::translate(trans, glm::vec3(1.0f, 1.0f, 0.0f));
//    vec = trans * vec;
    //正常应该是这个样子，但是打印处理是不太对。
//    2.000000 = | (1.000000, 0.000000, 0.000000, 1.000000) | * 1.000000
//    1.000000 = | (0.000000, 1.000000, 0.000000, 1.000000) | * 0.000000
//    0.000000 = | (0.000000, 0.000000, 1.000000, 0.000000) | * 0.000000
//    1.000000 = | (0.000000, 0.000000, 0.000000, 1.000000) | * 1.000000
    Eigen::Vector4d v(1, 0, 0,1);
    std::cout << v << std::endl;


    MatrixXd m(4, 4);

    m(0, 0) = 1;
    m(1, 1) = 1;
    m(2, 2) = 1;
    m(1, 3) = 1;
    m(0, 3) = 1;

    std::cout << m << std::endl;
    std::cout << m * v << std::endl;
}