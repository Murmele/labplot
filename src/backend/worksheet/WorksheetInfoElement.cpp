/***************************************************************************
	File                 : WorksheetInfoElement.cpp
	Project              : LabPlot
	Description          : Marker which can highlight points of curves and
						   show their values
	--------------------------------------------------------------------
	Copyright            : (C) 2019 Martin Marmsoler (martin.marmsoler@gmail.com)
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

#include "WorksheetInfoElement.h"

#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
#include "backend/worksheet/plots/cartesian/XYCurve.h"
#include "backend/worksheet/plots/cartesian/CartesianCoordinateSystem.h"
#include "backend/worksheet/WorksheetInfoElementPrivate.h"
#include "backend/worksheet/plots/cartesian/CustomPoint.h"
#include "backend/worksheet/TextLabel.h"
#include "backend/lib/commandtemplates.h"
#include "backend/lib/XmlStreamReader.h"

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QKeyEvent>
#include <QAction>
#include <QMenu>
#include <QTextEdit>
#include <QDateTime>


WorksheetInfoElement::WorksheetInfoElement(const QString &name, CartesianPlot *plot):
	WorksheetElement(name),
	d_ptr(new WorksheetInfoElementPrivate(this,plot))
{
	Q_D(WorksheetInfoElement);
	init();
	setVisible(false);
	d->retransform();
}

WorksheetInfoElement::WorksheetInfoElement(const QString &name, CartesianPlot *plot, const XYCurve *curve, double pos):
	WorksheetElement(name, AspectType::WorksheetInfoElement),
    // must be at least, because otherwise label ist not a nullptr
    d_ptr(new WorksheetInfoElementPrivate(this,plot,curve))
{
	Q_D(WorksheetInfoElement);

	init();

	if (curve) {
		CustomPoint* custompoint = new CustomPoint(plot, "Markerpoint");
		addChild(custompoint);
		struct WorksheetInfoElement::MarkerPoints_T markerpoint = {custompoint, curve, curve->path()};
		markerpoints.append(markerpoint);
		// setpos after label was created
		bool valueFound;
		double xpos;
		double y = curve->y(pos,xpos,valueFound);
		if (valueFound) {
			d->x_pos = xpos;
			markerpoints.last().x = xpos;
			markerpoints.last().y = y;
			custompoint->setPosition(QPointF(xpos,y));
			DEBUG("Value found");
		} else {
			d->x_pos = 0;
			markerpoints.last().x = 0;
			markerpoints.last().y = 0;
			custompoint->setPosition(d->cSystem->mapSceneToLogical(QPointF(0,0)));
			DEBUG("Value not found");
		}

		setVisible(true);
	}else
		setVisible(false);

	TextLabel::TextWrapper text;
	text.placeHolder = true;

	if (!markerpoints.empty()) {
		QString textString;
		textString = QString::number(markerpoints[0].x)+ ", ";
		textString.append(QString(QString(markerpoints[0].curve->name()+":")));
		textString.append(QString::number(markerpoints[0].y));
		text.text = textString;
		// TODO: Find better solution than using textedit
		QTextEdit textedit(QString("&(x), ")+ QString(markerpoints[0].curve->name()+":"+"&("+markerpoints[0].curve->name()+")"));
		text.textPlaceHolder = textedit.toHtml();
	} else
		text.textPlaceHolder = "Please Add Text here";
	label->setText(text);

	d->retransform();
}

WorksheetInfoElement::~WorksheetInfoElement() {
}

void WorksheetInfoElement::init() {

	Q_D(WorksheetInfoElement);

	initActions();
	initMenus();

	connect(this, &WorksheetInfoElement::aspectRemoved, this, &WorksheetInfoElement::childRemoved);
	connect(this, &WorksheetInfoElement::aspectAdded, this, &WorksheetInfoElement::childAdded);

	label = new TextLabel("Markerlabel", d->plot);
	addChild(label);
	label->enableCoordBinding(true);
	label->setCoordBinding(true);
	TextLabel::TextWrapper text;
	text.placeHolder = true;
	label->setText(text); // set placeHolder to true
}

void WorksheetInfoElement::initActions() {
	visibilityAction = new QAction(i18n("Visible"), this);
	visibilityAction->setCheckable(true);
	connect(visibilityAction, &QAction::triggered, this, &WorksheetInfoElement::setVisible);
}

void WorksheetInfoElement::initMenus() {
	m_menusInitialized = true;
}

QMenu* WorksheetInfoElement::createContextMenu() {
	if (!m_menusInitialized)
		initMenus();

	QMenu* menu = WorksheetElement::createContextMenu();
	QAction* firstAction = menu->actions().at(1);

	visibilityAction->setChecked(isVisible());
	menu->insertAction(firstAction, visibilityAction);

	return menu;
}
/*!
 * @brief WorksheetInfoElement::addCurve
 * Adds a new markerpoint to the plot which is placed on the curve curve
 * @param curve Curve on which the markerpoints sits
 * @param custompoint Use existing point, if the project was loaded the custompoint can have different settings
 */
void WorksheetInfoElement::addCurve(const XYCurve* curve, CustomPoint* custompoint) {
	Q_D(WorksheetInfoElement);

	for (auto markerpoint: markerpoints) {
		if (curve == markerpoint.curve)
			return;
	}
	if (!custompoint) {
		custompoint = new CustomPoint(d->plot, "Markerpoint");
		addChild(custompoint);
		bool valueFound;
		double x_new;
		double y = curve->y(d->x_pos, x_new, valueFound);
		custompoint->setPosition(QPointF(x_new,y));
	} else
		addChild(custompoint);

	struct MarkerPoints_T markerpoint = {custompoint, curve, curve->path()};
	markerpoints.append(markerpoint);
}

