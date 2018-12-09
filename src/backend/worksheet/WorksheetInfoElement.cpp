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
#include <QPainter>
#include <QKeyEvent>

WorksheetInfoElement::WorksheetInfoElement(const QString &name, CartesianPlot *plot):
    WorksheetElement(name),
    d_ptr(new WorksheetInfoElementPrivate(this,plot))
{

}

WorksheetInfoElement::WorksheetInfoElement(const QString &name, CartesianPlot *plot, const XYCurve *curve):
	WorksheetElement(name, AspectType::WorksheetInfoElement),
    d_ptr(new WorksheetInfoElementPrivate(this,plot,curve))
{
    graphicsItem()->setFlag(QGraphicsItem::ItemIsMovable, true);
    graphicsItem()->setFlag(QGraphicsItem::ItemClipsChildrenToShape, true);
    graphicsItem()->setFlag(QGraphicsItem::ItemIsSelectable, true);
    graphicsItem()->setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    graphicsItem()->setFlag(QGraphicsItem::ItemIsFocusable, true);

    Q_D(WorksheetInfoElement);

    if(curve){
        CustomPoint* custompoint = new CustomPoint(plot, "TestPoint");
        custompoint->setParentGraphicsItem(plot->plotArea()->graphicsItem());
        connect(custompoint, &CustomPoint::positionChanged, this, &WorksheetInfoElement::pointPositionChanged);
        addChild(custompoint);
        struct WorksheetInfoElement::MarkerPoints_T markerpoint = {custompoint, curve, curve->path()};
        markerpoints.append(markerpoint);
    }
    label = new TextLabel("Test");
    addChild(label);
    label->enableCoordBinding(true,plot);
    label->setCoordBinding(true);
    label->setParentGraphicsItem(plot->plotArea()->graphicsItem());
    connect(label, &TextLabel::positionChanged, this, &WorksheetInfoElement::labelPositionChanged);

    d->init();
}

void WorksheetInfoElement::init(){

}

/*!
 * \brief WorksheetInfoElement::addCurve
 * Adds a new markerpoint to the plot which is placed on the curve curve
 * \param curve Curve on which the markerpoints sits
 * \param custompoint Use existing point, if the project was loaded the custompoint can have different settings
 */
void WorksheetInfoElement::addCurve(XYCurve* curve, CustomPoint* custompoint){
    Q_D(WorksheetInfoElement);

    for(auto markerpoint: markerpoints){
        if(curve == markerpoint.curve)
            return;
    }
    if(!custompoint){
        custompoint = new CustomPoint(d->plot, "Markerpoint");
        custompoint->setParentGraphicsItem(d->plot->plotArea()->graphicsItem());
        connect(custompoint, &CustomPoint::positionChanged, this, &WorksheetInfoElement::pointPositionChanged);
    }
    addChild(custompoint);
    struct MarkerPoints_T markerpoint = {custompoint, curve, curve->path()};
    markerpoints.append(markerpoint);
}

/*!
 * \brief WorksheetInfoElement::addCurvePath
 * When loading worksheetinfoelement from xml file, there is no information available, which curves are loaded.
 * So only the path will be stored and after all curves where loaded the curves will be assigned to the WorksheetInfoElement
 * with the function assignCurve
 * \param curvePath path from the curve
 * \param custompoint adding already created custom point
 */
void WorksheetInfoElement::addCurvePath(QString &curvePath, CustomPoint* custompoint){
    Q_D(WorksheetInfoElement);

    for(auto markerpoint: markerpoints){
        if(curvePath == markerpoint.curvePath)
            return;
    }
    if(!custompoint){
        custompoint = new CustomPoint(d->plot, "Markerpoint");
        custompoint->setParentGraphicsItem(d->plot->plotArea()->graphicsItem());
        connect(custompoint, &CustomPoint::positionChanged, this, &WorksheetInfoElement::pointPositionChanged);
    }
    addChild(custompoint);
    struct MarkerPoints_T markerpoint = {custompoint, nullptr, curvePath};
    markerpoints.append(markerpoint);
}

/*!
 * \brief assignCurve
 * Finds the curve with the path stored in the markerpoints and assigns the pointer to markerpoints
 * \param curves
 * \return true if all markerpoints are assigned with a curve, false if one or more markerpoints don't have a curve assigned
 */
bool WorksheetInfoElement::assignCurve(const QVector<XYCurve *> &curves){

    for(int i =0; i< markerpoints.length(); i++){
        for(auto curve: curves){
            if(markerpoints[i].curvePath == curve->path()){
                markerpoints[i].curve = curve;
                break;
            }
        }
    }

    // check if all markerpoints have a valid curve otherwise return false
    for(auto markerpoint: markerpoints){
        if(markerpoint.curve == nullptr){
            return false;
        }
    }
    return true;
}

