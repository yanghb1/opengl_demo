#include "glwidget.h"

#include <QFile>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QDateTime>
#include <QtMath>
#include <QDebug>

GLWidget::GLWidget() {}

void GLWidget::SetFilePath(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Cannot open file:" << file.errorString();
        return;
    }

    QTextStream in(&file);
    QString sixthLine;

    // 读取文件的前六行
    for (int i = 0; i < 6 && !in.atEnd(); ++i) {
        sixthLine = in.readLine();
    }

    if (sixthLine.isEmpty()) {
        qDebug() << "The sixth line is empty or does not exist.";
        return;
    }

    // 按逗号分割第六行内容
    QStringList elements = sixthLine.split(',');
    // 查找 "INS_Odom_X_Pos" 的下标
    int index_x = elements.indexOf("INS_Odom_X_Pos") - 1;
    int index_y = elements.indexOf("INS_Odom_Y_Pos") - 1;
    int index_timestamp = elements.indexOf("timestamp");
    int index_VehSpd = elements.indexOf("VehSpd") - 1;
    int index_ACUYawRate = elements.indexOf("ACUYawRate") - 1;

    double angle = 0;
    qint64 lastTimestamp = -1;

    //处理空行
    in.readLine();
    if(in.atEnd())
        return;

    QWriteLocker locker(&readWriteLock);
    points.clear();
    points2.clear();
    points3.clear();
    int point2_index = 0;
    points2.append(QVector2D(0,0));

    double dis_x = 0;

    bool point_3_init = false;
    bool point_3_init2 = false;
    double piont_3_x = 0;
    double piont_3_y = 0;
    double piont_3_angle = 0;
    points3.append(QVector2D(0,0));

    int count_point_3 = 0;

    QString content = in.readLine();
    while (!in.atEnd()) {
        QStringList values = content.split(',');

        float point_x = values.at(index_x).toFloat();
        float point_y = values.at(index_y).toFloat();

        {
            if(point_3_init && point_3_init2){
                double distance = std::sqrt(std::pow(point_x - piont_3_x, 2) + std::pow(point_y - piont_3_y, 2));
                float angle = piont_3_angle - qAtan((point_y - piont_3_y) / (point_x - piont_3_x));
                QVector2D point;
                point.setX(qSin(angle) * distance);
                point.setY(qCos(angle) * distance);
                points3.append(point);
            }
        }

        {
            QVector2D point;
            point.setX(point_x);
            point.setY(point_y);
            points.append(point);

            if(! point_3_init){
                piont_3_x = point_x;
                piont_3_y = point_y;
                point_3_init = true;
            }
            if(++count_point_3 == m_startIndex){
                piont_3_angle = qAtan((point_y - piont_3_y) / (point_x-piont_3_x));
                point_3_init2 = true;
            }
        }

        //-------------------------------------------------------

        {
            QVector2D point2;
            QString dateTimeStr = values.at(index_timestamp).split(" ").at(0)
                                  + " "
                                  + values.at(index_timestamp).split(" ").at(1);

            QDateTime dateTime = QDateTime::fromString(dateTimeStr, "yyyy-MM-dd HH:mm:ss.zzz");
            // 转换为 Unix 时间戳（毫秒）
            qint64 timestamp = dateTime.toMSecsSinceEpoch();
            if(lastTimestamp != -1){
                qint64 time = timestamp - lastTimestamp;
                // qDebug() << "time : " << time;

                double distance = values.at(index_VehSpd).toFloat() * 5.0 / 18.0 * time / 1000.0;
                angle += values.at(index_ACUYawRate).toFloat() * time / 1000.0;

                // qDebug() << "distance : " << distance;
                // qDebug() << "angle : " << angle;
                float x, y;
                coordinate_convert(x, y, points2.at(point2_index-1).x(), points2.at(point2_index-1).y(), distance, angle);
                point2.setX(x);
                point2.setY(y);
                points2.append(point2);

                if(values.at(index_ACUYawRate).toFloat() > 0){
                    dis_x += distance * qSin(qDegreesToRadians(angle));
                }
                if(values.at(index_ACUYawRate).toFloat() < 0){
                    dis_x -= distance * qSin(qDegreesToRadians(angle));
                }
            }
            lastTimestamp = timestamp;
            ++point2_index;
        }



        content = in.readLine();
    }

    auto dis_1 = std::sqrt(std::pow(points.last().x() - points.first().x(), 2) + std::pow(points.last().y() - points.first().y(), 2));
    auto dis_2 = std::sqrt(std::pow(points2.last().x() - points2.first().x(), 2) + std::pow(points2.last().y() - points2.first().y(), 2));

    // for(int i = 0; i < points2.size(); ++i){
    //     qDebug() << points2.at(i).x() - points3.at(i).x();
    //     qDebug() << points2.at(i).y() - points3.at(i).y();
    //     qDebug() << "---------------";
    // }
}

