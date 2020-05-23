#ifndef QTMAIN_H
#define QTMAIN_H

#include <QDialog>

namespace Ui {
class qtMain;
}

class qtMain : public QDialog
{
    Q_OBJECT

public:
    explicit qtMain(QWidget *parent = 0);
    ~qtMain();

private:
    Ui::qtMain *ui;
};

#endif // QTMAIN_H
