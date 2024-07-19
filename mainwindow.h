#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QDebug>
#include <QSystemTrayIcon>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    Ui::MainWindow *ui;



private slots:
    void on_start_btn_clicked();




    void on_grab_btn_pressed();


    void on_pushButton_2_clicked();

    void on_pushButton_clicked();
    void on_tableWidget_cellDoubleClicked(int row, int column);

    void on_add_from_btn_clicked();

    void on_pushButton_3_clicked();

public slots:
    void on_grab_btn_clicked();
    void recovery();
    void updTable();

private:
    bool m_isStart = true;
    HWND task;
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    bool m_move;
    QPoint m_startPoint;
    QPoint m_windowPoint;
};
#endif // MAINWINDOW_H
