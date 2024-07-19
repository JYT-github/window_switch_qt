#include "processdialog.h"
#include "ui_processdialog.h"
#include <QInputDialog>
#include <windows.h>
#include <QDebug>

#pragma comment  (lib, "User32.lib")
char m_Name[MAXBYTE];
char m_Title[MAXBYTE];
int m_nNum;

QMap<int,HWND> process_hwnd_map;

int keys_arr_length1=4;
int keys_arr1[4] = {49,50,51,52};// 数字键 1234

ProcessDialog *mm;//定义全局
ProcessDialog::ProcessDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ProcessDialog)
{
    mm=this;
    ui->setupUi(this);
    // 设置表格
    QStringList strListColumnHander;

    strListColumnHander << QObject::tr("ID") << QObject::tr("窗口句柄") << QObject::tr("窗口标题") << QObject::tr("窗口类名");

    ui->tableWidget->setColumnCount(4); //设置列数目

    ui->tableWidget->setHorizontalHeaderLabels(strListColumnHander); //设置列表头
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);  //单击选择一行
    ui->tableWidget->setSelectionMode(QAbstractItemView::SingleSelection); //设置只能选择一行，不能多行选中
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);   //设置每行内容不可更改
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);//设置列宽等间距
}

ProcessDialog::~ProcessDialog()
{
    delete ui;

}
BOOL CALLBACK enumAllWindow(HWND Hwnd, LPARAM IParm)//系统返还给你的窗口句柄,API调用进来的参数
{


    //每次Hwnd返回回来，都需要获取他的类名和标题
    GetClassNameA(Hwnd,m_Name,MAXBYTE);//获得指定窗口所属的类的类名
    GetWindowTextA(Hwnd,m_Title,MAXBYTE);//查找标题
    m_nNum++;
    QString strName = QString::fromLocal8Bit(m_Name);
    QString strTitle = QString::fromLocal8Bit(m_Title);
    qDebug()<<"ID:"<<m_nNum<<"窗口句柄:"<<Hwnd<<"窗口类名:"<<strName<<"窗口标题:"<<strTitle;

    process_hwnd_map.insert(m_nNum,Hwnd);
    int rowIndex=mm->ui->tableWidget->rowCount();

    mm->ui->tableWidget->insertRow(rowIndex);
    mm->ui->tableWidget->setItem(rowIndex,0,new QTableWidgetItem(QString::number(m_nNum)));
    mm->ui->tableWidget->setItem(rowIndex,1,new QTableWidgetItem(QString::number(((int)Hwnd))));
    mm->ui->tableWidget->setItem(rowIndex,2,new QTableWidgetItem((QString)strName));
    mm->ui->tableWidget->setItem(rowIndex,3,new QTableWidgetItem((QString)strTitle));


//    return false;//枚举一次就不枚举了
    return true;//枚举到完毕
}
//accept();
void ProcessDialog::on_pushButton_clicked()
{
    accept();
}


void ProcessDialog::on_pushButton_2_clicked()
{
    close();
}

void ProcessDialog::on_pushButton_3_clicked()
{
//    process_hwnd_map.clear();
//    for(int i = 0 ; i <=  m->ui->tableWidget->rowCount(); i++)//清空列表
//    {
//        m->ui->tableWidget->removeRow(0);
//    }


    m_nNum = 0;
    EnumWindows(enumAllWindow,(LPARAM)"");


}


int alt_key=0;
HWND form_hwnd;
QString form_name;

int ProcessDialog::GetAltKeyValue(){
    return alt_key;
}

HWND ProcessDialog::GetFormHwndValue(){
    return form_hwnd;
}

QString ProcessDialog::GetFormNameValue(){
    return form_name;
}


void ProcessDialog::on_tableWidget_cellDoubleClicked(int row, int column)
{
    QStringList list;
         for (int i = 0; i < keys_arr_length1; ++i) {
             list <<(QString)keys_arr1[i];
         }

       bool ok;
       QInputDialog *inputDialog = new QInputDialog(mm);
       inputDialog->setWindowTitle("绑定快捷键");
       inputDialog->setLabelText("请选择快捷键:（Alt）");

       inputDialog->setComboBoxItems(list);
       inputDialog->setOkButtonText(QString::fromUtf8("确定"));
       inputDialog->setCancelButtonText(QString::fromUtf8("取消"));
       inputDialog->setWindowFlags(Qt::Dialog|Qt::WindowCloseButtonHint );

       inputDialog->setSizeGripEnabled(true);
       int ret = inputDialog->exec();
       if(ret){
           qDebug() <<"鼠标绑定了窗口";
           QByteArray array = inputDialog->textValue().toLatin1();

           alt_key=(int)array.at(0);
           form_hwnd=process_hwnd_map.value(row);
           QString that_strTitle = mm->ui->tableWidget->model()->index(row,3).data().toString();
           if(that_strTitle==""){
               form_name = mm->ui->tableWidget->model()->index(row,2).data().toString();
           }else{
               form_name = that_strTitle;
           }

           mm->accept();
       }

}

