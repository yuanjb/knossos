#ifndef EVENTMODEL_H
#define EVENTMODEL_H

/*
 *  This file is a part of KNOSSOS.
 *
 *  (C) Copyright 2007-2013
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
 */

/*
 * For further information, visit http://www.knossostool.org or contact
 *     Joergen.Kornfeld@mpimf-heidelberg.mpg.de or
 *     Fabian.Svara@mpimf-heidelberg.mpg.de
 */

#include "knossos-global.h"
#include <QObject>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QWheelEvent>
#include <QShortcut>

/**
  * @class EventModel
  * @brief This is the EventHandler from Knossos 3.2 adjusted for the QT version
  *
  * The class adopts the core functionality from EventHandler with the exception that
  * SDL Events are replaced through the corresponding QT-Events.
  *
  * @warning The api documentation for event-handlig in QT 5 seems to be outdated :
  * Keyboard modifier like SHIFT, ALT and CONTROl has to be requested from QApplication
  * If two modifier are pressed at the same time the method is to use the testFlag method
  * of the QKeyModifiers object. All other variants were not successful.
  * If new functionality has to be integrated which uses modifier just use the way it is done in this class
  * You will save a couple of hours ..
  */

Coordinate getCoordinateFromOrthogonalClick(const int x_dist, const int y_dist, int VPfound);
uint64_t segmentationColorPicking(int x, int y, const int viewportId);
void merging(QMouseEvent *event, const int vp);

class EventModel : public QObject
{
    Q_OBJECT
    bool validPosition(QMouseEvent *event, int VPfound);

public:
    explicit EventModel(QObject *parent = 0);
    bool handleMouseButtonLeft(QMouseEvent *event, int VPfound);
    bool handleMouseButtonMiddle(QMouseEvent *event, int VPfound);
    void handleMouseButtonRight(QMouseEvent *event, int VPfound);
    bool handleMouseMotion(QMouseEvent *event, int VPfound);
    bool handleMouseMotionLeftHold(QMouseEvent *event, int VPfound);
    bool handleMouseMotionMiddleHold(QMouseEvent *event, int VPfound);
    void handleMouseMotionRightHold(QMouseEvent *event, int VPfound);
    void handleMouseReleaseLeft(QMouseEvent *event, int VPfound);
    void handleMouseReleaseRight(QMouseEvent *event, int VPfound);
    void handleMouseReleaseMiddle(QMouseEvent *event, int VPfound);
    void handleMouseWheel(QWheelEvent * const event, int VPfound);
    void handleKeyPress(QKeyEvent *event, int VPfound);
    void handleKeyRelease(QKeyEvent *event);
    void startNodeSelection(int x, int y, int vpId);
    void nodeSelection(int x, int y, int vpId);
    Coordinate getMouseCoordinate(int VPfound);
    int xrel(int x);
    int yrel(int y);
    int rightMouseDownX;
    int rightMouseDownY;
    int mouseDownX;
    int mouseDownY;
    int mouseX;
    int mouseY;
    int mousePosX;
    int mousePosY;
    bool grap;
signals:
    void userMoveSignal(int x, int y, int z);
    void userMoveArbSignal(float x, float y, float z);
    void rotationSignal(float x, float y, float z, float angle);
    void pasteCoordinateSignal();
    void zoomReset();
    void zoomOrthoSignal(float step);
    void zoomInSkeletonVPSignal();
    void zoomOutSkeletonVPSignal();
    void updateViewerStateSignal();
    void showSelectedTreesAndNodesSignal();

    void updateWidgetSignal();

    void genTestNodesSignal(uint number);
    bool addSkeletonNodeSignal(Coordinate *clickedCoordinate, Byte VPtype);

    void setActiveNodeSignal(nodeListElement *node, uint nodeID);
    void setRecenteringPositionSignal(float x, float y, float z);
    void setRecenteringPositionWithRotationSignal(float x, float y, float z, uint vp);
    void delSegmentSignal(uint sourceNodeID, uint targetNodeID, segmentListElement *segToDel);
    void editNodeSignal(uint nodeID, nodeListElement *node, float newRadius, int newXPos, int newYPos, int newZPos, int inMag);
    void addCommentSignal(const char *content, nodeListElement *node, uint nodeID);
    bool editCommentSignal(commentListElement *currentComment, uint nodeID, char *newContent, nodeListElement *newNode, uint newNodeID);
    void addSegmentSignal(uint sourceNodeID, uint targetNodeID);
    void jumpToActiveNodeSignal();
    void updateTreeviewSignal();
    void updateCommentsTable();
    void updateSlicePlaneWidgetSignal();
    void pushBranchNodeSignal(int setBranchNodeFlag, int checkDoubleBranchpoint, nodeListElement *branchNode, uint branchNodeID);
    void popBranchNodeSignal();

    void moveToNextTreeSignal();
    void moveToPrevTreeSignal(bool *isSuccess = NULL);
    void moveToPrevNodeSignal();
    void moveToNextNodeSignal();

    nodeListElement *findNodeInRadiusSignal(Coordinate searchPosition);
    segmentListElement *findSegmentByNodeIDSignal(uint sourceNodeID, uint targetNodeID);
    uint addSkeletonNodeAndLinkWithActiveSignal(Coordinate *clickedCoordinate, Byte VPtype, int makeNodeActive);
    treeListElement *addTreeListElement(int treeID, color4F color);
    void setViewportOrientationSignal(int orientation);
    void unselectNodesSignal();

    void treeAddedSignal(treeListElement *tree);
    void nodeAddedSignal();
    void deleteSelectedNodesSignal();
    void nodeActivatedSignal();
    void nodeRadiusChangedSignal(nodeListElement *node);
    void nodePositionChangedSignal(nodeListElement *node);

    void compressionRatioToggled();
};

#endif // EVENTMODEL_H
