#include "MainWindow.hpp"
#include "ProcessTableModel.hpp"
#include <QTableView>
#include <QTimer>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QSortFilterProxyModel>
#include <QMessageBox>
#include <QDebug>
#include <csignal>

namespace sysmon::ui::qt {

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent) {

    setWindowTitle("sysmon");
    resize(1100, 700);

    auto* central = new QWidget(this);
    auto* main_layout = new QVBoxLayout(central);

    auto* top_panel = new QHBoxLayout();

    QFont mono_font("monospace");
    mono_font.setPointSize(12);

    cpu_label_  = new QLabel("CPU: --", this);
    mem_label_  = new QLabel("RAM: --", this);
    proc_label_ = new QLabel("Processes: --", this);

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

    model_ = new ProcessTableModel(this);

    proxy_ = new QSortFilterProxyModel(this);
    proxy_->setSourceModel(model_);
    proxy_->setSortRole(Qt::UserRole);   // сортировать по числу, а не строке

    table_ = new QTableView(this);
    table_->setModel(proxy_);
    table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    table_->setSelectionMode(QAbstractItemView::SingleSelection);
    table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table_->setAlternatingRowColors(true);
    table_->setSortingEnabled(true);
    table_->verticalHeader()->setVisible(false);
    table_->horizontalHeader()->setStretchLastSection(true);
    table_->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);

    main_layout->addWidget(table_);
    setCentralWidget(central);

    timer_ = new QTimer(this);
    connect(timer_, &QTimer::timeout, this, &MainWindow::updateSnapshot);
    timer_->start(1000);

    monitor_.take_snapshot();

    updateSnapshot();

    table_->sortByColumn(2, Qt::DescendingOrder);
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