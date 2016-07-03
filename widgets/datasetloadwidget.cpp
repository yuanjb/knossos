/*
 *  This file is a part of KNOSSOS.
 *
 *  (C) Copyright 2007-2016
 *  Max-Planck-Gesellschaft zur Foerderung der Wissenschaften e.V.
 *
 *  KNOSSOS is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 of
 *  the License as published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 *  For further information, visit http://www.knossostool.org
 *  or contact knossos-team@mpimf-heidelberg.mpg.de
 */

#include "datasetloadwidget.h"

#include "dataset.h"
#include "GuiConstants.h"
#include "loader.h"
#include "mainwindow.h"
#include "network.h"
#include "skeleton/skeletonizer.h"
#include "viewer.h"

#include <QApplication>
#include <QComboBox>
#include <QFileDialog>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QProcess>
#include <QPushButton>
#include <QSettings>
#include <QVBoxLayout>

#include <stdexcept>

DatasetLoadWidget::DatasetLoadWidget(QWidget *parent) : QDialog(parent) {
    setModal(true);
    setWindowTitle("Load Dataset");

    tableWidget.setColumnCount(3);
    tableWidget.verticalHeader()->setVisible(false);
    tableWidget.horizontalHeader()->setVisible(false);
    tableWidget.setSelectionBehavior(QAbstractItemView::SelectRows);
    tableWidget.setSelectionMode(QAbstractItemView::SingleSelection);
    tableWidget.horizontalHeader()->resizeSection(1, 25);
    tableWidget.horizontalHeader()->resizeSection(2, 20);
    tableWidget.horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    infoLabel.setWordWrap(true);//allows shrinking below minimum width
    splitter.setOrientation(Qt::Vertical);
    splitter.addWidget(&tableWidget);
    splitter.addWidget(&infoLabel);
    splitter.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    line.setFrameShape(QFrame::HLine);
    line.setFrameShadow(QFrame::Sunken);

    cubeEdgeSpin.setRange(1, 256);
    cubeEdgeSpin.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    superCubeEdgeSpin.setMinimum(3);
    superCubeEdgeSpin.setSingleStep(2);
    superCubeEdgeSpin.setAlignment(Qt::AlignLeft);
    superCubeEdgeSpin.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    superCubeEdgeHLayout.addWidget(&superCubeEdgeSpin);
    superCubeEdgeHLayout.addWidget(&superCubeSizeLabel);

    cubeEdgeHLayout.addWidget(&cubeEdgeSpin);
    cubeEdgeHLayout.addWidget(&cubeEdgeLabel);

    buttonHLayout.addWidget(&processButton);
    buttonHLayout.addWidget(&cancelButton);
    mainLayout.addWidget(&splitter);
    mainLayout.addWidget(&line);
    mainLayout.addLayout(&superCubeEdgeHLayout);
    mainLayout.addLayout(&cubeEdgeHLayout);
    mainLayout.addWidget(&segmentationOverlayCheckbox);
    mainLayout.addLayout(&buttonHLayout);

    setLayout(&mainLayout);

    QObject::connect(&tableWidget, &QTableWidget::cellChanged, this, &DatasetLoadWidget::datasetCellChanged);
    QObject::connect(&tableWidget, &QTableWidget::itemSelectionChanged, this, &DatasetLoadWidget::updateDatasetInfo);
    QObject::connect(&cubeEdgeSpin, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &DatasetLoadWidget::adaptMemoryConsumption);
    QObject::connect(&superCubeEdgeSpin, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &DatasetLoadWidget::adaptMemoryConsumption);
    QObject::connect(&segmentationOverlayCheckbox, &QCheckBox::stateChanged, this, &DatasetLoadWidget::adaptMemoryConsumption);
    QObject::connect(&processButton, &QPushButton::clicked, this, &DatasetLoadWidget::processButtonClicked);
    QObject::connect(&cancelButton, &QPushButton::clicked, this, &DatasetLoadWidget::cancelButtonClicked);
    resize(600, 600);//random default size, will be overriden by settings if present

    this->setWindowFlags(this->windowFlags() & (~Qt::WindowContextHelpButtonHint));
}

