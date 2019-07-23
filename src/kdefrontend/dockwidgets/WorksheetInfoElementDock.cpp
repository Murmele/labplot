#include "WorksheetInfoElementDock.h"

#include "backend/worksheet/WorksheetInfoElement.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
#include "backend/worksheet/plots/cartesian/XYCurve.h"
#include "ui_worksheetinfoelementdock.h"

WorksheetInfoElementDock::WorksheetInfoElementDock(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WorksheetInfoElementDock),
    m_initializing(false)
{
    ui->setupUi(this);

    connect(ui->chbVisible, &QCheckBox::toggled, this, &WorksheetInfoElementDock::visibilityChanged);
    connect(ui->btnAddCurve, &QPushButton::clicked, this, &WorksheetInfoElementDock::addCurve);
    connect(ui->btnRemoveCurve, &QPushButton::clicked, this, &WorksheetInfoElementDock::removeCurve);
    connect(ui->sbXPosLineWidth, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &WorksheetInfoElementDock::xposLineWidthChanged);
    connect(ui->sbConnectionLineWidth, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &WorksheetInfoElementDock::connectionLineWidthChanged);
    // From Qt 5.7 qOverload can be used:
    //connect(ui->sbXPosLineWidth, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &WorksheetInfoElementDock::xposLineWidthChanged);
    //connect(ui->sbConnectionLineWidth, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &WorksheetInfoElementDock::connectionLineWidthChanged);
    connect(ui->kcbXPosLineColor, &KColorButton::changed, this, &WorksheetInfoElementDock::xposLineColorChanged);
    connect(ui->kcbConnectionLineColor, &KColorButton::changed, this, &WorksheetInfoElementDock::connectionLineColorChanged);
    connect(ui->chbXPosLineVisible, &QCheckBox::toggled, this, &WorksheetInfoElementDock::xposLineVisibilityChanged);

	connect(ui->cb_gluePoint, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &WorksheetInfoElementDock::gluePointChanged);
	connect(ui->cb_curve, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &WorksheetInfoElementDock::curveChanged);
}

void WorksheetInfoElementDock::setWorksheetInfoElements(QList<WorksheetInfoElement *> &list, bool sameParent) {
    m_elements = list;
    m_element = list.first();
    m_sameParent = sameParent;

    ui->lstAvailableCurves->clear();
    ui->lstSelectedCurves->clear();
	ui->cb_curve->clear();

    ui->chbVisible->setChecked(m_element->isVisible());

    if (sameParent) {
        QVector<XYCurve*> curves = m_element->getPlot()->children<XYCurve>();
        for (int i=0; i< curves.length(); i++)
				ui->lstAvailableCurves->addItem(curves[i]->name());

        for (int i=0; i<m_element->markerPointsCount(); i++)
			if (m_element->markerPointAt(i).curve != nullptr) {
				ui->lstSelectedCurves->addItem(m_element->markerPointAt(i).curve->name());
				ui->cb_curve->addItem(m_element->markerPointAt(i).curve->name());
			}

    } else {
        ui->lstAvailableCurves->setEnabled(false);
        ui->lstSelectedCurves->setEnabled(false);
    }
	QString curveName = m_element->connectionLineCurveName();
	for (int i=0; i< ui->cb_curve->count(); i++) {
		if (ui->cb_curve->itemData(i, Qt::DisplayRole).toString().compare(curveName) == 0) {
			ui->cb_curve->setCurrentIndex(i);
			break;
		}
	}

	if (m_element->isTextLabel()) {
		elementLabelBorderShapeChanged(m_element->gluePointsCount());
		ui->cb_gluePoint->setCurrentIndex(m_element->gluePointIndex()+1);
	}

    ui->chbXPosLineVisible->setChecked(m_element->xposLineVisible());
    ui->kcbXPosLineColor->setColor(m_element->xposLineColor());
    ui->kcbConnectionLineColor->setColor(m_element->connectionLineColor());
    ui->sbXPosLineWidth->setValue(m_element->xposLineWidth());
    ui->sbConnectionLineWidth->setValue(m_element->connectionLineWidth());

    initConnections();
}

