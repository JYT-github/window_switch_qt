#ifndef PROCESSDIALOG_H
#define PROCESSDIALOG_H

#include <QDialog>

namespace Ui {
class ProcessDialog;
}

class ProcessDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ProcessDialog(QWidget *parent = nullptr);
    ~ProcessDialog();
    Ui::ProcessDialog *ui;
    int GetAltKeyValue();

    HWND GetFormHwndValue();

    QString GetFormNameValue();

private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();




    void on_tableWidget_cellDoubleClicked(int row, int column);
};

#endif // PROCESSDIALOG_H
