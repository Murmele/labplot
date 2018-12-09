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
    connect(d->label, &TextLabel::positionChanged, this, &WorksheetInfoElement::labelPositionChanged);
    connect(d->point, &CustomPoint::positionChanged, this, &WorksheetInfoElement::pointPositionChanged);

    d->init();
}

void WorksheetInfoElement::init(){

}

void WorksheetInfoElement::labelPositionChanged(TextLabel::PositionWrapper position){
    Q_UNUSED(position)
    Q_D(WorksheetInfoElement);

    d->retransform();
}

void WorksheetInfoElement::pointPositionChanged(QPointF pos){
    Q_UNUSED(pos)
    Q_D(WorksheetInfoElement);

    // TODO: Find better solution, this is not a good solution!
    if(d->point->graphicsItem()->flags().operator&=(QGraphicsItem::ItemSendsGeometryChanges)){
        d->retransform();
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
void WorksheetInfoElement::setVisible(bool on){
    Q_D(WorksheetInfoElement);
    d->setVisible(on);
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

WorksheetInfoElementPrivate::WorksheetInfoElementPrivate(WorksheetInfoElement* owner,CartesianPlot *plot, const XYCurve* curve):
    q(owner),
    plot(plot),
    cSystem( dynamic_cast<const CartesianCoordinateSystem*>(plot->coordinateSystem()))
{
    curves.append(curve);
    label = new TextLabel("Test");
    owner->addChild(label);
    label->enableCoordBinding(true,plot);
    label->setCoordBinding(true);
    label->setParentGraphicsItem(plot->plotArea()->graphicsItem());
    // TODO: connect(d->label, &TextLabel::rotationChanged, d, &WorksheetInfoElementPrivate::retransform);
    point = new CustomPoint(plot, "TestPoint");
    point->setParentGraphicsItem(plot->plotArea()->graphicsItem());
    owner->addChild(point);
    setVisible(false);
}

void WorksheetInfoElementPrivate::init(){

    setVisible(true);

    bool valueFound;
    double value = curves.first()->y(10,valueFound);
    if(valueFound){
        QPointF logicalPos(10,value);
        point->setPosition(logicalPos);
    }
}

/*!
    calculates the position and the bounding box of the WorksheetInfoElement. Called on geometry changes.
    Or when the label or the point where moved
 */
void WorksheetInfoElementPrivate::retransform() {

    point->graphicsItem()->setFlag(QGraphicsItem::ItemSendsGeometryChanges, false);
    label->graphicsItem()->setFlag(QGraphicsItem::ItemSendsGeometryChanges, false);
    label->retransform();
    point->retransform();
    point->graphicsItem()->setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    label->graphicsItem()->setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);

    QPointF pointPos = cSystem->mapLogicalToScene(point->position());
    QRectF labelSize = label->getSize();

    QPointF labelPos = cSystem->mapLogicalToScene(label->getLogicalPos());//QPointF(labelSize.x(),labelSize.y());

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

void WorksheetInfoElementPrivate::setVisible(bool on){
    label->setVisible(on);
    point->setVisible(on);
    m_visible = on;
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



}
QVariant WorksheetInfoElementPrivate::itemChange(GraphicsItemChange change, const QVariant &value){
    if(change == QGraphicsItem::ItemPositionChange){

        point->graphicsItem()->setFlag(QGraphicsItem::ItemSendsGeometryChanges, false);
        label->graphicsItem()->setFlag(QGraphicsItem::ItemSendsGeometryChanges, false);
        point->setPosition(cSystem->mapSceneToLogical(value.toPointF()+sceneDeltaPoint));
        label->setPosition(value.toPointF()+sceneDeltaTextLabel);
        point->graphicsItem()->setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
        label->graphicsItem()->setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
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
        curves.first()->getNextValue(point->position().x(),index,x,y,valueFound);
        QPointF pointPosition(x,y);
        QPointF labelPosition = label->getLogicalPos();
        labelPosition.setX(labelPosition.x()+ (x-point->position().x()));
        if(valueFound){
            point->setPosition(pointPosition);
            //label->setPosition(cSystem->mapLogicalToScene(labelPosition));
            TextLabel::TextWrapper text;
            text.text = "Value: " + QString::number(y);
            label->setText(text);
        }

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

    d->label->save(writer);
    d->point->save(writer);

    writer->writeEndElement(); // close "worksheetInfoElement"
}

bool WorksheetInfoElement::load(XmlStreamReader* reader, bool preview){
    if (!readBasicAttributes(reader))
        return false;

    Q_D(WorksheetInfoElement);

    while(!reader->atEnd()){
        if (reader->isEndElement() && reader->name() == "worksheetInfoElement")
            break;

        if (!reader->isStartElement())
            continue;

        if(!d->label->load(reader, preview)){
            return false;
        }
        if(!d->point->load(reader, preview)){
            return false;
        }

    }
    return true;


}
