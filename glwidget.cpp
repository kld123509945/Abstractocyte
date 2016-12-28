// todo:    1- render 3d segmentation
//          2- render the skeleton

#include "glwidget.h"
#include <QResource>

GLWidget::GLWidget(QWidget *parent)
    :  QOpenGLWidget(parent),
       m_vbo_circle( QOpenGLBuffer::VertexBuffer )
{
    std::vector<QVector3D> out_vertices;
    QString path = "://data/mouse03.obj";
    loadOBJ(path, out_vertices);
}

GLWidget::~GLWidget()
{
    qDebug() << "~GLWidget()";

    makeCurrent();
    delete m_program_circle;
    m_vao_circle.destroy();
    m_vbo_circle.destroy();
    doneCurrent();
}


void GLWidget::initializeGL()
{
    initializeOpenGLFunctions();
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    QFont f;
    f.setPixelSize(50);
    initText(f);

    /* testing */
    m_projection.setToIdentity();

    m_program_circle = new QOpenGLShaderProgram(this);
    bool res = initShader(m_program_circle, ":/shaders/shader.vert", ":/shaders/shader.geom", ":/shaders/shader.frag");
    if(res == false)
        return;

    qDebug() << "Initializing curser VAO";
    // create vbos and vaos
    m_vao_circle.create();
    m_vao_circle.bind();

    m_vbo_circle.create();
    m_vbo_circle.setUsagePattern( QOpenGLBuffer::StaticDraw);
    if ( !m_vbo_circle.bind() ) {
        qDebug() << "Could not bind vertex buffer to the context.";
        return;
    }

    GLfloat points[] = { 0.5f, 0.5f };

    m_vbo_circle.allocate(points, 1 /*elements*/ * 2 /*corrdinates*/ * sizeof(GLfloat));

    m_program_circle->bind();
    m_program_circle->setUniformValue("pMatrix", m_projection);
    m_program_circle->enableAttributeArray("posAttr");
    m_program_circle->setAttributeBuffer("posAttr", GL_FLOAT, 0, 2);
    m_program_circle->release();

    m_vbo_circle.release();
    m_vao_circle.release();

    /* end testing */

    glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
}

void GLWidget::paintGL()
{
    // paint the text here
    qDebug() << "draw";

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    const QString text = "Juxtastrocyte";
    float scaleX = 1.0/width();
    float scaleY = 1.0/height();
    float x = 0.0;
    float y = 0.0;
    renderText( x, y, scaleX, scaleY, text);

    m_vao_circle.bind();
    m_program_circle->bind();
    m_program_circle->setUniformValue("pMatrix", m_projection);
    glDrawArrays(GL_POINTS, 0, 1);
    m_program_circle->release();
    m_vao_circle.release();
}

void GLWidget::resizeGL(int w, int h)
{
    qDebug() << "Func: resizeGL: " << w << " " <<   width() <<  " " << h << " " << height();
    // Calculate aspect ratio
    const qreal retinaScale = devicePixelRatio();
    h = (h == 0) ? 1 : h;
    glViewport(0, 0, w * retinaScale, h * retinaScale);
    m_projection.setToIdentity();
    m_projection.ortho( 0.0f,  1.0f, 0.0f, 1.0f, -1.0, 1.0 );
}

void GLWidget::mouseMoveEvent(QMouseEvent *event)
{
}

void GLWidget::getSliderX(int value)
{
    qDebug() << value;
}

void GLWidget::getSliderY(int value)
{
    qDebug() << value;
}
