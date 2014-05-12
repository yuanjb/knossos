#include "pythonpropertywidget.h"
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QFormLayout>
#include <QCheckBox>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include <QDirIterator>
#include <QSettings>
#include <QApplication>
#include <QDesktopWidget>
#include "GuiConstants.h"
#include "knossos-global.h"
#include <stdio.h>
#include <unistd.h>

extern stateInfo *state;

PythonPropertyWidget::PythonPropertyWidget(QWidget *parent) :
    QDialog(parent)
{

    setWindowTitle("Python Properties");
    QFormLayout *layout = new QFormLayout();

    pythonInterpreterField = new QLineEdit();
    pythonInterpreterButton = new QPushButton("Select Python Interpreter");
    pythonInterpreterField->setToolTip("The complete path to ./python. This will be checked on save");
    autoStartFolder = new QLineEdit();
    autoStartFolder->setToolTip("Scripts in this folder were automatically started with KNOSSOS");
    autoStartFolderButton = new QPushButton("Select Autostart Folder");
    autoStartTerminal = new QCheckBox("Open Terminal On Start");

    saveButton = new QPushButton("save");

    layout->addRow(pythonInterpreterField, pythonInterpreterButton);
    layout->addRow(autoStartFolder, autoStartFolderButton);
    layout->addWidget(autoStartTerminal);


    setLayout(layout);
    this->setWindowFlags(this->windowFlags() & (~Qt::WindowContextHelpButtonHint));

    connect(pythonInterpreterButton, SIGNAL(clicked()), this, SLOT(pythonInterpreterButtonClicked()));
    connect(autoStartFolderButton, SIGNAL(clicked()), this, SLOT(autoStartFolderButtonClicked()));

}

void PythonPropertyWidget::closeEvent(QCloseEvent *e) {
    this->hide();
}

void PythonPropertyWidget::pythonInterpreterButtonClicked() {
    state->viewerState->renderInterval = SLOW;
     QString selection = QFileDialog::getOpenFileName(this, "select the python interpreter", QDir::homePath());
     if(!selection.isEmpty()) {
         pythonInterpreterField->setText(selection);
     }

     state->viewerState->renderInterval = FAST;
}

void PythonPropertyWidget::autoStartFolderButtonClicked() {
    state->viewerState->renderInterval = SLOW;
     QString selection = QFileDialog::getExistingDirectory(this, "select the autostart folder", QDir::homePath());
     if(!selection.isEmpty()) {
         autoStartFolder->setText(selection);
     }
     state->viewerState->renderInterval = FAST;
}

void PythonPropertyWidget::autoConf() {
    FILE *pipe = NULL;
    QString command;
#ifdef Q_OS_UNIX
    command = QString("/bin/sh -c 'which python'");
#endif
#ifdef Q_OS_WIN
    command = QString("where python");
#endif
    pipe = popen(command.toUtf8().data(), "r");
    if(!pipe) {
        pythonInterpreterField->setText("Error");
        return;
    }

    char buffer[128];
    if(!fgets(buffer, sizeof(buffer), pipe)) {
        pythonInterpreterField->setText("Error");
        return;
    }

    qDebug() << buffer;
    pythonInterpreterField->setText(QString(buffer));
    pclose(pipe);
    memset(&buffer[0], 0, sizeof(buffer));

#ifdef Q_OS_UNIX
    command = QString("/bin/sh -c 'python -V'"); // where in windows
#endif
#ifdef Q_OS_WIN
    command = QString("python -V");
#endif

    pipe = popen(command.toUtf8().data(), "r");
    if(!pipe) {
        return;
    }


    if(!fgets(buffer, sizeof(buffer), pipe)) {
        return;
    }

    QString version(buffer);


    pclose(pipe);

}

