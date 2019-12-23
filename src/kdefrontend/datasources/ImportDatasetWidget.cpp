/***************************************************************************
	File                 : ImportDatasetWidget.cpp
	Project              : LabPlot
	Description          : import online dataset widget
	--------------------------------------------------------------------
	Copyright            : (C) 2019 Kovacs Ferencz (kferike98@gmail.com)
	Copyright            : (C) 2019 by Alexander Semke (alexander.semke@web.de)

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

#include "backend/datasources/DatasetHandler.h"
#include "kdefrontend/datasources/ImportDatasetWidget.h"
#include "kdefrontend/DatasetModel.h"

#include <QCompleter>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonValue>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QStandardPaths>

#include <KLocalizedString>

/*!
	\class ImportDatasetWidget
	\brief Widget for importing data from a dataset.

	\ingroup kdefrontend
 */
ImportDatasetWidget::ImportDatasetWidget(QWidget* parent) : QWidget(parent),
	m_networkManager(new QNetworkAccessManager(this)),
	m_rootCategoryItem(new QTreeWidgetItem(QStringList(i18n("All")))) {

	ui.setupUi(this);

	m_jsonDir = QStandardPaths::locate(QStandardPaths::AppDataLocation, QLatin1String("datasets"), QStandardPaths::LocateDirectory);
	loadCategories();

	ui.lwDatasets->setSelectionMode(QAbstractItemView::SingleSelection);
	ui.twCategories->setSelectionMode(QAbstractItemView::SingleSelection);

	const int size = ui.leSearch->height();
	ui.lSearch->setPixmap( QIcon::fromTheme(QLatin1String("go-next")).pixmap(size, size) );

	QString info = i18n("Enter the keyword you want to search for.");
	ui.lSearch->setToolTip(info);
	ui.leSearch->setToolTip(info);
	ui.leSearch->setPlaceholderText(i18n("Search..."));

	connect(ui.cbCollections, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
			this, &ImportDatasetWidget::collectionChanged);
	connect(ui.twCategories, &QTreeWidget::itemDoubleClicked, this, &ImportDatasetWidget::updateDatasets);
	connect(ui.twCategories, &QTreeWidget::itemSelectionChanged, [this] {
		if(!m_loadingCategories)
			updateDatasets(ui.twCategories->selectedItems().first());
	});

	connect(ui.leSearch, &QLineEdit::textChanged, this, &ImportDatasetWidget::updateCategories);
	connect(ui.lwDatasets, &QListWidget::itemSelectionChanged, [this]() { datasetChanged(); });
	connect(ui.lwDatasets, &QListWidget::doubleClicked, [this]() {emit datasetDoubleClicked(); });
	connect(m_networkManager, &QNetworkAccessManager::finished, this, &ImportDatasetWidget::downloadFinished);
}

/**
 * @brief Processes the json metadata file that contains the list of categories and subcategories and their datasets.
 */
void ImportDatasetWidget::loadCategories() {
	m_datasetsMap.clear();
	ui.cbCollections->clear();

	const QString collectionsFileName = m_jsonDir + QLatin1String("/DatasetCollections.json");
	QFile file(collectionsFileName);

	if (file.open(QIODevice::ReadOnly)) {
		QJsonDocument document = QJsonDocument::fromJson(file.readAll());
		file.close();
		if (!document.isArray()) {
			QDEBUG("Invalid definition of " + collectionsFileName);
			return;
		}

		m_collections = document.array();

		for (int col = 0; col < m_collections.size(); col++) {
			const QJsonObject& collection = m_collections[col].toObject();
			const QString& m_collection = collection["name"].toString();

			QString path = m_jsonDir + QLatin1Char('/') + m_collection + ".json";
			QFile collectionFile(path);
			if (collectionFile.open(QIODevice::ReadOnly)) {
				QJsonDocument collectionDocument = QJsonDocument::fromJson(collectionFile.readAll());
				if (!collectionDocument.isObject()) {
					QDEBUG("Invalid definition of " + path);
					continue;
				}

				QJsonObject collectionObject = collectionDocument.object();
				QJsonArray categoryArray = collectionObject.value("categories").toArray();

				//processing categories
				for(int i = 0 ; i < categoryArray.size(); ++i) {
					const QJsonObject& currentCategory = categoryArray[i].toObject();
					const QString& categoryName = currentCategory.value("name").toString();
					const QJsonArray& subcategories = currentCategory.value("subcategories").toArray();

					//processing subcategories
					for(int j = 0; j < subcategories.size(); ++j) {
						QJsonObject currentSubCategory = subcategories[j].toObject();
						QString subcategoryName = currentSubCategory.value("name").toString();
						const QJsonArray& datasetArray = currentSubCategory.value("datasets").toArray();

						//processing the datasets of the actual subcategory
						for (const auto& dataset : datasetArray)
							m_datasetsMap[m_collection][categoryName][subcategoryName].push_back(dataset.toObject().value("filename").toString());
					}
				}
			}
		}

		if(m_model)
			delete m_model;
		m_model = new DatasetModel(m_datasetsMap);

		//Fill up collections combo box
		ui.cbCollections->addItem(i18n("All") + QString(" (" + QString::number(m_model->allDatasetsList().toStringList().size()) + ")"), QLatin1String("All"));
		for (QString collection : m_model->collections())
			ui.cbCollections->addItem(collection + " (" + QString::number(m_model->datasetCount(collection)) + ")", collection);

		collectionChanged(ui.cbCollections->currentIndex());
	} else
		QMessageBox::critical(this, i18n("File not found"),
							  i18n("Couldn't open the dataset collections file %1. Please check your installation.", collectionsFileName));
}

