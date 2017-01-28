// todo:    1- render 3d segmentation (done)
//          2- render the skeleton
//          3- mesh normals (to be fixed)


/*
 * FinalMatrix = Projection * View * Model
 * Model = RotationAroundOrigin * TranslationFromOrigin * RotationAroundObjectCenter
 */
#include "glwidget.h"
#include <QResource>
#include "colors.h"

GLWidget::GLWidget(QWidget *parent)
    :   QOpenGLWidget(parent),
        m_vbo_mesh( QOpenGLBuffer::VertexBuffer ),
        m_vbo_skeleton( QOpenGLBuffer::VertexBuffer ),
        m_isRotatable(true),
        m_yaxis(0),
        m_xaxis(0),
        m_state(0)
{
    m_2dspace = new AbstractionSpace(100, 100);


    // to do: combine all these files in one .obj file
    QString path= "://data/skeleton_astrocyte_m3/mouse3_astro_skelton.obj";
    m_mesh.loadObj(path);
    path = "://data/mouse03_skeleton_centroid.obj";
    m_mesh.loadObj(path);
    path = "://data/mouse03_astro_skeleton.sk";
    m_mesh.loadSkeletonPoints(path); // 11638884, 19131720
    path = "://data/mouse03_skeletons.sk";
    m_mesh.loadSkeletonPoints(path); // 11638884, 19131720

    // todo: one graph manager, with all the graphs manipulations

    m_distance = 0.2;
    m_rotation = QQuaternion();
    //reset rotation
    m_rotation.setScalar(1.0f);
    m_rotation.setX(0.0f);
    m_rotation.setY(0.0f);
    m_rotation.setZ(0.0f);
    //reset translation
    m_translation = QVector3D(0.0, 0.0, 0.0);
}

GLWidget::~GLWidget()
{
    qDebug() << "~GLWidget()";

    makeCurrent();
    glDeleteProgram(m_program_skeleton);
    glDeleteProgram(m_program_mesh);

    m_vao_mesh.destroy();
    m_vbo_mesh.destroy();


    for (std::size_t i = 0; i != m_skeleton_obj.size(); i++) {
        delete m_skeleton_obj[i];
    }

    delete m_2dspace;

    doneCurrent();
}

void GLWidget::setMVPAttrib(GLuint program)
{
    // calculate model view transformation
    // world/model matrix: determines the position and orientation of an object in 3D space
    m_mMatrix.setToIdentity();

    // Scale
    m_mMatrix.translate(m_cameraPosition);
    m_mMatrix.scale(m_distance);
    m_mMatrix.translate(-1.0 * m_cameraPosition);

    // Translation
    m_mMatrix.translate(m_translation);

    // Rotation
    m_mMatrix.translate(m_cameraPosition);
    m_mMatrix.rotate(m_rotation);
    m_mMatrix.translate(-1.0 * m_cameraPosition);

    GLuint mMatrix = glGetUniformLocation(program, "mMatrix");
    glUniformMatrix4fv(mMatrix, 1, GL_FALSE, m_mMatrix.data());

    GLuint vMatrix = glGetUniformLocation(program, "vMatrix");
    glUniformMatrix4fv(vMatrix, 1, GL_FALSE, m_vMatrix.data());

    GLuint pMatrix = glGetUniformLocation(program, "pMatrix");
    glUniformMatrix4fv(pMatrix, 1, GL_FALSE, m_projection.data());
}

