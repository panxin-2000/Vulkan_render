//
// Created by 潘鑫 on 2026/10/11.
//

#include <gtest/gtest.h>
#include <STEPControl_Reader.hxx>
#include <TopoDS_Shape.hxx>
#include <Interface_Static.hxx>
#include <iostream>

#include <BRepMesh_IncrementalMesh.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
#include <BRep_Tool.hxx>
#include <Poly_Triangulation.hxx>
#include <gp_Pnt.hxx>
#include <vector>
#include <iostream>


TopoDS_Shape importStepModel(const Standard_CString &filePath) {
    // 1. 创建 STEP 读取器
    STEPControl_Reader reader;

    // 2. 读取文件
    IFSelect_ReturnStatus status = reader.ReadFile(filePath);
    if (status != IFSelect_RetDone) {
        std::cerr << "Error: Can't read file: " << filePath << std::endl;
        return {};
    }

    // 3. 检查并传输根节点
    Standard_Integer nRoots = reader.NbRootsForTransfer();
    std::cout << "Number of roots: " << nRoots << std::endl;
    reader.TransferRoots();

    // 4. 获取生成的 TopoDS_Shape 模型对象
    TopoDS_Shape shape = reader.OneShape();
    return shape;
}


// 定义一个简单的结构体用于存储渲染数据
struct RenderMesh {
    std::vector<float> vertices;       // 顶点坐标 [x1, y1, z1, x2, y2, z2, ...]
    std::vector<float> normals;        // 法线向量 [nx1, ny1, nz1, ...] （渲染光照必需）
    std::vector<unsigned int> indices; // 顶点索引 [i1, i2, i3, ...] (组成三角形)
};


RenderMesh convertShapeToRenderMesh(const TopoDS_Shape &shape) {
    RenderMesh meshData;
    unsigned int vertexOffset = 0; // 用于计算全局索引偏移

    // 1. 离散化（网格化）：将连续的数学曲面打碎成细小的三角面片
    // 参数 0.1 是线性偏差（Linear Deflection），值越小网格越精细，但计算越慢
    Standard_Real deflection = 0.1;
    BRepMesh_IncrementalMesh meshGen(shape, deflection);
    meshGen.Perform();

    // 2. 遍历 Shape 内部的所有面（Face），提取三角网格数据
    TopExp_Explorer faceExp(shape, TopAbs_FACE);
    for (; faceExp.More(); faceExp.Next()) {
        TopoDS_Face face = TopoDS::Face(faceExp.Current());

        // 获取该面对应的三角网格数据结构
        TopLoc_Location location;
        Handle(Poly_Triangulation) triangulation = BRep_Tool::Triangulation(face, location);

        if (triangulation.IsNull()) continue; // 如果该面没有被成功网格化，跳过

        // 检查面是否反转（决定了网格法线的朝向）
        Standard_Boolean isReversed = (face.Orientation() == TopAbs_REVERSED);

        // 3. 提取顶点和法线
        Standard_Integer nbNodes = triangulation->NbNodes();

        // 提示：自 OCCT 7.6+ 起，推荐使用内部现代化的方法计算法线
        // 如果你的 OCCT 版本较老，法线可能需要通过底层面（Geom_Surface）计算
        if (!triangulation->HasNormals()) {
            triangulation->ComputeNormals();
        }

        for (Standard_Integer i = 1; i <= nbNodes; ++i) {
            // 获取顶点并应用位置变换（XDE或装配体中的位置偏移）
            gp_Pnt pnt = triangulation->Node(i).Transformed(location);
            meshData.vertices.push_back(static_cast<float>(pnt.X()));
            meshData.vertices.push_back(static_cast<float>(pnt.Y()));
            meshData.vertices.push_back(static_cast<float>(pnt.Z()));

            // 获取法线数据
            gp_Dir normal = triangulation->Normal(i);
            if (isReversed) normal.Reverse(); // 如果面反转了，法线也反转
            meshData.vertices.push_back(static_cast<float>(normal.X()));
            meshData.vertices.push_back(static_cast<float>(normal.Y()));
            meshData.vertices.push_back(static_cast<float>(normal.Z()));
        }

        // 4. 提取拓扑索引（Triangle Indices）
        Standard_Integer nbTriangles = triangulation->NbTriangles();
        for (Standard_Integer i = 1; i <= nbTriangles; ++i) {
            Poly_Triangle triangle = triangulation->Triangle(i);
            Standard_Integer n1, n2, n3;
            triangle.Get(n1, n2, n3); // 获取三角形的三个局部节点编号（从1开始）

            // 转为从0开始的渲染引擎通用全局索引，并考虑面反转导致的正反面裁剪（Cull Face）问题
            if (isReversed) {
                meshData.indices.push_back(vertexOffset + (n1 - 1));
                meshData.indices.push_back(vertexOffset + (n3 - 1));
                meshData.indices.push_back(vertexOffset + (n2 - 1));
            } else {
                meshData.indices.push_back(vertexOffset + (n1 - 1));
                meshData.indices.push_back(vertexOffset + (n2 - 1));
                meshData.indices.push_back(vertexOffset + (n3 - 1));
            }
        }

        // 累加顶点偏移，供下一个面使用
        vertexOffset += nbNodes;
    }

    return meshData;
}


TEST(test_occt, tbb) {
    auto result = importStepModel("/Users/panxin/Desktop/as1-oc-214.step");
    // 现在能够正确的拿到了
    // 之后需要什么呢?
    auto mesh = convertShapeToRenderMesh(result);
    // 这里也是对的, 直接复制过来的代码是能够使用的
    

    int a = 0;
}
