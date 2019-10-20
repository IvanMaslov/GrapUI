#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->pushButton, &QPushButton::clicked, this, [this] {
        if(job != nullptr) job->stop();
        ui->textBrowser->clear();
        job = std::make_shared<grep_job>(processor, ui->lineEdit->text(), ui->lineEdit_2->text());
        try {
            grep_job::start(job);
        } catch (const std::runtime_error& e) {
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
