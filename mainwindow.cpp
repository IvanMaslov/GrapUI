#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->pushButton, &QPushButton::clicked, this, [this] {
        // ui->textBrowser->append("searched: in " + ui->lineEdit->text() + " for " + ui->lineEdit_2->text());
        job.start(ui->lineEdit->text().toStdString(), ui->lineEdit_2->text());
    });

    connect(ui->pushButton_2, &QPushButton::clicked, this, [this] {
        ui->textBrowser->clear();
        // ui->textBrowser->append("cancelled");
        job.stop();
    });

    timer.setInterval(200);
    timer.start();
    connect(&timer, &QTimer::timeout, this, [this]{
        QString responce = job.patch_result();
        if(responce.isEmpty())
            return;
        ui->textBrowser->append(responce);
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}
