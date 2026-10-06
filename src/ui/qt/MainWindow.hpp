#pragma once
#include <QMainWindow>
#include "../../core/monitor.hpp"

class QTableView;
class QTimer;
class QLabel;
class QSortFilterProxyModel;
class QPushButton;

namespace sysmon::ui::qt {

class ProcessTableModel;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override = default;

private slots:
    void updateSnapshot();
    void killSelectedProcess();

private:
    core::Monitor monitor_;

    QLabel* cpu_label_ = nullptr;
    QLabel* mem_label_ = nullptr;
    QLabel* proc_label_ = nullptr;
    QPushButton* kill_button_ = nullptr;

    QTableView* table_ = nullptr;
    ProcessTableModel* model_ = nullptr;
    QSortFilterProxyModel* proxy_ = nullptr;
    QTimer* timer_ = nullptr;
};

}