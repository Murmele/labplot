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
class QAction;
class QMenu;

class WorksheetInfoElement : public WorksheetElement {
    Q_OBJECT
public:
    WorksheetInfoElement(const QString& name, CartesianPlot *plot);
    WorksheetInfoElement(const QString& name, CartesianPlot *plot, const XYCurve* curve, double pos);
    void setParentGraphicsItem(QGraphicsItem* item);
	~WorksheetInfoElement();

    struct MarkerPoints_T{
        CustomPoint* customPoint;
        const XYCurve* curve;
        QString curvePath;
        double x; // x Value
        double y; // y Value
    };

    void save(QXmlStreamWriter*) const override;
    bool load(XmlStreamReader*, bool preview) override;
    void init();
	void initActions();
	void initMenus();
    void addCurve(const XYCurve* curve, CustomPoint *custompoint= nullptr);
    void addCurvePath(QString &curvePath, CustomPoint* custompoint = nullptr);
    bool assignCurve(const QVector<XYCurve*> &curves);
    void removeCurve(const XYCurve* curve);
    void setZValue(qreal) override;
    int markerPointsCount();
    MarkerPoints_T markerPointAt(int index);
	TextLabel::TextWrapper createTextLabelText();
	QMenu* createContextMenu();
    CartesianPlot* getPlot();
    bool isVisible() const override;

    QGraphicsItem* graphicsItem() const override;
    void setPrinting(bool on) override;

    void retransform() override;
    void handleResize(double horizontalRatio, double verticalRatio, bool pageResize) override;

    BASIC_D_ACCESSOR_DECL(bool, xposLineVisible, XPosLineVisible);
    BASIC_D_ACCESSOR_DECL(double, xposLineWidth, XPosLineWidth);
    BASIC_D_ACCESSOR_DECL(QColor, xposLineColor, XPosLineColor);
    BASIC_D_ACCESSOR_DECL(double, connectionLineWidth, ConnectionLineWidth);
    BASIC_D_ACCESSOR_DECL(QColor, connectionLineColor, ConnectionLineColor);
    BASIC_D_ACCESSOR_DECL(bool, visible, Visible);

    typedef  WorksheetInfoElementPrivate Private;

public slots:
    void labelPositionChanged(TextLabel::PositionWrapper position);
    void pointPositionChanged(QPointF pos);
    void childRemoved(const AbstractAspect *parent, const AbstractAspect *before, const AbstractAspect *child);
protected:
    WorksheetInfoElementPrivate* const d_ptr;
private:
    Q_DECLARE_PRIVATE(WorksheetInfoElement)
    TextLabel* label;
    QVector<struct MarkerPoints_T> markerpoints;
	bool m_menusInitialized;
	bool m_suppressChildRemoved;

	// Actions
	QAction* visibilityAction;
signals:
    void xposLineVisibleChanged(const bool visible);
    void xposLineWidthChanged(const double width);
    void xposLineColorChanged(const QColor color);
    void connectionLineWidthChanged(const double width);
    void connectionLineColorChanged(const QColor color);
    void visibleChanged(const bool visible);
};

#endif // WORKSHEETINFOELEMENT_H
