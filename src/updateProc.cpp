#include "updateproc.h"

using namespace std;

#include <vector>

vector<string> split(const string &text, char sep) {
	vector<string> tokens;
	unsigned int start = 0;
	unsigned int end = 0;

	while ((end = text.find(sep, start)) != string::npos) {
		tokens.push_back(text.substr(start, end - start));
		start = end + 1;
	}

	tokens.push_back(text.substr(start));

	return tokens;
}

string getDir(string path) {
	size_t pos = path.rfind('/');

	if (pos != string::npos) {
		return path.substr(0, pos);
	}

	pos = path.rfind('\\');

	if (pos != string::npos) {
		return path.substr(0, pos);
	}

	return string();
}

void makeDir(string path) {
	if (!getDir(path).empty()) {
		makeDir(getDir(path));
	}

	CreateDirectoryA(path.c_str(), NULL);
	/*string makeDirCmd;

	makeDirCmd.append("mkdir 1>NUL 2>NUL \"");
	makeDirCmd.append(path);
	makeDirCmd.append("\"");

	system(makeDirCmd.c_str());*/
}

size_t write_string(void *ptr, size_t size, size_t nmemb, string *target) {
	target->append((char*)ptr, size * nmemb);

	return (size * nmemb);
}

size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    size_t written = fwrite(ptr, size, nmemb, stream);

    return written;
}

int count = 0;

UpdateProc::requestFile_return UpdateProc::requestFile(string host, string remotePath, string *target) {
	for (unsigned int i = 0; i < remotePath.size(); i++) {
		if (remotePath[i] == '\\') {
			remotePath[i] = '/';
		}
	}

	string url;

	url.append(host);
	url.append(remotePath);

	curl_easy_setopt(m_curl, CURLOPT_URL, url.c_str());
	curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, write_string);

	curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, target);
	//curl_easy_setopt(m_curl, CURLOPT_VERBOSE, 1L);
	curl_easy_setopt(m_curl, CURLOPT_FAILONERROR, true);

	CURLcode res = curl_easy_perform(m_curl);

	requestFile_return ret;

	ret.retCode = res;

	return ret;
}

UpdateProc::requestFile_return UpdateProc::requestFile(string host, string remotePath, string targetDir) {
	/*string url;

	url.append(host);
	url.append(remotePath);

	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);

	curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
	//curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
	curl_easy_setopt(curl, CURLOPT_FAILONERROR, true);

	CURLcode res = curl_easy_perform(curl);

	if (res != CURLE_OK) {
		cout << "could not download file " << remotePath << "(error " << res << ")" << '\n';

		requestFile_return ret;

		ret.data = 0;
		ret.retCode = res;

		return ret;
	}*/

	string data;

	requestFile_return ret = requestFile(host, remotePath, &data);

	if (ret.retCode != CURLE_OK) {
		return ret;
	}

	string localPath;

	localPath.append(targetDir);
	localPath.append(remotePath);

	string localDir = getDir(localPath);

	if (!localDir.empty()) {
		makeDir(localDir);
	}

	FILE *fp = fopen(localPath.c_str(), "wb+");

	fwrite(data.c_str(), 1, data.size(), fp);

	fclose(fp);

	return ret;
}

struct file {
	string path;
	string md5;
};

#include <map>

void parseListfile(vector<string> &lines, map<string,file> &container) {
	unsigned int pathCol = 0;
	unsigned int md5Col = 0;

	for (unsigned int i = 0; i < lines.size(); i++) {
		string line = lines.at(i);

		while (line.find('\r') != string::npos) {
			line.erase(line.find('\r'));
		}

		if (!line.empty()) {
			vector<string> vals = split(line, '\t');

			if (i == 0) {
				for (unsigned int j = 0; j < vals.size(); j++) {
					if (vals.at(j) == "path") {
						pathCol = j;
					} else if (vals.at(j) == "md5") {
						md5Col = j;
					}
				}
			} else {
				if (vals.size() > pathCol) {
					string path = vals.at(pathCol);

					if (path.size() > 0) {
						if (((path.at(pathCol) >= '0') and (path.at(pathCol) <= '9')) or ((path.at(pathCol) >= 'A') and (path.at(pathCol) <= 'z'))) {
							file f;

							f.path = path;
							if (vals.size() > md5Col) {
								f.md5 = vals.at(md5Col);
							}

							container.insert(pair<string,file>(path, f));
						}
					}
				}
			}
		}
	}
}

string exec(string cmd) {
    FILE* pipe = _popen(cmd.c_str(), "r");

	if (!pipe) return "ERROR";

	char buffer[128];

	string result = "";

	while (!feof(pipe)) {
        if (fgets(buffer, 128, pipe) != NULL)
            result += buffer;
    }

	_pclose(pipe);

	return result;
}

vector<string> getFiles(string dir) {
	string cmd = "dir /b /s /a-d \"";

	cmd.append(dir);
	cmd.append("\"");

	string s = exec(cmd);

	vector<string> lines = split(s, '\n');

	vector<string> ret;

	for (unsigned int i = 0; i < lines.size(); i++) {
		string line = lines.at(i);

		if ((line.size() > 0) and (line.at(0) >= 'A') and (line.at(0) <= 'z')) {
			ret.push_back(line);
		}
	}

	return ret;
}

