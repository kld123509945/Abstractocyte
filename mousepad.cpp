// todo:    1) draw the grid with colors first
//          2) enable selection buffer
//          3) use ID for each color to retrieve the properties of the area we are inside

#include "mousepad.h"
#include "colors.h"

MousePad::MousePad(QWidget *parent)
    :  QOpenGLWidget(parent),
       m_vbo_circle( QOpenGLBuffer::VertexBuffer ),
       m_vbo_2DSpaceVerts( QOpenGLBuffer::VertexBuffer ),
       m_vbo_2DSpaceIndix( QOpenGLBuffer::IndexBuffer ),
       m_updatedPointer(true)
{
    m_bindColorIdx = 1;
    qDebug() << "MousePad";
    circle.x = 0.0;
    circle.y = 0.0;
}

MousePad::~MousePad()
{
    qDebug() << "~MousePad";
    makeCurrent();
    delete m_program_circle;
    delete m_program_selection;
    m_vao_circle.destroy();
    m_vbo_circle.destroy();

    m_vao_selection.destroy();

    glDeleteRenderbuffers(1, &m_rbo_depth);
    glDeleteTextures(1, &m_fbo_texture);
    glDeleteFramebuffers(1, &m_fbo);

    /* free_resources */
    glDeleteBuffers(1, &m_vbo_fbo_vertices);

    doneCurrent();
}

void MousePad::initSelectionPointerGL()
{
    m_program_circle = new QOpenGLShaderProgram(this);
    bool res = initShader(m_program_circle, ":/shaders/shader.vert", ":/shaders/shader.geom", ":/shaders/shader.frag");
    if(res == false)
        return;

    // create vbos and vaos
    m_vao_circle.create();
    m_vao_circle.bind();

    m_vbo_circle.create();
    m_vbo_circle.setUsagePattern( QOpenGLBuffer::DynamicDraw);
    if ( !m_vbo_circle.bind() ) {
        qDebug() << "Could not bind vertex buffer to the context.";
        return;
    }

    GLfloat points[] = { circle.x, circle.y };

    m_vbo_circle.allocate(points, 1 /*elements*/ * 2 /*corrdinates*/ * sizeof(GLfloat));

    m_program_circle->bind();
    m_program_circle->setUniformValue("pMatrix", m_projection);
    m_program_circle->enableAttributeArray("posAttr");
    m_program_circle->setAttributeBuffer("posAttr", GL_FLOAT, 0, 2);
    m_program_circle->release();

    m_vbo_circle.release();
    m_vao_circle.release();

    /* selection buffer */

    m_program_selection = new QOpenGLShaderProgram(this);
    res = initShader(m_program_selection, ":/shaders/selection.vert", ":/shaders/selection.geom", ":/shaders/selection.frag");
    if(res == false)
        return;

    // create vbos and vaos
    m_vao_selection.create();
    m_vao_selection.bind();
    if ( !m_vbo_circle.bind() ) {
        qDebug() << "Could not bind vertex buffer to the context.";
        return;
    }

    m_program_selection->bind();
    m_program_selection->enableAttributeArray("posAttr");
    m_program_selection->setAttributeBuffer("posAttr", GL_FLOAT, 0, 2);
    m_program_selection->setUniformValue("pMatrix",   m_projection);
    m_program_selection->release();

    m_vbo_circle.release();
    m_vao_selection.release();
}

void MousePad::initRect(QVector2D p00, float dimX, float dimY, int ID)
{
    struct abstractionPoint p = {p00, ID};
    p.ID = ID;
    int offset = m_vertices.size();

    m_vertices.push_back(p);        // p00

    p.point.setX(p00.x() + dimX);   // p10
    p.point.setY(p00.y());
    m_vertices.push_back(p);

    p.point.setX(p00.x());          // p01
    p.point.setY(p00.y() + dimY);
    m_vertices.push_back(p);

    p.point.setX(p00.x() + dimX);   // p11
    p.point.setY(p00.y() + dimY);
    m_vertices.push_back(p);


    m_indices.push_back(offset + 0);
    m_indices.push_back(offset + 1);
    m_indices.push_back(offset + 2);

    m_indices.push_back(offset + 1);
    m_indices.push_back(offset + 3);
    m_indices.push_back(offset + 2);
}

