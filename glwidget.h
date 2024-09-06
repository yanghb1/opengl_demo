#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QVector2D>
#include <QReadWriteLock>

class GLWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT
public:
    GLWidget();

    void SetFilePath(const QString &filePath);
    void SetScalef(float scale);
    void SetStartIndex(int index);

protected:
    void initializeGL();

    void resizeGL(int w, int h);

    void paintGL();

    void wheelEvent(QWheelEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    void coordinate_convert(float &x, float &y, float last_x, float last_y, double distance, double angle);

private:
    QVector<QVector2D> points;
    QVector<QVector2D> points2;
    QVector<QVector2D> points3;
    QReadWriteLock readWriteLock;

    GLfloat m_scale = 0.005f;

    int m_dragging = 0; // 1:左键 2:右键
    int m_lastX = 0;
    int m_lastY = 0;
    GLfloat m_translateX = 0;
    GLfloat m_translateY = 0;
    GLfloat m_rotate = 0; //角度

    int m_startIndex = 30;
};

#endif // GLWIDGET_H
