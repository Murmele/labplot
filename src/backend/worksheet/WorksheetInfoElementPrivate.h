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
    WorksheetInfoElementPrivate(WorksheetInfoElement *owner, CartesianPlot *plot);
    WorksheetInfoElementPrivate(WorksheetInfoElement *owner, CartesianPlot *plot, const XYCurve *curve);

    //reimplemented from QGraphicsItem
    QRectF boundingRect() const override;
    void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget* widget = nullptr) override;
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;
    void keyPressEvent(QKeyEvent * event) override;

    void init();
    void updatePosition();
    void setVisible(bool on);
    bool isVisible() const;
    void retransform();


    bool m_visible;
    bool m_printing;

	double x_pos;

    CartesianPlot* plot;
    const CartesianCoordinateSystem* cSystem;
private:
    WorksheetInfoElement* const q;


    QPointF sceneDeltaPoint; // delta position from worksheetinfoElementPrivate to the first marker point in scene coords
    QPointF sceneDeltaTextLabel;

    QRectF boundingRectangle; //bounding rectangle of the connection line between CustomPoint and TextLabel
    QLineF connectionLine; // line between CustomPoint and TextLabel
	QLineF xposLine; // Line which connects all markerpoints, when there are more than 1
	double xposLineWidth; // drawing linewidth
};

#endif // WORKSHEETINFOELEMENTPRIVATE_H
