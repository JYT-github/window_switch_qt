#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QApplication>
#include <windows.h>
#include <QDebug>
#include <QMap>
#include <QtWidgets/QWidget>
#include <QMouseEvent>

#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QInputDialog>
#include <QObject>
#include <QStringListModel>

#include "processdialog.h"



MainWindow *m;//定义全局
HHOOK keyHook=NULL;
HHOOK mouseHook=NULL;

bool LeftTag=false;
bool RightTag=false;
bool ctrlKeyTag=false;
bool altKeyTag=false;
int keys_arr_length=4;
int keys_arr[4] = {49,50,51,52};// 数字键 1234


// 基础功能 窗口快捷切换功能 map 和 是否显示
QMap<int,HWND> keys_hwnd_map;
QMap<int,int> keys_hwnd_isshow;


//  alt+数字键隐藏程序
int table_index=0;
QMap<int,HWND> alt_keys_hwnd_hide_map;
QMap<int,int> alt_keys_hwnd_hide_key;
QMap<int,int> alt_keys_hwnd_hide_isshow;
QMap<int,QString> alt_keys_hwnd_hide_title_map;



// 判断是否 点击了 窗口捕获 按钮
bool is_grad_btn=false;


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    m=this;
    ui->setupUi(this);
    setWindowFlags(windowFlags()& ~Qt::WindowMaximizeButtonHint);
    setFixedSize(this->width(), this->height());

    SetWindowPos((HWND)(m->winId()), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint ); // 无边框
    setAttribute(Qt::WA_TranslucentBackground);

    // 设置 应用图标 任务栏图标
    this->setWindowIcon(QIcon(":/icon1.png"));
    // 隐藏任务栏图标
    this->setWindowFlags(Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint|Qt::Popup|Qt::Tool);

//    //关闭按钮关闭窗口
//     connect(closeBtn,SIGNAL(clicked(bool)),this,SLOT(close()));
//     //最小化按钮的实现
//     connect(cutBtn,SIGNAL(clicked(bool)),this,SLOT(showMinimized()));
    //    设置当窗口关闭时不退出程序
        QApplication::setQuitOnLastWindowClosed(false);

    //    定义托盘图标，并设置父控件为mainwindow
        QSystemTrayIcon *icon = new QSystemTrayIcon(this);
    //    挑一个顺眼的icon，路径从qrc中复制即可
        icon->setIcon(QIcon(":/icon1.png"));

    //    添加右键菜单
        QMenu *menu = new QMenu;
        QAction *normal, *quit;//, *min, *max
        normal = new QAction("显示");
        connect(normal, &QAction::triggered, [&](){
            ShowWindow((HWND)(m->winId()),1);
            //SetWindowPos((HWND)(m->winId()), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
        });
//        min = new QAction("最小化");
//        connect(min, &QAction::triggered, [&](){
//            showMinimized();
//        });
//        max = new QAction("最大化");
//        connect(max, &QAction::triggered, [&](){
//            showMaximized();
//        });
        quit = new QAction("退出");
        connect(quit, &QAction::triggered, [&](){
            m->recovery();
            QCoreApplication::quit();
        });

        menu->addAction(normal);
//        menu->addAction(min);
//        menu->addAction(max);
        menu->addSeparator();
        menu->addAction(quit);

        icon->setContextMenu(menu);

    //    设置单击显示主界面
        connect(icon, &QSystemTrayIcon::activated, [=](QSystemTrayIcon::ActivationReason r){
            if(r == QSystemTrayIcon::ActivationReason::Trigger) emit normal->triggered();
        });

    //    显示托盘图标
        icon->show();


        // 设置表格
        QStringList strListColumnHander;

        strListColumnHander << QObject::tr("编号") << QObject::tr("名称") << QObject::tr("快捷键");

        ui->tableWidget->setColumnCount(3); //设置列数目

        ui->tableWidget->setHorizontalHeaderLabels(strListColumnHander); //设置列表头
        ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);  //单击选择一行
        ui->tableWidget->setSelectionMode(QAbstractItemView::SingleSelection); //设置只能选择一行，不能多行选中
        ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);   //设置每行内容不可更改
        ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);//设置列宽等间距
}
void MainWindow::recovery(){
    for (int i = 0; i < keys_arr_length; ++i) {
        ShowWindow(alt_keys_hwnd_hide_map.value(keys_arr[i]),1);
    }
    QMapIterator<int,HWND>i(alt_keys_hwnd_hide_map);
    for(;i.hasNext();){
        // 判断第一行标号是否等于key
        i.next();
        ShowWindow(i.value(),1);
    }
}
MainWindow::~MainWindow()
{
    delete ui;
}