/*!
 * \brief WorksheetInfoElement::addCurvePath
 * When loading worksheetinfoelement from xml file, there is no information available, which curves are loaded.
 * So only the path will be stored and after all curves where loaded the curves will be assigned to the WorksheetInfoElement
 * with the function assignCurve
 * Assumption: if custompoint!=nullptr then the custompoint was already added to the WorksheetInfoElement previously. Here
 * only new created CustomPoints will be added to the WorksheetInfoElement
 * @param curvePath path from the curve
 * @param custompoint adding already created custom point
 */
void WorksheetInfoElement::addCurvePath(QString &curvePath, CustomPoint* custompoint) {
	Q_D(WorksheetInfoElement);

	for(auto markerpoint: markerpoints) {
		if(curvePath == markerpoint.curvePath)
			return;
	}
	if (!custompoint) {
		custompoint = new CustomPoint(d->plot, "Markerpoint");
		custompoint->setVisible(false);
		addChild(custompoint);
	}
	struct MarkerPoints_T markerpoint = {custompoint, nullptr, curvePath};
	markerpoints.append(markerpoint);
}

/*!
 * \brief assignCurve
 * Finds the curve with the path stored in the markerpoints and assigns the pointer to markerpoints
 * @param curves
 * \return true if all markerpoints are assigned with a curve, false if one or more markerpoints don't have a curve assigned
 */
bool WorksheetInfoElement::assignCurve(const QVector<XYCurve *> &curves) {

	for (int i =0; i< markerpoints.length(); i++) {
		for (auto curve: curves) {
			QString curvePath = curve->path();
			if(markerpoints[i].curvePath == curve->path()) {
				markerpoints[i].curve = curve;
				break;
			}
		}
	}

	// check if all markerpoints have a valid curve otherwise return false
	for (auto markerpoint: markerpoints){
		if (markerpoint.curve == nullptr)
			return false;
	}
	return true;
}

/*!
 * Remove markerpoint from a curve
 * @param curve
 */
void WorksheetInfoElement::removeCurve(const XYCurve* curve) {
	for (int i=0; i< markerpoints.length(); i++) {
		if (markerpoints[i].curve == curve)
			removeChild(markerpoints[i].customPoint);
	}
}

/*!
 * Set the z value of the label and the custompoints higher than the worksheetinfoelement
 * @param value
 */
void WorksheetInfoElement::setZValue(qreal value) {
	graphicsItem()->setZValue(value);

	label->setZValue(value+1);

	for (auto markerpoint: markerpoints)
		markerpoint.customPoint->setZValue(value+1);
}

/*!
 * Returns the amount of markerpoints. Used in the WorksheetInfoElementDock to fill listWidget.
 */
int WorksheetInfoElement::markerPointsCount() {
	return markerpoints.length();
}

/*!
 * Returns the Markerpoint at index \p index. Used in the WorksheetInfoElementDock to fill listWidget
 * @param index
 */
WorksheetInfoElement::MarkerPoints_T WorksheetInfoElement::markerPointAt(int index) {
	return markerpoints.at(index);
}

/*!
 * create Text which will be shown in the TextLabel
 * @return Text
 */
TextLabel::TextWrapper WorksheetInfoElement::createTextLabelText() {

	if (!label)
		return TextLabel::TextWrapper();
	// TODO: save positions of the variables in extra variables to replace faster, because replace takes long time
	TextLabel::TextWrapper wrapper = label->text();

	AbstractColumn::ColumnMode columnMode = markerpoints[0].curve->xColumn()->columnMode();
	QString placeHolderText = wrapper.textPlaceHolder;
	if (!wrapper.teXUsed) {
		double value = markerpoints[0].x;
		if (columnMode== AbstractColumn::ColumnMode::Numeric ||
			columnMode == AbstractColumn::ColumnMode::Integer)
			placeHolderText.replace("&amp;(x)",QString::number(value));
		else if (columnMode== AbstractColumn::ColumnMode::Day ||
				 columnMode == AbstractColumn::ColumnMode::Month ||
				 columnMode == AbstractColumn::ColumnMode::DateTime) {
			QDateTime dateTime;
			dateTime.setTime_t(value);
			QString dateTimeString = dateTime.toString();
			placeHolderText.replace("&amp;(x)",dateTimeString);
		}
	} else {
		if (columnMode== AbstractColumn::ColumnMode::Numeric ||
			columnMode == AbstractColumn::ColumnMode::Integer)
			placeHolderText.replace("&(x)",QString::number(markerpoints[0].x));
		else if (columnMode== AbstractColumn::ColumnMode::Day ||
				 columnMode == AbstractColumn::ColumnMode::Month ||
				 columnMode == AbstractColumn::ColumnMode::DateTime) {
			QDateTime dateTime;
			dateTime.setTime_t(markerpoints[0].x);
			QString dateTimeString = dateTime.toString();
			placeHolderText.replace("&(x)",dateTimeString);
		}
	}

	for (int i=0; i< markerpoints.length(); i++){

		QString replace;
		if(!wrapper.teXUsed)
			replace = QString("&amp;(");
		else
			replace = QString("&(");

		replace+=  markerpoints[i].curve->name() + QString(")");
		placeHolderText.replace(replace, QString::number(markerpoints[i].y));
	}
	wrapper.text = placeHolderText;
	return wrapper;
}

