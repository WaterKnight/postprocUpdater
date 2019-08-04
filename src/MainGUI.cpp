#include <cstdlib>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string>

#include "updateProc.h"

#include "MainGUI.h"

using namespace std;

BEGIN_EVENT_TABLE(MainWindow, wxFrame)
EVT_COMMAND(EVT_ID_UPDATE_PROC_ENTER_PHASE, wxEVT_COMMAND_TEXT_UPDATED, MainWindow::on_updateProc_enterPhase)
EVT_COMMAND(EVT_ID_UPDATE_PROC_GOT_DOWNLOADS_COUNT, wxEVT_COMMAND_TEXT_UPDATED, MainWindow::on_updateProc_gotDownloadsCount)
EVT_COMMAND(EVT_ID_UPDATE_PROC_NEXT_FILE, wxEVT_COMMAND_TEXT_UPDATED, MainWindow::on_updateProc_nextFile)
EVT_COMMAND(EVT_ID_UPDATE_PROC_FINISHED, wxEVT_COMMAND_TEXT_UPDATED, MainWindow::on_updateProc_finished)
END_EVENT_TABLE()

MainWindow::MainWindow(wxString title, wxPoint pos, wxSize size, string targetDir) : wxFrame(NULL, wxID_ANY, title, pos, size, wxCLIP_CHILDREN | wxCAPTION) {
	m_sizer = new wxBoxSizer(wxVERTICAL);

	m_phaseLabel = new wxStaticText(this, wxID_ANY, "initializing");

	m_gauge = new wxGauge(this, wxID_ANY, 0);

	m_curFileLabel = new wxStaticText(this, wxID_ANY, "");

	m_doneButton = new wxButton(this, wxID_ANY, "done");

	m_sizer->Add(m_phaseLabel, 1, wxEXPAND);
	m_sizer->Add(m_gauge, 2, wxEXPAND);
	m_sizer->Add(m_curFileLabel, 1, wxEXPAND);
	m_sizer->Add(m_doneButton, 1, wxEXPAND);

	SetSizer(m_sizer);
	Show(true);
	Centre();

	m_doneButton->Connect(wxID_ANY, wxID_ANY, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(MainWindow::on_doneButton_clicked), NULL, this);
	m_doneButton->Hide();

	m_updateProc = new UpdateProc(this, targetDir);

	m_updateProc->Run();
}

void MainWindow::on_updateProc_enterPhase(wxCommandEvent &event) {
	wxString s = event.GetString();

	m_phaseLabel->SetLabel(s);
}

void MainWindow::on_updateProc_gotDownloadsCount(wxCommandEvent &event) {
	int c = event.GetInt();

	m_gauge->SetRange(c);
}

void MainWindow::on_updateProc_nextFile(wxCommandEvent &event) {
	wxString filePath = event.GetString();
	int c = event.GetInt();

	m_gauge->SetValue(c);
	m_curFileLabel->SetLabel(filePath);
}

void MainWindow::on_doneButton_clicked(wxCommandEvent &event) {
	Close(true);
}

void MainWindow::on_updateProc_finished(wxCommandEvent &event) {
	int retCode = event.GetInt();
	wxString errMsg = event.GetString();

	if (retCode) {
		m_phaseLabel->SetLabel(errMsg);
	}

	m_curFileLabel->SetLabel("");

	m_curFileLabel->Hide();
	m_doneButton->Show();

	SetSizer(m_sizer);
}

IMPLEMENT_APP(MainGUI)

bool MainGUI::OnInit() {
	if (argc < 2) {
		fprintf(stderr, "no target path specified");
		fflush(stderr);

		return FALSE;
	}

	string targetDir = wxGetApp().argv.GetArguments().Item(1).ToStdString();

	while (targetDir.find("/") != string::npos) {
		targetDir.replace(targetDir.find("/"), 1, "\\");
	}

	while (targetDir.find("\\\\") != string::npos) {
		targetDir.replace(targetDir.find("\\\\"), 2, "\\");
	}

	if ((targetDir.rfind("\\") != targetDir.length() - 1)) {
		targetDir.append("\\");
	}

	MainWindow *window = new MainWindow("postproc updater", wxDefaultPosition, wxSize(600, 100), targetDir);

	return TRUE;
}
