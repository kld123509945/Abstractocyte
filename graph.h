// how to reoresent a whole skeleton in a graph?
// all the nodes of a skeleton should have the same hvgx id
// so I cant index the map with it

#ifndef GRAPH_H
#define GRAPH_H

#include "node.h"
#include "edge.h"
#include "spatialhash.h"
#include "objectmanager.h"

#include <QDebug>
#include <QFile>
#include <QVector2D>
#include <QVector3D>
#include <QMatrix4x4>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
// store graph per object?

struct BufferNode
{
    QVector3D coord3D; // vertex center, todo: w: ID
    QVector2D coord2D; // layouted coordinate
    int ID;
};

/*
 * NODE_NODE:
 * nodes IDs are hvgx ID
 * edges simple edge between nodes using hvgx ID
 *
 * NODE_SKELETON:
 *
 */
enum class Graph_t { NODE_NODE, NODE_SKELETON,  SKELETON_SKELETON };


class Graph
{
public:
    Graph(Graph_t graphType);
    ~Graph();

    bool createGraph(ObjectManager *objectManager);
    bool parseNODE_NODE(ObjectManager *objectManager);
    bool parseSKELETON(ObjectManager *objectManager);

    bool loadNodes(QString filename);
    bool loadEdges(QString filename);

    Node* addNode(int nID, float x, float y, float z);
    Edge* addEdge(int eID, int nID1, int nID2);

    Node* getNode(int nID);
    Edge* getEdge(int eID);


    std::map<int, Node*> getNodes() { return m_nodes; }
    std::map<int, Edge*> getEdges() { return m_edges; }

    int getNodesCount(){ return m_nodesCounter; }
    int getEdgesCount() { return m_edgesCounter; }
    int getDupEdgesCount() { return m_dupEdges; }

    std::map<int, Node*>::iterator getNodesBegin()  { return m_nodes.begin(); }
    std::map<int, Node*>::iterator getNodesEnd()    { return m_nodes.end(); }
    std::map<int, Edge*>::iterator getEdgesBegin()  { return m_edges.begin(); }
    std::map<int, Edge*>::iterator getEdgesEnd()    { return m_edges.end(); }

    // force directed layout functions
    void runforceDirectedLayout();
    void attractConnectedNodes(Edge *edge, float k);
    void repulseNodes(Node *node1, Node *node2, float k);
    QVector2D attractionForce(float x1, float y1, float x2, float y2, float k);
    QVector2D repulsiveForce(float x1, float y1, float x2, float y2, float k);
    void resetCoordinates(QMatrix4x4 rotationMatrix);

    // opengl related functions
    size_t vertexBufferSize() { return m_bufferNodes.size(); }
    size_t indexBufferSize() { return m_bufferIndices.size(); }

    void allocateBVertices(QOpenGLBuffer vertexVbo);
    void allocateBIndices(QOpenGLBuffer indexVbo);

    void terminateFDL()  { m_FDL_terminate = true; }

    // spatial hashing
    void updateNode(Node *node);
    void drawGrid(struct GlobalUniforms grid_uniforms);
    void initGridBuffers();
    void updateGridUniforms(struct GlobalUniforms grid_uniforms);

protected:
    Graph_t                         m_gType;

    std::map<int, Node*>            m_nodes;    // IDs are unique to identify nodes
                                                // if more than a skeleton
                                                // IDs for skeleton are offsets, that is later
                                                // stored in the skeleton itself

    std::map<int, Edge*>            m_edges;    // IDs refer to m_nodes IDs

    std::map<int, Skeleton*>        m_skeletons; // ID are hvgx

    int m_nodesCounter;
    int m_edgesCounter;
    int m_dupEdges;

    // for opengl buffer
    std::vector<struct BufferNode>  m_bufferNodes;
    std::vector<GLuint>             m_bufferIndices;

    // force directed laout
    float                           m_MAX_DISTANCE;
    float                           m_AABBdim;
    float                           m_Cr;
    float                           m_Ca;
    int                             m_ITERATIONS;
    float                           m_MAX_VERTEX_MOVEMENT;
    float                           m_SLOW_FACTOR;
    float                           m_MAX_FORCE;

    bool                            m_FDL_terminate;

    // spatial hashing
    SpatialHash                     *hashGrid;

};

#endif // GRAPH_H
