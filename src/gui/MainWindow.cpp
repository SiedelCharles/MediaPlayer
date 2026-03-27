#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QDebug>
#include "setting.h"
#include "widgetside.h"
#include "widgettitle.h"
#include "stackwidgetfordisplay.h"
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->centralwidget->setContentsMargins(0,0,0,0);
    setWindowFlag(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);

    _Widget_title = new WidgetTitle(this);
    _Widget_title->setObjectName("WindowTitle");
    // ui->layout_title->addWidget(_Widget_title);//This is an attempt to control the layout of components in the UI design interface.
    setMenuWidget(_Widget_title);
    connect(dynamic_cast<WidgetTitle*>(_Widget_title), &WidgetTitle::signal_slot_TitleMinimizeClicked
            , this, &MainWindow::slot_MinimizeClose);
    connect(dynamic_cast<WidgetTitle*>(_Widget_title), &WidgetTitle::signal_slot_TitleResizeClicked
            , this, &MainWindow::slot_ResizeClose);
    connect(dynamic_cast<WidgetTitle*>(_Widget_title), &WidgetTitle::signal_slot_TitleCloseClicked
            , this, &MainWindow::slot_TitleClose);

    _Widget_side = new WidgetSide(this);
    _Widget_side->setObjectName("SideMenu");
    _Widget_side->setFixedWidth(SIDE_LENGTH);
    ui->layout_side->addWidget(_Widget_side);
    _StackWidget_display = new StackWidgetForDisplay(this);
    _StackWidget_display->setObjectName("DisplayDialog");
    ui->layout_dialog->addWidget(_StackWidget_display);

    auto widgetside = static_cast<WidgetSide*>(_Widget_side);
    auto stackwidget = static_cast<StackWidgetForDisplay*>(_StackWidget_display);
    connect(widgetside, &WidgetSide::currentRowChanged, stackwidget, &StackWidgetForDisplay::slot_ChangeCurrentPage);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::paintEvent(QPaintEvent *paint_event)
{
    QStyleOption opt;
    opt.initFrom(this);
    QPainter painter(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);
}

void MainWindow::slot_MinimizeClose(bool checked)
{
    showMinimized();
}

void MainWindow::slot_ResizeClose(bool checked)
{
    if(isFullScreen()) {
        showNormal();
    } else {
        showFullScreen();
    }
}

void MainWindow::slot_TitleClose(bool checked)
{
    close();
}
