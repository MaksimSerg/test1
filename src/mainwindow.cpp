#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "src/appsettings.h"

#include <QPushButton>
#include <QValidator>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    maze(nullptr)
{
    ui->setupUi(this);

    // settings.setValue("General/language", sLang);


    maze = new MazeView(this);
    ui->graphicsBox->layout()->addWidget(maze);

    int defaultMatrixSize = 150;
    ui->inputWidth->setValidator( new QIntValidator(0, 999, this) );
    ui->inputWidth->setText(QString::number(defaultMatrixSize));
    ui->inputHeight->setValidator( new QIntValidator(0, 999, this) );
    ui->inputHeight->setText(QString::number(defaultMatrixSize));

    btnGenerateOnClick();

    connect(ui->btnGenerate, SIGNAL(clicked()), this, SLOT(btnGenerateOnClick()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::btnGenerateOnClick()
{
    if (!maze) return;

    int mWidth  = ui->inputWidth->text().toInt();
    int mHeight = ui->inputHeight->text().toInt();

    maze->generateMatrix(mWidth, mHeight);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    AppSettings& appset = AppSettings::getInstance();

    appset.setValue("Windows.primary/geometry", QVariant(this->geometry()));
    appset.setValue("Windows.primary/fullscreen", QVariant(this->isMaximized()));

    QMainWindow::closeEvent(event);
}
