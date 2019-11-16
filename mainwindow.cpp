#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QCommonStyle>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QCommonStyle style;
    ui->actionScanDir->setIcon(style.standardIcon(QCommonStyle::SP_DialogOpenButton));
    connect(ui->actionScanDir, &QAction::triggered, this, &MainWindow::select_dir);

    connect(ui->pushButton, &QPushButton::clicked, this, [this] {
        if(job != nullptr) job->stop();
        ui->textBrowser->clear();
        job = std::make_shared<grep_job>(processor, ui->lineEdit->text(), ui->lineEdit_2->text());
        try {
            grep_job::start(job);
        } catch (const task_error& e) {
            if(processor.is_shutdown()) processor.restart();
        }
    });

    connect(ui->pushButton_2, &QPushButton::clicked, this, [this] {
        if(processor.is_shutdown()) processor.restart();
        if(job == nullptr) return;
        job->stop();
        job->patch_result();
        ui->textBrowser->clear();
        job.reset();
    });

    timer.setInterval(40);
    timer.start();
    connect(&timer, &QTimer::timeout, this, [this]{
        ui->lcdNumber->display(static_cast<int>(processor.sheduled_tasks()));

        if(job == nullptr)
            return;
        QString responce = job->patch_result();
        if(responce.isEmpty())
            return;

        //ui->textBrowser->append(responce);
        ui->textBrowser->insertPlainText(responce);
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::select_dir() {
    ui->lineEdit->setText(QFileDialog::getExistingDirectory(this, "Select Directory for Scanning",
                                                    QString(), QFileDialog::ShowDirsOnly
                                                            | QFileDialog::ReadOnly
                                                            | QFileDialog::DontResolveSymlinks));
}
