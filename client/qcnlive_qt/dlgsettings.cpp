#include "dlgsettings.h"
#ifdef _WIN32
#include "csensor_win_usb_jw.h"
#include "csensor_win_usb_jw24f14.h"
#else
#ifdef __APPLE_CC__
#include "csensor_mac_usb_jw.h"
#include "csensor_mac_usb_jw24f14.h"
#else
#include "csensor_linux_usb_jw.h"
#endif
#endif

CDialogSettings::CDialogSettings(QWidget* parent, Qt::WindowFlags f)  : QDialog(parent, f)
{
	setModal(true);  // make it an application level modal window
	setWindowModality(Qt::ApplicationModal);
	setWindowTitle(tr("Local Settings for QCNLive"));
	
    InitPointers();

    if (sm->dMyLatitude == NO_LAT) 
		m_strLatitude.clear();
    else
		m_strLatitude.sprintf("%.6g", sm->dMyLatitude);
	
    if (sm->dMyLongitude == NO_LNG) 
		m_strLongitude.clear();
    else
		m_strLongitude.sprintf("%.6g", sm->dMyLongitude);
	
    m_strStation = (char *) sm->strMyStation;
	
    m_strElevationMeter.sprintf("%.6g", sm->dMyElevationMeter);
    m_strElevationFloor.sprintf("%d", sm->iMyElevationFloor);
	
    CreateControls();
}


CDialogSettings::~CDialogSettings()
{
	// get rid of all our objects if they exist
    if (m_psms) delete m_psms;  

    if (m_textctrlLatitude) delete m_textctrlLatitude;
    if (m_textctrlLongitude) delete m_textctrlLongitude;
	if (m_textctrlStation) delete m_textctrlStation;
	if (m_spinctrlElevationFloor) delete m_spinctrlElevationFloor;
	if (m_textctrlElevationMeter) delete m_textctrlElevationMeter;

	if (m_comboSensor) delete m_comboSensor;
	if (m_radioSAC) delete m_radioSAC;
	if (m_radioCSV) delete m_radioCSV;
	
	if (m_buttonSave) delete m_buttonSave;
	if (m_buttonCancel) delete m_buttonCancel;

	if (m_layoutMain) delete m_layoutMain;
	if (m_layoutButton) delete m_layoutButton;

	if (m_groupMain) delete m_groupMain;
	if (m_groupButton) delete m_groupButton;

	if (m_labelLatitude) m_labelLatitude = NULL;
	if (m_labelLongitude) m_labelLongitude = NULL;
	if (m_labelStation) m_labelStation = NULL;
	if (m_labelElevationMeter) m_labelElevationMeter = NULL;
	if (m_labelElevationFloor) m_labelElevationFloor = NULL;
	if (m_labelSensor) m_labelSensor = NULL;	
	if (m_gridlayout) m_gridlayout = NULL;
}

void CDialogSettings::InitPointers()
{
    m_strLatitude.clear();
    m_strLongitude.clear();
    m_strStation.clear();
    m_strElevationMeter.clear();
    m_strElevationFloor.clear();

    // init ptrs to null to test for deletion later
    m_textctrlLatitude = NULL; 
    m_textctrlLongitude = NULL; 
    m_textctrlStation = NULL; 
    m_textctrlElevationMeter = NULL; 
    m_spinctrlElevationFloor = NULL; 
	
	m_psms = NULL;
	m_comboSensor = NULL;
	
	m_radioSAC = NULL;
	m_radioCSV = NULL;
	
	m_buttonSave = NULL;
	m_buttonCancel = NULL;
	m_layoutMain = NULL;
	m_layoutButton = NULL;
	m_groupMain = NULL;
	m_groupButton = NULL;

	m_labelLatitude = NULL;
	m_labelLongitude = NULL;
	m_labelStation = NULL;
	m_labelElevationMeter = NULL;
	m_labelElevationFloor = NULL;
	m_labelSensor = NULL;	
	m_gridlayout = NULL;
	
#ifdef _WIN32
	m_psms = new CSensorWinUSBJW;
#else
#ifdef __APPLE_CC__
	m_psms = new CSensorMacUSBJW;
#else
	m_psms = new CSensorLinuxUSBJW;
#endif
#endif
}

