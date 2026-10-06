#include "MainWindow.hpp"
#include "ProcessTableModel.hpp"
#include "GraphWidget.hpp"
#include <QTableView>
#include <QTimer>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QSortFilterProxyModel>
#include <QTabWidget>
#include <QMessageBox>
#include <QDebug>
#include <csignal>

namespace sysmon::ui::qt {

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent) {

    setWindowTitle("sysmon");
    resize(1200, 800);

    auto* central = new QWidget(this);
    auto* main_layout = new QVBoxLayout(central);

    auto* top_panel = new QHBoxLayout();

    QFont mono_font("monospace");
    mono_font.setPointSize(12);

    cpu_label_  = new QLabel("CPU: --", this);
    mem_label_  = new QLabel("RAM: --", this);
    proc_label_ = new QLabel("Processes: --", this);

    auto* sep1 = new QLabel("|", this);
    auto* sep2 = new QLabel("|", this);
    sep1->setStyleSheet("color: gray;");
    sep2->setStyleSheet("color: gray;");

    top_panel->addWidget(cpu_label_);
    top_panel->addWidget(sep1);
    top_panel->addWidget(mem_label_);
    top_panel->addWidget(sep2);
    top_panel->addWidget(proc_label_);
    top_panel->addStretch();
    top_panel->addWidget(kill_button_);

    cpu_label_->setFont(mono_font);
    mem_label_->setFont(mono_font);
    proc_label_->setFont(mono_font);

    kill_button_ = new QPushButton("Kill selected", this);
    connect(kill_button_, &QPushButton::clicked,
            this, &MainWindow::killSelectedProcess);

    top_panel->addWidget(cpu_label_);
    top_panel->addWidget(mem_label_);
    top_panel->addWidget(proc_label_);
    top_panel->addStretch();
    top_panel->addWidget(kill_button_);

    main_layout->addLayout(top_panel);

    tabs_ = new QTabWidget(this);
    buildProcessesTab(tabs_);
    buildGraphsTab(tabs_);

    connect(tabs_, &QTabWidget::currentChanged, this, [this](int index) {
        kill_button_->setVisible(index == 0);
    });

    main_layout->addWidget(tabs_);
    setCentralWidget(central);

    timer_ = new QTimer(this);
    connect(timer_, &QTimer::timeout, this, &MainWindow::updateSnapshot);
    timer_->start(1000);

    monitor_.take_snapshot();
    updateSnapshot();
}

void MainWindow::buildProcessesTab(QTabWidget* tabs) {
    auto* page = new QWidget(this);
    auto* layout = new QVBoxLayout(page);

    model_ = new ProcessTableModel(this);

    proxy_ = new QSortFilterProxyModel(this);
    proxy_->setSourceModel(model_);
    proxy_->setSortRole(Qt::UserRole);

    table_ = new QTableView(page);
    table_->setModel(proxy_);
    table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    table_->setSelectionMode(QAbstractItemView::SingleSelection);
    table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table_->setAlternatingRowColors(true);
    table_->setSortingEnabled(true);
    table_->verticalHeader()->setVisible(false);
    table_->horizontalHeader()->setStretchLastSection(true);
    table_->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);

    layout->addWidget(table_);
    tabs->addTab(page, "Процессы");

    table_->sortByColumn(2, Qt::DescendingOrder);
}

void MainWindow::buildGraphsTab(QTabWidget* tabs) {
    auto* page = new QWidget(this);
    auto* layout = new QGridLayout(page);

    cpu_graph_ = new GraphWidget("CPU", &monitor_.cpu_history(), 100.0, page);
    cpu_graph_->setAutoScale(true); 

    mem_graph_ = new GraphWidget("RAM", &monitor_.mem_history(), 100.0, page);
    mem_graph_->setAutoScale(false);

    layout->addWidget(cpu_graph_, 0, 0);
    layout->addWidget(mem_graph_, 0, 1);

    tabs->addTab(page, "Графики");
}

void MainWindow::updateSnapshot() {
    auto snap = monitor_.take_snapshot();
    model_->setSnapshot(snap);

    int col = table_->horizontalHeader()->sortIndicatorSection();
    Qt::SortOrder order = table_->horizontalHeader()->sortIndicatorOrder();
    table_->sortByColumn(col, order);

    cpu_label_->setText(QString("CPU: %1 %")
                            .arg(snap.cpu_total_percent, 0, 'f', 1));

    double mem_pct = 0.0;
    if (snap.mem_total_kb > 0) {
        mem_pct = 100.0 * snap.mem_used_kb / snap.mem_total_kb;
    }
    mem_label_->setText(QString("RAM: %1 / %2 MB (%3 %)")
                            .arg(snap.mem_used_kb / 1024)
                            .arg(snap.mem_total_kb / 1024)
                            .arg(mem_pct, 0, 'f', 1));

    proc_label_->setText(QString("Processes: %1")
                            .arg(snap.processes.size()));

    const auto& core_hist = monitor_.cores_history();
    if (core_graphs_.empty() && !core_hist.empty()) {
        QWidget* graphs_page = tabs_->widget(1);
        auto* grid = qobject_cast<QGridLayout*>(graphs_page->layout());
        if (grid) {
            const int cols = 3;
            for (std::size_t i = 0; i < core_hist.size(); ++i) {
                auto* g = new GraphWidget(
                    QString("core %1").arg(i),
                    &core_hist[i],
                    100.0,
                    graphs_page);
                g->setAutoScale(false);
                int row = 1 + static_cast<int>(i) / cols;
                int col = static_cast<int>(i) % cols;
                grid->addWidget(g, row, col);
                core_graphs_.push_back(g);
            }
        }
    }

    if (cpu_graph_) cpu_graph_->update();
    if (mem_graph_) mem_graph_->update();
    for (auto* g : core_graphs_) g->update();
}

void MainWindow::killSelectedProcess() {
    auto selection = table_->selectionModel()->selectedRows();
    if (selection.isEmpty()) {
        QMessageBox::information(this, "Kill",
            "Сначала выбери процесс в таблице.");
        return;
    }

    QModelIndex proxy_index = selection.first();
    QModelIndex src_index = proxy_->mapToSource(proxy_index);
    int row = src_index.row();

    const auto* proc = model_->processAt(row);
    if (!proc) return;

    int pid = proc->pid;
    QString name = QString::fromStdString(proc->name);

    auto reply = QMessageBox::question(this, "Kill process",
        QString("Убить процесс %1 (PID %2)?").arg(name).arg(pid),
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        if (::kill(pid, SIGTERM) == 0) {
            qDebug() << "SIGTERM sent to" << pid;
        } else {
            QMessageBox::warning(this, "Kill",
                QString("Не удалось убить процесс %1").arg(pid));
        }
    }
}

}