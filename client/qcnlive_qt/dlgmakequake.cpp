#include "dlgmakequake.h"

CDialogMakeQuake::CDialogMakeQuake(const int iTime, const int iCountdown, QWidget* parent, Qt::WindowFlags f)  : QDialog(parent, f)
{
	setModal(true);  // make it an application level modal window
	setWindowModality(Qt::ApplicationModal);
	setWindowTitle(tr("Make Your Own Earthquake"));
	setFixedSize(QSize(500,270)); // spin controls aren't showing up if I let Qt decide on size, at least on Mac
	
	m_bStart = false;
	
	m_iTime = iTime;
	m_iCountdown = iCountdown;

    CreateControls();
}


CDialogMakeQuake::~CDialogMakeQuake()
{
	// get rid of all our objects if they exist
	if (m_labelName) delete m_labelName;
	
    if (m_textctrlName) delete m_textctrlName;
    if (m_spinctrlTime) delete m_spinctrlTime;
    if (m_spinctrlCountdown) delete m_spinctrlCountdown;
	
	if (m_buttonStart) delete m_buttonStart;
	if (m_buttonCancel) delete m_buttonCancel;

	if (m_layoutMain) delete m_layoutMain;
	if (m_layoutSpin) delete m_layoutSpin;
	if (m_layoutButton) delete m_layoutButton;
	
	if (m_gridlayout) delete m_gridlayout;

	if (m_groupName) delete m_groupName;
	if (m_groupSpin) delete m_groupSpin;
	if (m_groupButton) delete m_groupButton;

}

void CDialogMakeQuake::InitPointers()
{
    m_strName.clear();

    // init ptrs to null to test for deletion later
    m_textctrlName = NULL; 
    m_spinctrlTime = NULL; 	
    m_spinctrlCountdown = NULL; 	
	
	m_buttonStart = NULL;
	m_buttonCancel = NULL;
	
	m_layoutMain = NULL;
	m_layoutSpin = NULL;
    m_layoutButton = NULL;

	m_groupName = NULL;
	m_groupSpin = NULL;
	m_groupButton = NULL;

	m_labelName = NULL;
	
	m_gridlayout = NULL;
}

void CDialogMakeQuake::getUserString(char* strName)
{ 
	memset(strName, 0x00, sizeof(char) * 64);
	strncpy(strName, m_strName.toAscii(), 64);
}


/*
bool CDialogMakeQuake::saveValues(QString& strError)
{
	strError.clear();
	m_strLatitude = m_textctrlLatitude->text();
    double dTest = atof((const char*) m_strLatitude.toAscii());
    if (dTest >= -90.0f && dTest <= 90.0f) {
        sm->dMyLatitude = dTest;
	}
	else {
		strError += "Error in latitude<BR> - not between -90 and 90<BR><BR>"; 
	}
	
	m_strLongitude = m_textctrlLongitude->text();
    dTest = atof((const char*) m_strLongitude.toAscii());
    if (dTest >= -180.0f && dTest <= 180.0f) {
        sm->dMyLongitude = dTest;
	}
	else {
		strError += "Error in longitude<BR> - not between -180 and 180<BR><BR>"; 
	}

	m_strStation = m_textctrlStation->text();
    memset((char*) sm->strMyStation, 0x00, SIZEOF_STATION_STRING);
    strlcpy((char*) sm->strMyStation, (const char*) m_strStation.toAscii(), SIZEOF_STATION_STRING);	

	m_strElevationMeter = m_textctrlElevationMeter->text();
    sm->dMyElevationMeter = atof((const char*) m_strElevationMeter.toAscii());
    sm->iMyElevationFloor = m_spinctrlElevationFloor->value();  // spin controls are ints anyway!

	QVariant qvChoice = m_comboSensor->itemData(m_comboSensor->currentIndex());
	int iChoice = (qvChoice == QVariant::Invalid ? -1 : qvChoice.toInt());
	if (iChoice >= MIN_SENSOR_USB && iChoice <= MAX_SENSOR_USB) sm->iMySensor = iChoice; // set if a valid choice
	else sm->iMySensor = -1;
	
	if (m_radioCSV->isChecked()) sm->bMyOutputSAC = false;
	else if (m_radioSAC->isChecked()) sm->bMyOutputSAC = true;
	
	return (bool) (strError.length() == 0);
	
}
 */

