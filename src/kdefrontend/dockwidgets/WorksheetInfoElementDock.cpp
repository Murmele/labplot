#include "WorksheetInfoElementDock.h"
#include "ui_worksheetinfoelementdock.h"

WorksheetInfoElementDock::WorksheetInfoElementDock(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WorksheetInfoElementDock)
{
    ui->setupUi(this);
}

WorksheetInfoElementDock::~WorksheetInfoElementDock()
{
    delete ui;
}
