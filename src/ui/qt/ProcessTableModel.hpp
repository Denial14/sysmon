#pragma once
#include <QAbstractTableModel>
#include "../../core/snapshot.hpp"

namespace sysmon::ui::qt {

class ProcessTableModel : public QAbstractTableModel {
    Q_OBJECT

public:
    explicit ProcessTableModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = {}) const override;
    int columnCount(const QModelIndex& parent = {}) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role) const override;

    void setSnapshot(const core::Snapshot& snap);

    const core::ProcessMetrics* processAt(int row) const;

private:
    core::Snapshot snapshot_;
};

}