bool CDialogSettings::SaveValues()
{
    double dTest = atof((const char*) m_strLatitude.toAscii());
    if (dTest >= -90.0f && dTest <= 90.0f)
        sm->dMyLatitude = dTest;

    dTest = atof((const char*) m_strLongitude.toAscii());
    if (dTest >= -180.0f && dTest <= 180.0f)
        sm->dMyLongitude = dTest;

    memset((char*) sm->strMyStation, 0x00, SIZEOF_STATION_STRING);
    strlcpy((char*) sm->strMyStation, (const char*) m_strStation.toAscii(), SIZEOF_STATION_STRING);	

    sm->dMyElevationMeter = atof((const char*) m_strElevationMeter.toAscii());
    sm->iMyElevationFloor = atoi((const char*) m_strElevationFloor.toAscii());
	
	// for the sensor combo, save the value of the combo -1 + SENSOR_USB_
	sm->iMySensor = -1;
	QString strCombo = m_comboSensor->currentText();
	for (int i = MIN_SENSOR_USB; i <= MAX_SENSOR_USB; i++)   {// usb sensors are between the values MIN & MAX_SENSOR_USB given in define.h
		if (!strCombo.compare(m_psms->getTypeStr(i))) { // strings match
			sm->iMySensor = i;
			break;
		}
	}

	if (m_radioCSV->isChecked()) sm->bMyOutputSAC = false;
	else if (m_radioSAC->isChecked()) sm->bMyOutputSAC = true;
	
}

void CDialogSettings::CreateControls()
{
    m_groupMain    = new QGroupBox(this);
	m_groupButton = new QGroupBox(this);

	m_buttonSave = new QPushButton(tr("Save"));
	m_buttonCancel = new QPushButton(tr("Cancel"));
	
    connect(m_buttonSave, SIGNAL(clicked()), this, SLOT(onSave()));
    connect(m_buttonCancel, SIGNAL(clicked()), this, SLOT(close()));

    // control 1 - Latitude
    m_labelLatitude = new QLabel(tr("Latitude (-90 [SP] to 90 [NP]):"), this);
    m_textctrlLatitude = new QLineEdit(this);	
	m_textctrlLatitude->setText(m_strLatitude);

	
    // control 2 - Longitude
    m_labelLongitude = new QLabel(tr("Longitude (-180 [W] to 180 [E]):"), this);
    m_textctrlLongitude = new QLineEdit(this);
	m_textctrlLongitude->setText(m_strLongitude);
	
    // control 3 - Station Name
    m_labelStation = new QLabel(tr("Station Name (7 chars max):"), this);
    m_textctrlStation = new QLineEdit(this);
	m_textctrlStation->setMaxLength(7);
	m_textctrlStation->setText(m_strStation);
	
    // control 4 - Elevation (meters)
    m_labelElevationMeter = new QLabel(tr("Elevation (In Meters):"), this);
    m_textctrlElevationMeter = new QLineEdit(this);
	m_textctrlElevationMeter->setText(m_strElevationMeter);
							
    // control 5 - Elevation (floor #)
    m_labelElevationFloor = new QLabel(tr("Floor # (-1=Basement, 0=Ground etc):"), this);
    m_spinctrlElevationFloor = new QSpinBox(this);
    m_spinctrlElevationFloor->setMinimum(-200);
    m_spinctrlElevationFloor->setMaximum(200);
    m_spinctrlElevationFloor->setSingleStep(1);
	m_spinctrlElevationFloor->setValue(atoi((const char*) m_strElevationFloor.toAscii()));

	// control 6 & 7 -- radio buttons to pick CSV/text or SAC output
	m_radioCSV = new QRadioButton(tr("Record CSV/Text Files"), this);
	m_radioSAC = new QRadioButton(tr("Record SAC Files"), this);
	
	m_radioCSV->setChecked(!sm->bMyOutputSAC);
	m_radioSAC->setChecked(sm->bMyOutputSAC);
	
	// control 8 - combo box to force sensor selection (i.e. for demos/displays)
    m_labelSensor = new QLabel(tr("Force A Specific USB Sensor To Be Used:"), this);	
	m_comboSensor = new QComboBox(this);
	
	// create an array of strings of the USB sensor choices
	int iMyIndex = 0;
	m_comboSensor->addItem(tr("No Preference"), 0);
	for (int i = MIN_SENSOR_USB; m_psms && i <= MAX_SENSOR_USB; i++) {  // usb sensors are between the values MIN & MAX_SENSOR_USB given in define.h
		if (i == sm->iMySensor) { // this is the desired sensor, so save the value to set in the combo box below
			iMyIndex = i - MIN_SENSOR_USB + 1;
		}
		m_comboSensor->addItem(m_psms->getTypeStr(i), i);
	}
	
	// set default setting, will at least be 0 = No Preference
	m_comboSensor->setCurrentIndex(iMyIndex);
	
	// now do the grid layout of the controls
    m_gridlayout = new QGridLayout(this);
    m_gridlayout->addWidget(m_labelLatitude, 0, 0);
    m_gridlayout->addWidget(m_textctrlLatitude, 0, 1);
	
    m_gridlayout->addWidget(m_labelLongitude, 1, 0);
    m_gridlayout->addWidget(m_textctrlLongitude, 1, 1);
	
    m_gridlayout->addWidget(m_labelStation, 2, 0);
    m_gridlayout->addWidget(m_textctrlStation, 2, 1);
	
    m_gridlayout->addWidget(m_labelElevationMeter, 3, 0);
    m_gridlayout->addWidget(m_textctrlElevationMeter, 3, 1);

    m_gridlayout->addWidget(m_labelElevationFloor, 4, 0);
    m_gridlayout->addWidget(m_spinctrlElevationFloor, 4, 1);
	
    m_gridlayout->addWidget(m_radioCSV, 5, 0);
    m_gridlayout->addWidget(m_radioSAC, 5, 1);

    m_gridlayout->addWidget(m_labelSensor, 6, 0);
    m_gridlayout->addWidget(m_comboSensor, 6, 1);

	m_groupMain->setLayout(m_gridlayout);
	
	// layout buttons
	m_layoutButton = new QHBoxLayout(this);
    m_layoutButton->addWidget(m_buttonSave, 1, Qt::AlignLeft);
    m_layoutButton->addWidget(m_buttonCancel, 1, Qt::AlignRight);
	m_groupButton->setLayout(m_layoutButton);
	
	// do the overall layout for the dialog box

    m_layoutMain = new QVBoxLayout(this);
    m_layoutMain->addWidget(m_groupMain);
    m_layoutMain->addSpacing(12);
    m_layoutMain->addWidget(m_groupButton);
    m_layoutMain->addStretch(1);
	
    setLayout(m_layoutMain);
	
}
 
