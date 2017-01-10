// todo:    1- render 3d segmentation (done)
//          2- render the skeleton
//          3- mesh normals (to be fixed)


/*
 * FinalMatrix = Projection * View * Model
 * Model = RotationAroundOrigin * TranslationFromOrigin * RotationAroundObjectCenter
 */
#include "glwidget.h"
#include <QResource>

GLWidget::GLWidget(QWidget *parent)
    :   QOpenGLWidget(parent),
        m_vbo_mesh( QOpenGLBuffer::VertexBuffer ),
        m_vbo_skeleton( QOpenGLBuffer::VertexBuffer ),
        m_isRotatable(true),
        m_yaxis(0),
        m_xaxis(0),
        m_state(0)
{
    QString path = "://data/mouse03_clean_faces.obj";
    m_vertices_size = loadOBJ(path, m_objects);
    qDebug() << "mesh size: " << m_objects.size();

    path = "://data/m3_astrocytes_points.csv"; //m3_points_ids
    m_skeleton_vertices_size = loadSkeletonPoints(path, m_skeleton_obj, 0);

    qDebug() << "skeleton size: " << m_skeleton_obj.size();

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
    glDeleteProgram(m_program_mesh);
    glDeleteProgram(m_program_skeleton);

    glDeleteBuffers(1, &uboMatrices);
    m_vao_mesh.destroy();
    m_vbo_mesh.destroy();
    for (std::size_t i = 0; i != m_objects.size(); i++) {
        delete m_objects[i];
    }

    for (std::size_t i = 0; i != m_skeleton_obj.size(); i++) {
        delete m_skeleton_obj[i];
    }

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
    initializeOpenGLFunctions();

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    qDebug() << "initializeGL";

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


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

    m_vbo_mesh.create();
    m_vbo_mesh.setUsagePattern( QOpenGLBuffer::StaticDraw);
    if ( !m_vbo_mesh.bind() ) {
        qDebug() << "Could not bind vertex buffer to the context.";
    }

    m_vbo_mesh.allocate(NULL, m_vertices_size * /* m_objects[0]->getSize() */ sizeof(QVector3D));

    for (std::size_t i = 0; i != 1; i++) {
        qDebug() << "allocating: " << m_objects[i]->getName().data();
        m_vbo_mesh.write(0, &m_objects[i]->getVertices()[0], m_objects[i]->getSize() * sizeof(QVector3D));
        // should not be uniform!
        // have an ID for each cell type, and determine the color in the fragment based on ID
        // int per vertex
   }

    GLint posAttr;
    const char* posAttr_name = "posAttr";
    posAttr = glGetAttribLocation(m_program_mesh,posAttr_name);
    qDebug() << "pos Attr: " << posAttr;
    glEnableVertexAttribArray(posAttr);
    if (posAttr == -1) {
        qDebug() << "Could not bind attribute " << posAttr_name;
        return;
    }

    // attribute, number of elements per vertex, here (x,y,z),
    // the type of each element, stride, offset
    glVertexAttribPointer(posAttr, 3, GL_FLOAT, GL_FALSE, 0,  0);

    m_vbo_mesh.release();
    m_vao_mesh.release();

    /********** START SKELETON **************/
    qDebug() << "point";

    m_program_skeleton = glCreateProgram();
    res = initShader(m_program_skeleton, ":/shaders/skeleton_point.vert", ":/shaders/skeleton_point.geom", ":/shaders/skeleton_point.frag");
    if (res == false)
        return;

    qDebug() << "Initializing SKELETON 1";
    m_vao_skeleton.create();
    m_vao_skeleton.bind();

    glUseProgram(m_program_skeleton); // m_program_mesh->bind();
    setMVPAttrib(m_program_skeleton);

    GLuint color_idx = glGetUniformLocation(m_program_skeleton, "color");
    QVector3D color = QVector3D(1.0, 1.0, 0.0);
    glUniform3fv(color_idx, 1, &color[0]);

    m_vbo_skeleton.create();
    m_vbo_skeleton.setUsagePattern( QOpenGLBuffer::StaticDraw);
    if ( !m_vbo_skeleton.bind() ) {
        qDebug() << "Could not bind vertex buffer to the context.";
    }

    m_vbo_skeleton.allocate(NULL, /*m_vertices_size*/  m_skeleton_obj[0]->getSize() * sizeof(QVector3D));

    for (std::size_t i = 0; i != 1; i++) {
        qDebug() << "allocating: " << m_skeleton_obj[i]->getName().data();
        m_vbo_skeleton.write(0, &m_skeleton_obj[i]->getVertices()[0], m_skeleton_obj[i]->getSize() * sizeof(QVector3D));
   }

    posAttr_name = "posAttr";
    posAttr = glGetAttribLocation(m_program_skeleton, posAttr_name);
    qDebug() << "pos Attr: " << posAttr;
    glEnableVertexAttribArray(posAttr);
    if (posAttr == -1) {
        qDebug() << "Could not bind attribute " << posAttr_name;
        return;
    }

    // attribute, number of elements per vertex, here (x,y,z),
    // the type of each element, stride, offset
    glVertexAttribPointer(posAttr, 3, GL_FLOAT, GL_FALSE, 0,  0);

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

    m_vao_mesh.bind();
    glUseProgram(m_program_mesh);
    setMVPAttrib(m_program_mesh);

    GLuint bindingPoint = 1, blockIndex;
    blockIndex = glGetUniformBlockIndex(m_program_mesh, "Matrices");
    glUniformBlockBinding(m_program_mesh, blockIndex, bindingPoint);

    glGenBuffers(1, &uboMatrices);
    glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
    QMatrix4x4 matrices[2];
    matrices[0] = m_vMatrix;
    matrices[1] = m_projection;
    glBufferData(GL_UNIFORM_BUFFER, sizeof(matrices), matrices, GL_DYNAMIC_DRAW);
    qDebug() << "sizeof(matrices): " << sizeof(matrices);
    qDebug() << "sizeof(QMatrix4x4): " << sizeof(m_projection);
    qDebug() << "sizeof(GLfloat) * 4 * 4: " << sizeof(GLfloat) * 4 * 4;
    glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, uboMatrices);

