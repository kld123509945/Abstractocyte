#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QOpenGLWidget>
#include <QWidget>
#include <QtOpenGL>

#include "graphmanager.h"
#include "mainopengl.h"
#include "abstractionspace.h"
#include "objectmanager.h"

class GLWidget : public QOpenGLWidget, MainOpenGL
{
    Q_OBJECT

public:
    GLWidget(QWidget *parent = 0);
    ~GLWidget();

public slots:
    void getSliderX(int value);
    void getSliderY(int value);
    void getIntervalID(int ID);

protected:
    void initializeGL() Q_DECL_OVERRIDE;
    void resizeGL(int w, int h) Q_DECL_OVERRIDE;
    void paintGL() Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void wheelEvent(QWheelEvent *event) Q_DECL_OVERRIDE;
    void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;

    void updateMVPAttrib();
    void loadMesh();

    AbstractionSpace            *m_2dspace;

    /* mesh */
    ObjectManager                        *m_mesh;

    GraphManager                *m_graphManager;

    struct MeshUniforms         m_mesh_uniforms;
    struct GlobalUniforms        m_graph_uniforms;

    /* matrices */
    QMatrix4x4                  m_projection;
    QMatrix4x4                  m_mMatrix;
    QMatrix4x4                  m_vMatrix;
    QMatrix4x4                  m_model_noRotation;
    QMatrix4x4                  m_rotationMatrix;

    QVector3D                   m_cameraPosition;
    QVector3D                   m_center;


    /* rotation */
    QPoint                      m_lastPos;
    double                      m_distance;
    bool                        m_isRotatable;
    QQuaternion                 m_rotation;
    QVector3D                   m_rotationAxis;
    QVector3D                   m_translation;

    /* mouse pad */
    int                         m_yaxis;
    int                         m_xaxis;

    // force directed layout
    bool                        m_FDL_running;

    QTimer                      *timer;

};


#endif // GLWIDGET_H
