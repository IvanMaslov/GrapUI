#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->pushButton, &QPushButton::clicked, this, [this] {
        if(job != nullptr){
            job->stop();
            delete job;
        }
        ui->textBrowser->clear();
        job = new grep_job(processor, ui->lineEdit->text(), ui->lineEdit_2->text());
        job->start();
    });

    connect(ui->pushButton_2, &QPushButton::clicked, this, [this] {
        processor.start();
        if(job == nullptr)
            return;
        job->stop();
        job->patch_result();
        ui->textBrowser->clear();
    });

    timer.setInterval(100);
    timer.start();
    connect(&timer, &QTimer::timeout, this, [this]{
        if(job == nullptr)
            return;
        QString responce = job->patch_result();
        if(responce.isEmpty())
            return;
        ui->textBrowser->append(responce);
    });

    connect(&timer, &QTimer::timeout, this, [this]{
        ui->lcdNumber->display(static_cast<int>(processor.sheduled_tasks()));
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}
