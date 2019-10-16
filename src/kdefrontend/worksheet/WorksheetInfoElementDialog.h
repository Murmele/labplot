#ifndef WORKSHEETINFOELEMENTDIALOG_H
#define WORKSHEETINFOELEMENTDIALOG_H

#include <QDialog>

class CartesianPlot;
class XYCurve;
class QListWidgetItem;

namespace Ui {
class WorksheetInfoElementDialog;
}

class WorksheetInfoElementDialog : public QDialog
{
	Q_OBJECT

public:
	explicit WorksheetInfoElementDialog(QWidget *parent = nullptr);
	~WorksheetInfoElementDialog();
	void setPlot(CartesianPlot* plot);
	void setActiveCurve(const XYCurve* curve, double pos);
	void updateSettings();
private:
	void createElement();
	void updateSelectedCurveLabel(QListWidgetItem *item);
	void removePlot();
	void validateSettings();

private:
	Ui::WorksheetInfoElementDialog *ui;
	CartesianPlot* m_plot{nullptr};


};

#endif // WORKSHEETINFOELEMENTDIALOG_H