void GLWidget::SetScalef(float scale)
{
    m_scale = scale;

    update();
}

void GLWidget::SetStartIndex(int index)
{
    m_startIndex = index;
}

void GLWidget::initializeGL()
{
    initializeOpenGLFunctions();
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // 背景色
}

void GLWidget::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
}

void GLWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity(); // 重置模型视图矩阵

    QReadLocker locker(&readWriteLock);
    if(points.empty())
        return;

    glScalef(m_scale, m_scale, m_scale);
    glTranslatef( m_translateX, - m_translateY, 0);

    if(true){
        glPushMatrix();
        glTranslatef(-points.at(0).x(), -points.at(0).y(), 0);
        glBegin(GL_POINTS);

        glColor3f(1.0f, 0.0f, 0.0f); // 红色
        glLineWidth(2.0f); // 设置线宽为 5 像素
        glPointSize(1.0f);

        for(int i = 0; i < points.size(); ++i){
            glVertex2f(points.at(i).x(), points.at(i).y());
        }

        glEnd();
        glPopMatrix();
    }

    if(true){
        glPushMatrix();
        glTranslatef(-points2.at(0).x(), -points2.at(0).y(), 0);
        glRotatef(-m_rotate, 0, 0, 1);
        glBegin(GL_POINTS);

        glColor3f(0.0f, 1.0f, 0.0f); // 绿色
        glLineWidth(2.0f); // 设置线宽为 5 像素
        glPointSize(1.0f);

        for(int i = 0; i < points2.size(); ++i){
            glVertex2f(points2.at(i).x(), points2.at(i).y());
        }

        glEnd();
        glPopMatrix();
    }
    {
        glPushMatrix();
        glBegin(GL_POINTS);

        glColor3f(0.0f, 0.0f, 1.0f); // 红色
        glLineWidth(2.0f); // 设置线宽为 5 像素
        glPointSize(1.0f);

        for(int i = 0; i < points3.size(); ++i){
            glVertex2f(points3.at(i).x(), points3.at(i).y());
        }

        glEnd();
        glPopMatrix();
    }

}

void GLWidget::wheelEvent(QWheelEvent *event) {
    int numDegrees = event->angleDelta().y() / 8;
    int numSteps = numDegrees / 15;
    m_scale += numSteps * 0.0005f;

    // Ensure scale is positive
    if (m_scale < 0.0005f) {
        m_scale = 0.0005f;
    }
    update();
}

void GLWidget::mousePressEvent(QMouseEvent *event)
{
    m_lastX = event->x();
    m_lastY = event->y();

    m_dragging = event->button() == Qt::LeftButton ? 1 : 2;
}

void GLWidget::mouseReleaseEvent(QMouseEvent *event)
{
    m_dragging = 0;
}

void GLWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (! m_dragging)
        return;

    int dx = event->x() - m_lastX;
    int dy = event->y() - m_lastY;

    if(m_dragging == 1)
    {
        m_translateX += dx * 0.2f;
        m_translateY += dy * 0.2f;
    }else if(m_dragging == 2){
        m_rotate += dx * 0.2;
    }

    m_lastX = event->x();
    m_lastY = event->y();


    update();
}

void GLWidget::coordinate_convert(float &x, float &y, float last_x, float last_y, double distance, double angle){
    x = last_x + distance * qSin(qDegreesToRadians(angle));
    y = last_y + distance * qCos(qDegreesToRadians(angle));
}
