#ifndef MAINWINDOW_H
#define MAINWINDOW_H

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

#include <array>
#include <memory>

#define FILE_DIALOG_HISTORY_MAX_ENTRIES 10
#define LOCK_VP_ORIENTATION_DEFAULT (true)

#include <QMainWindow>
#include <QDropEvent>
#include <QQueue>
#include <QComboBox>
#include <QUndoStack>
#include "widgetcontainer.h"

namespace Ui {
    class MainWindow;     
}



class QLabel;
class QToolBar;
class QToolButton;
class QPushButton;
class QSpinBox;
class QCheckBox;
class QMessageBox;
class QGridLayout;
class QFile;
class Viewport;

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void updateSkeletonFileName(QString &fileName);
    bool eventFilter(QObject *object, QEvent *event);
    void closeEvent(QCloseEvent *event);
    void updateTitlebar();

    static void reloadDataSizeWin();
    static void datasetColorAdjustmentsChanged();

signals:
    bool changeDatasetMagSignal(uint serverMovement);
    void recalcTextureOffsetsSignal();    
    void clearSkeletonSignal(int targetRevision, int loadingSkeleton);
    void updateSkeletonFileNameSignal(int targetRevision, int increment, char *filename);
    bool loadSkeletonSignal(QString fileName);
    bool saveSkeletonSignal(QString fileName);
    void recentFileSelectSignal(int index);
    void updateToolsSignal();
    void updateTreeviewSignal();
    void updateCommentsTableSignal();
    void userMoveSignal(int x, int y, int z, int serverMovement);

    void stopRenderTimerSignal();
    void startRenderTimerSignal(int frequency);
    void updateTreeColorsSignal();
    void loadTreeLUTFallback();

    TreeListElement *addTreeListElementSignal(int sync, int targetRevision, int treeID, Color4F color, int serialize);
    void nextCommentSignal(QString searchString);
    void previousCommentSignal(QString searchString);
    /* */
    void moveToPrevNodeSignal();
    void moveToNextNodeSignal();
    void moveToPrevTreeSignal();
    void moveToNextTreeSignal();
    bool popBranchNodeSignal();
    bool pushBranchNodeSignal(int targetRevision, int setBranchNodeFlag, int checkDoubleBranchpoint, nodeListElement *branchNode, int branchNodeID, int serialize);
    void jumpToActiveNodeSignal();

    bool addCommentSignal(int targetRevision, QString content, nodeListElement *node, int nodeID, int serialize);
    bool editCommentSignal(int targetRevision, commentListElement *currentComment, int nodeID, QString newContent, nodeListElement *newNode, int newNodeID, int serialize);

    void updateTaskDescriptionSignal(QString description);
    void updateTaskCommentSignal(QString comment);

    void treeAddedSignal(TreeListElement *tree);
    void branchPushedSignal();
    void branchPoppedSignal();
    void nodeCommentChangedSignal(nodeListElement *node);
    void viewportDecorationSignal(bool visible);
