#include "glycogencluster.h"
#include "object.h"
#include "glycogenanalysismanager.h"
#include "glwidget.h"
#include "openglmanager.h"

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <map>

//------------------------------------------------------
//
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    mainwindow_ui(new Ui::MainWindow)
{
    mainwindow_ui->setupUi(this);
	m_currentSelectedCluster = 0;
	m_clusters = 0;

    // setup the table view
    tableView = new QStandardItemModel(0,2,this); //0 Rows and 2 Columns
    tableView->setHorizontalHeaderItem(0, new QStandardItem(QString("ID")));
    tableView->setHorizontalHeaderItem(1, new QStandardItem(QString("Name")));
    mainwindow_ui->tableView->setModel(tableView);
    QObject::connect(getGLWidget(), SIGNAL(object_clicked(QList<QStandardItem*>)),
                     this, SLOT(on_object_clicked(QList<QStandardItem*>)));

    QObject::connect(getGLWidget(), SIGNAL(clearRowsTable()),
                     this, SLOT(clearTable()));

    QObject::connect(getGLWidget(), SIGNAL(RemoveRowAt_GL(QModelIndex)),
                     this, SLOT(RemoveRowAt(QModelIndex)));

    QObject::connect(this, SIGNAL(getDeletedData(int)),
                     getGLWidget(), SLOT(getDeletedHVGXID(int)));

    QObject::connect(getGLWidget(), SIGNAL(checkAllListWidget_GL()),
                     this, SLOT(checkAllListWidget()));

}

//------------------------------------------------------
//
MainWindow::~MainWindow()
{
    delete mainwindow_ui;
}

//------------------------------------------------------
//
GLWidget* MainWindow::getGLWidget()
{
	return mainwindow_ui->openGLWidget;
}

//------------------------------------------------------
//
MousePad* MainWindow::getMousePad()
{
	return mainwindow_ui->openGLWidget_2;
}

//------------------------------------------------------
//
void MainWindow::on_object_clicked(QList<QStandardItem*> items)
{
    tableView->appendRow(items);
}

//------------------------------------------------------
//
void MainWindow::clearTable()
{
    tableView->removeRows(0, tableView->rowCount(), QModelIndex());

}

//------------------------------------------------------
//
void MainWindow::checkAllListWidget()
{
    qDebug() << "checkAllListWidget";
    for (int row = 0; row < mainwindow_ui->listWidget->count(); row++) {
        QListWidgetItem *item = mainwindow_ui->listWidget->item(row);
        item->setCheckState(Qt::Checked);
    }
}

//------------------------------------------------------
//
void MainWindow::RemoveRowAt(QModelIndex selectedIndex)
{
    QModelIndex hvgxID_index = tableView->index(selectedIndex.row() , 0);
    QVariant hvgxID_item =  tableView->data(hvgxID_index);
    int hvgxID = hvgxID_item.toInt();
    getDeletedData(hvgxID);
    tableView->removeRows(selectedIndex.row(), 1, QModelIndex());
}


//------------------------------------------------------
//
void MainWindow::on_glycogenVisibilityCheckBox_toggled(bool checked)
{
	//qDebug() << "glycogen visibility toggled";

	getGLWidget()->getOpenGLManager()->setRenderGlycogenGranules(checked);
}

//------------------------------------------------------
//
void MainWindow::on_clusterButton_clicked()
{
	
	mainwindow_ui->glycogenClustersTreeWidget->clear();
	if (m_clusters)
		m_clusters->clear();

	//qDebug() << "cluster button clicked";
	GlycogenAnalysisManager* gam = getGLWidget()->getGlycogenAnalysisManager();
	double eps = mainwindow_ui->epsDoubleSpinBox->value();
	int minPts = mainwindow_ui->minPtsSpinBox->value();
	//run clustering
	m_clusters = gam->runDBScan(eps, minPts);


	mainwindow_ui->glycogenClustersTreeWidget->setColumnCount(3);
	QStringList headerLabels;
	headerLabels.push_back(tr("ClusterID"));
	headerLabels.push_back(tr("Count"));
	headerLabels.push_back(tr("Volume"));
	mainwindow_ui->glycogenClustersTreeWidget->setHeaderLabels(headerLabels);

	mainwindow_ui->glycogenClustersTreeWidget->setSortingEnabled(false);

	//fill clustering tree widget
	for (auto iter = m_clusters->begin(); iter != m_clusters->end(); iter++)
	{
		Clustering::GlycogenCluster* cluster = (*iter).second;
		//create cluster parent
		QTreeWidgetItem* cluster_item = new QTreeWidgetItem(mainwindow_ui->glycogenClustersTreeWidget);
		cluster_item->setText(0, QString::number(cluster->getID()));
		cluster_item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
		cluster_item->setCheckState(0, Qt::Checked);

		cluster_item->setText(1, QString::number(cluster->getClusterSize()));
		cluster_item->setText(2, QString::number(cluster->getTotalVolume() , 'f', 4));
	}
	mainwindow_ui->glycogenClustersTreeWidget->setSortingEnabled(true);
	mainwindow_ui->glycogenClustersTreeWidget->sortByColumn(2, Qt::SortOrder::DescendingOrder);
	getGLWidget()->getOpenGLManager()->updateGlycogenPoints();
	
}

