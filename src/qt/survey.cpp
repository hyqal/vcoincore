#include "survey.h"
#include "ui_survey.h"
#include "version.h"

#include "clientmodel.h"
#include "walletmodel.h"
#include "bitcoinunits.h"
#include "optionsmodel.h"
#include "guiutil.h"
#include "guiconstants.h"
#include "platformstyle.h"

#include <QShortcut>
#include <QMessageBox>
#include <QSettings>
#include <QDialog>
#include <QWidget>
#include <QString>
#include <QNetworkReply>

class QDialog;

Survey::Survey(const PlatformStyle *platformStyle, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Survey),
    clientModel(0),
    walletModel(0)
{
	ui->setupUi(this);
	ui->buttonBox->buttons().first()->setText(tr("Send"));
	this->adjustSize();
	QShortcut *closeKey = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_W), this);
	connect(closeKey, SIGNAL(activated()), this, SLOT(close()));
	QShortcut *quitKey = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Q), this);
	connect(quitKey, SIGNAL(activated()), parent, SLOT(close()));

    /*
    os = QString("ssrfVers=%1").arg(subsurface_version());
	os.append(QString("&prettyOsName=%1").arg(SubsurfaceSysInfo::prettyOsName()));
	QString arch = SubsurfaceSysInfo::buildCpuArchitecture();
	os.append(QString("&appCpuArch=%1").arg(arch));
	if (arch == "i386") {
		QString osArch = SubsurfaceSysInfo::currentCpuArchitecture();
		os.append(QString("&osCpuArch=%1").arg(osArch));
	}
	os.append(QString("&uiLang=%1").arg(uiLanguage(NULL)));
	os.append(QString("&uuid=%1").arg(UpdateManager::getUUID()));
	ui->system->setPlainText(getVersion());
    */
}

QString Survey::getVersion()
{
    /*
	QString arch;
	// fill in the system data
	QString sysInfo = QString("Subsurface %1").arg(subsurface_version());
	sysInfo.append(tr("\nOperating system: %1").arg(SubsurfaceSysInfo::prettyOsName()));
	arch = SubsurfaceSysInfo::buildCpuArchitecture();
	sysInfo.append(tr("\nCPU architecture: %1").arg(arch));
	if (arch == "i386")
		sysInfo.append(tr("\nOS CPU architecture: %1").arg(SubsurfaceSysInfo::currentCpuArchitecture()));
	sysInfo.append(tr("\nLanguage: %1").arg(uiLanguage(NULL)));
	return sysInfo;
    */
    return QString("");
}

Survey::~Survey()
{
	delete ui;
}

void Survey::setClientModel(PlatformStyle *platformStyle, ClientModel *model)
{
    this->clientModel = model;
    if(model)
    {
    }
}

void Survey::setWalletModel(WalletModel *model)
{
    this->walletModel = model;

}

#define ADD_OPTION(_name) values.append(ui->_name->isChecked() ? "&" #_name "=1" : "&" #_name "=0")

void Survey::on_buttonBox_accepted()
{
	// now we need to collect the data and submit it
	QString values = os;
	ADD_OPTION(recreational);
	ADD_OPTION(tech);
	ADD_OPTION(planning);
	ADD_OPTION(download);
	ADD_OPTION(divecomputer);
	ADD_OPTION(manual);
	ADD_OPTION(companion);
    // values.append(QString("&suggestion=%1").arg(ui->suggestions->toPlainText()));
    // SurveyServices uss(this);
    // connect(uss.sendSurvey(values), SIGNAL(finished()), SLOT(requestReceived()));
	hide();
}

void Survey::on_buttonBox_rejected()
{
	QMessageBox response(this);
	response.setText(tr("Should we ask you later?"));
	response.addButton(tr("Don't ask me again"), QMessageBox::RejectRole);
	response.addButton(tr("Ask later"), QMessageBox::AcceptRole);
	response.setWindowTitle(tr("Ask again?")); // Not displayed on MacOSX as described in Qt API
	response.setIcon(QMessageBox::Question);
	response.setWindowModality(Qt::WindowModal);
	switch (response.exec()) {
	case QDialog::Accepted:
		// nothing to do here, we'll just ask again the next time they start
		break;
	case QDialog::Rejected:
		QSettings s;
        s.beginGroup("Survey");
		s.setValue("SurveyDone", "declined");
		break;
	}
	hide();
}

void Survey::requestReceived()
{
	QMessageBox msgbox;
    QString msgTitle = tr("Submit survey response.");
    QString msgText = "<h3>" + tr("Unable to submit the survey response.") + "</h3>";

	QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
	if (reply->error() != QNetworkReply::NoError) {
		//Network Error
		msgText = msgText + "<br/><b>" + tr("The following error occurred:") + "</b><br/>" + reply->errorString()
				+ "<br/><br/><b>" + tr("Please check your internet connection.") + "</b>";
	} else {
		//No network error
		QString response(reply->readAll());
		QString responseBody = response.split("\"").at(1);

		msgbox.setIcon(QMessageBox::Information);

		if (responseBody == "OK") {
            msgText = tr("Survey response successfully submitted.");
			QSettings s;
            s.beginGroup("Survey");
			s.setValue("SurveyDone", "submitted");
		} else {
			msgText = tr("There was an error while trying to check for updates.<br/><br/>%1").arg(responseBody);
			msgbox.setIcon(QMessageBox::Warning);
		}
	}

	msgbox.setWindowTitle(msgTitle);
    msgbox.setWindowIcon(QIcon(":/icons/survey"));
	msgbox.setText(msgText);
	msgbox.setTextFormat(Qt::RichText);
	msgbox.exec();
	reply->deleteLater();
}
