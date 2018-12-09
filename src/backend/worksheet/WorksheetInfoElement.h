#ifndef WORKSHEETINFOELEMENT_H
#define WORKSHEETINFOELEMENT_H

#include "WorksheetElement.h"

#include "backend/worksheet/TextLabel.h"

class TextLabel;
class CustomPoint;
class CartesianPlot;
class WorksheetInfoElementPrivate;
class QGraphicsItem;
class XYCurve;

class WorksheetInfoElement : public WorksheetElement
{
    Q_OBJECT
public:
    WorksheetInfoElement(const QString& name, CartesianPlot *plot);
    WorksheetInfoElement(const QString& name, CartesianPlot *plot, const XYCurve* curve);
    void setParentGraphicsItem(QGraphicsItem* item);

    void save(QXmlStreamWriter*) const override;
    bool load(XmlStreamReader*, bool preview) override;
    void init();
    void addCurve(XYCurve* curve, CustomPoint *custompoint= nullptr);
    void addCurvePath(QString &curvePath, CustomPoint* custompoint = nullptr);
    bool assignCurve(const QVector<XYCurve*> &curves);
    void removeCurve(XYCurve* curve);

    QGraphicsItem* graphicsItem() const override;

    struct MarkerPoints_T{
        CustomPoint* customPoint;
        const XYCurve* curve;
        QString curvePath;
    };

    bool isVisible() const override;
    void setVisible(bool on) override;
    void setPrinting(bool on) override;

    void retransform() override;
    void handleResize(double horizontalRatio, double verticalRatio, bool pageResize) override;
public slots:
    void labelPositionChanged(TextLabel::PositionWrapper position);
    void pointPositionChanged(QPointF pos);
protected:
    WorksheetInfoElementPrivate* const d_ptr;
private:
    TextLabel* label;
    Q_DECLARE_PRIVATE(WorksheetInfoElement)
    QVector<struct MarkerPoints_T> markerpoints;
};

#endif // WORKSHEETINFOELEMENT_H
