#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <src/widgets/mazeview/mazeview.h>
#include <QMainWindow>


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private Q_SLOTS:
    void btnGenerateOnClick();

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    Ui::MainWindow *ui;
    MazeView *maze;
};

#endif // MAINWINDOW_H
