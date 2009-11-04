#ifndef _DLG_SETTINGS_H_
#define _DLG_SETTINGS_H_

#include "qcnwx.h"

enum eSettingsCtrlID {
     ID_TEXTCTRLLATITUDE = 1001,
     ID_TEXTCTRLLONGITUDE,
     ID_TEXTCTRLSTATION,
     ID_TEXTCTRLELEVATIONMETER,
     ID_TEXTCTRLELEVATIONFLOOR,
	 ID_COMBOSENSOR
};

class CDialogSettings  : public wxDialog
{
    DECLARE_DYNAMIC_CLASS(CDialogSettings)
    DECLARE_EVENT_TABLE()

    wxString m_strLatitude;
    wxString m_strLongitude;
    wxString m_strStation;
    wxString m_strElevationMeter;
    wxString m_strElevationFloor;
    wxString m_strSensor;

    wxTextCtrl* m_textctrlLatitude; 
    wxTextCtrl* m_textctrlLongitude; 
    wxTextCtrl* m_textctrlStation; 
    wxTextCtrl* m_textctrlElevationMeter; 
    wxTextCtrl* m_textctrlElevationFloor; 
	
	wxComboBox* m_comboSensor;

    class wxTextValidatorLatLng : public wxTextValidator
    {
    private:
        wxString* m_ptrString;
        int m_iControl;
        wxArrayString as;
    public:
        wxTextValidatorLatLng(wxString* ptrStr, int iControl);
        /*
        virtual bool TransferToWindow();
        virtual bool TransferFromWindow();
        bool Validate(wxWindow* parent);
        */
    }; // *pvalidatorLatitude, *pvalidatorLongitude;

public:
    CDialogSettings();
    CDialogSettings(wxWindow* parent, wxWindowID id);
    //~CDialogSettings();

    void InitPointers();
    bool Create(wxWindow* parent, wxWindowID id);

    /// Creates the controls and sizers
    void CreateControls();
	void SaveValues();

    void OnLatitudeUpdated( wxCommandEvent& event );
    void OnLongitudeUpdated( wxCommandEvent& event );
    void OnStationUpdated( wxCommandEvent& event );
    //void OnClose( wxCloseEvent& evt );

};

#endif // _DLG_SETTINGS_H_