/*!
 * Returns plot, where this marker is used. Needed in the worksheetinfoelement Dock
 * @return
 */
CartesianPlot* WorksheetInfoElement::getPlot() {
	Q_D(WorksheetInfoElement);
	return d->plot;
}

bool WorksheetInfoElement::isVisible() const {
	Q_D(const WorksheetInfoElement);
	return d->visible;
}

/*!
 * Will be called, when the label changes his position
 * @param position
 */
void WorksheetInfoElement::labelPositionChanged(TextLabel::PositionWrapper position) {
	Q_UNUSED(position)
	Q_D(WorksheetInfoElement);
	d->retransform();
}

/*!
 * Delete child and remove from markerpoint list if it is a markerpoint. If it is a textlabel delete complete WorksheetInfoElement
 */
void WorksheetInfoElement::childRemoved(const AbstractAspect* parent, const AbstractAspect* before, const AbstractAspect* child) {
	Q_D(WorksheetInfoElement);

	if (m_suppressChildRemoved)
		return;

	if (parent != this)
		return;

	const CustomPoint* point = dynamic_cast<const CustomPoint*> (child);
	if (point != nullptr){
		for (int i =0; i< markerpoints.length(); i++)
			if (point == markerpoints[i].customPoint)
				markerpoints.removeAt(i);
	}
	if (markerpoints.empty()) {
		m_suppressChildRemoved = true;
		removeChild(label);
		m_suppressChildRemoved = false;
		remove();
	} else
		d->retransform();

	const TextLabel* textlabel = dynamic_cast<const TextLabel*>(child);
	if (label != nullptr) {
		if (textlabel == label) {
			for (auto markerpoint : markerpoints) {
				m_suppressChildRemoved = true;
				removeChild(markerpoint.customPoint);
				m_suppressChildRemoved = false;
			}
			remove();
		}
	}
}

void WorksheetInfoElement::childAdded(const AbstractAspect* child) {
	Q_D(const WorksheetInfoElement);
	const CustomPoint* point = dynamic_cast<const CustomPoint*>(child);
	if (point) {
		connect(point, &CustomPoint::positionChanged, this, &WorksheetInfoElement::pointPositionChanged);

		CustomPoint* p = const_cast<CustomPoint*>(point);
		p->setParentGraphicsItem(d->plot->graphicsItem());
		return;
	}

	const TextLabel* labelChild = dynamic_cast<const TextLabel*>(child);
	if (labelChild) {
		connect(label, &TextLabel::positionChanged, this, &WorksheetInfoElement::labelPositionChanged);

		TextLabel* l = const_cast<TextLabel*>(labelChild);
		l->setParentGraphicsItem(d->plot->graphicsItem());
	}
}
/*!
 * Will be called, when the customPoint changes his position
 * @param pos
 */
void WorksheetInfoElement::pointPositionChanged(QPointF pos) {
	Q_UNUSED(pos)
	Q_D(WorksheetInfoElement);

	if (m_suppressPointPositionChanged)
		return;

	CustomPoint* point = dynamic_cast<CustomPoint*>(QObject::sender());
	if (point == nullptr)
		return;

	// TODO: Find better solution, this is not a good solution!
	// Problem: pointPositionChanged will also be called outside of itemchange() in custompoint. Don't know why
	//    if (!point->graphicsItem()->flags().operator&=(QGraphicsItem::ItemSendsGeometryChanges))
	//        return;

	// caĺculate new y value
	double x = point->position().x();
	double x_new;
	for (int i=0; i<markerpoints.length(); i++) {
		bool valueFound;
		double y = markerpoints[i].curve->y(x,x_new, valueFound);
		d->x_pos = x_new;
		if (valueFound) {
			m_suppressPointPositionChanged = true;
			markerpoints[i].customPoint->graphicsItem()->setFlag(QGraphicsItem::ItemSendsGeometryChanges, false);
			DEBUG("WorksheetInfoElement::pointPositionChanged, Set Position: ("<< x_new << "," << y << ")");
			markerpoints[i].customPoint->setPosition(QPointF(x_new,y));
			markerpoints[i].customPoint->graphicsItem()->setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
			QPointF position = d->cSystem->mapSceneToLogical(markerpoints[i].customPoint->graphicsItem()->pos());
			m_suppressPointPositionChanged = false;
		}
	}
	label->setText(createTextLabelText());
	d->retransform();
}

void WorksheetInfoElement::setParentGraphicsItem(QGraphicsItem* item) {
	Q_D(WorksheetInfoElement);
	d->setParentItem(item);
	d->updatePosition();
}

QGraphicsItem* WorksheetInfoElement::graphicsItem() const {
	return d_ptr;
}

void WorksheetInfoElement::setPrinting(bool on) {
	Q_D(WorksheetInfoElement);
	d->m_printing = on;
}

void WorksheetInfoElement::retransform() {
	Q_D(WorksheetInfoElement);
	d->retransform();
}
void WorksheetInfoElement::handleResize(double horizontalRatio, double verticalRatio, bool pageResize) {

}

//##############################################################################
//######  Getter and setter methods ############################################
//##############################################################################

