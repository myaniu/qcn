#ifndef _DLG_SETTINGS_H_
#define _DLG_SETTINGS_H_

#include "qcnqt.h"
#include "csensor.h"

class CDialogSettings  : public QDialog
{	
private:
	QString m_strLatitude;
    QString m_strLongitude;
    QString m_strStation;
    QString m_strElevationMeter;
    QString m_strElevationFloor;
    QString m_strSensor;

    QTextEdit* m_textctrlLatitude; 
    QTextEdit* m_textctrlLongitude; 
    QTextEdit* m_textctrlStation; 
    QTextEdit* m_textctrlElevationMeter; 
    QTextEdit* m_textctrlElevationFloor; 
	
	CSensor* m_psms;  // just a dummy sensor obj to get string names
	
	QComboBox* m_comboSensor;
	
	QRadioButton* m_radioSAC;
	QRadioButton* m_radioCSV;
	
	QPushButton* m_buttonSave;
	QPushButton* m_buttonCancel;
	
	QVBoxLayout* m_layoutMain;
	QHBoxLayout* m_layoutButtons;

    void InitPointers();
    void CreateControls();
	
/*
    class wxTextValidatorLatLng : public wxTextValidator
    {
    private:
		QString* m_ptrString;
        int m_iControl;
        QStringList as;
    public:
        wxTextValidatorLatLng(wxString* ptrStr, int iControl);
    }; // *pvalidatorLatitude, *pvalidatorLongitude;
*/

private slots:
    //void OnLatitudeUpdated( wxCommandEvent& event );
    //void OnLongitudeUpdated( wxCommandEvent& event );
    //void OnStationUpdated( wxCommandEvent& event );
    //void OnClose( wxCloseEvent& evt );
	
public:
    CDialogSettings(QWidget* parent = NULL, Qt::WindowFlags f = 0);
    ~CDialogSettings();

	void SaveValues();
};

#endif // _DLG_SETTINGS_H_