void MousePad::initData()
{

    // init intervals

    struct properties ast1, ast2, ast3, ast4;
    struct properties neu1, neu2, neu3;

    ast1.pos_alpha = QVector4D(0, 20, 0.0, 1.0);
    ast1.trans_alpha = QVector4D(0, 20, 1.0, 1.0);
    ast1.color_alpha = QVector4D(0, 20, 0, 1.0); // color_intp (toon/phong)
    ast1.point_size = QVector4D(0, 20, 1, 1);
    ast1.extra_info = QVector4D(0.0f, 0, 1, 1);
    ast1.render_type = QVector4D(1, 0, 0, 0);

    ast2.pos_alpha = QVector4D(20, 40, 0.0, 0.5);
    ast2.trans_alpha = QVector4D(20, 40, 1.0, 0.0); // mesh points would flip this (0->1)
    ast2.color_alpha = QVector4D(20, 40, 1, 1.0); // color_intp (toon/phong)
    ast2.point_size = QVector4D(20, 40, 1, 3);
    ast2.extra_info = QVector4D(0.0f, 0, 1, 2);
    ast2.render_type = QVector4D(1, 1, 0, 0);

    ast3.pos_alpha = QVector4D(40, 90, 0.5, 1.0);
    ast3.trans_alpha = QVector4D(40, 90, 1.0, 1.0);
    ast3.color_alpha = QVector4D(40, 90, 1, 1.0); // color_intp (toon/phong)
    ast3.point_size = QVector4D(40, 90, 3, 6);
    ast3.extra_info = QVector4D(0.0f, 0, 1, 2);
    ast3.render_type = QVector4D(0, 1, 1, 0);
    ast4.pos_alpha = QVector4D(90, 99, 0.0, 1.0);
    ast4.trans_alpha = QVector4D(90, 99, 1.0, 0.0);
    ast4.color_alpha = QVector4D(90, 99, 1, 1); // color_intp (toon/phong)
    ast4.point_size = QVector4D(90, 99, 6, 6);
    ast4.extra_info = QVector4D(0.05f, 0, 2, 2);
    ast4.render_type = QVector4D(0, 0, 1, 0);

    neu1.pos_alpha  = QVector4D(0, 20, 0, 1.0);      // position interpolation
    neu1.trans_alpha = QVector4D(0, 20, 1.0, 1.0);    // alpha
    neu1.color_alpha = QVector4D(0, 20, 0, 1.0);       // color intp (toon/phong)
    neu1.point_size = QVector4D(0, 20, 1, 1);        // point size
    neu1.extra_info = QVector4D(0.0f, 0.0f, 1, 1);       // alpha limit, div, pos1, pos2
    neu1.render_type = QVector4D(1, 0, 0, 0);

    neu2.pos_alpha  = QVector4D(20, 50, 0, 1.0);      // position interpolation
    neu2.trans_alpha = QVector4D(20, 50, 1.0, 0.0);    // alpha
    neu2.color_alpha = QVector4D(20, 50, 1, 1.0);       // color intp (toon/phong)
    neu2.point_size = QVector4D(20, 50, 1, 7);        // point size
    neu2.extra_info = QVector4D(0.0f, 0.0f, 1, 2);       // alpha limit, div, pos1, pos2
    neu2.render_type = QVector4D(1, 1, 1, 0);

    neu3.pos_alpha  = QVector4D(50, 99, 0, 1);      // position interpolation
    neu3.trans_alpha = QVector4D(50, 99, 1, 1);    // alpha
    neu3.color_alpha = QVector4D(50, 99, 1, 1);       // color intp (toon/phong)
    neu3.point_size = QVector4D(50, 99, 7, 20);        // point size
    neu3.extra_info = QVector4D(0.0f, 0.0f, 2, 3);       // alpha limit, div, pos1, pos2
    neu3.render_type = QVector4D(0, 1, 0, 0);


    // TODO: refactor this and make it easier to construct the space with fewer parameters
    // TODO: connect abstraction space buffer update data with this one
    // find a place to decide on the data and group them together instead on seperate classes


    // init rects
    float p10 = 20.0/100.0;
    float p20 = 20.0/100.0;
    float p30 = 30.0/100.0;
    float p40 = 40.0/100.0;
    float p50 = 50.0/100.0;
    float p80 = 80.0/100.0;
    float p100 = 100.0/100.0;

    int ID = 1;
    QVector2D p = QVector2D(0, 0);
    // x (0, 20)
    // y (0, 20)
    //m_IntervalXY.push_back({ast1, neu1});
    initRect(p, p20, p20, ID++); // 0

    // x (0, 20)
    // y (20, 40)
    p = QVector2D(0, p20);
    //m_IntervalXY.push_back({ast2, neu1});
    initRect(p, p20, p20, ID++); // 1

    // x (0, 20)
    // y (40, 80)
    p = QVector2D(0,  p40);
  //  m_IntervalXY.push_back({ast3, neu1});
    initRect(p, p20, p40, ID++); // 2

    // x (0, 20)
    // y (80, 100)
    p = QVector2D(0, p80);
  //  m_IntervalXY.push_back({ast4, neu1});
    initRect(p, p20, p20, ID++); // 3

    // x (20, 50)
    // y (0, 20)
    p = QVector2D(p20, 0);
  //  m_IntervalXY.push_back({ast1, neu2});
    initRect(p, p30, p20, ID++); // 4

    // x (50, 100)
    // y (0, 20)
    p = QVector2D(p50, 0);
 //   m_IntervalXY.push_back({ast1, neu3});
    initRect(p, p50, p20, ID++); // 5

    // x (20, 50)
    // y (20, 40)
    p = QVector2D(p20, p20);
  //  m_IntervalXY.push_back({ast2, neu2});
    initRect(p, p30, p20, ID++); // 6

    // x (20, 50)
    // y (40, 80)
    p = QVector2D(p20, p40);
  //  m_IntervalXY.push_back({ast3, neu2});
    initRect(p, p30, p40, ID++); // 7


    // x (20, 50)
    // y (80, 100)
    p = QVector2D(p20, p80);
  //  m_IntervalXY.push_back({ast4, neu2});
    initRect(p, p30, p20, ID++); // 8

    // x (50, 100)
    // y (20, 40)
    p = QVector2D(p50, p20);
   // m_IntervalXY.push_back({ast2, neu3});
    initRect(p, p50, p20, ID++); // 9


    // x (50, 100)
    // y (40, 80)
    p = QVector2D(p50, p40);
    //m_IntervalXY.push_back({ast3, neu3});
    initRect(p, p50, p40, ID++); // 10

    // x (50, 100)
    // y (80, 100)
    p = QVector2D(p50, p80);
   // m_IntervalXY.push_back({ast4, neu3});
    initRect(p, p50, p20, ID++); // 11

}