//    GLuint ssbo;
//    glGenBuffers(1, &ssbo);

    GLuint y_axis = glGetUniformLocation(m_program_mesh, "y_axis");
    glUniform1iv(y_axis, 1, &m_yaxis);

    GLuint x_axis = glGetUniformLocation(m_program_mesh, "x_axis");
    glUniform1iv(x_axis, 1, &m_xaxis);

    GLuint state = glGetUniformLocation(m_program_mesh, "state");
    glUniform1iv(state, 1, &m_state);

    glDrawArrays(GL_TRIANGLES, 0,  m_vertices_size );

    m_vao_mesh.release();

    /************************/

    m_vao_skeleton.bind();
    glUseProgram(m_program_skeleton);
    setMVPAttrib(m_program_skeleton);

    y_axis = glGetUniformLocation(m_program_mesh, "y_axis");
    glUniform1iv(y_axis, 1, &m_yaxis);

    x_axis = glGetUniformLocation(m_program_mesh, "x_axis");
    glUniform1iv(x_axis, 1, &m_xaxis);

    state = glGetUniformLocation(m_program_mesh, "state");
    glUniform1iv(state, 1, &m_state);

    glDrawArrays(GL_POINTS, 0,  m_skeleton_vertices_size );

    m_vao_skeleton.release();
}

void GLWidget::resizeGL(int w, int h)
{
    qDebug() << "Func: resizeGL: " << w << " " <<   width() <<  " " << h << " " << height();
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
        case(Qt::Key_0):
            m_flag_mesh_rotation = !m_flag_mesh_rotation;
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