void GLWidget::initializeGL()
{
    qDebug() << "initializeGL";
    int offset = 0;
    initializeOpenGLFunctions();
    m_2dspace->initOpenGLFunctions();
    m_mesh.initOpenGLFunctions();

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    /******************** SSBO Data ***************************/
    qDebug() << " m_mesh.getSSBOSize(): " << m_mesh.getBufferSize();
    m_mesh.initBuffer();
    /******************** Abstraction Space ********************/
    m_2dspace->initBuffer();
    /******************** END *********************************/

    /* start initializing mesh */
    qDebug() << "Initializing MESH";
    m_program_mesh = glCreateProgram();
    bool res = initShader(m_program_mesh, ":/shaders/mesh.vert", ":/shaders/mesh.geom", ":/shaders/mesh.frag");
    if (res == false)
        return;

    // create vbos and vaos
    m_vao_mesh.create();
    m_vao_mesh.bind();

    glUseProgram(m_program_mesh); // m_program_mesh->bind();
    setMVPAttrib(m_program_mesh);

    QVector3D lightDir = QVector3D(-2.5f, -2.5f, -0.9f);
    GLuint lightDir_loc = glGetUniformLocation(m_program_mesh, "diffuseLightDirection");
    glUniform3fv(lightDir_loc, 1, &lightDir[0]);

    m_vbo_mesh.create();
    m_vbo_mesh.setUsagePattern( QOpenGLBuffer::StaticDraw );
    if ( !m_vbo_mesh.bind() ) {
        qDebug() << "Could not bind vertex buffer to the context.";
    }

    m_mesh.initVBO(m_vbo_mesh);
    m_mesh.initVertexAttrib();

    m_vbo_mesh.release();
    m_vao_mesh.release();

    /***************************************/
    qDebug() << "Initializing MESH POINTS";
    m_program_mesh_points = glCreateProgram();
    res = initShader(m_program_mesh_points, ":/shaders/mesh.vert", ":/shaders/mesh_points.geom", ":/shaders/mesh_points.frag");
    if (res == false)
        return;

    // create vbos and vaos
    m_vao_mesh_points.create();
    m_vao_mesh_points.bind();

    glUseProgram(m_program_mesh_points);

    lightDir_loc = glGetUniformLocation(m_program_mesh_points, "diffuseLightDirection");
    glUniform3fv(lightDir_loc, 1, &lightDir[0]);


    if ( !m_vbo_mesh.bind() ) {
        qDebug() << "Could not bind vertex buffer to the context.";
    }

    setMVPAttrib(m_program_mesh_points);
    m_mesh.initVertexAttrib();

    m_vbo_mesh.release();
    m_vao_mesh_points.release();

    /********** START SKELETON **************/
    qDebug() << "point";

    m_program_skeleton = glCreateProgram();
    res = initShader(m_program_skeleton, ":/shaders/skeleton_point.vert", ":/shaders/skeleton_point.geom", ":/shaders/skeleton_point.frag");
    if (res == false)
        return;

    qDebug() << "Initializing SKELETON 1";
    m_vao_skeleton.create();
    m_vao_skeleton.bind();

    glUseProgram(m_program_skeleton);
    setMVPAttrib(m_program_skeleton);

    m_vbo_skeleton.create();
    m_vbo_skeleton.setUsagePattern( QOpenGLBuffer::StaticDraw);
    if ( !m_vbo_skeleton.bind() ) {
        qDebug() << "Could not bind vertex buffer to the context.";
    }

    m_vbo_skeleton.allocate(NULL, m_mesh.getNodesCount() * sizeof(SkeletonVertex));
    m_mesh.initSkeletonVBO(m_vbo_skeleton);

    qDebug() << " m_mesh.getNodesCount(): " << m_mesh.getNodesCount();
    qDebug() << " m_mesh.getNodesCount()* sizeof(QVector3D): " << m_mesh.getNodesCount()* sizeof(SkeletonVertex);

    GL_Error();

    offset = 0;
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(SkeletonVertex),  0);

    offset += sizeof(QVector3D);
    glEnableVertexAttribArray(1);
    glVertexAttribIPointer(1, 1, GL_INT, sizeof(SkeletonVertex), (GLvoid*)offset);

    GL_Error();

    m_vbo_skeleton.release();
    m_vao_skeleton.release();


    glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    // to enable transparency
    glEnable (GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
}