void MousePad::initBuffer()
{

    m_buffer_color_data.push_back(red);
    m_buffer_color_data.push_back(orange);
    m_buffer_color_data.push_back(blueviolet);
    m_buffer_color_data.push_back(steelblue);
    m_buffer_color_data.push_back(seagreen);
    m_buffer_color_data.push_back(brown);
    m_buffer_color_data.push_back(violet);
    m_buffer_color_data.push_back(peru);
    m_buffer_color_data.push_back(mediumspringgreen);
    m_buffer_color_data.push_back(gainsboro);
    m_buffer_color_data.push_back(honeydew);
    m_buffer_color_data.push_back(darkkhaki);

    int bufferSize =  m_buffer_color_data.size() * sizeof(QVector4D);

    glGenBuffers(1, &m_buffer_color);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_buffer_color);
    glBufferData(GL_SHADER_STORAGE_BUFFER, bufferSize , NULL, GL_DYNAMIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, m_bindColorIdx, m_buffer_color);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_buffer_color);
    GLvoid* p = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_WRITE_ONLY);
    memcpy(p,   m_buffer_color_data.data(),  bufferSize);
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

    /* init selection buffer */
    // texture
    glActiveTexture(GL_TEXTURE1);
    glGenTextures(1, &m_fbo_texture);
    glBindTexture(GL_TEXTURE_2D, m_fbo_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_w, m_h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Depth buffer
    glGenRenderbuffers(1, &m_rbo_depth);
    glBindRenderbuffer(GL_RENDERBUFFER, m_rbo_depth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, m_w, m_h);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    // Framebuffer to link everything together
    glGenFramebuffers(1, &m_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_fbo_texture, 0); // attach 2D tex image to fbo
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_rbo_depth);
    GLenum status;
    if ((status = glCheckFramebufferStatus(GL_FRAMEBUFFER)) != GL_FRAMEBUFFER_COMPLETE) {
      fprintf(stderr, "glCheckFramebufferStatus: error %p", status);
      return;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    /* init_resources */
      GLfloat fbo_vertices[] = {
        -1, -1,
         1, -1,
        -1,  1,
         1,  1,
      };
      glGenBuffers(1, &m_vbo_fbo_vertices);
      glBindBuffer(GL_ARRAY_BUFFER, m_vbo_fbo_vertices);
      glBufferData(GL_ARRAY_BUFFER, sizeof(fbo_vertices), fbo_vertices, GL_STATIC_DRAW);
      glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void MousePad::init2DSpaceGL()
{
    m_program_2DSpace = glCreateProgram();
    bool res = initShader(m_program_2DSpace, ":/shaders/space2d.vert", ":/shaders/space2d.geom",
                          ":/shaders/space2d.frag");
    if(res == false)
        return;

    glUseProgram(m_program_2DSpace);
    qDebug() << "init buffers";
    // create vbos and vaos
    m_vao_2DSpace.create();
    m_vao_2DSpace.bind();

    m_vbo_2DSpaceVerts.create();
    m_vbo_2DSpaceVerts.setUsagePattern( QOpenGLBuffer::DynamicDraw);
    m_vbo_2DSpaceVerts.bind();

    m_vbo_2DSpaceVerts.allocate( m_vertices.data(), m_vertices.size() * sizeof(QVector3D) );

    m_vbo_2DSpaceIndix.create();
    m_vbo_2DSpaceIndix.bind();
    m_vbo_2DSpaceIndix.allocate( m_indices.data(), m_indices.size() * sizeof(GLuint) );


    int offset = 0;
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(struct abstractionPoint),  0);

    offset += sizeof(QVector2D);
    glEnableVertexAttribArray(1);
    glVertexAttribIPointer(1, 1, GL_INT, sizeof(struct abstractionPoint), (GLvoid*)offset);

    int isSelectionLoc = glGetUniformLocation(m_program_2DSpace, "is_selection_shader");
    int isSelection = 0;
    glUniform1i(isSelectionLoc, isSelection);


    m_vbo_2DSpaceIndix.release();
    m_vbo_2DSpaceVerts.release();
    m_vao_2DSpace.release();
    GL_Error();

    /* selection buffer */
    m_program_2DSpace_Selection  = glCreateProgram();
    res = initShader(m_program_2DSpace_Selection, ":/shaders/space2d.vert", ":/shaders/space2d.geom",
                     ":/shaders/space2d.frag");
    if(res == false)
        return;

    glUseProgram(m_program_2DSpace_Selection);

    // create vbos and vaos
    m_vao_2DSpace_Selection.create();
    m_vao_2DSpace_Selection.bind();
    m_vbo_2DSpaceVerts.bind();

    offset = 0;
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(struct abstractionPoint),  0);

    offset += sizeof(QVector2D);
    glEnableVertexAttribArray(1);
    glVertexAttribIPointer(1, 1, GL_INT, sizeof(struct abstractionPoint), (GLvoid*)offset);


    isSelectionLoc = glGetUniformLocation(m_program_2DSpace_Selection, "is_selection_shader");
    isSelection = 1;
    glUniform1i(isSelectionLoc, isSelection);

    m_vbo_2DSpaceVerts.release();
    m_vao_2DSpace_Selection.release();
}

