#include "pagedisplay.h"
#include "ui_pagedisplay.h"

#include <QMenu>
#include <QAction>
#include <QMenuBar>
#include <QFileDialog>

// #include "filelistwidget.h"
#include "audioplaypage.h"
#include "filetreewidget.h"


PageDisplay::PageDisplay(QWidget *parent)
    : QWidget(parent),
    ui(new Ui::PageDisplay)
{
    ui->setupUi(this);
    _menu = new QMenuBar(this);

    QMenu *file_menu = _menu->addMenu("File(&F)");
    QAction *file_action = new QAction("Open File", this);
    file_action->setIcon(QIcon(":/resource/130180068_p0_master1200.jpg"));
    file_menu->addAction(file_action);
    connect(file_action, &QAction::triggered, this, &PageDisplay::slot_OpenFile);

    _file_list = new FileTreeWidget(this);
    auto file_list = dynamic_cast<FileTreeWidget*>(_file_list);
    file_list->header()->hide();
    connect(this, &PageDisplay::Sig_OpenFile, file_list, &FileTreeWidget::slot_OpenFile);
    _menu->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    _menu->setFixedHeight(_menu->sizeHint().height());
    ui->file_layout->insertWidget(0, _menu, 0, Qt::AlignTop);
    ui->file_layout->addWidget(_file_list);

    _audio_player = new AudioPlayPage(this);
    ui->audio_layout->addWidget(_audio_player);
    connect(file_list, &FileTreeWidget::itemClicked, dynamic_cast<AudioPlayPage*>(_audio_player), &AudioPlayPage::slot_PlayMusic);
}

PageDisplay::~PageDisplay()
{
    delete ui;
}

void PageDisplay::slot_OpenFile(bool checked)
{
    QFileDialog file_dialog;
    file_dialog.setFileMode(QFileDialog::Directory);
    file_dialog.setWindowTitle("choose file");
    file_dialog.setDirectory(QDir::currentPath());
    file_dialog.setViewMode(QFileDialog::Detail);
    QStringList fileNames;
    if(file_dialog.exec()){
        fileNames = file_dialog.selectedFiles();
    }
    if(fileNames.size()<=0) {
        return ;
    }
    QString import_path = fileNames.at(0);
    emit  Sig_OpenFile(import_path);
}

