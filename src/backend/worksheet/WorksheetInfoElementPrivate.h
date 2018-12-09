#ifndef WORKSHEETINFOELEMENTPRIVATE_H
#define WORKSHEETINFOELEMENTPRIVATE_H

#include "QGraphicsItem"

class WorksheetInfoElement;
class TextLabel;
class CustomPoint;
class CartesianPlot;
class CartesianCoordinateSystem;
class XYCurve;

class WorksheetInfoElementPrivate: public QGraphicsItem
{
public:
    WorksheetInfoElementPrivate(WorksheetInfoElement *owner, CartesianPlot *plot, const XYCurve *curve);

    //reimplemented from QGraphicsItem
    QRectF boundingRect() const override;
    void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget* widget = nullptr) override;
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;
    void keyPressEvent(QKeyEvent * event) override;

    void updatePosition();
    void setVisible(bool on);
    bool isVisible() const;
    void retransform();

    bool m_visible;
    bool m_printing;

    TextLabel* label;
    CustomPoint* point;
private:
    QVector<const XYCurve*> curves;
    WorksheetInfoElement* const q;

    const CartesianPlot* plot;
    const CartesianCoordinateSystem* cSystem;

    QPointF sceneDeltaPoint;
    QPointF sceneDeltaTextLabel;


    QRectF boundingRectangle; //bounding rectangle of the connection line between CustomPoint and TextLabel
    QLineF connectionLine; // line between CustomPoint and TextLabel
};

#endif // WORKSHEETINFOELEMENTPRIVATE_H
