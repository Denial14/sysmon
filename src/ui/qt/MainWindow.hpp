#pragma once
#include <QMainWindow>
#include "../../core/monitor.hpp"
#include <vector>

class QTableView;
class QTimer;
class QLabel;
class QSortFilterProxyModel;
class QPushButton;
class QTabWidget;

namespace sysmon::ui::qt {

class ProcessTableModel;
class GraphWidget;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override = default;

private slots:
    void updateSnapshot();
    void killSelectedProcess();

private:
    void buildProcessesTab(QTabWidget* tabs);
    void buildGraphsTab(QTabWidget* tabs);

    core::Monitor monitor_;

    QLabel* cpu_label_ = nullptr;
    QLabel* mem_label_ = nullptr;
    QLabel* proc_label_ = nullptr;
    QPushButton* kill_button_ = nullptr;

    QTabWidget* tabs_ = nullptr;

    QTableView* table_ = nullptr;
    ProcessTableModel* model_ = nullptr;
    QSortFilterProxyModel* proxy_ = nullptr;

    GraphWidget* cpu_graph_ = nullptr;
    GraphWidget* mem_graph_ = nullptr;
    std::vector<GraphWidget*> core_graphs_;

    QTimer* timer_ = nullptr;
};

}