/** @todo adjust path to the delivered ipython */
void PythonPropertyWidget::openTerminal() {
    pid_t pid = getpid();
    char *home = getenv("HOME");
    if(!home) {
        return;
    }

    QString path(QString("%1%2").arg(home).arg("/.ipython/profile_default/security"));


    QDirIterator it(path);
    while(it.hasNext()) {
        QString filename = it.next();
        if(filename.contains(QString("%1").arg(pid))) {
        QFileInfo info(filename);
        filename = info.fileName();
#ifdef Q_OS_OSX
        QString args = QString("/Library/Frameworks/Python.framework/Versions/2.7/bin/ipython console --existing %1").arg(filename);
        system(QString("/opt/X11/bin/xterm -e '%1' &").arg(args).toUtf8().data());
#endif
#ifdef Q_OS_LINUX
        QString args = QString("ipython console --existing '%1'").arg(filename);
        system(QString("/usr/bin/xterm -e '%1' &").arg(args).toUtf8().data());
#endif
#ifdef Q_OS_WIN
           system(QString("'%1' &").arg(args).toUtf8().data());
#endif
        }
    }

}

void PythonPropertyWidget::saveSettings() {
    QSettings settings;
    settings.beginGroup(PYTHON_PROPERTY_WIDGET);
    settings.setValue(WIDTH, this->geometry().width());
    settings.setValue(HEIGHT, this->geometry().height());
    settings.setValue(POS_X, this->geometry().x());
    settings.setValue(POS_Y, this->geometry().y());
    settings.setValue(VISIBLE, this->isVisible());

    if(!this->pythonInterpreterField->text().isEmpty())
        settings.setValue(PYTHON_INTERPRETER, this->pythonInterpreterField->text());
    if(!this->autoStartFolder->text().isEmpty())
        settings.setValue(PYTHON_AUTOSTART_FOLDER, this->autoStartFolder->text());
    settings.setValue(PYTHON_AUTOSTART_TERMINAL, this->autoStartTerminal->isChecked());

    settings.endGroup();

}

void PythonPropertyWidget::loadSettings() {
    int width, height, x, y;
    bool visible;

    QSettings settings;
    settings.beginGroup(PYTHON_PROPERTY_WIDGET);
    width = (settings.value(WIDTH).isNull())? this->width() : settings.value(WIDTH).toInt();
    height = (settings.value(HEIGHT).isNull())? this->height() : settings.value(HEIGHT).toInt();
    if(settings.value(POS_X).isNull() or settings.value(POS_Y).isNull()) {
        x = QApplication::desktop()->screen()->rect().topRight().x() - this->width() - 20;
        y = QApplication::desktop()->screen()->rect().topRight().y() + 50;
    }
    else {
        x = settings.value(POS_X).toInt();
        y = settings.value(POS_Y).toInt();
    }
    visible = (settings.value(VISIBLE).isNull())? false : settings.value(VISIBLE).toBool();
    if(visible) {
        show();
    }
    else {
        hide();
    }

    this->move(x, y);
    this->resize(width, height);
    this->setVisible(visible);

    if(!settings.value(PYTHON_INTERPRETER).isNull() and !settings.value(PYTHON_INTERPRETER).toString().isEmpty()) {
        pythonInterpreterField->setText(settings.value(PYTHON_INTERPRETER).toString());
    } else {
        autoConf();
    }

    if(!settings.value(PYTHON_AUTOSTART_FOLDER).isNull() and !settings.value(PYTHON_AUTOSTART_FOLDER).toString().isEmpty()) {
        autoStartFolder->setText(settings.value(PYTHON_AUTOSTART_FOLDER).toString());
        emit executeUserScriptsSignal();
    }

    if(!settings.value(PYTHON_AUTOSTART_TERMINAL).isNull() and settings.value(PYTHON_AUTOSTART_TERMINAL).toBool()) {
        autoStartTerminal->setChecked(settings.value(PYTHON_AUTOSTART_TERMINAL).toBool());
        openTerminal();
    }

    settings.endGroup();
}

void PythonPropertyWidget::autoStartTerminalClicked(bool on) {
    QSettings settings;
    settings.beginGroup(PYTHON_PROPERTY_WIDGET);
    settings.setValue(PYTHON_AUTOSTART_TERMINAL, on);
    settings.endGroup();
}