/***************************************************************************
    File                 : ColumnDock.cpp
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2011 Alexander Semke
    Email (use @ for *)  : alexander.semke*web.de
    Description          : widget for column properties
                           
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

#include "ColumnDock.h"

#include "core/AbstractFilter.h"
#include "core/datatypes/SimpleCopyThroughFilter.h"
#include "core/datatypes/Double2StringFilter.h"
#include "core/datatypes/String2DoubleFilter.h"
#include "core/datatypes/DateTime2StringFilter.h"
#include "core/datatypes/String2DateTimeFilter.h"

#include <QDebug>

/*!
  \class SpreadsheetDock
  \brief Provides a widget for editing the properties of the spreadsheet columns currently selected in the project explorer.

  \ingroup kdefrontend
*/

ColumnDock::ColumnDock(QWidget *parent): QWidget(parent){
	ui.setupUi(this);
	
	dateStrings<<"yyyy-MM-dd";
	dateStrings<<"yyyy/MM/dd";
	dateStrings<<"dd/MM/yyyy"; 
	dateStrings<<"dd/MM/yy";
	dateStrings<<"dd.MM.yyyy";
	dateStrings<<"dd.MM.yy";
	dateStrings<<"MM/yyyy";
	dateStrings<<"dd.MM."; 
	dateStrings<<"yyyyMMdd";

	timeStrings<<"hh";
	timeStrings<<"hh ap";
	timeStrings<<"hh:mm";
	timeStrings<<"hh:mm ap";
	timeStrings<<"hh:mm:ss";
	timeStrings<<"hh:mm:ss.zzz";
	timeStrings<<"hh:mm:ss:zzz";
	timeStrings<<"mm:ss.zzz";
	timeStrings<<"hhmmss";
	
	connect(ui.cbType, SIGNAL(currentIndexChanged(int)), this, SLOT(typeChanged(int)));
	connect(ui.cbFormat, SIGNAL(currentIndexChanged(int)), this, SLOT(formatChanged(int)));
	connect( ui.sbPrecision, SIGNAL(valueChanged(int)), this, SLOT(precisionChanged(int)) );
	connect(ui.cbPlotDesignation, SIGNAL(currentIndexChanged(int)), this, SLOT(plotDesignationChanged(int)));
	
	retranslateUi();
}

void ColumnDock::setColumns(QList<Column*> list){
  m_initializing=true;
  m_columnsList = list;
  
  Column* column=list.first();
  
  if (list.size()==1){
	ui.lName->setEnabled(true);
	ui.leName->setEnabled(true);
	ui.lComment->setEnabled(true);
	ui.teComment->setEnabled(true);
	
	ui.leName->setText(column->name());
	ui.teComment->setText(column->comment());
  }else{
	ui.lName->setEnabled(false);
	ui.leName->setEnabled(false);
	ui.lComment->setEnabled(false);
	ui.teComment->setEnabled(false);
	
	ui.leName->setText("");
	ui.teComment->setText("");
  }
  
  //show the properties of the first column
  SciDAVis::ColumnMode columnMode = column->columnMode();
  qDebug()<<"set columns, mode "<< columnMode;
  ui.cbType->setCurrentIndex(ui.cbType->findData((int)columnMode));
  this->updateFormatWidgets(columnMode);

  switch(columnMode) {
	  case SciDAVis::Numeric:{
			  Double2StringFilter * filter = static_cast<Double2StringFilter*>(column->outputFilter());
			  ui.cbFormat->setCurrentIndex(ui.cbFormat->findData(filter->numericFormat()));
			  qDebug()<<"set columns, numeric format"<<filter->numericFormat();
			  ui.sbPrecision->setValue(filter->numDigits());
			  break;
		  }
	  case SciDAVis::Month:
	  case SciDAVis::Day:
	  case SciDAVis::DateTime: {
			  DateTime2StringFilter * filter = static_cast<DateTime2StringFilter*>(column->outputFilter());
			  ui.cbFormat->setCurrentIndex(ui.cbFormat->findData(filter->format()));
			  break;
		  }
	  default:
		  break;
  }
  
  ui.cbPlotDesignation->setCurrentIndex( int(column->plotDesignation()) );
  
  m_initializing=false;
}

