#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <iostream>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->pushButton, &QPushButton::clicked, this, [this] {
        std::cerr << "JOB-: " << (job.get()) << std::endl;
        if(job != nullptr){
            job->stop();
        }
        ui->textBrowser->clear();
        job.reset(new grep_job(processor, ui->lineEdit->text(), ui->lineEdit_2->text()));
        std::cerr << "JOB+: " << (job.get()) << std::endl;
        std::cerr << "START:-" << std::endl;
        job->start();
        std::cerr << "START:+" << std::endl;
    });

    connect(ui->pushButton_2, &QPushButton::clicked, this, [this] {
        std::cerr << "PROCESSOR:-" << std::endl;
        processor.start();
        std::cerr << "PROCESSOR:+" << std::endl;
        std::cerr << "JOB: " << (job.get()) << std::endl;
        if(job == nullptr)
            return;
        std::cerr << "STOP:-" << std::endl;
        job->stop();
        std::cerr << "STOP:+" << std::endl;
        job->patch_result();
        ui->textBrowser->clear();
        job.reset();
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