void WorksheetInfoElementDock::initConnections() {
    connect( m_element, &WorksheetInfoElement::visibleChanged,
             this, &WorksheetInfoElementDock::elementVisibilityChanged );
    connect( m_element, &WorksheetInfoElement::connectionLineWidthChanged,
             this, &WorksheetInfoElementDock::elementConnectionLineWidthChanged );
    connect( m_element, &WorksheetInfoElement::connectionLineColorChanged,
             this, &WorksheetInfoElementDock::elementConnectionLineColorChanged );
    connect( m_element, &WorksheetInfoElement::xposLineWidthChanged,
             this, &WorksheetInfoElementDock::elementXPosLineWidthChanged );
    connect( m_element, &WorksheetInfoElement::xposLineColorChanged,
             this, &WorksheetInfoElementDock::elementXposLineColorChanged );
    connect( m_element, &WorksheetInfoElement::xposLineVisibleChanged,
             this, &WorksheetInfoElementDock::elementXPosLineVisibleChanged );
	connect (m_element, &WorksheetInfoElement::gluePointIndexChanged,
			 this, &WorksheetInfoElementDock::elementGluePointIndexChanged );
	connect (m_element, &WorksheetInfoElement::connectionLineCurveNameChanged,
			 this, &WorksheetInfoElementDock::elementConnectionLineCurveChanged );
	connect (m_element, &WorksheetInfoElement::labelBorderShapeChangedSignal,
			 this, &WorksheetInfoElementDock::elementLabelBorderShapeChanged );
	connect (m_element, &WorksheetInfoElement::curveRemoved,
			 this, &WorksheetInfoElementDock::elementCurveRemoved);

}

WorksheetInfoElementDock::~WorksheetInfoElementDock() {
    delete ui;
}

void WorksheetInfoElementDock::visibilityChanged(bool state) {
    if (m_initializing)
        return;

    for (auto* worksheetInfoElement : m_elements)
        worksheetInfoElement->setVisible(state);
}

void WorksheetInfoElementDock::connectionLineWidthChanged(double width) {
    if (m_initializing)
        return;

    for (auto* worksheetInfoElement: m_elements)
        worksheetInfoElement->setConnectionLineWidth(width);
}

void WorksheetInfoElementDock::connectionLineColorChanged(QColor color) {
    if (m_initializing)
        return;

    for (auto* worksheetInfoElement: m_elements)
        worksheetInfoElement->setConnectionLineColor(color);
}

void WorksheetInfoElementDock::xposLineWidthChanged(double value) {
    if (m_initializing)
        return;

    for (auto* worksheetInfoElement: m_elements)
        worksheetInfoElement->setXPosLineWidth(value);
}

void WorksheetInfoElementDock::xposLineColorChanged(QColor color) {
    if (m_initializing)
        return;

    for (auto* worksheetInfoElement: m_elements)
        worksheetInfoElement->setXPosLineColor(color);
}

void WorksheetInfoElementDock::xposLineVisibilityChanged(bool visible) {
    if (m_initializing)
        return;

    for (auto* worksheetInfoElement: m_elements)
        worksheetInfoElement->setXPosLineVisible(visible);
}

void WorksheetInfoElementDock::gluePointChanged(int index) {
	if (m_initializing)
		return;

	for (auto* worksheetInfoElement: m_elements)
		worksheetInfoElement->setGluePointIndex(index-1); // index 0 means automatic, which is defined as -1
}

void WorksheetInfoElementDock::curveChanged(int index) {
	if (m_initializing)
		return;

	QString name = ui->cb_curve->currentText();
	for (auto* worksheetInfoElement: m_elements)
		worksheetInfoElement->setConnectionLineCurveName(name); // index 0 means automatic, which is defined as -1
}

void WorksheetInfoElementDock::addCurve() {

    if (!m_sameParent)
        return;

    QList<QListWidgetItem*> list = ui->lstAvailableCurves->selectedItems();

    bool curveAlreadyExist;
    for (QListWidgetItem* selectedItem: list) {
        QString curveName = selectedItem->data(Qt::DisplayRole).toString();

        curveAlreadyExist = false;
        for (int i=0; i<ui->lstSelectedCurves->count(); i++) {
            QListWidgetItem* item = ui->lstSelectedCurves->item(i);

            if (item->data(Qt::DisplayRole) == selectedItem->data(Qt::DisplayRole)) {
                curveAlreadyExist = true;
                break;
            }
        }
        if (curveAlreadyExist)
            continue;

        XYCurve* curve;
        for (int i=0; i < m_elements[0]->getPlot()->children<XYCurve>().count(); i++) {
            if (m_elements[0]->getPlot()->children<XYCurve>()[i]->name() == curveName) {
                curve = m_elements[0]->getPlot()->children<XYCurve>()[i];
            }
        }

        ui->lstSelectedCurves->addItem(selectedItem->data(Qt::DisplayRole).toString());
        for (int i=0; i< list.count(); i++) {
            for (int j=0; j < m_elements.count(); j++) {
                for (int k=0; k < m_elements[j]->markerPointsCount(); k++) {
                        m_elements[j]->addCurve(curve);
                }
            }
        }
    }

}

