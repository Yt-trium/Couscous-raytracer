#ifndef RENDERIMG_H
#define RENDERIMG_H

// QT headers.
#include <QImage>
#include <QMouseEvent>
#include <QPainter>
#include <QPaintEvent>
#include <QTransform>
#include <QWheelEvent>
#include <QWidget>

// Widget that display a 2D image.
class FrameViewer : public QWidget
{
    Q_OBJECT

  public:
    FrameViewer(
        const size_t    width,
        const size_t    heigth,
        QWidget*        parent = nullptr);

    void resize(
        const size_t    width,
        const size_t    height);

    void clear();

    // Clear the frame.
    void on_render_begin();

    // Display the given image.
    void on_render_end(const QImage& image);

  public slots:
    void fit_to_viewport();
    void reset_transform();
    void translate_viewport(const int dx, const int dy);
    void zoom_viewport(const float delta);

  private:
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

    QImage      m_image;
    QPainter    m_painter;
    bool        m_left_pressed;
    QPoint      m_last_mouse_pos;
    qint64      m_time_last_mouse_press;
    float       m_scale_factor;
    int         m_translate_x;
    int         m_translate_y;
};

#endif // RENDERIMG_H