void UpdateProc::makeListfile(string dir, vector<string> &lines) {
	lines.push_back("path\tmd5\t\n");

	vector<string> paths = getFiles(dir);

	for (unsigned int i = 0; i < paths.size(); i++) {
		string path = paths.at(i);

		string shortPath = path.substr(dir.size());

		string line;

		line.append(shortPath);
		line.append("\t");

		Crypto::md5_digest_return dig = m_crypto->md5_digest(path);

		if (dig.status) {
			cout << "could not md5_digest file " << path << " (" << dig.errorMsg << ")" << '\n';
		}

		line.append(dig.val, Crypto::MD5_LEN_HEX);

		lines.push_back(line);
	}
}

UpdateProc::UpdateProc(wxFrame *parent, string targetDir) {
	m_parent = parent;
	m_targetDir = targetDir;
}

void UpdateProc::enterPhase(string s) {
	wxCommandEvent event(wxEVT_COMMAND_TEXT_UPDATED, EVT_ID_UPDATE_PROC_ENTER_PHASE);

	event.SetString(s);

	m_parent->GetEventHandler()->AddPendingEvent(event);
}

ExitCode UpdateProc::finish(int retCode, string errMsg)
{
	if (retCode) {
		fprintf(stderr, errMsg.c_str());
	}

	wxCommandEvent event(wxEVT_COMMAND_TEXT_UPDATED, EVT_ID_UPDATE_PROC_FINISHED);

	event.SetInt(retCode);
	event.SetString(errMsg);

	m_parent->GetEventHandler()->AddPendingEvent(event);

	Delete((ExitCode*)retCode);

	return (ExitCode)retCode;
}

ExitCode UpdateProc::Entry() {
	string targetDir = m_targetDir;

	string logPath = targetDir;

	logPath.append("updateLog.txt");

	m_log = new ofstream(logPath.c_str());

	cout.rdbuf(m_log->rdbuf());

	cout << "postproc updater log\n\n";
cout << targetDir  << '\n';
	curl_global_init(CURL_GLOBAL_ALL);

	m_curl = curl_easy_init();
	m_crypto = new Crypto();

	if (!m_curl) {
		return finish(1, "failed to initialize curl");
	}

	string host = "http://moonlightflower.net/wc3/postproc/release/";

	//collect local files
	enterPhase("checking local files...");

	map<string,file> localFiles;

	{
		vector<string> lines;

		makeListfile(targetDir, lines);

		parseListfile(lines, localFiles);
	}

	//collect remote files
	enterPhase("checking remote files...");

	string remoteListfileS = "";

	requestFile_return ret = requestFile(host, "listfile.txt", &remoteListfileS);

	if (ret.retCode) {
		char c[100];

		snprintf(c, 100, "could not retrieve listfile (error %i)", ret.retCode);

		string s(c);

		return finish(1, s);
	}

	vector<string> lines = split(remoteListfileS, '\n');

	map<string,file> remoteFiles;

	parseListfile(lines, remoteFiles);

	//detect differences
	enterPhase("detect differences...");

	vector<string> pathsToTake;

	for (map<string,file>::iterator i = remoteFiles.begin(); i != remoteFiles.end(); i++) {
		string path = i->first;
		file f = i->second;

		bool take = false;

		if ((localFiles.count(path) == 0) or (localFiles.at(path).md5.compare(f.md5) != 0)) {
			take = true;
		}

		if (take) {
			pathsToTake.push_back(path);
		}
	}

	wxCommandEvent event(wxEVT_COMMAND_TEXT_UPDATED, EVT_ID_UPDATE_PROC_GOT_DOWNLOADS_COUNT);

	event.SetInt(pathsToTake.size());

	m_parent->GetEventHandler()->AddPendingEvent(event);

	enterPhase("downloading files...");

	if (pathsToTake.size() > 0) {
		for (unsigned int i = 0; i < pathsToTake.size(); i++) {
			char c[100];

			snprintf(c, 100, "downloading files... %.0f%% (%i/%i)", i * 100.0f / pathsToTake.size(), i + 1, pathsToTake.size());

			string s(c);

			enterPhase(s);

			string path = pathsToTake.at(i);

			wxCommandEvent event(wxEVT_COMMAND_TEXT_UPDATED, EVT_ID_UPDATE_PROC_NEXT_FILE);

			event.SetInt(i);
			event.SetString(path);

			m_parent->GetEventHandler()->AddPendingEvent(event);

			requestFile_return ret = requestFile(host, path, targetDir);

			if (ret.retCode) {
				string s = "could not download file ";

				s.append(path);

				s.append(" (error %s)");

				fprintf(stderr, s.c_str(), ret.retCode);
			}
		}

		enterPhase("update complete");
	} else {
		enterPhase("nothing to update");
	}

	curl_easy_cleanup(m_curl);
	curl_global_cleanup();

	m_log->close();

	return finish(0);
}

void UpdateProc::OnExit() {

}