void DatasetLoadWidget::insertDatasetRow(const QString & dataset, const int row) {
    tableWidget.insertRow(row);

    auto rowFromCell = [this](int column, QPushButton * const button){
        for(int row = 0; row < tableWidget.rowCount(); ++row) {
            if (button == tableWidget.cellWidget(row, column)) {
                return row;
            }
        }
        return -1;
    };

    QPushButton *addDatasetButton = new QPushButton("…");
    addDatasetButton->setToolTip(tr("Select a dataset from file…"));
    QObject::connect(addDatasetButton, &QPushButton::clicked, [this, rowFromCell, addDatasetButton](){
        state->viewerState->renderInterval = SLOW;
        const auto selectedFile = QFileDialog::getOpenFileUrl(this, "Select a KNOSSOS dataset", QDir::homePath(), "*.conf").toString();
        state->viewerState->renderInterval = FAST;
        if (!selectedFile.isEmpty()) {
            QTableWidgetItem * const datasetPathItem = new QTableWidgetItem(selectedFile);
            const int row = rowFromCell(1, addDatasetButton);
            tableWidget.setItem(row, 0, datasetPathItem);
        }
    });

    QPushButton *removeDatasetButton = new QPushButton("×");
    removeDatasetButton->setToolTip(tr("Remove this dataset from the list"));
    QObject::connect(removeDatasetButton, &QPushButton::clicked, [this, rowFromCell, removeDatasetButton](){
        const int row = rowFromCell(2, removeDatasetButton);
        tableWidget.removeRow(row);
    });

    QTableWidgetItem *datasetPathItem = new QTableWidgetItem(dataset);
    tableWidget.setItem(row, 0, datasetPathItem);
    tableWidget.setCellWidget(row, 1, addDatasetButton);
    tableWidget.setCellWidget(row, 2, removeDatasetButton);
}

void DatasetLoadWidget::datasetCellChanged(int row, int col) {
    if (col == 0 && row == tableWidget.rowCount() - 1 && tableWidget.item(row, 0)->text() != "") {
        const auto dataset = tableWidget.item(row, 0)->text();
        const auto blockState = tableWidget.signalsBlocked();
        tableWidget.blockSignals(true);//changing an item would land here again

        tableWidget.item(row, 0)->setText("");//clear edit row
        insertDatasetRow(dataset, tableWidget.rowCount() - 1);//insert before edit row
        tableWidget.selectRow(row);//select new item

        tableWidget.blockSignals(blockState);
    }
    updateDatasetInfo();
}

void DatasetLoadWidget::updateDatasetInfo() {
    bool bad = tableWidget.selectedItems().empty();
    QString dataset;
    bad = bad || (dataset = tableWidget.selectedItems().front()->text()).isEmpty();
    decltype(Network::singleton().refresh(std::declval<QUrl>())) download;
    const QUrl url{dataset};
    bad = bad || !(download = Network::singleton().refresh(url)).first;
    if (bad) {
        infoLabel.setText("");
        return;
    }

    const auto ocp = url.toString().contains("/ocp/ca/");
    const auto datasetinfo = ocp ? Dataset::parseOpenConnectomeJson(url, download.second) : Dataset::fromLegacyConf(url, download.second);

    //make sure supercubeedge is small again
    auto supercubeedge = superCubeEdgeSpin.value() * cubeEdgeSpin.value() / datasetinfo.cubeEdgeLength;
    supercubeedge = std::max(3, supercubeedge - !(supercubeedge % 2));
    superCubeEdgeSpin.setValue(supercubeedge);
    cubeEdgeSpin.setValue(datasetinfo.cubeEdgeLength);
    adaptMemoryConsumption();

    QString infotext = tr("<b>%1 Dataset</b><br />%2");
    if (datasetinfo.remote) {
        infotext = infotext.arg("Remote").arg("URL: %1<br />").arg(datasetinfo.url.toString());
    } else {
        infotext = infotext.arg("Local").arg("");
    }
    infotext += QString("Boundary (x y z): %1 %2 %3<br />Compression: %4<br />cubeEdgeLength: %5<br />Magnification: %6<br />Scale (x y z): %7 %8 %9")
        .arg(datasetinfo.boundary.x).arg(datasetinfo.boundary.y).arg(datasetinfo.boundary.z)
        .arg(datasetinfo.compressionRatio)
        .arg(datasetinfo.cubeEdgeLength)
        .arg(datasetinfo.magnification)
        .arg(datasetinfo.scale.x)
        .arg(datasetinfo.scale.y)
        .arg(datasetinfo.scale.z);

    infoLabel.setText(infotext);
}

QStringList DatasetLoadWidget::getRecentPathItems() {
    QStringList recentPaths;

    for(int row = 0; row < tableWidget.rowCount() - 1; ++row) {
        if(tableWidget.item(row, 0)->text() != "") {
            recentPaths.append(tableWidget.item(row, 0)->text());
        }
    }

    return recentPaths;
}

