#ifndef WORKSHEETINFOELEMENTPRIVATE_H
#define WORKSHEETINFOELEMENTPRIVATE_H

#include "QGraphicsItem"

class WorksheetInfoElement;
class TextLabel;
class CustomPoint;
class CartesianPlot;
class CartesianCoordinateSystem;
class XYCurve;
class QGraphicsSceneMouseEvent;

class WorksheetInfoElementPrivate: public QGraphicsItem
{
public:
    WorksheetInfoElementPrivate(WorksheetInfoElement *owner, CartesianPlot *plot);
    WorksheetInfoElementPrivate(WorksheetInfoElement *owner, CartesianPlot *plot, const XYCurve *curve);
    QString name() const;

    //reimplemented from QGraphicsItem
    QRectF boundingRect() const override;
    void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget* widget = nullptr) override;
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;
    void keyPressEvent(QKeyEvent * event) override;
	void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;

    void init();
    void updatePosition();
    void retransform();
    void updateXPosLine();
    void updateConnectionLine();
    void visibilityChanged();

    bool visible;
    bool m_printing;
	double x_pos;

    QColor connectionLineColor;
    double connectionLineWidth; // drawing linewidth
    bool xposLineVisible;
    QColor xposLineColor;
    double xposLineWidth; // drawing linewidth

    CartesianPlot* plot;
    const CartesianCoordinateSystem* cSystem;

    WorksheetInfoElement* const q;
private:


    QPointF sceneDeltaPoint; // delta position from worksheetinfoElementPrivate to the first marker point in scene coords
    QPointF sceneDeltaTextLabel;

    QRectF boundingRectangle; //bounding rectangle of the connection line between CustomPoint and TextLabel
    QLineF connectionLine; // line between CustomPoint and TextLabel
	QLineF xposLine; // Line which connects all markerpoints, when there are more than 1
    QPointF oldMousePos;
};

#endif // WORKSHEETINFOELEMENTPRIVATE_H
