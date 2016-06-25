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

#ifndef NODESTAB_H
#define NODESTAB_H

#include <QAbstractListModel>
#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFrame>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QSettings>
#include <QWidget>

class PropertyModel : public QAbstractListModel {
    Q_OBJECT
    friend class NodesTab;
    std::vector<QString> properties;
public:
    virtual int rowCount(const QModelIndex &) const override;
    virtual QVariant data(const QModelIndex & index, int role) const override;
    void recreate(const QSet<QString> & numberProperties);
};

class NodesTab : public QWidget
{
    friend class AppearanceWidget;
    Q_OBJECT
    QGridLayout mainLayout;
    QFrame nodeSeparator;
    QLabel idLabel{"Show node IDs"};
    QComboBox idCombo;
    QCheckBox nodeCommentsCheck{"Show node comments"};
    QCheckBox overrideNodeRadiusCheck{"Override node radius"};
    QDoubleSpinBox nodeRadiusSpin;
    QLabel edgeNodeRatioLabel{"Edge : Node radius ratio"};
    QDoubleSpinBox edgeNodeRatioSpin;
    // property visualization
    QLabel propertyHeader{"<strong>Property Highlighting</strong>"};
    PropertyModel propertyModel;
    QComboBox propertyRadiusCombo;
    QLabel propertyRadiusLabel{"Scale property and use as node radius:"};
    QDoubleSpinBox propertyRadiusScaleSpin;
    QComboBox propertyColorCombo;
    QDoubleSpinBox propertyMinSpin;
    QDoubleSpinBox propertyMaxSpin;
    QLabel propertyColorLabel{"Map property and use as node color:"};
    QString lutPath;
    QLabel lutLabel{"Current LUT: none"};
    QPushButton propertyLUTButton{"Load color LUT …"};
    void loadNodeLUTRequest(QString path = "");
    void saveSettings(QSettings &settings) const;
    void loadSettings(const QSettings &settings);
public:
    explicit NodesTab(QWidget *parent = 0);
    void updateProperties(const QSet<QString> & numberProperties);
signals:

public slots:
};

#endif // NODESTAB_H
