/***************************************************************************
    File                 : ImportFileWidget.h
    Project              : LabPlot
    Description          : import file data widget
    --------------------------------------------------------------------
    Copyright            : (C) 2009-2017 by Stefan Gerlach (stefan.gerlach@uni-konstanz.de)
    Copyright            : (C) 2009-2019 Alexander Semke (alexander.semke@web.de)
    Copyright            : (C) 2017-2018 Fabian Kristof (fkristofszabolcs@gmail.com)
    Copyright            : (C) 2018-2019 Kovacs Ferencz (kferike98@gmail.com)

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
#ifndef IMPORTFILEWIDGET_H
#define IMPORTFILEWIDGET_H

#include "ui_importfilewidget.h"
#include "backend/datasources/LiveDataSource.h"
#include <memory>

#ifdef HAVE_MQTT
#include "backend/datasources/MQTTClient.h"
class MQTTSubscriptionWidget;
#endif

#include <QVector>

class AbstractFileFilter;
class AsciiOptionsWidget;
class BinaryOptionsWidget;
class HDF5OptionsWidget;
class ImageOptionsWidget;
class NetCDFOptionsWidget;
class FITSOptionsWidget;
class JsonOptionsWidget;
class ROOTOptionsWidget;
class QTableWidget;
class QCompleter;
class QTimer;
class QTreeWidgetItem;
class QStringList;
class LiveDataHandler;

class ImportFileWidget : public QWidget {
	Q_OBJECT

public:
	explicit ImportFileWidget(QWidget*, bool liveDataSource, const QString& fileName = QString());
	~ImportFileWidget() override;

	void showOptions(bool);
	void saveSettings(LiveDataSource*) const;
	void loadSettings();
	AbstractFileFilter::FileType currentFileType() const;
	LiveDataSource::SourceType currentSourceType() const;
	AbstractFileFilter* currentFileFilter() const;
	QString fileName() const;
	QString selectedObject() const;
	bool isFileEmpty() const;
	const QStringList selectedHDF5Names() const;
	const QStringList selectedNetCDFNames() const;
	const QStringList selectedFITSExtensions() const;
	const QStringList selectedROOTNames() const;
	void showAsciiHeaderOptions(bool);
	void showJsonModel(bool);

	QString host() const; // LiveDataHandler
	QString port() const; // LiveDataHandler
	QString serialPort() const; // LiveDataHandler
	int baudRate() const; // LiveDataHandler

private:
	Ui::ImportFileWidget ui;
	void setMQTTVisible(bool);
	void updateContent(const QString&);
	void initOptionsWidget();
	void initSlots();

	std::unique_ptr<AsciiOptionsWidget> m_asciiOptionsWidget;
	std::unique_ptr<BinaryOptionsWidget> m_binaryOptionsWidget;
	std::unique_ptr<HDF5OptionsWidget> m_hdf5OptionsWidget;
	std::unique_ptr<ImageOptionsWidget> m_imageOptionsWidget;
	std::unique_ptr<NetCDFOptionsWidget> m_netcdfOptionsWidget;
	std::unique_ptr<FITSOptionsWidget> m_fitsOptionsWidget;
	std::unique_ptr<JsonOptionsWidget> m_jsonOptionsWidget;
	std::unique_ptr<ROOTOptionsWidget> m_rootOptionsWidget;

	mutable std::unique_ptr<AbstractFileFilter> m_currentFilter;

	QTableWidget* m_twPreview;
	const QString& m_fileName;
	bool m_fileEmpty{false};
	bool m_liveDataSource;
	bool m_suppressRefresh{false};
	LiveDataHandler* m_liveDataHandler{nullptr};

private slots:
	void fileNameChanged(const QString&);
	void fileTypeChanged(int);

	void updateTypeChanged(int);
	void sourceTypeChanged(int);
	void readingTypeChanged(int);

	void saveFilter();
	void manageFilters();
	void filterChanged(int);
	void selectFile();
	void fileInfoDialog();
	void refreshPreview();
	void settingsChanged();
	void updatePreviewWindow(QVector<QStringList> importedStrings, QStringList vectorNameList, QVector<AbstractColumn::ColumnMode> columnModes);
signals:
	void fileNameChanged();
	void sourceTypeChanged();
	void previewRefreshed();
	void preview(int nbrOfLines);
	void checkedFitsTableToMatrix(const bool enable);
	void checkOKButton(bool deviceOK, QString toolTip);
	void fileTypeToAscii(int index, int startRow, int endRow, int startColumn, int endColumn);
	void fileTypeToBinary(int index, int startRow, int endRow);
	void fileTypeToROOT(int startRow, int endRow, QVector<QStringList> columns);
	void fileTypeToHDF5(int startRow, int endRow, int startColumn, int endColumn);
	void fileTypeToNETCDF(int startRow, int endRow, int startColumn, int endColumn);
	void fileTypeToFITS(int startRow, int endRow, int startColumn, int endColumn);
	void fileTypeToImage(int startRow, int endRow, int startColumn, int endColumn);
	void fileTypeToNgspiceRawAscii(int startRow, int endRow);
	void fileTypeToNgspiceRawBinary(int startRow, int endRow);
	void filterTypeToJSON(int startRow, int endRow, int startColumn, int endColumn);



	friend class HDF5OptionsWidget;	// to access refreshPreview()
	friend class NetCDFOptionsWidget;	// to access refreshPreview() and others
	friend class FITSOptionsWidget;
	friend class JsonOptionsWidget;
	friend class ROOTOptionsWidget;	// to access refreshPreview() and others

#ifdef HAVE_MQTT
private:
    void unsubscribeFromTopic(const QString&, QVector<QTreeWidgetItem*> children);

	QMqttClient* m_client{nullptr};
    QVector<QMqttSubscription*> m_mqttSubscriptions;
	QTimer* m_connectTimeoutTimer;
	QMap<QMqttTopicName, QMqttMessage> m_lastMessage;
	QVector<QString> m_subscribedTopicNames;
	QVector<QString> m_addedTopics;
	QString m_configPath;
	bool m_initialisingMQTT{false};
	bool m_connectionTimedOut{false};
	MQTTClient::MQTTWill m_willSettings;
    MQTTSubscriptionWidget* m_subscriptionWidget;

public:
	void saveMQTTSettings(MQTTClient*) const;
	bool isMqttValid();

signals:
    void newTopic(const QString&);
	void subscriptionsChanged();
	void checkFileType();
    void updateSubscriptionTree(const QVector<QString>&);
    void MQTTClearTopics();

private slots:
	void mqttConnectionChanged();
    void onMqttConnect();
    void mqttSubscribe(const QString&, uint);
    void mqttMessageReceived(const QByteArray&, const QMqttTopicName&);
	void mqttSubscriptionMessageReceived(const QMqttMessage& );
	void onMqttDisconnect();
    void mqttErrorChanged(QMqttClient::ClientError);
	void mqttConnectTimeout();
	void showMQTTConnectionManager();
	void readMQTTConnections();
	void showWillSettings();
    void enableWill(bool);
#endif
};

#endif
