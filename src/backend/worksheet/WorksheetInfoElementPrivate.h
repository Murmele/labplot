/***************************************************************************
	File                 : WorksheetInfoElementPrivate.h
	Project              : LabPlot
	Description          : Private members of WorksheetInfoElement
	--------------------------------------------------------------------
	Copyright            : (C) 2019 by Martin Marmsoler (martin.marmsoler@gmail.com)

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
 *   Boston, MA  02110-1301  USA                                           *
 *                                                                         *
 ***************************************************************************/

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

	// TextLabel Gluepoint
	bool automaticGluePoint{true};
	int gluePointIndex{0}; // negative value means automatic mode
	// connect to this curve
	QString connectionLineCurveName;

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
	bool m_suppressKeyPressEvents{false};
};

#endif // WORKSHEETINFOELEMENTPRIVATE_H