void MousePad::initializeGL()
{
    qDebug() << "MousePad::initializeGL()";
    initializeOpenGLFunctions();
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    qDebug() << "MousePad::initData()";
    initData();
    GL_Error();

    qDebug() << "MousePad::initBuffer()";
    initBuffer();
    GL_Error();

    qDebug() << "MousePad::initSelectionPointerGL()";
    initSelectionPointerGL();
    GL_Error();

    qDebug() << "MousePad::initializeGL()";
    init2DSpaceGL();

    qDebug() << "Widget OpenGl: " << format().majorVersion() << "." << format().minorVersion();
    qDebug() << "Context valid: " << context()->isValid();
    qDebug() << "Really used OpenGl: " << context()->format().majorVersion() << "." << context()->format().minorVersion();
    qDebug() << "OpenGl information: VENDOR:       " << (const char*)glGetString(GL_VENDOR);
    qDebug() << "                    RENDERDER:    " << (const char*)glGetString(GL_RENDERER);
    qDebug() << "                    VERSION:      " << (const char*)glGetString(GL_VERSION);
    qDebug() << "                    GLSL VERSION: " << (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);

    m_projection.setToIdentity();

    glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
}

void MousePad::paintGL()
{
    glViewport( 0, 0, m_w, m_h);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_vao_circle.bind();
    m_program_circle->bind();
    m_program_circle->setUniformValue("pMatrix", m_projection);
    glDrawArrays(GL_POINTS, 0, 1);
    m_program_circle->release();
    m_vao_circle.release();

    m_vao_2DSpace.bind();
    m_vbo_2DSpaceIndix.bind();
    glUseProgram(m_program_2DSpace);

    GLuint pMatrix = glGetUniformLocation(m_program_2DSpace, "pMatrix");
    glUniformMatrix4fv(pMatrix, 1, GL_FALSE, m_projection.data());

    glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, 0);

    m_vbo_2DSpaceIndix.release();
    m_vao_2DSpace.release();

    if (m_updatedPointer == false)
        return;

    qDebug() << "HERER!";
    m_updatedPointer = false;
}