//键盘钩子过程
LRESULT CALLBACK keyProc(int nCode,WPARAM wParam,LPARAM lParam )
{
    //在WH_KEYBOARD_LL模式下lParam 是指向KBDLLHOOKSTRUCT类型地址
    KBDLLHOOKSTRUCT *pkbhs = (KBDLLHOOKSTRUCT *) lParam;
    //如果nCode等于HC_ACTION则处理该消息，如果小于0，则钩子子程就必须将该消息传递给 CallNextHookEx

    if(nCode == HC_ACTION) //当nCode等于HC_ACTION时，要求得到处理
    {
        //qDebug() <<pkbhs->flags;

        //        if(pkbhs->vkCode == VK_ESCAPE && GetAsyncKeyState(VK_CONTROL)& 0x8000 && GetAsyncKeyState(VK_SHIFT)&0x8000){
        //                    qDebug() << "Ctrl+Shift+Esc";
        //        }else if(pkbhs->vkCode == VK_ESCAPE && GetAsyncKeyState(VK_CONTROL) & 0x8000){
        //            qDebug() << "Ctrl+Esc";
        //        }else if(pkbhs->vkCode == VK_TAB && pkbhs->flags & LLKHF_ALTDOWN){
        //            qDebug() << "Alt+Tab";
        //        }else if(pkbhs->vkCode == VK_ESCAPE && pkbhs->flags &LLKHF_ALTDOWN){
        //            qDebug() << "Alt+Esc";
        //        }else if(pkbhs->vkCode == VK_LWIN || pkbhs->vkCode == VK_RWIN){
        //            qDebug() << "LWIN/RWIN";
        //        }else if(pkbhs->vkCode == VK_F4 && pkbhs->flags & LLKHF_ALTDOWN){
        //            qDebug() << "Alt+F4";
        //        }
        //        if(pkbhs->vkCode == VK_F12)
        //        {
        //            void unHook();
        //            qApp->quit();
        //        }


        // 判断是否是按下

        // 按下
        if(wParam==256||wParam==260){

            // 判断是否按下 ctrl
            if(GetKeyState(VK_CONTROL)& 0x8000){
                ctrlKeyTag=true;
                //return 0;//返回1表示截取消息不再传递,返回0表示不作处理,消息继续传递
            }

            // 判断是否按下 alt
            if(pkbhs->vkCode == 164){
                //qDebug() <<"判断是否按下 alt";
                altKeyTag=true;
                //return 0;//返回1表示截取消息不再传递,返回0表示不作处理,消息继续传递
            }


            if(altKeyTag){
                QMapIterator<int,int>key_i(alt_keys_hwnd_hide_key);
                bool is_key_show=false;
                for(;key_i.hasNext();){
                    key_i.next();
                    if(pkbhs->vkCode==key_i.value()){

                        if(alt_keys_hwnd_hide_isshow.value(key_i.key())==1){
                            alt_keys_hwnd_hide_isshow.insert(key_i.key(),0);
                        }else{
                            alt_keys_hwnd_hide_isshow.insert(key_i.key(),1);
                        }
                        if (alt_keys_hwnd_hide_isshow.value(key_i.key())!=0){
                            ShowWindow(alt_keys_hwnd_hide_map.value(key_i.key()),alt_keys_hwnd_hide_isshow.value(key_i.key())|5);
                        }else{
                            ShowWindow(alt_keys_hwnd_hide_map.value(key_i.key()),alt_keys_hwnd_hide_isshow.value(key_i.key()));
                        }

                        is_key_show=true;
                    }
                }
                if(is_key_show){
                     return 1;//如果是这些事件产生的话，就返回真，即屏蔽掉这些事件，也就是安装有同类型的其他钩子，捕获不到同样的事件
                }
            }


            // ctrl + 数字键 + 鼠标 左键

            if(ctrlKeyTag==true && LeftTag==true){
                qDebug() <<"按下了ctrl+鼠标左键";
                // 判断是否 按下了数字键
                for (int i = 0; i < keys_arr_length; ++i) {
                    if(pkbhs->vkCode==keys_arr[i]){
                        POINT curpos;
                        GetCursorPos(&curpos);
                        HWND grab_formHandle = WindowFromPoint(curpos);
                        while (0 != GetParent(grab_formHandle))
                        {

                            grab_formHandle = GetParent(grab_formHandle);
                        };

                        // 将 数字键 与 获取的窗口句柄 绑定存在了

                         keys_hwnd_map.insert(pkbhs->vkCode,grab_formHandle);


                        return 1;//如果是这些事件产生的话，就返回真，即屏蔽掉这些事件，也就是安装有同类型的其他钩子，捕获不到同样的事件
                    }
                }

            }

            // 判断是否 按下了数字键
            for (int i = 0; i < keys_arr_length; ++i) {
                if(pkbhs->vkCode==keys_arr[i] && LeftTag==true){
                    qDebug() <<"按下了数字键-------------";
                    if(keys_hwnd_map.contains(pkbhs->vkCode)){




                        //  判断窗口是否时最大化
                        bool isMaxForm = IsZoomed(keys_hwnd_map.value(pkbhs->vkCode));
                        if(isMaxForm){
                            ShowWindow(keys_hwnd_map.value(pkbhs->vkCode),1|SW_MAXIMIZE);
                        }

                        bool isMinForm = IsIconic(keys_hwnd_map.value(pkbhs->vkCode));
                        if(isMinForm){
                            ShowWindow(keys_hwnd_map.value(pkbhs->vkCode),1|SW_MINIMIZE);
                        }

                        SetWindowPos(keys_hwnd_map.value(pkbhs->vkCode), HWND(-1), 0, 0, 0, 0, 0x0001 | 0x0002);
                        SetWindowPos(keys_hwnd_map.value(pkbhs->vkCode), HWND(-2), 0, 0, 0, 0, 0x0001 | 0x0002);
                        SetWindowPos(keys_hwnd_map.value(pkbhs->vkCode), HWND(-2), 0, 0, 0, 0, 0x0001 | 0x0002);
                        SetWindowPos(keys_hwnd_map.value(pkbhs->vkCode), HWND(-2), 0, 0, 0, 0, 0x0001 | 0x0002);
                        SetWindowPos(keys_hwnd_map.value(pkbhs->vkCode), HWND(-2), 0, 0, 0, 0, 0x0001 | 0x0002);
                        SetWindowPos(keys_hwnd_map.value(pkbhs->vkCode), HWND(-2), 0, 0, 0, 0, 0x0001 | 0x0002);
                        return 1;//返回1表示截取消息不再传递,返回0表示不作处理,消息继续传递
                    }
                    return 1;//返回1表示截取消息不再传递,返回0表示不作处理,消息继续传递
                }
            }





        }else{


            //判断抬起ctrl
            if(GetKeyState(VK_CONTROL)& 0x8000){
                ctrlKeyTag=false;
                return 0;//返回1表示截取消息不再传递,返回0表示不作处理,消息继续传递
            }
            //qDebug() <<pkbhs->flags;
            //判断抬起ctrl
            if(pkbhs->vkCode == 164){
                //qDebug() <<"判断是否抬起 alt";
                altKeyTag=false;
                return 0;//返回1表示截取消息不再传递,返回0表示不作处理,消息继续传递
            }
        }


    }
    return CallNextHookEx(keyHook, nCode, wParam, lParam);
}
void MainWindow::on_tableWidget_cellDoubleClicked(int row, int column)
{
    QString that_index = ui->tableWidget->model()->index(row,0).data().toString();

    // 将要 删除的键盘key 绑定的窗体显示出来
//    for (int i = 0; i < keys_arr_length; ++i) {
//        if(keys_arr[i]==(int)that_key_array.at(0)){
//              ShowWindow(alt_keys_hwnd_hide_map.value(keys_arr[i]),1);
//        }

//    }

    QMapIterator<int,HWND>i(alt_keys_hwnd_hide_map);
    for(;i.hasNext();){
        // 判断第一行标号是否等于key
        i.next();
        if((QString)that_index==QString("%1").arg(i.key(),0,10)){
            ShowWindow(i.value(),1);
        }

    }


    //alt_keys_hwnd_hide_key.remove(that_index_data);
    for(QMap<int, int>::iterator it = alt_keys_hwnd_hide_key.begin();it != alt_keys_hwnd_hide_key.end();it++ )
      {
          //LOG(INFO) << "---- key :" << it->first << "value " << it->second;
        if(QString::number(it.key())==that_index){
          alt_keys_hwnd_hide_key.erase(it);
          break;
        }
      }
    //alt_keys_hwnd_hide_map.remove(that_index_data);
    for(QMap<int, HWND>::iterator it = alt_keys_hwnd_hide_map.begin();it != alt_keys_hwnd_hide_map.end();it++ )
      {
          //LOG(INFO) << "---- key :" << it->first << "value " << it->second;
        if(QString::number(it.key())==that_index){
          alt_keys_hwnd_hide_map.erase(it);
          break;
        }
      }
    //alt_keys_hwnd_hide_isshow.remove(that_index_data);
    for(QMap<int, int>::iterator it = alt_keys_hwnd_hide_isshow.begin();it != alt_keys_hwnd_hide_isshow.end();it++ )
      {
          //LOG(INFO) << "---- key :" << it->first << "value " << it->second;
        if(QString::number(it.key())==that_index){
            alt_keys_hwnd_hide_isshow.erase(it);
            break;
        }

      }

    //alt_keys_hwnd_hide_title_map.remove(that_index_data);
    for(QMap<int, QString>::iterator it = alt_keys_hwnd_hide_title_map.begin();it != alt_keys_hwnd_hide_title_map.end();it++ )
      {
          //LOG(INFO) << "---- key :" << it->first << "value " << it->second;
        if(QString::number(it.key())==that_index){
          alt_keys_hwnd_hide_title_map.erase(it);
          break;
        }
      }
    m->ui->tableWidget->removeRow(row);
}
//鼠标钩子过程
LRESULT CALLBACK mouseProc(int nCode,WPARAM wParam,LPARAM lParam )
{

       if(nCode == HC_ACTION) //当nCode等于HC_ACTION时，要求得到处理
       {
           //鼠标 左键
           if(
                 //          wParam==WM_MOUSEWHEEL//屏蔽滑轮
                 //           ||wParam==WM_RBUTTONDOWN//屏蔽鼠标右键按下
                wParam==WM_LBUTTONDOWN//屏蔽鼠标左键按下
                 //           ||wParam==WM_RBUTTONUP//屏蔽鼠标右键弹起的消息
                 //           ||wParam==WM_LBUTTONUP//屏蔽鼠标左键弹起的消息
                 //           ||wParam==WM_MOUSEMOVE
            )//屏蔽鼠标的移动
           {
               // 判断是否按下了窗口捕获按钮

               if(is_grad_btn){
                   m->on_grab_btn_clicked();
                   POINT curpos;
                   GetCursorPos(&curpos);
                   HWND grab_formHandle = WindowFromPoint(curpos);
                   while (0 != GetParent(grab_formHandle))
                   {
                       grab_formHandle = GetParent(grab_formHandle);
                   };

                   // 弹出窗口

                   QStringList list;
                        for (int i = 0; i < keys_arr_length; ++i) {
                            list <<(QString)keys_arr[i];
                        }


                      QInputDialog *inputDialog = new QInputDialog(m);
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
                          table_index+=1;

                          alt_keys_hwnd_hide_key.insert(table_index,(int)array.at(0));
                          alt_keys_hwnd_hide_map.insert(table_index,grab_formHandle);
                          alt_keys_hwnd_hide_isshow.insert(table_index,1);

                          char buff[255];
                          GetWindowTextA(grab_formHandle,(LPSTR)buff,sizeof(buff));
                           QString title = QString::fromLocal8Bit(buff);
                           alt_keys_hwnd_hide_title_map.insert(table_index,title);

                            // 更新表
                            m->updTable();

                      }


                        // 将 alt+数字绑定的快捷键 与 要隐藏的窗口程序句柄 存入map
                        delete inputDialog;


                return 1;
               }




                LeftTag=true;
               // ctrl + alt + 鼠标 左键 窗口置顶
               if(ctrlKeyTag==true&&altKeyTag==true){
                   qDebug() << "鼠标左键点击";
                   //qDebug() << GetForegroundWindow();// 获取 当前激活的窗口
                    POINT curpos;
                    GetCursorPos(&curpos);
                    HWND grab_formHandle = WindowFromPoint(curpos);
                    while (0 != GetParent(grab_formHandle))
                    {

                        grab_formHandle = GetParent(grab_formHandle);
                    };
                   SetWindowPos(grab_formHandle, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);

                    return 1;//如果是这些事件产生的话，就返回真，即屏蔽掉这些事件，也就是安装有同类型的其他钩子，捕获不到同样的事件
               }






           }else if(wParam==WM_LBUTTONUP){
               LeftTag=false;
           }


           //鼠标 右键

           if(
                 //          wParam==WM_MOUSEWHEEL//屏蔽滑轮
                 //           ||wParam==WM_RBUTTONDOWN//屏蔽鼠标右键按下
                wParam==WM_RBUTTONDOWN//屏蔽鼠标左键按下
                 //           ||wParam==WM_RBUTTONUP//屏蔽鼠标右键弹起的消息
                 //           ||wParam==WM_LBUTTONUP//屏蔽鼠标左键弹起的消息
                 //           ||wParam==WM_MOUSEMOVE
            )//屏蔽鼠标的移动
           {
               RightTag=true;
               // ctrl + alt + 鼠标 右键 窗口置顶
               if(ctrlKeyTag==true&&altKeyTag==true){
                   qDebug() << "鼠标右键键点击";
                   qDebug() << GetForegroundWindow();// 获取 当前激活的窗口
                   POINT curpos;
                   GetCursorPos(&curpos);
                   HWND grab_formHandle = WindowFromPoint(curpos);
                   while (0 != GetParent(grab_formHandle))
                   {

                       grab_formHandle = GetParent(grab_formHandle);
                   };
                   SetWindowPos(grab_formHandle, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);

                   return 1;//如果是这些事件产生的话，就返回真，即屏蔽掉这些事件，也就是安装有同类型的其他钩子，捕获不到同样的事件
               }



                 return 0;//如果是这些事件产生的话，就返回真，即屏蔽掉这些事件，也就是安装有同类型的其他钩子，捕获不到同样的事件
           }else if(wParam==WM_RBUTTONUP){
               RightTag=false;
           }
       }
    return 0;

}
void MainWindow::updTable(){


    m->ui->tableWidget->setRowCount(0);
    QMapIterator<int,int>key_i(alt_keys_hwnd_hide_key);
    qDebug() <<"插入表格开始----";
    for(;key_i.hasNext();){
         key_i.next();
        int rowIndex=m->ui->tableWidget->rowCount();
        qDebug() <<key_i.key();
        m->ui->tableWidget->insertRow(rowIndex);
        m->ui->tableWidget->setItem(rowIndex,0,new QTableWidgetItem(QString::number((int)key_i.key())));
        m->ui->tableWidget->setItem(rowIndex,1,new QTableWidgetItem(alt_keys_hwnd_hide_title_map.value(key_i.key())));
        m->ui->tableWidget->setItem(rowIndex,2,new QTableWidgetItem((QString)key_i.value()));

    }
    qDebug() <<"插入表格结束----";



//    QMapIterator<int,QString>modi(alt_keys_hwnd_hide_title_map);
//      // 删除 所有行
//      for(int i = 0 ; i <=  m->ui->tableWidget->rowCount(); i++)//清空列表
//      {
//          m->ui->tableWidget->removeRow(0);
//      }
//        for(;modi.hasNext();){
//            int rowIndex=m->ui->tableWidget->rowCount();
//            modi.next();
//            qDebug()<<" "<<modi.key()<<" "<<modi.value();
//            m->ui->tableWidget->insertRow(rowIndex);
//            m->ui->tableWidget->setItem(rowIndex,0,new QTableWidgetItem(modi.value()));
//            m->ui->tableWidget->setItem(rowIndex,1,new QTableWidgetItem((QString)modi.key()));

//        }
}