void DatasetLoadWidget::adaptMemoryConsumption() {
    const auto cubeEdge = cubeEdgeSpin.value();
    const auto superCubeEdge = superCubeEdgeSpin.value();
    auto mebibytes = std::pow(cubeEdge, 3) * std::pow(superCubeEdge, 3) / std::pow(1024, 2);
    mebibytes += segmentationOverlayCheckbox.isChecked() * OBJID_BYTES * mebibytes;
    const auto fov = cubeEdge * (superCubeEdge - 1);
    auto text = QString("Data cache cube edge length (%1 MiB RAM)\nFOV %2 pixel per dimension").arg(mebibytes).arg(fov);
    superCubeSizeLabel.setText(text);
}

void DatasetLoadWidget::cancelButtonClicked() {
    this->hide();
}

void DatasetLoadWidget::processButtonClicked() {
    const auto dataset = tableWidget.item(tableWidget.currentRow(), 0)->text();
    if (dataset.isEmpty()) {
        QMessageBox::information(this, "Unable to load", "No path selected");
    } else if (loadDataset(boost::none, dataset)) {
        hide(); //hide datasetloadwidget only if we could successfully load a dataset
    }
}

/* dataset can be selected in three ways:
 * 1. by selecting the folder containing a k.conf (for multires datasets it's a "magX" folder)
 * 2. for multires datasets: by selecting the dataset folder (the folder containing the "magX" subfolders)
 * 3. by specifying a .conf directly.
 */
bool DatasetLoadWidget::loadDataset(const boost::optional<bool> loadOverlay, QUrl path,  const bool silent) {
    if (path.isEmpty() && datasetUrl.isEmpty()) {//no dataset available to load
        open();
        return false;
    } else if (path.isEmpty()) {//if empty reload previous
        path = datasetUrl;
    }
    const auto download = Network::singleton().refresh(path);
    if (!download.first) {
        if (!silent) {
            QMessageBox box(this);
            box.setIcon(QMessageBox::Warning);
            box.setText("Unable to load Daataset.");
            box.setInformativeText(QString("Failed to read config file from %1").arg(path.toString()));
            box.exec();
            open();
        }
        qDebug() << "no config at" << path;
        return false;
    }

    if (!silent && (!Session::singleton().annotationFilename.isEmpty() || Session::singleton().unsavedChanges)) {
        QMessageBox question(this);
        question.setIcon(QMessageBox::Question);
        question.setText(tr("Keep the current annotation for the new dataset?"));
        question.setInformativeText(tr("It only makes sense to keep the annotation if the new dataset matches it."));
        question.addButton(tr("Yes, &keep"), QMessageBox::YesRole);
        const auto * const clearButton = question.addButton(tr("No, start &new one"), QMessageBox::NoRole);
        const auto * const cancelButton = question.addButton(QMessageBox::Cancel);
        question.exec();
        if (question.clickedButton() == cancelButton || (question.clickedButton() == clearButton && !state->viewer->window->newAnnotationSlot())) {// clear skeleton, mergelist and snappy cubes
            return false;
        }
    }

    Dataset info;
    Dataset::CubeType raw_compression;
    if (path.toString().contains("/ocp/ca/")) {
        info = Dataset::parseOpenConnectomeJson(path, download.second);
    } else {
        info = Dataset::fromLegacyConf(path, download.second);
        try {
            info.checkMagnifications();
        } catch (std::exception &) {
            if (!silent) {
                QMessageBox box(this);
                box.setIcon(QMessageBox::Warning);
                box.setText("Dataset will not be loaded.");
                box.setInformativeText("No magnifications could be detected. (knossos.conf in mag folder)");
                box.exec();
                open();
            }
            qDebug() << "no mags";
            return false;
        }
    }
    datasetUrl = {path};//remember config url
    Loader::Controller::singleton().suspendLoader();//we change variables the loader uses
    info.applyToState();
    raw_compression = info.compressionRatio == 0 ? Dataset::CubeType::RAW_UNCOMPRESSED : info.compressionRatio == 1000 ? Dataset::CubeType::RAW_JPG
            : info.compressionRatio == 6 ? Dataset::CubeType::RAW_JP2_6 : Dataset::CubeType::RAW_J2K;

    // check if a fundamental geometry variable has changed. If so, the loader requires reinitialization
    state->cubeEdgeLength = cubeEdgeSpin.text().toInt();
    state->M = superCubeEdgeSpin.value();
    if (loadOverlay != boost::none) {
        segmentationOverlayCheckbox.setChecked(loadOverlay.get());
    }
    state->overlay = segmentationOverlayCheckbox.isChecked();

    state->viewer->resizeTexEdgeLength(state->cubeEdgeLength, state->M);

    applyGeometrySettings();

    emit datasetSwitchZoomDefaults();

    // reset skeleton viewport
    if (state->skeletonState->rotationcounter == 0) {
        state->skeletonState->definedSkeletonVpView = SKELVP_RESET;
    }

    Loader::Controller::singleton().restart(info.url, info.api, raw_compression, Dataset::CubeType::SEGMENTATION_SZ_ZIP, info.experimentname);

    emit updateDatasetCompression();

    Session::singleton().updateMovementArea({0, 0, 0}, state->boundary);
    // ...beginning with loading the middle of dataset
    state->viewerState->currentPosition = {state->boundary / 2};
    state->viewer->updateDatasetMag();
    state->viewer->userMove({0, 0, 0}, USERMOVE_NEUTRAL);
    emit datasetChanged(segmentationOverlayCheckbox.isChecked());

    return true;
}

