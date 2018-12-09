#include "WorksheetInfoElementDock.h"

#include "backend/worksheet/WorksheetInfoElement.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
#include "backend/worksheet/plots/cartesian/XYCurve.h"
#include "ui_worksheetinfoelementdock.h"

WorksheetInfoElementDock::WorksheetInfoElementDock(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WorksheetInfoElementDock)
{
    ui->setupUi(this);
}

void WorksheetInfoElementDock::setWorksheetInfoElements(QList<WorksheetInfoElement *> &list){
    m_elements = list;

    QVector<XYCurve*> curves = m_elements[0]->getPlot()->children<XYCurve>();
    ui->combo_curves->clear();

    for(int i=0; i< curves.length(); i++){
        ui->combo_curves->addItem(curves[i]->name(),i);
    }


}

WorksheetInfoElementDock::~WorksheetInfoElementDock()
{
    delete ui;
}

void WorksheetInfoElementDock::on_pb_add_clicked()
{
    m_elements[0]->addCurve(m_elements[0]->getPlot()->children<XYCurve>()[ui->combo_curves->currentIndex()]);
}
