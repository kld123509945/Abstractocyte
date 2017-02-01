// TODO: draw the boundry of grid

#ifndef SPATIALHASH_H
#define SPATIALHASH_H

#include <map>
#include <vector>
#include <set>
#include <unordered_map>

#include "mainopengl.h"
#include "node.h"

struct GridUniforms {
        GLint y_axis;
        GLint x_axis;
        float* mMatrix;
        float* vMatrix;
        float* pMatrix;
        float* modelNoRotMatrix;
        QMatrix4x4 rMatrix;
 };

struct pair_hash {
    template <class T1, class T2>
    std::size_t operator () (const std::pair<T1,T2> &p) const {
        auto h1 = std::hash<T1>{}(p.first);
        auto h2 = std::hash<T2>{}(p.second);
        return h1 ^ h2;
    }
};

class SpatialHash: public MainOpenGL
{
public:
    SpatialHash(int col, float min, float max);
    ~SpatialHash();


    // insert point in grid
    std::pair<int, int> hash(float x, float y);
    std::set< std::pair<int, int> > hashAABB(float x, float y, float r);
    void insert(Node *node);
    void queryAABB(Node *node, float r, std::vector<Node*> &dataCell);
    void updateNode(Node *node);
    void clear();

    // opengl functions
     bool initOpenGLFunctions();
     void drawGrid(struct GridUniforms grid_uniforms);
     bool init_Shaders_Buffers();
     void fillGridDataPoints();

protected:
    float   m_cellSize;
    float   m_min;
    float   m_max;
    float   m_col;
    std::unordered_map< std::pair<int, int>, std::vector<Node*>, pair_hash > m_hashMap;

    // opengl functions to draw grid
    QOpenGLVertexArrayObject        m_GridVAO;
    QOpenGLBuffer                   m_GridVBO;
    GLuint                          m_program_grid;

    bool                            m_glFunctionsSet;
    std::vector< QVector2D >        m_gridDataPoints;
    struct GridUniforms             m_uniforms;
};

#endif // SPATIALHASH_H