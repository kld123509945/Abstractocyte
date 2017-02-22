#ifndef GRAPHMANAGER_H
#define GRAPHMANAGER_H

#include "mainopengl.h"
#include "graph.h"
#include "objectmanager.h"
#include "glsluniform_structs.h"
#include <thread>

// allocate vbo with enough space for all skeletons using this struct
// use this for all skeleton stages (simplified one)
// initialize this when while contructing the skeletons
// with their edges
struct Skeleton_Node {
    QVector4D vertex;   // original position  (simplified)  -> w: hvgx ID
    QVector2D layout1;  // layouted position (all skeletons)
    QVector2D layout2;  // layouted position (no neurites)
    QVector2D layout3;  // layouted position (no astrocyte)
};

// (1) neurite-neurite graph (2) neurite-astrocyte skeleton (3) neurites skeletons - astrocyte skeleton (4) neuries skeletons
#define max_graphs 4

class GraphManager : public MainOpenGL
{
public:
    GraphManager(ObjectManager *objectManager);
    ~GraphManager();

    void ExtractGraphFromMesh();

    bool initOpenGLFunctions();
    void drawNeuritesGraph();
    bool initVBO();
    void updateUniformsLocation(GLuint program);
    void updateUniforms(struct GlobalUniforms graph_uniforms);

    void initVertexAttribPointer();

    // force directed layout
    void startForceDirectedLayout(int graphIdx);
    void stopForceDirectedLayout(int graphIdx);

    void drawGrid(struct GlobalUniforms grid_uniforms);
    void initGrid();
    void update2Dflag(bool is2D);

    void updateGraphParam1(double value);
    void updateGraphParam2(double value);
    void updateGraphParam3(double value);
    void updateGraphParam4(double value);
    void updateGraphParam5(double value);
    void updateGraphParam6(double value);
    void updateGraphParam7(double value);

protected:
    ObjectManager                       *m_obj_mngr;
    Graph                               *m_graph[max_graphs];

    // most abstract data
    // todo: flag in ssbo to indicate this object on/off
    std::vector<GLuint>                 m_neurites_nodes; // (place holders for neurites nodes) this initialized once and not changed after words
    std::vector<GLuint>                 m_neurites_edges; // (place holders for neurites edges) this as well

    std::vector<struct Skeleton_Node>   m_skeletons_data; // all skeletons here
    std::vector<GLuint>                 m_skeletons_edges;

    bool                                m_glFunctionsSet;

    // one for the 2D space
    QOpenGLVertexArrayObject            m_NeuritesGraphVAO;
    QOpenGLBuffer                       m_NeuritesIndexVBO;
    QOpenGLBuffer                       m_NeuritesNodesVBO;



    // shaders
    GLuint                              m_program_nodes;
    GLuint                              m_program_Index;

    struct GlobalUniforms               m_uniforms;

    // thread management
    std::thread                         m_layout_thread1;
    bool                                m_FDL_running;
    bool                                m_2D;
};

#endif // GRAPHMANAGER_H