/**
 * Shows all categories and sub-categories for the currently selected collection
 */
void ImportDatasetWidget::collectionChanged(int index) {
	m_allCollections = (index == 0);

	if (!m_allCollections)
		m_collection = ui.cbCollections->itemData(index).toString();
	else
		m_collection = "";

	//update the info field
	QString info;
	if (!m_allCollections) {
		for (int i = 0; i < m_collections.size(); ++i) {
			const QJsonObject& collection = m_collections[i].toObject();
			if ( m_collection == collection["name"].toString() ) {
				info += collection["description"].toString();
				info += "<br><br></hline><br><br>";
				break;
			}
		}
	} else {
		for (int i = 0; i < m_collections.size(); ++i) {
			const QJsonObject& collection = m_collections[i].toObject();
			info += collection["description"].toString();
			info += "<br><br>";
		}
	}
	ui.lInfo->setText(info);
	updateCategories();

	//update the completer
	if(m_completer)
		delete m_completer;

	//add all categories, sub-categories and the dataset names for the current collection
	QStringList keywords;
	for(auto category : m_model->categories(m_collection)) {
		keywords << category;
		for(auto subcategory : m_model->subcategories(m_collection, category)) {
			keywords << subcategory;
			for (QString dataset : m_model->datasets(m_collection, category, subcategory))
				keywords << dataset;
		}
	}

	m_completer = new QCompleter(keywords, this);
	m_completer->setCompletionMode(QCompleter::PopupCompletion);
	m_completer->setCaseSensitivity(Qt::CaseSensitive);
	ui.leSearch->setCompleter(m_completer);
}

void ImportDatasetWidget::updateCategories() {
	m_loadingCategories = true;
	ui.twCategories->clear();
	ui.lwDatasets->clear();

	QTreeWidgetItem* rootItem = new QTreeWidgetItem(QStringList(i18n("All")));
	ui.twCategories->addTopLevelItem(rootItem);

	const QString& filter = ui.leSearch->text();
	bool categoryMatch = false;
	bool subcategoryMatch = false;
	bool datasetMatch = false;

	//add categories
	for(auto category : m_model->categories(m_collection)) {
		categoryMatch = (filter.isEmpty() || category.startsWith(filter, Qt::CaseInsensitive));

		if (categoryMatch) {
			QTreeWidgetItem* const item = new QTreeWidgetItem(QStringList(category));
			rootItem->addChild(item);

			//add all sub-categories
			for(auto subcategory : m_model->subcategories(m_collection, category))
				item->addChild(new QTreeWidgetItem(QStringList(subcategory)));
		} else {
			QTreeWidgetItem* item = nullptr;
			for(auto subcategory : m_model->subcategories(m_collection, category)) {
				subcategoryMatch = subcategory.startsWith(filter, Qt::CaseInsensitive);

				if (subcategoryMatch) {
					if (!item) {
						item = new QTreeWidgetItem(QStringList(category));
						rootItem->addChild(item);
						item->setExpanded(true);
					}
					item->addChild(new QTreeWidgetItem(QStringList(subcategory)));
				} else {
					for (QString dataset : m_model->datasets(m_collection, category, subcategory)) {
						datasetMatch = dataset.startsWith(filter, Qt::CaseInsensitive);
						if (datasetMatch) {
							if (!item) {
								item = new QTreeWidgetItem(QStringList(category));
								rootItem->addChild(item);
								item->setExpanded(true);
							}
							item->addChild(new QTreeWidgetItem(QStringList(subcategory)));
							break;
						}
					}
				}
			}
		}
	}

	rootItem->setExpanded(true);
	m_loadingCategories = false;
}

/**
 * @brief Restores the lastly selected collection, category and subcategory making it the selected QTreeWidgetItem and also lists the datasets belonigng to it
 */