void GLWidget::paintGL()
{
    // paint the text here
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    const qreal retinaScale = devicePixelRatio();
    glViewport(0, 0, width() * retinaScale, height() * retinaScale);

    GLuint y_axis, x_axis;

    m_vao_skeleton.bind();
    glUseProgram(m_program_skeleton);
    setMVPAttrib(m_program_skeleton);

    y_axis = glGetUniformLocation(m_program_skeleton, "y_axis");
    glUniform1iv(y_axis, 1, &m_yaxis);

    x_axis = glGetUniformLocation(m_program_skeleton, "x_axis");
    glUniform1iv(x_axis, 1, &m_xaxis);

    glDrawArrays(GL_POINTS, 0,  m_mesh.getNodesCount() );
    m_vao_skeleton.release();

    /************************/
    m_vao_mesh.bind();
    glUseProgram(m_program_mesh);
    setMVPAttrib(m_program_mesh);

    y_axis = glGetUniformLocation(m_program_mesh, "y_axis");
    glUniform1iv(y_axis, 1, &m_yaxis);

    x_axis = glGetUniformLocation(m_program_mesh, "x_axis");
    glUniform1iv(x_axis, 1, &m_xaxis);

    glDrawArrays(GL_TRIANGLES, 0,   m_mesh.getVertixCount() );

    m_vao_mesh.release();
    /************************/
    m_vao_mesh_points.bind();
    glUseProgram(m_program_mesh_points);
    setMVPAttrib(m_program_mesh_points);

    y_axis = glGetUniformLocation(m_program_mesh_points, "y_axis");
    glUniform1iv(y_axis, 1, &m_yaxis);

    x_axis = glGetUniformLocation(m_program_mesh_points, "x_axis");
    glUniform1iv(x_axis, 1, &m_xaxis);

    glDrawArrays(GL_POINTS, 0,  m_mesh.getVertixCount() );
    m_vao_mesh_points.release();
    /************************/

}

void GLWidget::resizeGL(int w, int h)
{
    // Calculate aspect ratio
    const qreal retinaScale = devicePixelRatio();
    h = (h == 0) ? 1 : h;
    glViewport(0, 0, w * retinaScale, h * retinaScale);

    m_projection.setToIdentity();
    m_projection.ortho(GLfloat(-w) / GLfloat(h),  GLfloat(w) / GLfloat(h), -1.0,  1.0f, -10.0, 10.0 );

    // set up view
    // view matrix: transform a model's vertices from world space to view space, represents the camera
    m_center = QVector3D(0.0, 0.0, 0.0);
    m_cameraPosition = QVector3D(2.5, 2.5, 2.5);
    QVector3D  cameraUpDirection = QVector3D(0.0, 1.0, 0.0);
    m_vMatrix.setToIdentity();
    m_vMatrix.lookAt(m_cameraPosition, QVector3D(0.0, 0.0, 0.0), cameraUpDirection);

    update();
}

void GLWidget::mousePressEvent(QMouseEvent *event)
{
    m_lastPos = event->pos();
    setFocus();
    event->accept();
}

void GLWidget::mouseMoveEvent(QMouseEvent *event)
{
    int deltaX = event->x() - m_lastPos.x();
    int deltaY = event->y() - m_lastPos.y();

    if (m_isRotatable) {
        // Mouse release position - mouse press position
        QVector2D diff = QVector2D(deltaX, deltaY);
        // Rotation axis is perpendicular to the mouse position difference
        QVector3D n = QVector3D(diff.y(), diff.x(), 0.0).normalized();
        // Accelerate angular speed relative to the length of the mouse sweep
        qreal acc = diff.length()/2.0;;

        // Calculate new rotation axis as weighted sum
        m_rotationAxis = (m_rotationAxis + n).normalized();
        // angle in degrees and rotation axis
        m_rotation = QQuaternion::fromAxisAndAngle(m_rotationAxis, acc) * m_rotation;
    } else {
        m_translation = QVector3D( m_translation.x() + deltaX/(float)width(), m_translation.y() +  -1.0 * (deltaY/(float)height()), 0.0);
    }

    m_lastPos = event->pos();
    event->accept();
    update();
}

void GLWidget::wheelEvent(QWheelEvent *event)
{
    int delta = event->delta();

    if (event->orientation() == Qt::Vertical) {
        if (delta < 0) {
            m_distance *= 1.1;
        } else {
            m_distance *= 0.9;
        }

        update();
    }
}

void GLWidget::keyPressEvent(QKeyEvent *event)
{
    switch( event->key() ) {
        case(Qt::Key_S): // reset
            m_rotation.setScalar(1.0f);
            m_rotation.setX(0.0f);
            m_rotation.setY(0.0f);
            m_rotation.setZ(0.0f);
            //reset translation
            m_translation = QVector3D(0.0, 0.0, 0.0);
            //reset zoom
            m_distance = 0.2f;
            update();
            break;
        case(Qt::Key_T):
            m_isRotatable = !m_isRotatable;
            break;
    }
}

void GLWidget::getSliderX(int value)
{
    m_xaxis = value;
    update();
}

void GLWidget::getSliderY(int value)
{
    m_yaxis = value;
    update();
}
