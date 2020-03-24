#ifndef INFOELEMENTDOCK_H
#define INFOELEMENTDOCK_H

#include "kdefrontend/dockwidgets/BaseDock.h"

class InfoElement;

namespace Ui {
class InfoElementDock;
}

class InfoElementDock : public BaseDock {
	Q_OBJECT

public:
	explicit InfoElementDock(QWidget *parent = nullptr);
	~InfoElementDock();
	void setInfoElements(QList<InfoElement*> &list, bool sameParent);
	void initConnections();
public slots:
	void elementCurveRemoved(QString name);

private slots:
	void visibilityChanged(bool state);
	void addCurve();
	void removeCurve();
	void connectionLineWidthChanged(double width);
	void connectionLineColorChanged(QColor color);
	void xposLineWidthChanged(double value);
	void xposLineColorChanged(QColor color);
	void xposLineVisibilityChanged(bool visible);
	void connectionLineVisibilityChanged(bool visible);
	void gluePointChanged(int index);
	void curveChanged(int index);

	// slots triggered in the InfoElement
	void elementConnectionLineWidthChanged(const double width);
	void elementConnectionLineColorChanged(const QColor color);
	void elementXPosLineWidthChanged(const double width);
	void elementXposLineColorChanged(const QColor color);
	void elementXPosLineVisibleChanged(const bool visible);
	void elementConnectionLineVisibleChanged(const bool visible);
	void elementVisibilityChanged(const bool visible);
	void elementGluePointIndexChanged(const int index);
	void elementConnectionLineCurveChanged(const QString name);
	void elementLabelBorderShapeChanged(const int gluePointCount);

private:
	Ui::InfoElementDock *ui;
	InfoElement* m_element;
	QList<InfoElement*> m_elements;
	bool m_sameParent;
};

#endif // INFOELEMENTDOCK_H