void ImportDatasetWidget::restoreSelectedSubcategory(const QString& m_collection) {
	if(m_model->categories(m_collection).contains(m_category)) {
		const QTreeWidgetItem* const categoryItem = ui.twCategories->findItems(m_category, Qt::MatchExactly).first();

		if(m_model->subcategories(m_collection, m_category).contains(m_subcategory)) {
			for(int i = 0; i < categoryItem->childCount(); ++i)	{
				if(categoryItem->child(i)->text(0).compare(m_subcategory) == 0) {
					QTreeWidgetItem* const subcategoryItem = categoryItem->child(i);
					ui.twCategories->setCurrentItem(subcategoryItem);
					subcategoryItem->setSelected(true);
					m_subcategory.clear();
					updateDatasets(subcategoryItem);
					break;
				}
			}
		}
	}
}

/**
 * @brief Populates lwDatasets with the datasets of the selected subcategory or its parent
 * @param item the selected subcategory
 */
void ImportDatasetWidget::updateDatasets(QTreeWidgetItem* item) {
	ui.lwDatasets->clear();

	const QString& filter = ui.leSearch->text();

	if(item->childCount() == 0) {
		//sub-category was selected -> show all its datasets
		m_category = item->parent()->text(0);
		m_subcategory = item->text(0);

		for (QString dataset : m_model->datasets(m_collection, m_category, m_subcategory))
			if (filter.isEmpty() || dataset.startsWith(filter, Qt::CaseInsensitive))
				ui.lwDatasets->addItem(new QListWidgetItem(dataset));
	} else {
		if (!item->parent()) {
			//top-level item "All" was selected -> show datasets for all categories and their sub-categories
			m_category = "";
			m_subcategory = "";

			for (auto category : m_model->categories(m_collection)) {
				for (auto subcategory : m_model->subcategories(m_collection, category)) {
					for (QString dataset : m_model->datasets(m_collection, category, subcategory)) {
						if (filter.isEmpty() || dataset.startsWith(filter, Qt::CaseInsensitive))
							ui.lwDatasets->addItem(new QListWidgetItem(dataset));
					}
				}
			}
		} else {
			//a category was selected -> show all its datasets
			m_category = item->text(0);
			m_subcategory = "";

			for (auto subcategory : m_model->subcategories(m_collection, m_category)) {
				for (QString dataset : m_model->datasets(m_collection, m_category, subcategory)) {
					if (filter.isEmpty() || dataset.startsWith(filter, Qt::CaseInsensitive))
						ui.lwDatasets->addItem(new QListWidgetItem(dataset));
				}
			}
		}
	}
}

/**
 * @brief Returns the name of the selected dataset
 */
QString ImportDatasetWidget::getSelectedDataset() const {
	if (ui.lwDatasets->selectedItems().count() > 0)
		return ui.lwDatasets->selectedItems().at(0)->text();
	else
		return QString();
}

/**
 * @brief Initiates the processing of the dataset's metadata file and of the dataset itself.
 * @param datasetHandler the DatasetHanlder that downloads processes the dataset
 */
void ImportDatasetWidget::import(DatasetHandler* datasetHandler) {
	datasetHandler->processMetadata(m_datasetObject);
}

/**
 * @brief Returns the QJsonObject associated with the currently selected dataset.
 */
QJsonObject ImportDatasetWidget::loadDatasetObject() {
	for (int i = 0; i < m_collections.size(); ++i) {
		const QJsonObject& collectionJson = m_collections[i].toObject();
		const QString& collection = collectionJson["name"].toString();

		//we have to find the selected collection in the metadata file.
		if(m_allCollections || m_collection == collection) {
			QFile file(m_jsonDir + QLatin1Char('/') + collection + ".json");

			//open the metadata file of the current collection
			if (file.open(QIODevice::ReadOnly)) {
				QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
				file.close();
				if(!doc.isObject()) {
					DEBUG("The " +  collection.toStdString() + ".json file is invalid");
					return QJsonObject();
				}

				QJsonArray categoryArray = doc.object().value("categories").toArray();

				//processing categories
				for(int i = 0 ; i < categoryArray.size(); ++i) {
					const QJsonObject currentCategory = categoryArray[i].toObject();
					const QString categoryName = currentCategory.value("name").toString();
					if(m_category.isEmpty() || categoryName.compare(m_category) == 0) {
						const QJsonArray subcategories = currentCategory.value("subcategories").toArray();

						//processing subcategories
						for(int j = 0; j < subcategories.size(); ++j) {
							QJsonObject currentSubCategory = subcategories[j].toObject();
							QString subcategoryName = currentSubCategory.value("name").toString();

							if(m_subcategory.isEmpty() || subcategoryName.compare(m_subcategory) == 0) {
								const QJsonArray datasetArray = currentSubCategory.value("datasets").toArray();

								//processing the datasets of the actual subcategory
								for (const auto& dataset : datasetArray) {
									if(getSelectedDataset().compare(dataset.toObject().value("filename").toString()) == 0)
										return dataset.toObject();
								}

								if (!m_subcategory.isEmpty())
									break;
							}
						}

						if (!m_category.isEmpty())
							break;
					}
				}
			}

			if (!m_allCollections)
				break;
		}
	}

	return QJsonObject();
}