void CDialogMakeQuake::CreateControls()
{
    m_groupName    = new QGroupBox(this);
    m_groupSpin    = new QGroupBox(this);
	m_groupButton = new QGroupBox(this);

	m_buttonStart = new QPushButton(tr("&Start"));
	m_buttonCancel = new QPushButton(tr("&Cancel"));
	
    connect(m_buttonStart, SIGNAL(clicked()), this, SLOT(onStart()));
    connect(m_buttonCancel, SIGNAL(clicked()), this, SLOT(close()));

    // control 1 - name
    m_labelName = new QLabel(tr("Name:"), this);
    m_textctrlName = new QLineEdit(this);
	m_textctrlName->setMaxLength(64);
	m_textctrlName->setText(m_strName);

	
    // control 2 - countdown
    //m_labelTime = new QLabel(tr(""), this);
    m_spinctrlCountdown = new QSpinBox(this);
	m_spinctrlCountdown->setPrefix(tr("Countdown is "));
	m_spinctrlCountdown->setSuffix(tr(" seconds long"));
    m_spinctrlCountdown->setMinimum(1);
    m_spinctrlCountdown->setMaximum(60);
    m_spinctrlCountdown->setSingleStep(1);
	m_spinctrlCountdown->setValue(m_iCountdown);
	
    // control 3 - shake time
    //m_labelTime = new QLabel(tr(""), this);
    m_spinctrlTime = new QSpinBox(this);
	m_spinctrlTime->setPrefix(tr("Quake record time is "));
	m_spinctrlTime->setSuffix(tr(" seconds"));
    m_spinctrlTime->setMinimum(1);
    m_spinctrlTime->setMaximum(60);
    m_spinctrlTime->setSingleStep(1);
	m_spinctrlTime->setValue(m_iTime);
	
	// now do the grid layout of the controls
    m_gridlayout = new QGridLayout(this);
    m_gridlayout->addWidget(m_labelName, 0, 0);
    m_gridlayout->addWidget(m_textctrlName, 0, 1);

	m_groupName->setLayout(m_gridlayout);

    //m_gridlayout->addWidget(m_labelTime, 1, 0);
	m_layoutSpin = new QVBoxLayout(this);
    m_layoutSpin->addWidget(m_spinctrlCountdown, 1, Qt::AlignTop);
    m_layoutSpin->addWidget(m_spinctrlTime, 1, Qt::AlignBottom);
	m_groupSpin->setLayout(m_layoutSpin);
	
	
	// layout buttons
	m_layoutButton = new QHBoxLayout(this);
    m_layoutButton->addWidget(m_buttonStart, 1, Qt::AlignLeft);
    m_layoutButton->addWidget(m_buttonCancel, 1, Qt::AlignRight);
	m_groupButton->setLayout(m_layoutButton);
	
	// do the overall layout for the dialog box

    m_layoutMain = new QVBoxLayout(this);
    m_layoutMain->addWidget(m_groupName);
    m_layoutMain->addWidget(m_groupSpin);
    m_layoutMain->addSpacing(12);
    m_layoutMain->addWidget(m_groupButton);
    m_layoutMain->addStretch(1);
	
    setLayout(m_layoutMain);
}
 
void CDialogMakeQuake::onStart()
{
	//QString strError;
	m_bStart = true;
	
	m_iTime = m_spinctrlTime->value();
	m_iCountdown = m_spinctrlCountdown->value();
	m_strName = m_textctrlName->text();
	
	//if (m_bStart) {
		close();
	//}
	//else { // must have an error
	//	QMessageBox::warning(this, tr("Error in Values"), QString(tr("Please correct the following error(s):<BR><BR>")) + strError, QMessageBox::Ok);
	//}
}
