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
    void setWorksheetInfoElements(QList<WorksheetInfoElement*> &list, bool sameParent);
    void initConnections();
public slots:
    void worksheetInfoElementCurveRemoved(QString curve);

private slots:
    void visibilityChanged(bool state);
    void addCurve();
    void removeCurve();
    void connectionLineWidthChanged(double width);
    void connectionLineColorChanged(QColor color);
    void xposLineWidthChanged(double value);
    void xposLineColorChanged(QColor color);
    void xposLineVisibilityChanged(bool visible);

    // slots triggered in the WorksheetInfoElement
    void elementConnectionLineWidthChanged(const double width);
    void elementConnectionLineColorChanged(const QColor color);
    void elementXPosLineWidthChanged(const double width);
    void elementXposLineColorChanged(const QColor color);
    void elementXPosLineVisibleChanged(const bool visible);
    void elementVisibilityChanged(const bool visible);

private:
    Ui::WorksheetInfoElementDock *ui;
    WorksheetInfoElement* m_element;
    QList<WorksheetInfoElement*> m_elements;
    bool m_sameParent;
    bool m_initializing;
};

#endif // WORKSHEETINFOELEMENTDOCK_H