//卸载钩子
void unHook()
{
    UnhookWindowsHookEx(keyHook);
    UnhookWindowsHookEx(mouseHook);

}
//安装钩子,调用该函数即安装钩子
void setHook()
{
    //这两个底层钩子,不要DLL就可以全局
    //底层键盘钩子
    keyHook =SetWindowsHookEx( WH_KEYBOARD_LL,keyProc,GetModuleHandle(NULL),0);
    //底层鼠标钩子
    mouseHook =SetWindowsHookEx( WH_MOUSE_LL,mouseProc,GetModuleHandle(NULL),0);
}


void MainWindow::on_start_btn_clicked()
{

    if(m_isStart){
        //注册 钩子
            setHook();
            qDebug() << "0000000";
        ui->start_btn->setText("停止");
        m_isStart=false;

    }else{
        // 销毁 钩子
           unHook();
           qDebug() << "33333333";
        ui->start_btn->setText("启动");
        m_isStart=true;
    }
    qDebug()<<"hello a="<<m_isStart<<endl;
}




void MainWindow::mousePressEvent(QMouseEvent *event)//窗口移动事件
{

        //当鼠标左键点击时.
          if (event->button() == Qt::LeftButton)
          {
              m_move = true;
              //记录鼠标的世界坐标.
              m_startPoint = event->globalPos();
              //记录窗体的世界坐标.
              m_windowPoint = this->frameGeometry().topLeft();
          }


}
void MainWindow::mouseMoveEvent(QMouseEvent *event)//窗口移动事件
{

        if (event->buttons() & Qt::LeftButton)
          {
              //移动中的鼠标位置相对于初始位置的相对位置.
              QPoint relativePos = event->globalPos() - m_startPoint;
              //然后移动窗体即可.
              this->move(m_windowPoint + relativePos );
          }

}