/*!
 * \brief WorksheetInfoElementPrivate::removeCurve
 * Remove markerpoint from a curve
 * \param curve
 */
void WorksheetInfoElement::removeCurve(XYCurve* curve){
    for(int i=0; i< markerpoints.length(); i++){
        if(markerpoints[i].curve == curve){
            delete markerpoints[i].customPoint;
            markerpoints.remove(i);
        }
    }
}

/*!
 * \brief getPlot
 * Returns plot, where this marker is used. Needed in the worksheetinfoelement Dock
 * \return
 */
CartesianPlot* WorksheetInfoElement::getPlot(){
    Q_D(WorksheetInfoElement);
    return d->plot;
}

/*!
 * \brief WorksheetInfoElement::labelPositionChanged
 * Will be called, when the label changes his position
 * \param position
 */
void WorksheetInfoElement::labelPositionChanged(TextLabel::PositionWrapper position){
    Q_UNUSED(position)
    Q_D(WorksheetInfoElement);

    d->retransform();
}

/*!
 * \brief WorksheetInfoElement::pointPositionChanged
 * Will be called, when the customPoint changes his position
 * \param pos
 */
void WorksheetInfoElement::pointPositionChanged(QPointF pos){
    Q_UNUSED(pos)
    Q_D(WorksheetInfoElement);

    // TODO: Find better solution, this is not a good solution!
    for(auto markerpoint: markerpoints){
        if(markerpoint.customPoint->graphicsItem()->flags().operator&=(QGraphicsItem::ItemSendsGeometryChanges)){
            d->retransform();
        }
    }

}

void WorksheetInfoElement::setParentGraphicsItem(QGraphicsItem* item) {
    Q_D(WorksheetInfoElement);
    d->setParentItem(item);
    d->updatePosition();
}

QGraphicsItem* WorksheetInfoElement::graphicsItem() const{
    return d_ptr;
}

bool WorksheetInfoElement::isVisible() const{
    Q_D(const WorksheetInfoElement);
    return d->isVisible();
}

/*!
 * \brief WorksheetInfoElement::setVisible
 * Sets the visibility of the WorksheetInfoElement including label and all custom points
 * \param on
 */
void WorksheetInfoElement::setVisible(bool on){
    Q_D(WorksheetInfoElement);
    for(auto markerpoint: markerpoints){
        markerpoint.customPoint->setVisible(on);
    }
    label->setVisible(on);
    d->m_visible = on;
}
void WorksheetInfoElement::setPrinting(bool on){
    Q_D(WorksheetInfoElement);
    d->m_printing = on;
}

void WorksheetInfoElement::retransform(){
    Q_D(WorksheetInfoElement);
    d->retransform();
}
void WorksheetInfoElement::handleResize(double horizontalRatio, double verticalRatio, bool pageResize){

}

//##############################################################################
//####################### Private implementation ###############################
//##############################################################################

WorksheetInfoElementPrivate::WorksheetInfoElementPrivate(WorksheetInfoElement* owner,CartesianPlot *plot):
    q(owner),
    plot(plot),
    cSystem( dynamic_cast<const CartesianCoordinateSystem*>(plot->coordinateSystem()))
{
}

WorksheetInfoElementPrivate::WorksheetInfoElementPrivate(WorksheetInfoElement* owner,CartesianPlot *plot, const XYCurve* curve):
    q(owner),
    plot(plot),
    cSystem( dynamic_cast<const CartesianCoordinateSystem*>(plot->coordinateSystem()))
{
}

void WorksheetInfoElementPrivate::init(){

    if(q->markerpoints.length() ==0)
        return;

    bool valueFound;
    double value = q->markerpoints.first().curve->y(10,valueFound);
    if(valueFound){
        QPointF logicalPos(10,value);
        //point->setPosition(logicalPos);
    }
}

/*!
    calculates the position and the bounding box of the WorksheetInfoElement. Called on geometry changes.
    Or when the label or the point where moved
 */