void MousePad::resizeGL(int w, int h)
{
    qDebug() << "Func: resizeGL: " << w << " " <<   width() <<  " " << h << " " << height();
    // Calculate aspect ratio
    const qreal retinaScale = devicePixelRatio();

    m_h = h * retinaScale;
    m_w = w * retinaScale;
    h = (h == 0) ? 1 : h;

    glViewport(0, 0, w * retinaScale, h * retinaScale);
    m_projection.setToIdentity();
    m_projection.ortho( 0.0f,  1.0f, 0.0f, 1.0f, -1.0, 1.0 );

    /* onReshape */
    // Rescale FBO and RBO as well
    glBindTexture(GL_TEXTURE_2D, m_fbo_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_w, m_h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glBindTexture(GL_TEXTURE_2D, 0);

    glBindRenderbuffer(GL_RENDERBUFFER, m_rbo_depth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, m_w, m_h);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    update();
}

QSize MousePad::minimumSizeHint() const
{
    return QSize(200, 200);
}
QSize MousePad::sizeHint() const
{
    return QSize(400, 400);
}

void MousePad::mouseMoveEvent(QMouseEvent *event)
{
    //qDebug() << "Func: mouseMoveEvent: " << event->pos().x() << " " << event->pos().y();
    // here update the mouse position as we move!


    makeCurrent();
    const qreal retinaScale = devicePixelRatio();
    GLint viewport[4]; //  return of glGetIntegerv() -> x, y, width, height of viewport
    glGetIntegerv(GL_VIEWPORT, viewport);

    int x = event->x() * retinaScale;
    int y = viewport[3] - event->y() * retinaScale;

    doneCurrent();
    // calculate the offset from press to release, then update the point position
    // get the position were we pressed
    processSelection(x, y);
}


void MousePad::mousePressEvent(QMouseEvent *event)
{
    //qDebug() << "Func: mousePressEvent: " << event->pos().x() << " " << event->pos().y();
    setFocus();
    event->accept();
}

void MousePad::mouseReleaseEvent(QMouseEvent *event)
{
    //qDebug() << "Func: mouseReleaseEvent " <<  event->x() << " " << event->y();
    setFocus();

    event->accept();
}

void MousePad::setSlotsX(int value)
{
    if (m_x == value)
        return;

    m_x = value;
    // minimum = 0, maximum = 99
    emit setSignalX(value);
    m_vbo_circle.bind();
    circle.x = (float)value/100.0;
    GLfloat points[] = { circle.x,  circle.y };
    m_vbo_circle.allocate(points, 1 /*elements*/ * 2 /*corrdinates*/ * sizeof(GLfloat));
    m_vbo_circle.release();
    m_updatedPointer = true;
    update();

}


void MousePad::setSlotsY(int value)
{
    if (m_y == value)
        return;

    m_y = value;
    // minimum = 0, maximum = 99
    emit setSignalY(value);
    m_vbo_circle.bind();
    circle.y = (float)(value)/100.0;
    GLfloat points[] = { circle.x,  circle.y };
    m_vbo_circle.allocate(points, 1 /*elements*/ * 2 /*corrdinates*/ * sizeof(GLfloat));
    m_vbo_circle.release();
    m_updatedPointer = true;
    update();
}

void MousePad::renderSelection(void)
{
    aboutToCompose();
    qDebug() << "Draw Selection!";
  //  glBindFramebuffer(GL_FRAMEBUFFER, m_selectionFrameBuffer);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_DITHER);
    m_vao_2DSpace.bind();
    m_vbo_2DSpaceIndix.bind();
    glUseProgram(m_program_2DSpace_Selection);

    GLuint pMatrix = glGetUniformLocation(m_program_2DSpace_Selection, "pMatrix");
    glUniformMatrix4fv(pMatrix, 1, GL_FALSE, m_projection.data());

    glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, 0);

    m_vbo_2DSpaceIndix.release();
    m_vao_2DSpace.release();


    m_vao_selection.bind();
    m_program_selection->bind();
    m_program_selection->setUniformValue("pMatrix",  m_projection);

    // set the uniform with the appropriate color code
    glDrawArrays(GL_POINTS, 0, 1);

    m_program_selection->release();
    m_vao_selection.release();

    glEnable(GL_DITHER);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
}

