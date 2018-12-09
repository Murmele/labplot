#ifndef WORKSHEETINFOELEMENTDOCK_H
#define WORKSHEETINFOELEMENTDOCK_H

#include <QWidget>

class WorksheetInfoElement;

namespace Ui {
class WorksheetInfoElementDock;
}

class WorksheetInfoElementDock : public QWidget
{
    Q_OBJECT

public:
    explicit WorksheetInfoElementDock(QWidget *parent = nullptr);
    ~WorksheetInfoElementDock();
    void setWorksheetInfoElements(QList<WorksheetInfoElement*> &list);

private slots:
    void on_pb_add_clicked();

private:
    Ui::WorksheetInfoElementDock *ui;
    QList<WorksheetInfoElement*> m_elements;
};

#endif // WORKSHEETINFOELEMENTDOCK_H
