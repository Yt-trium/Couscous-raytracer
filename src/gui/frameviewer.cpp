// Interface.
#include "frameviewer.h"

// QT headers.
#include <QColor>
#include <QDateTime>
#include <Qt>

// Standard headers.
#include <algorithm>

using namespace std;

FrameViewer::FrameViewer(
    const size_t    width,
    const size_t    height,
    QWidget*        parent)
  : QWidget(parent)
  , m_left_pressed(false)
  , m_time_last_mouse_press(0)
  , m_scale_factor(1.0f)
  , m_translate_x(0)
  , m_translate_y(0)
{
    setFocusPolicy(Qt::StrongFocus);
    setAutoFillBackground(false);
    setMouseTracking(true);

    resize(width, height);
}

void FrameViewer::resize(
    const size_t    width,
    const size_t    height)
{
    m_image = QImage(
        static_cast<int>(width),
        static_cast<int>(height),
        QImage::Format_RGB888);

    clear();
}

void FrameViewer::clear()
{
    m_image.fill(QColor(0, 0, 0));
}

void FrameViewer::on_render_begin(const size_t width, const size_t height)
{
    resize(width, height);
}

void FrameViewer::on_render_end(const QImage& image)
{
    m_image = image.copy();
    repaint();
}

void FrameViewer::fit_to_viewport()
{
    const float x_scale =
        width() / static_cast<float>(m_image.width());
    const float y_scale =
        height() / static_cast<float>(m_image.height());
    m_scale_factor = min(x_scale, y_scale);

    repaint();
}

void FrameViewer::reset_transform()
{
    m_scale_factor = 1.0f;
    m_translate_x = 0;
    m_translate_y = 0;
}

void FrameViewer::translate_viewport(const int dx, const int dy)
{
    m_translate_x += dx;
    m_translate_y += dy;
    repaint();
}

void FrameViewer::zoom_viewport(const float delta)
{
    m_scale_factor += delta;
    m_scale_factor = max(m_scale_factor, 0.1f);
    m_scale_factor = min(m_scale_factor, 5.0f);
    repaint();
}

void FrameViewer::mouseMoveEvent(QMouseEvent* event)
{
    if (event->modifiers() & Qt::ShiftModifier && m_left_pressed)
    {
        const QPoint& pos = event->pos();

        translate_viewport(
            pos.x() - m_last_mouse_pos.x(),
            pos.y() - m_last_mouse_pos.y());

        m_last_mouse_pos = pos;
    }
}

void FrameViewer::mousePressEvent(QMouseEvent* event)
{
    if (event->modifiers() & Qt::ShiftModifier)
    {
        const qint64 msec = QDateTime::currentMSecsSinceEpoch();

        if (event->button() == Qt::LeftButton)
        {
            m_left_pressed = true;
            m_last_mouse_pos = event->pos();
        }
        else if (event->button() == Qt::RightButton)
        {
            // Reset transform.
            reset_transform();

            // Fit image to viewport if button is pressed 2 times.
            if (msec - m_time_last_mouse_press <= 500)
            {
                fit_to_viewport();
            }
            else
            {
                repaint();
            }
        }

        m_time_last_mouse_press = msec;
    }
}

void FrameViewer::mouseReleaseEvent(QMouseEvent* event)
{
    m_left_pressed = false;
}

void FrameViewer::paintEvent(QPaintEvent* event)
{
    // Create transform matrix.
    // Place image's center at (0, 0)
    QTransform center_image;
    center_image.translate(-m_image.width() / 2, -m_image.height() / 2);

    // Apply scale.
    QTransform scale_image;
    scale_image.scale(m_scale_factor, m_scale_factor);

    // Place image at the center of the widget
    // and apply translation.
    QTransform translate_image;
    translate_image.translate(width() / 2, height() / 2);
    translate_image.translate(m_translate_x, m_translate_y);

    QTransform transform = center_image * scale_image * translate_image;

    // Draw the image.
    m_painter.begin(this);
    m_painter.setTransform(transform);
    m_painter.drawImage(QPoint(0, 0), m_image);
    m_painter.end();
}

void FrameViewer::wheelEvent(QWheelEvent* event)
{
    if (event->modifiers() & Qt::ShiftModifier)
    {
        if (event->orientation() == Qt::Vertical)
        {
            zoom_viewport(
                static_cast<float>(event->delta()) / 12000.0f);
        }
    }
}

