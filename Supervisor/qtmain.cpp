#include "qtmain.h"
#include "ui_qtmain.h"

qtMain::qtMain(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::qtMain)
{
    ui->setupUi(this);
}

qtMain::~qtMain()
{
    delete ui;
}
