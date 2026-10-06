#include "ProcessTableModel.hpp"
#include <QColor>

namespace sysmon::ui::qt {

ProcessTableModel::ProcessTableModel(QObject* parent)
    : QAbstractTableModel(parent) {}

int ProcessTableModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) return 0;
    return static_cast<int>(snapshot_.processes.size());
}

int ProcessTableModel::columnCount(const QModelIndex& parent) const {
    if (parent.isValid()) return 0;
    return 6;
}

QVariant ProcessTableModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid()) return {};

    int row = index.row();
    int col = index.column();

    if (row < 0 || row >= static_cast<int>(snapshot_.processes.size()))
        return {};

    const auto& p = snapshot_.processes[row];

    if (role == Qt::DisplayRole) {
        switch (col) {
            case 0: return p.pid;
            case 1: return QString::fromStdString(p.name);
            case 2: return QString::number(p.cpu_percent, 'f', 1);
            case 3: return QString::number(p.memory_percent, 'f', 1);
            case 4: return QString::number(p.rss_bytes / (1024 * 1024));
            case 5: return QString(QChar(p.state));
        }
    }

    if (role == Qt::TextAlignmentRole) {
        switch (col) {
            case 0:
            case 2:
            case 3:
            case 4:
                return int(Qt::AlignRight | Qt::AlignVCenter);
            default:
                return int(Qt::AlignLeft | Qt::AlignVCenter);
        }
    }

    if (role == Qt::ForegroundRole && col == 2) {
        if (p.cpu_percent > 50.0)      return QColor(Qt::red);
        else if (p.cpu_percent > 20.0) return QColor(255, 140, 0);  // orange
        else if (p.cpu_percent > 5.0)  return QColor(200, 200, 0);  // yellow
    }

    if (role == Qt::UserRole) {
        switch (col) {
            case 0: return p.pid;
            case 1: return QString::fromStdString(p.name);
            case 2: return p.cpu_percent;
            case 3: return p.memory_percent;
            case 4: return static_cast<qulonglong>(p.rss_bytes);
            case 5: return p.state;
        }
    }

    return {};
}

QVariant ProcessTableModel::headerData(int section, Qt::Orientation orientation,
                                       int role) const {
    if (role != Qt::DisplayRole) return {};

    if (orientation == Qt::Horizontal) {
        switch (section) {
            case 0: return "PID";
            case 1: return "Name";
            case 2: return "CPU %";
            case 3: return "MEM %";
            case 4: return "RSS (MB)";
            case 5: return "State";
        }
    } else {
        return section + 1;
    }
    return {};
}

void ProcessTableModel::setSnapshot(const core::Snapshot& snap) {
    beginResetModel();
    snapshot_ = snap;
    endResetModel();
}

const core::ProcessMetrics* ProcessTableModel::processAt(int row) const {
    if (row < 0 || row >= static_cast<int>(snapshot_.processes.size()))
        return nullptr;
    return &snapshot_.processes[row];
}

}