protected:
    void resizeEvent(QResizeEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
    void dragLeaveEvent(QDragLeaveEvent *event);
    void dropEvent(QDropEvent *event);
    void resizeViewports(int width, int height);
    void becomeFirstEntry(const QString &entry);
    QString openFileDirectory;
    QString saveFileDirectory;
public:
    Ui::MainWindow *ui;
    QToolBar *toolBar;
    QToolButton *copyButton;
    QToolButton *pasteButton;
    QLabel *xLabel, *yLabel, *zLabel;
    QSpinBox *xField, *yField, *zField;

    QMessageBox *prompt;

    QWidget *mainWidget;
    QGridLayout *gridLayout;
    std::array<std::unique_ptr<Viewport>, NUM_VP> viewports;

    // contains all widgets
    WidgetContainer *widgetContainer;

    /* file actions */
    QAction *recentFileAction;
    QAction **historyEntryActions;

    /* edit skeleton actions */
    QAction *addNodeAction;
    QAction *linkWithActiveNodeAction;
    QAction *dropNodesAction;
    QAction *skeletonStatisticsAction;
    QAction *clearSkeletonAction;

    QAction *newTreeAction;
    QAction *nextCommentAction;
    QAction *previousCommentAction;
    QAction *pushBranchNodeAction;
    QAction *popBranchNodeAction;
    QAction *moveToPrevNodeAction;
    QAction *moveToNextNodeAction;
    QAction *moveToPrevTreeAction;
    QAction *moveToNextTreeAction;
    QAction *jumpToActiveNodeAction;

    /* view actions */
    QAction *workModeViewAction;
    QAction *dragDatasetAction;
    QAction *recenterOnClickAction;
    QAction *zoomAndMultiresAction;
    QAction *tracingTimeAction;

    /* preferences actions */
    QAction *loadCustomPreferencesAction;
    QAction *saveCustomPreferencesAction;
    QAction *defaultPreferencesAction;
    QAction *datasetNavigationAction;
    QAction *synchronizationAction;
    QAction *dataSavingOptionsAction;
    QAction *viewportSettingsAction;

    /* window actions */
    QAction *toolsAction;
    QAction *taskAction;
    QAction *logAction;
    QAction *commentShortcutsAction;
    QAction *annotationAction;

    /* help actions */
    QAction *aboutAction;

    /* Qmenu-points */
    QMenuBar *customBar;
    QMenu *dataSetMenu;
    QMenu *fileMenu;
    QMenu *recentFileMenu;
    QMenu *editMenu;
    QMenu *workModeEditMenu;
    QMenu *viewMenu;
    QMenu *workModeViewMenu;
    QMenu *preferenceMenu;
    QMenu *windowMenu;
    QMenu *helpMenu;

    QQueue<QString> *skeletonFileHistory;
    QFile *loadedFile;


    QToolButton *open, *save;
    QToolButton *pythonButton;
    QToolButton *tracingTimeButton;
    QToolButton *zoomAndMultiresButton;
    QToolButton *syncButton;
    QToolButton *viewportSettingsButton;
    QToolButton *toolsButton;
    QToolButton *commentShortcutsButton;
    QPushButton *resetVPsButton;
    QPushButton *resetVPOrientButton;
    QCheckBox *lockVPOrientationCheckbox;
    QToolButton *taskManagementButton;
    QToolButton *annotationButton;

    void createViewports();

    // for creating action, menus and the toolbar
    void createActions();
    void createMenus();
    void createToolBar();


    // for save, load and clear settings
    void saveSettings();
    void loadSettings();
    void clearSettings();

public slots:
    // for the recent file menu
    bool loadSkeletonAfterUserDecision(const QString &fileName);
    void updateFileHistoryMenu();
    bool alreadyInMenu(const QString &path);
    bool addRecentFile(const QString &fileName);    


    /* dataset */
    void openDatasetSlot();
    /* skeleton menu */
    void openSlot();
    void openSlot(const QString &fileName); // for the drag n drop version
    void saveSlot();
    void saveAsSlot();
    void quitSlot();

    /* edit skeleton menu*/
    void addNodeSlot();
    void linkWithActiveNodeSlot();
    void dropNodesSlot();
    void skeletonStatisticsSlot();
    void clearSkeletonSlotNoGUI();
    void clearSkeletonSlotGUI();
    void clearSkeletonWithoutConfirmation();

    /* view menu */
    void dragDatasetSlot();
    void recenterOnClickSlot();
    void zoomAndMultiresSlot();
    void tracingTimeSlot();

    /* preferences menu */
    void loadCustomPreferencesSlot();
    void saveCustomPreferencesSlot();
    void defaultPreferencesSlot();
    void datatasetNavigationSlot();
    void synchronizationSlot();
    void dataSavingOptionsSlot();
    void viewportSettingsSlot();

    /* window menu */
    void taskSlot();
    void logSlot();
    void commentShortcutsSlots();
    void annotationSlot();

    /* help menu */
    void aboutSlot();
    void documentationSlot();

    /* toolbar slots */
    void copyClipboardCoordinates();
    void pasteClipboardCoordinates();
    void coordinateEditingFinished();

    void uncheckToolsAction();
    void uncheckViewportSettingAction();
    void uncheckCommentShortcutsAction();
    void uncheckConsoleAction();    
    void uncheckDataSavingAction();

    void uncheckSynchronizationAction();
    void uncheckNavigationAction();
    void updateCoordinateBar(int x, int y, int z);  
    void recentFileSelected();
    void treeColorAdjustmentsChanged();
    // viewports
    void resetViewports();
    void resetVPOrientation();
    void lockVPOrientation(bool lock);
    void showVPDecorationClicked();

    // from the event handler
    void newTreeSlot();
    void nextCommentNodeSlot();
    void previousCommentNodeSlot();
    void pushBranchNodeSlot();
    void popBranchNodeSlot();
    void moveToPrevNodeSlot();
    void moveToNextNodeSlot();
    void moveToPrevTreeSlot();
    void moveToNextTreeSlot();
    void jumpToActiveNodeSlot();
    void F1Slot();
    void F2Slot();
    void F3Slot();
    void F4Slot();
    void F5Slot();

};

#endif // MAINWINDOW_H
