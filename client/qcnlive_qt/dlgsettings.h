#ifndef _DLG_SETTINGS_H_
#define _DLG_SETTINGS_H_

#include "qcnqt.h"
#include "csensor.h"

class CDialogSettings  : public QDialog
{	
private:
	Q_OBJECT
    void InitPointers();
    void CreateControls();

	//data
	QString m_strLatitude;
    QString m_strLongitude;
    QString m_strStation;
    QString m_strElevationMeter;
    QString m_strElevationFloor;
    QString m_strSensor;

    QLineEdit* m_textctrlStation; 

    QLineEdit* m_textctrlLatitude; 
    QLineEdit* m_textctrlLongitude; 
    QLineEdit* m_textctrlElevationMeter; 
    QSpinBox* m_spinctrlElevationFloor; 
			
	CSensor* m_psms;  // just a dummy sensor obj to get string names
	
	QComboBox* m_comboSensor;
	
	QRadioButton* m_radioSAC;
	QRadioButton* m_radioCSV;
	
	QPushButton* m_buttonSave;
	QPushButton* m_buttonCancel;
	
	QVBoxLayout* m_layoutMain;
	QHBoxLayout* m_layoutButton;
	
	QGroupBox* m_groupMain;
	QGroupBox* m_groupButton;
	
	// informative labels for the various controls
	QLabel* m_labelLatitude;
	QLabel* m_labelLongitude;
	QLabel* m_labelStation;
	QLabel* m_labelElevationMeter;
	QLabel* m_labelElevationFloor;
	QLabel* m_labelSensor;	
	
	QGridLayout* m_gridlayout;

	
private slots:
	void onSave();		
	
public:
    CDialogSettings(QWidget* parent = NULL, Qt::WindowFlags f = 0);
    ~CDialogSettings();

	void SaveValues();
};

#endif // _DLG_SETTINGS_H_