/* ============================ getter methods ================= */
BASIC_SHARED_D_READER_IMPL(WorksheetInfoElement, bool, xposLineVisible, xposLineVisible);
BASIC_SHARED_D_READER_IMPL(WorksheetInfoElement, double, xposLineWidth, xposLineWidth);
BASIC_SHARED_D_READER_IMPL(WorksheetInfoElement, QColor, xposLineColor, xposLineColor);
BASIC_SHARED_D_READER_IMPL(WorksheetInfoElement, double, connectionLineWidth, connectionLineWidth);
BASIC_SHARED_D_READER_IMPL(WorksheetInfoElement, QColor, connectionLineColor, connectionLineColor);
BASIC_SHARED_D_READER_IMPL(WorksheetInfoElement, bool, visible, visible);
/* ============================ setter methods ================= */

// Problem: No member named 'Private' in 'WorksheetInfoElement':
// Solution:
// Define "typedef  WorksheetInfoElementPrivate Private;" in public section
// of WorksheetInfoElement

// Problem: WorksheetInfoElementPrivate has no member named 'name'
// Solution: implement function name()

STD_SETTER_CMD_IMPL_F_S(WorksheetInfoElement, SetXPosLineVisible, bool, xposLineVisible, updateXPosLine);
void WorksheetInfoElement::setXPosLineVisible(const bool xposLineVisible) {
	Q_D(WorksheetInfoElement);
	if (xposLineVisible != d->xposLineVisible)
		exec(new WorksheetInfoElementSetXPosLineVisibleCmd(d, xposLineVisible, ki18n("%1: set vertical line visible")));
}

STD_SETTER_CMD_IMPL_F_S(WorksheetInfoElement, SetXPosLineWidth, double, xposLineWidth, updateXPosLine);
void WorksheetInfoElement::setXPosLineWidth(const double xposLineWidth) {
	Q_D(WorksheetInfoElement);
	if (xposLineWidth != d->xposLineWidth)
		exec(new WorksheetInfoElementSetXPosLineWidthCmd(d, xposLineWidth, ki18n("%1: set vertical line width")));
}

STD_SETTER_CMD_IMPL_F_S(WorksheetInfoElement, SetXPosLineColor, QColor, xposLineColor, updateXPosLine);
void WorksheetInfoElement::setXPosLineColor(const QColor xposLineColor) {
	Q_D(WorksheetInfoElement);
	if (xposLineColor != d->xposLineColor)
		exec(new WorksheetInfoElementSetXPosLineColorCmd(d, xposLineColor, ki18n("%1: set vertical line color")));
}

STD_SETTER_CMD_IMPL_F_S(WorksheetInfoElement, SetConnectionLineWidth, double, connectionLineWidth, updateConnectionLine);
void WorksheetInfoElement::setConnectionLineWidth(const double connectionLineWidth) {
	Q_D(WorksheetInfoElement);
	if (connectionLineWidth != d->connectionLineWidth)
		exec(new WorksheetInfoElementSetConnectionLineWidthCmd(d, connectionLineWidth, ki18n("%1: set connection line width")));
}

STD_SETTER_CMD_IMPL_F_S(WorksheetInfoElement, SetConnectionLineColor, QColor, connectionLineColor, updateConnectionLine);
void WorksheetInfoElement::setConnectionLineColor(const QColor connectionLineColor) {
	Q_D(WorksheetInfoElement);
	if (connectionLineColor != d->connectionLineColor)
		exec(new WorksheetInfoElementSetConnectionLineColorCmd(d, connectionLineColor, ki18n("%1: set connection line color")));
}

STD_SETTER_CMD_IMPL_F_S(WorksheetInfoElement, SetVisible, bool, visible, visibilityChanged);
void WorksheetInfoElement::setVisible(const bool visible) {
	Q_D(WorksheetInfoElement);
	if (visible != d->visible)
		exec(new WorksheetInfoElementSetVisibleCmd(d, visible, ki18n("%1: set visible")));
}

//##############################################################################
//######  SLOTs for changes triggered via QActions in the context menu  ########
//##############################################################################

//##############################################################################
//####################### Private implementation ###############################
//##############################################################################

WorksheetInfoElementPrivate::WorksheetInfoElementPrivate(WorksheetInfoElement* owner,CartesianPlot *plot):
	q(owner),
	plot(plot),
	xposLineWidth(5),
	connectionLineWidth(5),
	xposLineVisible(true),
	connectionLineColor(QColor(Qt::black)),
	xposLineColor(QColor(Qt::black)),
	visible(true)
{
	init();
}

WorksheetInfoElementPrivate::WorksheetInfoElementPrivate(WorksheetInfoElement* owner, CartesianPlot *plot, const XYCurve* curve):
	q(owner),
	plot(plot),
	xposLineWidth(5),
	connectionLineWidth(5),
	xposLineVisible(true),
	connectionLineColor(QColor(Qt::black)),
	xposLineColor(QColor(Qt::black)),
	visible(true)
{
	init();
}

void WorksheetInfoElementPrivate::init() {

	setFlag(QGraphicsItem::ItemIsMovable, false);
	setFlag(QGraphicsItem::ItemClipsChildrenToShape, true);
	setFlag(QGraphicsItem::ItemIsSelectable, true);
	setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
	setFlag(QGraphicsItem::ItemIsFocusable, true);

	if(plot)
		cSystem =  dynamic_cast<const CartesianCoordinateSystem*>(plot->coordinateSystem());
	else
		cSystem = nullptr;
}

