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

class WorksheetInfoElement : public WorksheetElement
{
    Q_OBJECT
public:
    WorksheetInfoElement(const QString& name, CartesianPlot *plot);
    WorksheetInfoElement(const QString& name, CartesianPlot *plot, const XYCurve* curve, double pos);
    void setParentGraphicsItem(QGraphicsItem* item);
	~WorksheetInfoElement();

    void save(QXmlStreamWriter*) const override;
    bool load(XmlStreamReader*, bool preview) override;
    void init();
	void initActions();
	void initMenus();
    void addCurve(XYCurve* curve, CustomPoint *custompoint= nullptr);
    void addCurvePath(QString &curvePath, CustomPoint* custompoint = nullptr);
    bool assignCurve(const QVector<XYCurve*> &curves);
    void removeCurve(XYCurve* curve);
	QMenu* createContextMenu();
    CartesianPlot* getPlot();

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
	void childRemoved();
	void visibilityChanged(bool checked);
protected:
    WorksheetInfoElementPrivate* const d_ptr;
private:
    TextLabel* label;
    Q_DECLARE_PRIVATE(WorksheetInfoElement)
    QVector<struct MarkerPoints_T> markerpoints;
	bool m_menusInitialized;

	// Actions
	QAction* visibilityAction;
};

#endif // WORKSHEETINFOELEMENT_H