/*!
  depending on the currently selected column type (column mode) updates the widgets for the column format, 
  shows/hides the allowed widgets, fills the corresponding combobox with the possible entries.
  Called when the type (column mode) is changed.
*/
void ColumnDock::updateFormatWidgets(const SciDAVis::ColumnMode columnMode){
  ui.cbFormat->clear();

  switch (columnMode){
	case SciDAVis::Numeric:
	  ui.cbFormat->addItem(tr("Decimal"), QVariant('f'));
	  ui.cbFormat->addItem(tr("Scientific (e)"), QVariant('e'));
	  ui.cbFormat->addItem(tr("Scientific (E)"), QVariant('E'));
	  ui.cbFormat->addItem(tr("Automatic (e)"), QVariant('g'));
	  ui.cbFormat->addItem(tr("Automatic (E)"), QVariant('G'));
	  break;
	case SciDAVis::Text:
	  break;
	case SciDAVis::Month:
	  ui.cbFormat->addItem(tr("Number without leading zero"), QVariant("M"));
	  ui.cbFormat->addItem(tr("Number with leading zero"), QVariant("MM"));
	  ui.cbFormat->addItem(tr("Abbreviated month name"), QVariant("MMM"));
	  ui.cbFormat->addItem(tr("Full month name"), QVariant("MMMM"));
	  break;
	case SciDAVis::Day:
	  ui.cbFormat->addItem(tr("Number without leading zero"), QVariant("d"));
	  ui.cbFormat->addItem(tr("Number with leading zero"), QVariant("dd"));
	  ui.cbFormat->addItem(tr("Abbreviated day name"), QVariant("ddd"));
	  ui.cbFormat->addItem(tr("Full day name"), QVariant("dddd"));
	  break;
	case SciDAVis::DateTime:{
	  foreach(QString s, dateStrings)
		ui.cbFormat->addItem(s, QVariant(s));
	  
	  foreach(QString s, timeStrings)
		ui.cbFormat->addItem(s, QVariant(s));
	  
	  foreach(QString s1, dateStrings){
		foreach(QString s2, timeStrings)
		  ui.cbFormat->addItem(s1 + " " + s2, QVariant(s1 + " " + s2));
	  }
	  
	  break;
	}
	default:
		break;
  }
  
  if (columnMode == SciDAVis::Numeric){
	ui.lPrecision->show();
	ui.sbPrecision->show();
  }else{
	ui.lPrecision->hide();
	ui.sbPrecision->hide();
  }
  
  if (columnMode == SciDAVis::Text){
	ui.lFormat->hide();
	ui.cbFormat->hide();
  }else{
	ui.lFormat->show();
	ui.cbFormat->show();
	ui.cbFormat->setCurrentIndex(0);
  }
  
  if (columnMode == SciDAVis::DateTime){
	ui.cbFormat->setEditable( true );
  }else{
	ui.cbFormat->setEditable( false );
  }
}

// SLOTS 
void ColumnDock::retranslateUi(){
	m_initializing = true;
	
  	ui.cbType->clear();
	ui.cbType->addItem(i18n("Numeric"), QVariant(int(SciDAVis::Numeric)));
	ui.cbType->addItem(i18n("Text"), QVariant(int(SciDAVis::Text)));
	ui.cbType->addItem(i18n("Month names"), QVariant(int(SciDAVis::Month)));
	ui.cbType->addItem(i18n("Day names"), QVariant(int(SciDAVis::Day)));
	ui.cbType->addItem(i18n("Date and time"), QVariant(int(SciDAVis::DateTime)));
	
	ui.cbPlotDesignation->clear();
	ui.cbPlotDesignation->addItem(i18n("none"));
	ui.cbPlotDesignation->addItem(i18n("X"));
	ui.cbPlotDesignation->addItem(i18n("Y"));
	ui.cbPlotDesignation->addItem(i18n("Z"));
	ui.cbPlotDesignation->addItem(i18n("X-error"));
	ui.cbPlotDesignation->addItem(i18n("Y-error"));
	
	m_initializing = false;
}