QString WorksheetInfoElementPrivate::name() const {
	return q->name();
}

/*!
	calculates the position and the bounding box of the WorksheetInfoElement. Called on geometry changes.
	Or when the label or the point where moved
 */
void WorksheetInfoElementPrivate::retransform() {

	if (!q->label)
		return;
	if (q->markerpoints.isEmpty())
		return;

	// TODO: find better solution
	// Update position
	q->label->graphicsItem()->setFlag(QGraphicsItem::ItemSendsGeometryChanges, false);
	q->label->retransform();
	q->label->graphicsItem()->setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);

	double zValueMin = q->label->graphicsItem()->zValue();
	// TODO: find better solution
	for (auto markerpoint: q->markerpoints) {
		markerpoint.customPoint->graphicsItem()->setFlag(QGraphicsItem::ItemSendsGeometryChanges, false);
		markerpoint.customPoint->retransform();
		double zValuePoint = markerpoint.customPoint->graphicsItem()->zValue();
		if (zValuePoint < zValueMin)
			zValueMin = zValuePoint;
		markerpoint.customPoint->graphicsItem()->setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
	}
	setZValue(zValueMin-0.001); // low enough that it is not behind other elements

	// line goes to the fist pointPos
	// TODO: better would be to direct to the highest point or also possible to make it changeable
	QPointF pointPos = cSystem->mapLogicalToScene(q->markerpoints[0].customPoint->position());
	QRectF labelSize = q->label->getSize();

	QPointF labelPos = cSystem->mapLogicalToScene(q->label->getLogicalPos());

	double difference_x = pointPos.x() - labelPos.x();
	double difference_y = pointPos.y() - labelPos.y();

	double x,y;
	QPointF min_scene = cSystem->mapLogicalToScene(QPointF(plot->xMin(),plot->yMin()));
	QPointF max_scene = cSystem->mapLogicalToScene(QPointF(plot->xMax(),plot->yMax()));

	y = abs(max_scene.y()-min_scene.y())/2;
	x = abs(max_scene.x()-min_scene.x())/2;
	double xmax = x;
	QPointF labelPosItemCoords = mapFromParent(labelPos); // calculate item coords from scene coords
	QPointF pointPosItemCoords = mapFromParent(pointPos); // calculate item coords from scene coords

	double w = abs(labelPosItemCoords.x()-pointPosItemCoords.x());//abs(difference_x);
	double h = abs(labelPosItemCoords.y()-pointPosItemCoords.y());//abs(difference_y);


	setFlag(QGraphicsItem::ItemSendsGeometryChanges, false);

	xposLine = QLineF(pointPosItemCoords.x(), 0, pointPosItemCoords.x(), 2*y);

	double boundingRectX,boundingRectY,boundingRectW,boundingRectH;

	QPointF itemPos;
	//DEBUG("PointPosX: " << pointPos.x() << "Logical: " << cSystem->mapSceneToLogical(pointPos).x() << "Itemcoord: " << pointPosItemCoords.x());
	//DEBUG("LabelPosX: " << labelPos.x() << "Logical: " << cSystem->mapSceneToLogical(labelPos).x() << "Itemcoord: " << labelPosItemCoords.x());
	if (w > h) {
		// attach to right or left border of the textlabel
		if (difference_x > 0) {
			// point is more right than textlabel
			// attach to right border of textlabel
			x = pointPos.x() - w/2;

			if (difference_y >0) {
				// point is lower than TextLabel
				//DEBUG("RIGHT: point is more right and lower than TextLabel");
				connectionLine = QLineF(labelPosItemCoords.x()+labelSize.width()/2,labelPosItemCoords.y(),pointPosItemCoords.x(),pointPosItemCoords.y());
				sceneDeltaPoint = QPointF(w/2,pointPosItemCoords.y()); // w/2
				sceneDeltaTextLabel = QPointF(-w/2,labelPosItemCoords.y());
			} else {
				// point is higher than TextLabel
				//DEBUG("RIGHT: point is more right and higher than TextLabel");
				connectionLine = QLineF(labelPosItemCoords.x()+labelSize.width()/2,labelPosItemCoords.y(),pointPosItemCoords.x(),pointPosItemCoords.y());
				sceneDeltaPoint = QPointF(w/2,pointPosItemCoords.y());
				sceneDeltaTextLabel = QPointF(-w/2,labelPosItemCoords.y());
			}
			boundingRectX = labelPosItemCoords.x()+labelSize.width()/2;
			boundingRectW = w-labelSize.width()/2;
		} else {
			// point is more left than textlabel
			// attach to left border
			x = pointPos.x()+w/2;
			if (difference_y < 0) {
				// point is higher than TextLabel
				//DEBUG("LEFT: point is more left and higher than TextLabel");
				connectionLine = QLineF(pointPosItemCoords.x(),pointPosItemCoords.y(),labelPosItemCoords.x()-labelSize.width()/2,labelPosItemCoords.y());
				sceneDeltaPoint = QPointF(-w/2,pointPosItemCoords.y());
				sceneDeltaTextLabel = QPointF(w/2,labelPosItemCoords.y());
			} else {
				// point is lower than TextLabel
				//DEBUG("LEFT: point is more left and lower than TextLabel");
				connectionLine = QLineF(pointPosItemCoords.x(),pointPosItemCoords.y(),labelPosItemCoords.x()-labelSize.width()/2,labelPosItemCoords.y());
				sceneDeltaPoint = QPointF(-w/2,pointPosItemCoords.y());
				sceneDeltaTextLabel = QPointF(w/2,labelPosItemCoords.y());
			}
			if (pointPos.x() < labelPos.x()-labelSize.width()/2) {
				boundingRectX = pointPosItemCoords.x()-connectionLineWidth/2;
				boundingRectW = w-labelSize.width()/2+connectionLineWidth;
			} else {
				boundingRectX = labelPosItemCoords.x()-labelSize.width()/2-connectionLineWidth/2;
				boundingRectW = labelSize.width()/2+connectionLineWidth;
			}
		}
	} else {
		// attach to top or bottom border of the textlabel
		if (difference_y < 0) {
			// attach to top border
			if (difference_x > 0) {
				// point is more right than TextLabel
				//DEBUG("TOP: point is more right and higher than TextLabel");
				x = labelPos.x()+w/2;
				connectionLine = QLineF(labelPosItemCoords.x(),labelPosItemCoords.y()-labelSize.height()/2,pointPosItemCoords.x(),pointPosItemCoords.y());
				sceneDeltaPoint = QPointF(w/2,pointPosItemCoords.y());
				sceneDeltaTextLabel = QPointF(-w/2,labelPosItemCoords.y());
				boundingRectX = labelPosItemCoords.x();
				boundingRectW = w;
			} else {
				// point is more left than TextLabel
				//DEBUG("TOP: point is more left and higher than TextLabel");
				x = pointPos.x()+w/2;
				connectionLine = QLineF(pointPosItemCoords.x(),pointPosItemCoords.y(),labelPosItemCoords.x(),labelPosItemCoords.y()-labelSize.height()/2);
				sceneDeltaPoint = QPointF(-w/2,pointPosItemCoords.y());
				sceneDeltaTextLabel = QPointF(w/2,labelPosItemCoords.y());
				boundingRectX = pointPosItemCoords.x();
				boundingRectW = w;
			}
		} else {
			// attach to bottom border
			if (difference_x > 0) {
				// point is more right than TextLabel
				//DEBUG("BOTTOM: point is more right and lower than TextLabel");
				x = labelPos.x()+ w/2;
				connectionLine = QLineF(labelPosItemCoords.x(),labelPosItemCoords.y()+labelSize.height()/2,pointPosItemCoords.x(),pointPosItemCoords.y());
				sceneDeltaPoint = QPointF(w/2,pointPosItemCoords.y());
				sceneDeltaTextLabel = QPointF(-w/2,labelPosItemCoords.y());
				boundingRectX = labelPosItemCoords.x();
				boundingRectW = w;
			} else {
				// point is more left than TextLabel
				//DEBUG("BOTTOM: point is more left and lower than TextLabel");
				x = pointPos.x()+w/2;
				connectionLine = QLineF(pointPosItemCoords.x(),pointPosItemCoords.y(),labelPosItemCoords.x(),labelPosItemCoords.y()+labelSize.height()/2);
				sceneDeltaPoint = QPointF(-w/2,pointPosItemCoords.y());
				sceneDeltaTextLabel = QPointF(w/2,labelPosItemCoords.y());
				boundingRectX = pointPosItemCoords.x();
				boundingRectW = w;
			}
		}
	}

	boundingRectangle.setX(boundingRectX-2);
	boundingRectangle.setWidth(boundingRectW+4);
	//DEBUG("ConnectionLine: P1.x: " << (connectionLine.p1()).x() << "P2.x: " << (connectionLine.p2()).x());
	itemPos.setX(x); // x is always between the labelpos and the point pos
	if (max_scene.y() < min_scene.y())
		itemPos.setY(max_scene.y());
	else
		itemPos.setY(min_scene.y());

	if (max_scene.x() < min_scene.x())
		itemPos.setX(max_scene.x());
	else
		itemPos.setX(min_scene.x());

	setPos(itemPos);
	boundingRectangle.setX(0);
	boundingRectangle.setWidth(2*xmax);
	boundingRectangle.setY(0);
	boundingRectangle.setHeight(2*y);

	update(boundingRect());
	setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
}