/**
 * @brief Returns the structure containing the categories, subcategories and datasets.
 * @return the structure containing the categories, subcategories and datasets
 */
const DatasetsMap& ImportDatasetWidget::getDatasetsMap() {
	return m_datasetsMap;
}

/**
 * @brief Sets the currently selected collection
 * @param category the name of the collection
 */
void ImportDatasetWidget::setCollection(const QString& collection) {
	ui.cbCollections->setCurrentText(collection + " (" + QString(m_model->datasetCount(collection)) + ")");
}

/**
 * @brief Sets the currently selected category
 * @param category the name of the category
 */
void ImportDatasetWidget::setCategory(const QString &category) {
	for(int i = 0; i < ui.twCategories->topLevelItemCount(); i++) {
		if (ui.twCategories->topLevelItem(i)->text(0).compare(category) == 0) {
			updateDatasets(ui.twCategories->topLevelItem(i));
			break;
		}
	}
}

/**
 * @brief Sets the currently selected subcategory
 * @param subcategory the name of the subcategory
 */
void ImportDatasetWidget::setSubcategory(const QString &subcategory) {
	for(int i = 0; i < ui.twCategories->topLevelItemCount(); i++) {
		if (ui.twCategories->topLevelItem(i)->text(0).compare(m_category) == 0) {
			QTreeWidgetItem* categoryItem = ui.twCategories->topLevelItem(i);
			for(int j = 0; j <categoryItem->childCount(); j++) {
				if(categoryItem->child(j)->text(0).compare(subcategory) == 0) {
					updateDatasets(categoryItem->child(j));
					break;
				}
			}
			break;
		}
	}
}

/**
 * @brief  Sets the currently selected dataset
 * @param the currently selected dataset
 */
void ImportDatasetWidget::setDataset(const QString &datasetName) {
	for(int i = 0; i < ui.lwDatasets->count() ; i++) {
		if(ui.lwDatasets->item(i)->text().compare(datasetName) == 0) {
			ui.lwDatasets->item(i)->setSelected(true);
			break;
		}
	}
}

/**
 * @brief Updates the details of the currently selected dataset
 */
void ImportDatasetWidget::datasetChanged() {
	QString info;
	if (ui.cbCollections->currentIndex() != 0) {
		const QString& m_collection = ui.cbCollections->itemData(ui.cbCollections->currentIndex()).toString();
		for (int i = 0; i < m_collections.size(); ++i) {
			const QJsonObject& collection = m_collections[i].toObject();
			if ( m_collection.startsWith(collection["name"].toString()) ) {
				info += collection["description"].toString();
				info += "<br><br>";
				break;
			}
		}
	}

	if(!getSelectedDataset().isEmpty()) {
		m_datasetObject = loadDatasetObject();

		info += "<b>" + i18n("Dataset") + ":</b><br>";
		info += m_datasetObject["name"].toString();
		info += "<br><br>";
		info += "<b>" + i18n("Description") + ":</b><br>";

		if (m_datasetObject.contains("description_url")) {
			WAIT_CURSOR;
			if (m_networkManager->networkAccessible() == QNetworkAccessManager::Accessible)
				m_networkManager->get(QNetworkRequest(QUrl(m_datasetObject["description_url"].toString())));
			else
				info += m_datasetObject["description"].toString();
		} else
			info += m_datasetObject["description"].toString();
	} else
		m_datasetObject = QJsonObject();

	ui.lInfo->setText(info);
	emit datasetSelected();
}

void ImportDatasetWidget::downloadFinished(QNetworkReply* reply) {
	if (reply->error() == QNetworkReply::NoError) {
		QByteArray ba = reply->readAll();
		QString info(ba);
		info = info.replace(QLatin1Char('\n'), QLatin1String("<br>"));
		ui.lInfo->setText(ui.lInfo->text() + info);
	} else {
		DEBUG("Failed to fetch the description.");
		ui.lInfo->setText(ui.lInfo->text() + m_datasetObject["description"].toString());
	}
	reply->deleteLater();
	RESET_CURSOR;
}
