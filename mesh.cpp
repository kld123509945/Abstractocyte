#include "mesh.h"

Mesh::Mesh()
{

}

void Mesh::addVertex(struct VertexData vdata)
{
    verticesList.push_back(vdata);
}

bool Mesh::isValidFaces(int f1, int f2, int f3)
{
    if (    f1 > verticesList.size()
            || f2 > verticesList.size()
            || f3 > verticesList.size()  ) {
        qDebug() << "Error in obj file, verticesList size: " << verticesList.size();
        return false;
    }

    return true;
}

bool Mesh::MarkBleedingVertices(QStringList markersList, int vertex_offset)
{
    bool result = false;
    for (int i = 0; i < markersList.size(); ++i) {
        int rv_index = markersList.at(i).toInt() + vertex_offset;
        if (rv_index >= verticesList.size() || rv_index < 1) {
            qDebug() << "error! skiped rv " << rv_index << " vertices list size: " << verticesList.size();
            result =  false;
            continue;
        }
        verticesList[rv_index-1].skeleton_vertex.setW(1);
        result = true;
    }

    return result;
}


void Mesh::allocateVerticesVBO(QOpenGLBuffer vbo_mesh)
{
    vbo_mesh.allocate(verticesList.data(), verticesList.size() * sizeof(VertexData));
}