// 捕获按钮
void MainWindow::on_grab_btn_pressed()
{
    qDebug() <<"按下";
}




void MainWindow::on_pushButton_2_clicked()
{

    ShowWindow((HWND)(m->winId()),0);
}


void MainWindow::on_pushButton_clicked()
{
    ShowWindow((HWND)(m->winId()),0);
}


void MainWindow::on_grab_btn_clicked()
{
    if(is_grad_btn){
        is_grad_btn=false;
        ui->grab_btn->setStyleSheet(
                    //正常状态样式
                    "QPushButton{"
                    "color:#fff;"
                    "background:rgb(40, 48, 62);"
                    "border:1px solid #fff;"
                    "}"
                   );
    }else{
        is_grad_btn=true;
        ui->grab_btn->setStyleSheet(
                    //正常状态样式
                    "QPushButton{"
                    "color:#fff;"
                    "background:rgb(62, 72, 89);"
                    "border:1px solid #fff;"
                    "}"
                   );
    }


}





void MainWindow::on_add_from_btn_clicked()
{
     ProcessDialog *process_dialog = new ProcessDialog(this);
    Qt::WindowFlags flags = process_dialog->windowFlags();                     // 需要获取返回值
       process_dialog->setWindowFlags(flags | Qt::MSWindowsFixedSizeDialogHint);  // 设置对话框固定大小

       // 读取MainWindows参数并设置到Dialog
//       QString item = ui->lineEdit->text();
//       process_dialog->SetValue(item);

       int ref = process_dialog->exec();             // 以模态方式显示对话框
       if (ref==QDialog::Accepted)        // OK键被按下,对话框关闭
       {
           qDebug() <<"点击了确定";
           // 当BtnOk被按下时,则设置对话框中的数据
           int that_alt_key = process_dialog->GetAltKeyValue();
           HWND that_form_hwnd = process_dialog->GetFormHwndValue();

           QString that_form_name = process_dialog->GetFormNameValue();
           qDebug() <<"key:"<<that_alt_key<<" 句柄："<<that_form_hwnd<<" 名称："<<that_form_name;


           table_index+=1;

           alt_keys_hwnd_hide_key.insert(table_index,that_alt_key);
           alt_keys_hwnd_hide_map.insert(table_index,that_form_hwnd);
           alt_keys_hwnd_hide_isshow.insert(table_index,1);
           alt_keys_hwnd_hide_title_map.insert(table_index,that_form_name);


            // 更新表
            m->updTable();


       }

       // 删除释放对话框句柄
       delete process_dialog;
}

bool isMaxForm = false;
void MainWindow::on_pushButton_3_clicked()
{
    if(isMaxForm){
        isMaxForm = false;
        setFixedSize(570, 320);
    }else{
        isMaxForm = true;
        setFixedSize(570, 720);
    }



}

