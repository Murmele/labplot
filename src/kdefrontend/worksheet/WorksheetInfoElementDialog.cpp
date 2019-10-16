#include "WorksheetInfoElementDialog.h"
#include "ui_worksheetinfoelementdialog.h"

#include <QDialogButtonBox>
#include <QPushButton>

#include "backend/worksheet/plots/cartesian/CartesianCoordinateSystem.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
#include "backend/worksheet/plots/cartesian/XYCurve.h"

WorksheetInfoElementDialog::WorksheetInfoElementDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::WorksheetInfoElementDialog) {
	ui->setupUi(this);

	connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &WorksheetInfoElementDialog::createElement);
	connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &WorksheetInfoElementDialog::removePlot); // reset m_plot
	connect(ui->lst_useCurve, &QListWidget::currentItemChanged, this, &WorksheetInfoElementDialog::updateSelectedCurveLabel);
}

WorksheetInfoElementDialog::~WorksheetInfoElementDialog() {
	delete ui;
}

void WorksheetInfoElementDialog::setPlot(CartesianPlot* plot) {
	if (m_plot) // when in a new plot a marker should be created without creating marker in old plot
		removePlot();

	m_plot = plot;
	connect(m_plot, &CartesianPlot::aspectAboutToBeRemoved, this, &WorksheetInfoElementDialog::removePlot);
	connect(m_plot, &CartesianPlot::curveRemoved, this, &WorksheetInfoElementDialog::updateSettings);
	connect(m_plot, &CartesianPlot::curveAdded, this, &WorksheetInfoElementDialog::updateSettings);
	connect(m_plot, &CartesianPlot::xMinChanged, this, &WorksheetInfoElementDialog::updateSettings);
	connect(m_plot, &CartesianPlot::xMaxChanged, this, &WorksheetInfoElementDialog::updateSettings);

	updateSettings();


	setWindowTitle(i18n("WorksheetInfoElement creation in plot: ") + m_plot->name());
}

void WorksheetInfoElementDialog::setActiveCurve(const XYCurve* curve, double pos) {
	auto items = ui->lst_useCurve->findItems(curve->name(), Qt::MatchExactly);

	for (auto item: items) {
		if (item->text() == curve->name())
			ui->lst_useCurve->setCurrentItem(item);
	}

	ui->sb_Pos->setValue(pos);
}

/*!
 * \brief WorksheetInfoElementDialog::updateSettings
 * Called each time a setting in the cartesianplot is changed. Not each setting is handled differently.
 * This would be to complex for this simple dialog
 */
void WorksheetInfoElementDialog::updateSettings() {
	ui->lst_useCurve->clear();

	auto curves = m_plot->children<const XYCurve>();
	for (auto curve: curves)
		ui->lst_useCurve->addItem(curve->name());

	ui->lst_useCurve->setCurrentRow(0);
	if (ui->lst_useCurve->count())
		ui->l_selectedCurve->setText(ui->lst_useCurve->currentItem()->text());
	else
		ui->l_selectedCurve->setText("");

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

	validateSettings();
}

void WorksheetInfoElementDialog::createElement() {
	if (m_plot) {
		double pos = ui->sb_Pos->value();

		QString curveName = ui->lst_useCurve->currentItem()->text();
		for (auto curve: m_plot->children<const XYCurve>()) {
			if (curveName == curve->name())
				m_plot->addWorksheetInfoElement(curve, pos);
		}
		m_plot = nullptr;
	}
}

void WorksheetInfoElementDialog::updateSelectedCurveLabel(QListWidgetItem *item) {
	// when listwidget is cleared, there is anymore an item, but the signal is raised anyway
	if (!item)
		return;
	ui->l_selectedCurve->setText(item->text());
}

/*!
 * \brief WorksheetInfoElementDialog::removePlot
 * Called when dialog is closed or when the plot was deleted before creating marker
 */
void WorksheetInfoElementDialog::removePlot() {

	ui->lst_useCurve->clear();
	ui->l_selectedCurve->setText("");

	// remove all connections
	disconnect(m_plot, nullptr, this, nullptr);
	m_plot = nullptr;

	validateSettings();
}

void WorksheetInfoElementDialog::validateSettings() {

	if (ui->lst_useCurve->count() > 0 && m_plot) {
		ui->buttonBox->button(QDialogButtonBox::StandardButton::Ok)->setEnabled(true);
	}

	ui->buttonBox->button(QDialogButtonBox::StandardButton::Ok)->setEnabled(false);
}