void WorksheetInfoElementPrivate::updatePosition() {

}

/*!
 * Repainting to update xposLine
 */
void WorksheetInfoElementPrivate::updateXPosLine() {
	update(boundingRectangle);
}

/*!
 * Repainting to updateConnectionLine
 */
void WorksheetInfoElementPrivate::updateConnectionLine() {
	update(boundingRect());
}

void WorksheetInfoElementPrivate::visibilityChanged() {

	for(auto markerpoint: q->markerpoints) {
		markerpoint.customPoint->setVisible(visible);
	}
	if(q->label)
		q->label->setVisible(visible);
	update(boundingRect());
}


//reimplemented from QGraphicsItem
QRectF WorksheetInfoElementPrivate::boundingRect() const {
	return boundingRectangle;
}

void WorksheetInfoElementPrivate::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget* widget) {
	if (!visible)
		return;
	//DEBUG("Position: " << pos().x() << ", Y: "<< pos().y());
	//    painter->fillRect(boundingRectangle,QBrush(QColor(255,0,0,128)));
	//    if(boundingRectangle.width() > 40)
	//        painter->fillRect(QRectF(-20,0,40,40),QBrush(QColor(0,255,0,255)));
	//    else {
	//        painter->fillRect(QRectF(-boundingRectangle.width()/2,0,boundingRectangle.width(),40),QBrush(QColor(0,255,0,255)));
	//    }

	QPen pen(connectionLineColor, connectionLineWidth);
	painter->setPen(pen);
	painter->drawLine(connectionLine);

	// draw vertical line, which connects all points together
	if (xposLineVisible) {
		pen = QPen(xposLineColor, xposLineWidth);
		painter->setPen(pen);
		painter->drawLine(xposLine);
	}
}

