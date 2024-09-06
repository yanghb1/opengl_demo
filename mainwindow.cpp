#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QFileDialog>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    glWidget = new GLWidget;
    ui->widget_2->layout()->addWidget(glWidget);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    // 显示文件选择对话框
    QString filePath = QFileDialog::getOpenFileName(
        this,             // 父窗口
        "Open log File",    // 对话框标题
        "/mnt/sda2/work/跑偏检测/数据处理",                  // 初始目录（空表示默认）
        "log Files (*.log);;" // 文件过滤器
        );

    if (filePath.isEmpty()) {
        qDebug() << "No file selected";
        return;
    }

    glWidget->SetFilePath(filePath);
}



void MainWindow::on_pushButton_2_clicked()
{
    glWidget->SetStartIndex(ui->spinBox->value());
}

