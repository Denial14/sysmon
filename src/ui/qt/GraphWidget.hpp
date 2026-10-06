#pragma once
#include <QWidget>
#include <QString>
#include "../../core/ring_buffer.hpp"

namespace sysmon::ui::qt {

class GraphWidget : public QWidget {
    Q_OBJECT

public:
    using Buffer = core::RingBuffer<double, 60>;

    explicit GraphWidget(const QString& title,
                         const Buffer* buffer,
                         double max_value = 100.0,
                         QWidget* parent = nullptr);

    void setBuffer(const Buffer* buffer);

    void setMaxValue(double max_value);

    void setAutoScale(bool enable);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    QString title_;
    const Buffer* buffer_ = nullptr;
    double max_value_ = 100.0;
    bool auto_scale_ = false;
};

}