//------------------------------------------------------
//
void MainWindow::on_glycogenClustersTreeWidget_itemChanged(QTreeWidgetItem* item, int column)
{
	//qDebug() << "item changed " << item->text(column);
	if (item)
	{	
		int id = item->text(0).toInt();
		if (m_currentSelectedCluster)
		{
			int id2 = m_currentSelectedCluster->text(0).toInt();
			
			if (id != id2)
				m_clusters->at(id)->setState((item->checkState(0) == Qt::Checked) ? 1 : 0);
		}
		else
		{
			m_clusters->at(id)->setState((item->checkState(0) == Qt::Checked) ? 1 : 0);
		}

		getGLWidget()->getOpenGLManager()->updateGlycogenPoints();
	}
}

//------------------------------------------------------
//
void MainWindow::on_glycogenClustersTreeWidget_itemSelectionChanged()
{
	//qDebug() << "item selection changed";
	QList<QTreeWidgetItem*> selected_Items = mainwindow_ui->glycogenClustersTreeWidget->selectedItems();
	
	if (m_currentSelectedCluster)
	{

		int id = m_currentSelectedCluster->text(0).toInt();
		//unselect
		m_clusters->at(id)->setState((m_currentSelectedCluster->checkState(0) == Qt::Checked) ? 1 : 0);
		m_currentSelectedCluster = 0;
	}

	if (selected_Items.size())
	{
		//set selected
		m_currentSelectedCluster = selected_Items[0];
		int id = m_currentSelectedCluster->text(0).toInt();
		m_clusters->at(id)->setState(2);
	}
	getGLWidget()->getOpenGLManager()->updateGlycogenPoints();
}

//------------------------------------------------------
//
void MainWindow::on_mapGlycogenGranulesButton_clicked()
{
	GlycogenAnalysisManager* gam = getGLWidget()->getGlycogenAnalysisManager();

	bool boutons = mainwindow_ui->glycogenMapToBoutonsCheckBox->isChecked();
	bool spines = mainwindow_ui->glycogenMapToSpinesCheckBox->isChecked();
	bool clusters = false;

	std::map<int, std::map<int, int>>* results = gam->computeGlycogenMapping(boutons, spines, clusters);

	DataContainer* dc = getGLWidget()->getDataContainer();
	std::map<int, Object*>* objects = dc->getObjectsMapPtr();

	mainwindow_ui->glycogenMappingTreeWidget->setColumnCount(3);
	QStringList headerLabels;
	headerLabels.push_back(tr("Object ID"));
	headerLabels.push_back(tr("Name"));
	headerLabels.push_back(tr("Granules"));
	mainwindow_ui->glycogenMappingTreeWidget->setHeaderLabels(headerLabels);

	//fill mapping tree widget
	for (auto iter = results->begin(); iter != results->end(); iter++)
	{
		int object_id = iter->first;
		Object* current_object = objects->at(object_id);
		
		//create map item
		QTreeWidgetItem* map_item = new QTreeWidgetItem(mainwindow_ui->glycogenMappingTreeWidget);
		map_item->setText(0, QString::number(current_object->getHVGXID()));
		map_item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
		map_item->setCheckState(0, Qt::Checked);

		map_item->setText(1, QString(current_object->getName().c_str()));
		map_item->setText(2, QString::number(iter->second.size()));
	}
}

//------------------------------------------------------
//
void MainWindow::on_mapGlycogenClustersButton_clicked()
{
	if (m_clusters && m_clusters->size() > 0)
	{
		GlycogenAnalysisManager* gam = getGLWidget()->getGlycogenAnalysisManager();

		bool boutons = mainwindow_ui->glycogenMapToBoutonsCheckBox->isChecked();
		bool spines = mainwindow_ui->glycogenMapToSpinesCheckBox->isChecked();
		bool clusters = true;

		std::map<int, std::map<int, int>>* results = gam->computeGlycogenMapping(boutons, spines, clusters);
	
		DataContainer* dc = getGLWidget()->getDataContainer();
		std::map<int, Object*>* objects = dc->getObjectsMapPtr();

		mainwindow_ui->glycogenMappingTreeWidget->setColumnCount(3);
		QStringList headerLabels;
		headerLabels.push_back(tr("Object ID"));
		headerLabels.push_back(tr("Name"));
		headerLabels.push_back(tr("Clusters"));
		mainwindow_ui->glycogenMappingTreeWidget->setHeaderLabels(headerLabels);

		//fill mapping tree widget
		for (auto iter = results->begin(); iter != results->end(); iter++)
		{
			int object_id = iter->first;
			Object* current_object = objects->at(object_id);

			//create map item
			QTreeWidgetItem* map_item = new QTreeWidgetItem(mainwindow_ui->glycogenMappingTreeWidget);
			map_item->setText(0, QString::number(current_object->getHVGXID()));
			map_item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
			map_item->setCheckState(0, Qt::Checked);

			map_item->setText(1, QString(current_object->getName().c_str()));
			map_item->setText(2, QString::number(iter->second.size()));
		}


	}
	else
	{
		//show message saying: No clusters available, calculate clusters first
	}
}