QVariant WorksheetInfoElementPrivate::itemChange(GraphicsItemChange change, const QVariant &value) {
	return QGraphicsItem::itemChange(change, value);
}

void WorksheetInfoElementPrivate::mousePressEvent(QGraphicsSceneMouseEvent* event) {
	if (event->button() == Qt::MouseButton::LeftButton) {

		if (xposLineVisible) {
			if (abs(xposLine.x1()-event->pos().x())< ((xposLineWidth < 3)? 3: xposLineWidth)) {
				if (!isSelected());
					setSelected(true);
				m_suppressKeyPressEvents = false;
				oldMousePos = mapToParent(event->pos());
				event->accept();
				setFocus();
				return;
			}
		}

		// https://stackoverflow.com/questions/11604680/point-laying-near-line
		double dx12 = connectionLine.x2()-connectionLine.x1();
		double dy12 = connectionLine.y2()-connectionLine.y1();
		double vecLenght = sqrt(pow(dx12,2)+pow(dy12,2));
		QPointF unitvec(dx12/vecLenght,dy12/vecLenght);

		double dx1m = event->pos().x() - connectionLine.x1();
		double dy1m = event->pos().y() - connectionLine.y1();

		double dist_segm = abs(dx1m*unitvec.y() - dy1m*unitvec.x());
		double scalar_product = dx1m*unitvec.x()+dy1m*unitvec.y();
		DEBUG("DIST_SEGMENT   " << dist_segm << "SCALAR_PRODUCT: " << scalar_product << "VEC_LENGTH: " << vecLenght);

		if (scalar_product > 0) {
			if (scalar_product < vecLenght && dist_segm < ((connectionLineWidth < 3) ? 3: connectionLineWidth)) {
				event->accept();
				if (!isSelected())
					setSelected(true);
				oldMousePos = mapToParent(event->pos());
				m_suppressKeyPressEvents = false;
				event->accept();
				setFocus();
				return;
			}
		}

		m_suppressKeyPressEvents = true;
		event->ignore();
		if (isSelected())
			setSelected(false);
	}
	QGraphicsItem::mousePressEvent(event);
}

void WorksheetInfoElementPrivate::mouseMoveEvent(QGraphicsSceneMouseEvent* event) {

	QPointF eventPos = mapToParent(event->pos());
	DEBUG("EventPos: " << eventPos.x() << " Y: " << eventPos.y());
	QPointF delta = eventPos - oldMousePos;

	QPointF eventLogicPos = cSystem->mapSceneToLogical(eventPos);
	QPointF delta_logic =  eventLogicPos - cSystem->mapSceneToLogical(oldMousePos);

	if (!q->label)
		return;
	if (q->markerpoints.isEmpty())
		return;

	for (auto markerpoint: q->markerpoints)
		markerpoint.customPoint->graphicsItem()->setFlag(QGraphicsItem::ItemSendsGeometryChanges, false);

	q->label->graphicsItem()->setFlag(QGraphicsItem::ItemSendsGeometryChanges, false);
	double x = q->markerpoints[0].x+delta_logic.x();
	DEBUG("markerpoints[0].x: " << q->markerpoints[0].x << ", markerpoints[0].y: " << q->markerpoints[0].y << ", Scene xpos: " << x);
	for (int i =0; i < q->markerpoints.length(); i++) {
		bool valueFound;
		double x_new;

		double y;
		if (q->markerpoints[i].curve)
			y = q->markerpoints[i].curve->y(x, x_new, valueFound);
		else {
			valueFound = false;
			y = 0;
		}

		if (valueFound) {
			q->markerpoints[i].y = y;
			q->markerpoints[i].x = x_new;
			q->m_suppressPointPositionChanged = true;
			q->markerpoints[i].customPoint->setPosition(QPointF(x_new,y));
			q->m_suppressPointPositionChanged = false;
		} else
			DEBUG("No value found for Logicalpoint" << i);
	}
	double x_label = q->label->position().point.x() + delta.x();
	double y_label = q->label->position().point.y();

	q->label->setPosition(QPointF(x_label,y_label));
	q->label->setText(q->createTextLabelText());

	for (auto markerpoint: q->markerpoints)
		markerpoint.customPoint->graphicsItem()->setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);

	q->label->graphicsItem()->setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);

	retransform();

	oldMousePos = eventPos;
}