void WorksheetInfoElementPrivate::retransform() {

    // TODO: find better solution
    if(!q->label)
        return;
    if(q->markerpoints.isEmpty())
        return;

    q->label->graphicsItem()->setFlag(QGraphicsItem::ItemSendsGeometryChanges, false);
    q->label->retransform();
    q->label->graphicsItem()->setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);



    for(auto markerpoint: q->markerpoints){
        markerpoint.customPoint->graphicsItem()->setFlag(QGraphicsItem::ItemSendsGeometryChanges, false);
        markerpoint.customPoint->retransform();
        markerpoint.customPoint->graphicsItem()->setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    }

    // line goes to the fist pointPos
    // TODO: better would be to direct to the highest point or also possible to make it changeable
    QPointF pointPos = cSystem->mapLogicalToScene(q->markerpoints[0].customPoint->position());
    QRectF labelSize = q->label->getSize();

    QPointF labelPos = cSystem->mapLogicalToScene(q->label->getLogicalPos());//QPointF(labelSize.x(),labelSize.y());

    double difference_x = pointPos.x() - labelPos.x();
    double difference_y = pointPos.y() - labelPos.y();

    double w = abs(difference_x);
    double h = abs(difference_y);

    double x,y;


    if(w > h){
        // attach to right or left border of the textlabel
        if(difference_x > 0){
            // point is more right than textlabel
            // attach to right border of textlabel
            w -= labelSize.width()/2;
            x = pointPos.x() - w/2;
            if(difference_y >0){
                // point is lower than TextLabel
                y = labelPos.y()+h/2;
                connectionLine = QLineF(-w/2,-h/2,w/2,h/2);
                sceneDeltaPoint = QPointF(w/2,h/2);
                sceneDeltaTextLabel = QPointF(-w/2-labelSize.width()/2,-h/2);
            }else{
                y = labelPos.y()-h/2;
                connectionLine = QLineF(-w/2,h/2,w/2,-h/2);
                sceneDeltaPoint = QPointF(w/2,-h/2);
                sceneDeltaTextLabel = QPointF(-w/2-labelSize.width()/2,h/2);
            }
        }else{
            // point is more left than textlabel
            // attach to left border
            w -= labelSize.width()/2;
            x = pointPos.x()+w/2;
            if(difference_y < 0){
                // point is higher than TextLabel
                y = pointPos.y()+h/2;
                connectionLine = QLineF(-w/2,-h/2,w/2,h/2);
                sceneDeltaPoint = QPointF(-w/2,-h/2);
                sceneDeltaTextLabel = QPointF(w/2+labelSize.width()/2,h/2);
            }else{
                // point is lower than TextLabel
                y = pointPos.y()-h/2;
                connectionLine = QLineF(-w/2,h/2,w/2,-h/2);
                sceneDeltaPoint = QPointF(-w/2,h/2);
                sceneDeltaTextLabel = QPointF(w/2+labelSize.width()/2,-h/2);
            }
        }
    }else{
        // attach to top or bottom border of the textlabel
        if(difference_y < 0){
            // attach to top border
            h -= labelSize.height()/2;
            y = pointPos.y()+h/2;
            if(difference_x > 0){
                x = labelPos.x()+w/2;
                connectionLine = QLineF(-w/2,h/2,w/2,-h/2);
                sceneDeltaPoint = QPointF(w/2,-h/2);
                sceneDeltaTextLabel = QPointF(-w/2,h/2+labelSize.height()/2);
            }else{
                x = pointPos.x()+w/2;
                connectionLine = QLineF(-w/2,-h/2,w/2,h/2);
                sceneDeltaPoint = QPointF(-w/2,-h/2);
                sceneDeltaTextLabel = QPointF(w/2,h/2+labelSize.height()/2);
            }
        }else{
            // attach to bottom border
            h -= labelSize.height()/2;
            y = pointPos.y()-h/2;
            if(difference_x > 0){
                x = labelPos.x()+ w/2;
                connectionLine = QLineF(-w/2,-h/2,w/2,h/2);
                sceneDeltaPoint = QPointF(w/2,h/2);
                sceneDeltaTextLabel = QPointF(-w/2,-h/2-labelSize.height()/2);
            }else{
                x = pointPos.x()+w/2;
                connectionLine = QLineF(-w/2,h/2,w/2,-h/2);
                sceneDeltaPoint = QPointF(-w/2,h/2);
                sceneDeltaTextLabel = QPointF(w/2,-h/2-labelSize.height()/2);
            }

        }
    }

    setFlag(QGraphicsItem::ItemSendsGeometryChanges, false);
    QPointF itemPos(x,y);
    setPos(itemPos);

    boundingRectangle.setX(-w/2);
    boundingRectangle.setY(-h/2);
    boundingRectangle.setWidth(w);
    boundingRectangle.setHeight(h);

    update(boundingRect());
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
}

bool WorksheetInfoElementPrivate::isVisible() const {
    return m_visible;
}

void WorksheetInfoElementPrivate::updatePosition(){

}