void WorksheetInfoElementDock::removeCurve() {
    if (!m_sameParent)
        return;

    QList<QListWidgetItem*> list = ui->lstSelectedCurves->selectedItems();

    for (auto item: list)
        ui->lstSelectedCurves->takeItem(ui->lstSelectedCurves->row(item));

    for (int i=0; i<list.count(); i++){
        for (int j=0; j< m_elements.count(); j++) {
            for (int k=0; k < m_elements[j]->markerPointsCount(); k++) {
                if (m_elements[j]->markerPointAt(k).curve->name() == list.at(i)->data(Qt::DisplayRole)) {
                    m_elements[j]->removeCurve(m_elements[j]->markerPointAt(k).curve);
                    continue;
                }
            }
        }
    }

}

//***********************************************************
//****** SLOTs for changes triggered in WorksheetInfoElement
//***********************************************************

void WorksheetInfoElementDock::elementConnectionLineWidthChanged(const double width) {
    m_initializing = true;
    ui->sbConnectionLineWidth->setValue(width);
    m_initializing = false;
}
void WorksheetInfoElementDock::elementConnectionLineColorChanged(const QColor color) {
    m_initializing = true;
    ui->kcbConnectionLineColor->setColor(color);
    m_initializing = false;
}
void WorksheetInfoElementDock::elementXPosLineWidthChanged(const double width) {
    m_initializing = true;
    ui->sbXPosLineWidth->setValue(width);
    m_initializing = false;
}
void WorksheetInfoElementDock::elementXposLineColorChanged(const QColor color) {
    m_initializing = true;
    ui->kcbXPosLineColor->setColor(color);
    m_initializing = false;
}
void WorksheetInfoElementDock::elementXPosLineVisibleChanged(const bool visible) {
    m_initializing = true;
    ui->chbXPosLineVisible->setChecked(visible);
    m_initializing = false;
}

void WorksheetInfoElementDock::elementVisibilityChanged(const bool visible) {
    m_initializing = true;
    ui->chbVisible->setChecked(visible);
    m_initializing = false;
}

void WorksheetInfoElementDock::elementGluePointIndexChanged(const int index) {
	m_initializing = true;
	if (index < 0)
		ui->cb_gluePoint->setCurrentIndex(0);
	else
		ui->cb_gluePoint->setCurrentIndex(index + 1); // automatic label is in same combo box
	m_initializing = false;
}

void WorksheetInfoElementDock::elementConnectionLineCurveChanged(const QString name) {
	m_initializing = true;
	for (int i=0; i< ui->cb_curve->count(); i++) {
		if (ui->cb_curve->itemData(i).toString().compare(name) == 0) {
			ui->cb_curve->setCurrentIndex(i);
			break;
		}
	}
	m_initializing = false;
}

void WorksheetInfoElementDock::elementLabelBorderShapeChanged(const int gluePointCount) {
	m_initializing = true;
	ui->cb_gluePoint->clear();
	ui->cb_gluePoint->addItem("Automatic");
	for (int i=0; i < m_element->gluePointsCount(); i++) {
		ui->cb_gluePoint->addItem(QString::number(i+1));
	}
	m_initializing = false;
}

void WorksheetInfoElementDock::elementCurveRemoved(QString name) {
	m_initializing = true;
	for (int i=0; i < ui->lstSelectedCurves->count(); i++) {
		QListWidgetItem *item = ui->lstSelectedCurves->item(i);
		if (item->data(Qt::DisplayRole).toString().compare(name) == 0) {
			ui->lstSelectedCurves->takeItem(i);
			break;
		}
	}
	m_initializing = false;
}
