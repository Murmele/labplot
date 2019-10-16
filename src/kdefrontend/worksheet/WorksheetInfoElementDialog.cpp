#include "WorksheetInfoElementDialog.h"
#include "ui_worksheetinfoelementdialog.h"

#include <QDialogButtonBox>

#include "backend/worksheet/plots/cartesian/CartesianCoordinateSystem.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
#include "backend/worksheet/plots/cartesian/XYCurve.h"

WorksheetInfoElementDialog::WorksheetInfoElementDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::WorksheetInfoElementDialog) {
	ui->setupUi(this);

	connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &WorksheetInfoElementDialog::createElement);
	connect(ui->buttonBox, &QDialogButtonBox::rejected, this, [=]() {m_plot = nullptr;}); // reset m_plot
	connect(ui->lst_useCurve, &QListWidget::currentItemChanged, this, &WorksheetInfoElementDialog::updateSelectedCurveLabel);
}

WorksheetInfoElementDialog::~WorksheetInfoElementDialog() {
	delete ui;
}

void WorksheetInfoElementDialog::setPlot(CartesianPlot* plot) {
	m_plot = plot;

	ui->lst_useCurve->clear();

	auto curves = m_plot->children<const XYCurve>();
	for (auto curve: curves)
		ui->lst_useCurve->addItem(curve->name());

	ui->lst_useCurve->setCurrentRow(0);
	ui->l_selectedCurve->setText(ui->lst_useCurve->currentItem()->text());

	// TODO: set position in the mid of the screen as default
	CartesianCoordinateSystem* cSystem = dynamic_cast<CartesianCoordinateSystem*>(m_plot->coordinateSystem());
	if (cSystem) {
		double xMinScene = cSystem->mapLogicalToScene(QPointF(m_plot->xMin(), m_plot->yMin())).x();
		double xMaxScene = cSystem->mapLogicalToScene(QPointF(m_plot->xMax(), m_plot->yMin())).x();

		double midLogical = cSystem->mapSceneToLogical(QPointF(xMinScene + (xMaxScene - xMinScene)/2, 0)).x();

		ui->sb_Pos->setMinimum(m_plot->xMin());
		ui->sb_Pos->setMaximum(m_plot->xMax());
		ui->sb_Pos->setValue(midLogical);
	} else
		ui->sb_Pos->setValue(0);

}

void WorksheetInfoElementDialog::setActiveCurve(const XYCurve* curve, double pos) {
	auto items = ui->lst_useCurve->findItems(curve->name(), Qt::MatchExactly);

	for (auto item: items) {
		if (item->text() == curve->name())
			ui->lst_useCurve->setCurrentItem(item);
	}

	ui->sb_Pos->setValue(pos);
}

void WorksheetInfoElementDialog::createElement() {
	// TODO: m_plot might anymore valid. How to check?
	// signal that plot was deleted?
	if (m_plot) {
		double pos = ui->sb_Pos->value();

		QString curveName = ui->lst_useCurve->currentItem()->text();
		for (auto curve: m_plot->children<const XYCurve>()) {
			if (curveName == curve->name())
				m_plot->addWorksheetInfoElement(curve, pos);
		}
	}

	m_plot = nullptr;
}

void WorksheetInfoElementDialog::updateSelectedCurveLabel(QListWidgetItem *item) {
	// when listwidget is cleared, there is anymore an item, but the signal is raised anyway
	if (!item)
		return;
	ui->l_selectedCurve->setText(item->text());
}
