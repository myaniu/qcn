#ifndef _DLG_MAKEQUAKE_H_
#define _DLG_MAKEQUAKE_H_

#include "qcnqt.h"

class CDialogMakeQuake  : public QDialog
{	
private:
	Q_OBJECT
    void InitPointers();
    void CreateControls();

	//data
	QString m_strName;
	int m_iTime;
	int m_iCountdown;
	bool m_bStart; 

	// controls & layouts

    QLineEdit* m_textctrlName; 
    QSpinBox* m_spinctrlTime;
    QSpinBox* m_spinctrlCountdown;

	QPushButton* m_buttonStart;
	QPushButton* m_buttonCancel;
	
	QVBoxLayout* m_layoutMain;
	QVBoxLayout* m_layoutSpin;
	QHBoxLayout* m_layoutButton;
	
	QGroupBox* m_groupName;
	QGroupBox* m_groupSpin;
	QGroupBox* m_groupButton;
	
	// informative labels for the various controls
	QLabel* m_labelName;
	
	QGridLayout* m_gridlayout;	

	
private slots:
	void onStart();		
	
public:
	CDialogMakeQuake(const int iTime, const int iCountdown, QWidget* parent, Qt::WindowFlags f);
    ~CDialogMakeQuake();

	void getUserString(char** pstrName);
	const int getMakeQuakeTime() { return m_iTime; }
	const int getMakeQuakeCountdown() { return m_iCountdown; }
	bool start() { return m_bStart; }; 
};

#endif // _DLG_MAKEQUAKE_H_
