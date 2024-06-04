#include <iostream>
#include <Eigen/Dense>

using Eigen::MatrixXd;

int main() {


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