void MousePad::processSelection(float x, float y)
{
    makeCurrent();
    renderSelection();
    // wait until all the pending drawing commands are really done.
    // slow operation, there are usually a long time between glDrawElements() and all the fragments completely radterized
    glFlush();
    glFinish();

    // 4 bytes per pixel (RGBA), 1x1 bitmap
    // width * height * components (RGBA)
    unsigned char res[4];
    //qDebug() << "Pixel at: " << x << " " << y;
    glReadBuffer(GL_BACK);
    glReadPixels(x,  y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &res);
    int pickedID = res[0] + res[1] * 256 + res[2] * 256 * 256;

    //qDebug() <<  res[0] <<  " " << res[1] <<  " " << res[2];

    if (pickedID <= 0) {
        qDebug() << "Background, Picked ID: " << pickedID;
    } else { 
        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);

        circle.x = x/viewport[2];
        circle.y = y/viewport[3];

        m_vbo_circle.bind();
        GLfloat points[] = { circle.x,  circle.y };
        m_vbo_circle.allocate(points, 1 /*elements*/ * 2 /*corrdinates*/ * sizeof(GLfloat));
        m_vbo_circle.release();

        emit setSliderX(circle.x * 100);
        emit setSliderY(circle.y * 100);
        emit setIntervalID(pickedID - 1);
        //qDebug() << "Picked ID: " << pickedID << "-> " << circle.x << " " << circle.y;
    }

    // update the circle vbo
    update();
    doneCurrent();
}