//reimplemented from QGraphicsItem
QRectF WorksheetInfoElementPrivate::boundingRect() const{

    return boundingRectangle;
}
void WorksheetInfoElementPrivate::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget* widget){
    if(!m_visible)
        return;

    //painter->fillRect(boundingRectangle,QBrush(QColor(255,0,0,128)));
    QPen pen(Qt::black, 5);
    painter->setPen(pen);
    painter->drawLine(connectionLine);

    // draw vertical line if more than one custompoint
//    if(q->markerpoints.length()>1){
//        double xPos = q->markerpoints[0].customPoint->position().x();
//        double yMax = plot->yMax();
//        double yMin = plot->yMin();

//        QLineF line(cSystem->mapLogicalToScene(QPointF(xPos,yMin)),cSystem->mapLogicalToScene(QPointF(xPos,yMax)));
//        painter->drawLine(line);
//    }
}
QVariant WorksheetInfoElementPrivate::itemChange(GraphicsItemChange change, const QVariant &value){
    if(change == QGraphicsItem::ItemPositionChange){

        for(auto markerpoint: q->markerpoints){
            markerpoint.customPoint->graphicsItem()->setFlag(QGraphicsItem::ItemSendsGeometryChanges, false);
        }
        q->label->graphicsItem()->setFlag(QGraphicsItem::ItemSendsGeometryChanges, false);
        for(auto markerpoint: q->markerpoints){
            markerpoint.customPoint->setPosition(cSystem->mapSceneToLogical(value.toPointF()+sceneDeltaPoint));
        }
        q->label->setPosition(value.toPointF()+sceneDeltaTextLabel);
        for(auto markerpoint: q->markerpoints){
            markerpoint.customPoint->graphicsItem()->setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
        }
        q->label->graphicsItem()->setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    }
    return QGraphicsItem::itemChange(change, value);
}
void WorksheetInfoElementPrivate::keyPressEvent(QKeyEvent * event){
    if (event->key() == Qt::Key_Right || event->key() == Qt::Key_Left) {
        int index;
        if(event->key() == Qt::Key_Right){
            index = 1;
        }else{
            index = -1;
        }

        double x,y;
        bool valueFound;
        QPointF pointPosition;
        for(auto markerpoint: q->markerpoints){
            markerpoint.curve->getNextValue(markerpoint.customPoint->position().x(), index,x,y,valueFound);
            if(valueFound){
                pointPosition.setX(x);
                pointPosition.setY(y);
                markerpoint.customPoint->setPosition(pointPosition);
            }
        }
        //QPointF labelPosition = label->getLogicalPos();
        //labelPosition.setX(labelPosition.x()+ (x-markerpoints[0].first->position().x()));
        //label->setPosition(cSystem->mapLogicalToScene(labelPosition));
        TextLabel::TextWrapper text;
        text.text = "Value: " + QString::number(y);
        q->label->setText(text);

    }
}
//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################

void WorksheetInfoElement::save(QXmlStreamWriter* writer) const{
    Q_D(const WorksheetInfoElement);

    writer->writeStartElement( "worksheetInfoElement" );
    writeBasicAttributes(writer);
    writeCommentElement(writer);

    label->save(writer);
    QString path;
    for(auto custompoint: markerpoints){
        custompoint.customPoint->save(writer);
        path = custompoint.curve->path();
        writer->writeStartElement("markerPointCurve");
        writer->writeAttribute(QLatin1String("curvepath") , path);
        writer->writeEndElement(); // close "markerPointCurve
    }
    writer->writeEndElement(); // close "worksheetInfoElement"
}

bool WorksheetInfoElement::load(XmlStreamReader* reader, bool preview){
    if (!readBasicAttributes(reader))
        return false;

    Q_D(WorksheetInfoElement);

    while(!reader->atEnd()){
        reader->readNext();
        if (reader->isEndElement() && reader->name() == "worksheetInfoElement")
            break;

        if (!reader->isStartElement() || reader->name() != "worksheetInfoElement");
            continue;

        if(!label->load(reader, preview)){
            return false;
        }
        while (!(reader->isEndElement() && reader->name() == "worksheetInfoElement")) {
            reader->readNext();
            CustomPoint* markerpoint = new CustomPoint(d->plot, "Marker");
            markerpoint->load(reader,preview);
            QXmlStreamAttributes attribs;
            QString path;
            if(reader->isStartElement() && reader->name() == "markerPointCurve"){
                attribs = reader->attributes();
                path = attribs.value("curvepath").toString();
                addCurvePath(path, markerpoint);
            }else{
                delete markerpoint;
            }
        }

    }
    return true;
}