void WorksheetInfoElementPrivate::keyPressEvent(QKeyEvent * event) {
	if (m_suppressKeyPressEvents) {
		event->ignore();
		return QGraphicsItem::keyPressEvent(event);
	}

	TextLabel::TextWrapper text;
	if (event->key() == Qt::Key_Right || event->key() == Qt::Key_Left) {
		int index;
		if (event->key() == Qt::Key_Right)
			index = 1;
		else
			index = -1;

		double x,y;
		bool valueFound;
		QPointF pointPosition;
		for (int i =0; i< q->markerpoints.length(); i++) {
			QPointF position = q->markerpoints[i].customPoint->position();
			q->markerpoints[i].curve->getNextValue(q->markerpoints[i].customPoint->position().x(), index,x,y,valueFound);
			if (valueFound) {
				q->markerpoints[i].x = x;
				q->markerpoints[i].y = y;
				pointPosition.setX(x);
				pointPosition.setY(y);
				DEBUG("X_old: " << q->markerpoints[i].customPoint->position().x() << "X_new: " << x);
				q->m_suppressPointPositionChanged = true;
				q->markerpoints[i].customPoint->setPosition(pointPosition);
				q->m_suppressPointPositionChanged = false;
			}
		}
		q->label->setText(q->createTextLabelText());
		retransform();

	}
}
//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################

void WorksheetInfoElement::save(QXmlStreamWriter* writer) const {
	Q_D(const WorksheetInfoElement);

	writer->writeStartElement( "worksheetInfoElement" );
	writeBasicAttributes(writer);
	writeCommentElement(writer);

	//geometry
	writer->writeStartElement( "geometry" );
	writer->writeAttribute( "visible", QString::number(d->visible) );
	writer->writeAttribute("connectionLineWidth", QString::number(connectionLineWidth()));
	writer->writeAttribute("connectionLineColor_r", QString::number(connectionLineColor().red()));
	writer->writeAttribute("connectionLineColor_g", QString::number(connectionLineColor().green()));
	writer->writeAttribute("connectionLineColor_b", QString::number(connectionLineColor().blue()));
	writer->writeAttribute("xposLineWidth", QString::number(xposLineWidth()));
	writer->writeAttribute("xposLineColor_r", QString::number(xposLineColor().red()));
	writer->writeAttribute("xposLineColor_g", QString::number(xposLineColor().green()));
	writer->writeAttribute("xposLineColor_b", QString::number(xposLineColor().blue()));
	writer->writeAttribute("xposLineVisible", QString::number(xposLineVisible()));
	writer->writeEndElement();


	label->save(writer);
	QString path;
	for (auto custompoint: markerpoints) {
		custompoint.customPoint->save(writer);
		path = custompoint.curve->path();
		writer->writeStartElement("markerPointCurve");
		writer->writeAttribute(QLatin1String("curvepath") , path);
		writer->writeEndElement(); // close "markerPointCurve
	}
	writer->writeEndElement(); // close "worksheetInfoElement"
}

bool WorksheetInfoElement::load(XmlStreamReader* reader, bool preview) {
	if (!readBasicAttributes(reader))
		return false;

	Q_D(WorksheetInfoElement);

	CustomPoint* markerpoint = nullptr;
	bool markerpointFound = false;
	QXmlStreamAttributes attribs;
	QString str;
	KLocalizedString attributeWarning = ki18n("Attribute '%1' missing or empty, default value is used");

	while (!reader->atEnd()) {
		reader->readNext();
		QStringRef text =  reader->text();
		if (reader->isEndElement() && reader->name() == "worksheetInfoElement")
			break;

		if (!reader->isStartElement())
			continue;

		if (!preview && reader->name() == "comment") {
			if (!readCommentElement(reader)) return false;
		} else if (reader->name() == "geometry") {
			attribs = reader->attributes();

			str = attribs.value("visible").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("x").toString());
			else
				setVisible(str.toInt());

			str = attribs.value("connectionLineWidth").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("x").toString());
			else
				setConnectionLineWidth(str.toDouble());

			str = attribs.value("connectionLineColor_r").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("x").toString());
			else
				d->connectionLineColor.setRed(str.toInt());

			str = attribs.value("connectionLineColor_g").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("x").toString());
			else
				d->connectionLineColor.setGreen(str.toInt());

			str = attribs.value("connectionLineColor_b").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("x").toString());
			else
				d->connectionLineColor.setBlue(str.toInt());

			str = attribs.value("xposLineWidth").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("x").toString());
			else
				setXPosLineWidth(str.toDouble());

			str = attribs.value("xposLineColor_r").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("x").toString());
			else
				d->xposLineColor.setRed(str.toInt());

			str = attribs.value("xposLineColor_g").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("x").toString());
			else
				d->xposLineColor.setGreen(str.toInt());

			str = attribs.value("xposLineColor_b").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("x").toString());
			else
				d->xposLineColor.setBlue(str.toInt());

			str = attribs.value("xposLineVisible").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("x").toString());
			else
				setXPosLineVisible(str.toInt());

		} else if (reader->name() == "textLabel") {
			reader->readNext();
			if (!label) {
				label = new TextLabel("TextLabel", d->plot);
				this->addChild(label);
			}
			if (!label->load(reader, preview))
				return false;
		} else if (reader->name() == "customPoint") {
			// Marker must have at least one curve
			if (markerpointFound) { // must be cleared by markerPointCurve
				delete markerpoint;
				return false;
			}
			markerpoint = new CustomPoint(d->plot, "Marker");
			if (!markerpoint->load(reader,preview)) {
				delete  markerpoint;
				return false;
			}
			this->addChild(markerpoint);
			markerpointFound = true;

		} else if (reader->name() == "markerPointCurve") {
			markerpointFound = false;
			QString path;
			attribs = reader->attributes();
			path = attribs.value("curvepath").toString();
			addCurvePath(path, markerpoint);
		}
	}

	if (markerpointFound) {
		// problem, if a markerpoint has no markerPointCurve
		delete markerpoint;
		return false;
	}
	return true;
}