void ColumnDock::nameChanged(){
  if (m_initializing)
	return;
  
  m_columnsList.first()->setName(ui.leName->text());
}


void ColumnDock::commentChanged(){
  if (m_initializing)
	return;
  
  m_columnsList.first()->setComment(ui.teComment->toPlainText());
}

/*!
  called when the type (column mode - numeric, text etc.) of the column was changed.
*/ 
void ColumnDock::typeChanged(int index){
   if (m_initializing)
	return;


  SciDAVis::ColumnMode columnMode = (SciDAVis::ColumnMode)ui.cbType->itemData( index ).toInt();
  
  m_initializing = true;
  this->updateFormatWidgets(columnMode);
  m_initializing = false;
  
  int format_index = ui.cbFormat->currentIndex();

  switch(columnMode) {
	  case SciDAVis::Numeric:{
		int digits = ui.sbPrecision->value();
		Double2StringFilter * filter;
		foreach(Column* col, m_columnsList) {
		  col->beginMacro(QObject::tr("%1: change column type").arg(col->name()));
		  col->setColumnMode(columnMode);
		  filter = static_cast<Double2StringFilter*>(col->outputFilter());
		  filter->setNumericFormat(ui.cbFormat->itemData(format_index).toChar().toLatin1());
		  filter->setNumDigits(digits);
		  col->endMacro();
		}
		break;
	  }
	  case SciDAVis::Text:
		  foreach(Column* col, m_columnsList){
			  col->setColumnMode(columnMode);
		  }
		  break;
	  case SciDAVis::Month:
	  case SciDAVis::Day:
	  case SciDAVis::DateTime:{
		QString format;
		DateTime2StringFilter * filter;
		foreach(Column* col, m_columnsList) {
		  col->beginMacro(QObject::tr("%1: change column type").arg(col->name()));
		  format = ui.cbFormat->currentText();
		  col->setColumnMode(columnMode);
		  filter = static_cast<DateTime2StringFilter*>(col->outputFilter());
		  filter->setFormat(format);
		  col->endMacro();
		}
		break;
	  }
  }
}

/*!
  called when the format for the current type (column mode) was changed.
*/
void ColumnDock::formatChanged(int index){
  if (m_initializing)
	return;

  SciDAVis::ColumnMode mode = (SciDAVis::ColumnMode)ui.cbType->itemData( ui.cbType->currentIndex() ).toInt();
  int format_index = index;

  switch(mode) {
	  case SciDAVis::Numeric:{
		Double2StringFilter * filter;
		foreach(Column* col, m_columnsList) {
		  filter = static_cast<Double2StringFilter*>(col->outputFilter());
		  filter->setNumericFormat(ui.cbFormat->itemData(format_index).toChar().toLatin1());
		  qDebug()<<"format changed, numeric format "<<filter->numericFormat();
		}
		break;
	  }
	  case SciDAVis::Text:
		  break;
	  case SciDAVis::Month:
	  case SciDAVis::Day:
	  case SciDAVis::DateTime:{
		DateTime2StringFilter * filter;
		QString format = ui.cbFormat->itemData( ui.cbFormat->currentIndex() ).toString();
		foreach(Column* col, m_columnsList){
		  filter = static_cast<DateTime2StringFilter*>(col->outputFilter());
		  filter->setFormat(format);
		}
		break;
	  }
  }
}


void ColumnDock::precisionChanged(int digits){
  if (m_initializing)
	return;
  
  foreach(Column* col, m_columnsList){
	  Double2StringFilter * filter = static_cast<Double2StringFilter*>(col->outputFilter());
	  filter->setNumDigits(digits);
  }  
}


void ColumnDock::plotDesignationChanged(int index){
  if (m_initializing)
	return;
  
  SciDAVis::PlotDesignation pd=SciDAVis::PlotDesignation(index);
  foreach(Column* col, m_columnsList){
	col->setPlotDesignation(pd);
  }  
}