void CDialogSettings::onSave()
{
	QString strError;
	if (SaveValues(strError)) {
		close();
	}
	else {
		if (strError.length() > 0) {
		}		
	}

}
			
/*  CMC
void CDialogSettings::OnLatitudeUpdated( wxCommandEvent& WXUNUSED(event) )
{
    float fTest = atof(m_spinctrlLatitude->GetValue().toAscii());
	if (fTest < -90.0f || fTest > 90.0f) {
         wxMessageBox(
            _T("Error: Latitude must be between -90 (South Pole) and 90 (North Pole)"), 
            _T("Latitude Validation Error")
         );
	 }
}

void CDialogSettings::OnLongitudeUpdated( wxCommandEvent& WXUNUSED(event) )
{
    float fTest = atof(m_spinctrlLongitude->GetValue().toAscii());
	if (fTest < -180.0f || fTest > 180.0f) {
         wxMessageBox(
            _T("Error: Longitude must be between -180 (W) and 180 (E)\n(0 = Greenwich)"), 
            _T("Longitude Validation Error")
         );
	 }
}
 
 bool CDialogSettings::wxTextValidatorLatLng::Validate(wxWindow* parent)
 {
 double dTest = atof(((wxTextCtrl*) parent)->GetValue().toAscii());
 if (m_bIsLat && (dTest < -90.0f || dTest > 90.0f)) return false;
 else if (!m_bIsLat && (dTest < -180.0f || dTest > 180.0f)) return false;
 return true;
 }
 

void CDialogSettings::OnStationUpdated( wxCommandEvent& WXUNUSED(event) )
{
	int iLen = (int) m_spinctrlStation->GetValue().Len();
    if ( iLen > SIZEOF_STATION_STRING-1 ) {
	     char *strErr = new char[_MAX_PATH];
		 sprintf(strErr, "Error: Station should be a maximum of %d alphanumeric characters in length, not %d", 
             SIZEOF_STATION_STRING-1, iLen);
         wxMessageBox(
            strErr, 
            _T("Station Validation Error")
         );
		 delete [] strErr;
		 return;
	}
}
*/
