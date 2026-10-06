#include "GraphWidget.hpp"
#include <QPainter>
#include <QPainterPath>
#include <QPaintEvent>
#include <algorithm>
#include <cmath>

namespace sysmon::ui::qt {

GraphWidget::GraphWidget(const QString& title,
                         const Buffer* buffer,
                         double max_value,
                         QWidget* parent)
    : QWidget(parent)
    , title_(title)
    , buffer_(buffer)
    , max_value_(max_value) {
    setMinimumHeight(80);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void GraphWidget::setBuffer(const Buffer* buffer) {
    buffer_ = buffer;
    update();
}

void GraphWidget::setMaxValue(double max_value) {
    max_value_ = max_value;
    update();
}

void GraphWidget::setAutoScale(bool enable) {
    auto_scale_ = enable;
    update();
}

void GraphWidget::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    const int w = width();
    const int h = height();

    p.fillRect(rect(), QColor(30, 30, 30));

    p.setPen(QPen(QColor(60, 60, 60), 1));
    p.drawRect(0, 0, w - 1, h - 1);

    p.setPen(QColor(220, 220, 220));
    QFont title_font = p.font();
    title_font.setPointSize(9);
    title_font.setBold(true);
    p.setFont(title_font);
    p.drawText(8, 15, title_);

    if (!buffer_ || buffer_->size() < 2) {
        p.setPen(QColor(120, 120, 120));
        QFont hint_font = p.font();
        hint_font.setBold(false);
        p.setFont(hint_font);
        p.drawText(rect(), Qt::AlignCenter, "waiting for data...");
        return;
    }

    double max_val = max_value_;
    if (auto_scale_) {
        double bmax = buffer_->max();
        if (bmax < 10.0)  max_val = 10.0;
        else if (bmax < 25.0)  max_val = 25.0;
        else if (bmax < 50.0)  max_val = 50.0;
        else if (bmax < 75.0)  max_val = 75.0;
        else max_val = 100.0;
    }
    if (max_val <= 0.0) max_val = 1.0;

    const int margin_top = 20;
    const int margin_bottom = 4;
    const int margin_x = 4;
    const int plot_w = w - 2 * margin_x;
    const int plot_h = h - margin_top - margin_bottom;
    const int plot_x = margin_x;
    const int plot_y = margin_top;

    p.setPen(QPen(QColor(60, 60, 60), 1, Qt::DotLine));
    for (int i = 1; i <= 3; ++i) {
        int y = plot_y + (plot_h * i) / 4;
        p.drawLine(plot_x, y, plot_x + plot_w, y);
    }

    QPainterPath path;
    QPainterPath fill_path;

    const std::size_t n = buffer_->size();
    double step_x = (n > 1) ? static_cast<double>(plot_w) / (n - 1) : 0.0;

    for (std::size_t i = 0; i < n; ++i) {
        double v = buffer_->at(i);
        if (v < 0) v = 0;
        if (v > max_val) v = max_val;

        double x = plot_x + i * step_x;
        double y = plot_y + plot_h * (1.0 - v / max_val);

        if (i == 0) {
            path.moveTo(x, y);
            fill_path.moveTo(x, plot_y + plot_h);
            fill_path.lineTo(x, y);
        } else {
            path.lineTo(x, y);
            fill_path.lineTo(x, y);
        }
    }

    fill_path.lineTo(plot_x + plot_w, plot_y + plot_h);
    fill_path.closeSubpath();
    p.fillPath(fill_path, QColor(80, 160, 255, 60));

    p.setPen(QPen(QColor(80, 160, 255), 2));
    p.drawPath(path);

    double latest = buffer_->latest();
    p.setPen(QColor(220, 220, 220));
    QFont val_font = p.font();
    val_font.setBold(true);
    val_font.setPointSize(10);
    p.setFont(val_font);

    QString val_text = QString::number(latest, 'f', 1) + " %";
    int text_w = p.fontMetrics().horizontalAdvance(val_text);
    p.drawText(w - text_w - 10, 15, val_text);
}

}