void DatasetLoadWidget::saveSettings() {
    QSettings settings;
    settings.beginGroup(DATASET_WIDGET);

    settings.setValue(DATASET_GEOMETRY, saveGeometry());
    settings.setValue(DATASET_LAST_USED, datasetUrl);

    settings.setValue(DATASET_MRU, getRecentPathItems());

    settings.setValue(DATASET_CUBE_EDGE, state->cubeEdgeLength);
    settings.setValue(DATASET_SUPERCUBE_EDGE, state->M);
    settings.setValue(DATASET_OVERLAY, state->overlay);

    settings.endGroup();
}

void DatasetLoadWidget::applyGeometrySettings() {
    //settings depending on supercube and cube size
    state->cubeSliceArea = std::pow(state->cubeEdgeLength, 2);
    state->cubeBytes = std::pow(state->cubeEdgeLength, 3);
    state->cubeSetElements = std::pow(state->M, 3);
    state->cubeSetBytes = state->cubeSetElements * state->cubeBytes;

    state->viewer->window->resetTextureProperties();
}

void DatasetLoadWidget::loadSettings() {
    auto transitionedDataset = [](const QString & dataset){//update old files from settings
        QUrl url = dataset;
        if (QRegularExpression("^[A-Z]:").match(dataset).hasMatch()) {//set file scheme for windows drive letters
            url = QUrl::fromLocalFile(dataset);
        }
        if (url.isRelative()) {
            url = QUrl::fromLocalFile(dataset);
        }
        return url;
    };

    QSettings settings;
    settings.beginGroup(DATASET_WIDGET);

    restoreGeometry(settings.value(DATASET_GEOMETRY, "").toByteArray());
    datasetUrl = transitionedDataset(settings.value(DATASET_LAST_USED, "").toString());

    auto appendRowSelectIfLU = [this](const QString & dataset){
        insertDatasetRow(dataset, tableWidget.rowCount());
        if (dataset == datasetUrl.toString()) {
            tableWidget.selectRow(tableWidget.rowCount() - 1);
        }
    };

    tableWidget.blockSignals(true);

    //add datasets from file
    for (const auto & dataset : settings.value(DATASET_MRU).toStringList()) {
        appendRowSelectIfLU(transitionedDataset(dataset).toString());
    }
    //add public datasets
    auto datasetsDir = QDir(":/resources/datasets");
    for (const auto & dataset : datasetsDir.entryInfoList()) {
        const auto url = QUrl::fromLocalFile(dataset.absoluteFilePath()).toString();
        if (tableWidget.findItems(url, Qt::MatchExactly).empty()) {
            appendRowSelectIfLU(url);
        }
    }
    //add Empty row at the end
    appendRowSelectIfLU("");
    tableWidget.cellWidget(tableWidget.rowCount() - 1, 2)->setEnabled(false);//don’t delete empty row

    tableWidget.blockSignals(false);
    updateDatasetInfo();

    state->cubeEdgeLength = settings.value(DATASET_CUBE_EDGE, 128).toInt();
    if (QApplication::arguments().filter("supercube-edge").empty()) {//if not provided by cmdline
        state->M = settings.value(DATASET_SUPERCUBE_EDGE, 3).toInt();
    }
    if (QApplication::arguments().filter("overlay").empty()) {//if not provided by cmdline
        state->overlay = settings.value(DATASET_OVERLAY, false).toBool();
    }
    state->viewer->resizeTexEdgeLength(state->cubeEdgeLength, state->M);

    cubeEdgeSpin.setValue(state->cubeEdgeLength);
    superCubeEdgeSpin.setValue(state->M);
    segmentationOverlayCheckbox.setCheckState(state->overlay ? Qt::Checked : Qt::Unchecked);
    adaptMemoryConsumption();

    settings.endGroup();

    applyGeometrySettings();

    loadDataset();// load last used dataset or show
}
