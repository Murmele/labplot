#ifndef WORKSHEETINFOELEMENTDOCK_H
#define WORKSHEETINFOELEMENTDOCK_H

#include <QWidget>

namespace Ui {
class WorksheetInfoElementDock;
}

class WorksheetInfoElementDock : public QWidget
{
    Q_OBJECT

public:
    explicit WorksheetInfoElementDock(QWidget *parent = nullptr);
    ~WorksheetInfoElementDock();

private:
    Ui::WorksheetInfoElementDock *ui;
};

#endif // WORKSHEETINFOELEMENTDOCK_H
