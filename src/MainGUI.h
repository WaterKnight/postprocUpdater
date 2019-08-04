#ifndef MAINGUI_H_
#define MAINGUI_H_

#include <winsock2.h>

#include <wx/wx.h>

#include "updateProc.h"

class MainGUI : public wxApp {
public:
	virtual bool OnInit();
};

class MainWindow : public wxFrame {
public:
	MainWindow(wxString title, wxPoint pos, wxSize size, string targetDir);

	void on_updateProc_enterPhase(wxCommandEvent &event);
	void on_updateProc_gotDownloadsCount(wxCommandEvent &event);
	void on_updateProc_nextFile(wxCommandEvent &event);
	void on_updateProc_finished(wxCommandEvent &event);

	DECLARE_EVENT_TABLE();

private:
	wxBoxSizer *m_sizer;
	wxGauge *m_gauge;
	wxStaticText *m_phaseLabel;
	wxStaticText *m_curFileLabel;
	UpdateProc *m_updateProc;
	wxButton *m_doneButton;

	void on_doneButton_clicked(wxCommandEvent &event);
};

#endif /* MAINGUI_H_ */
