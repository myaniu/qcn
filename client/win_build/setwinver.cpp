// this deploys the programs to our QCN server (boinc apps as well as QCNLive)

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <direct.h>

#include "qcn_config_win.h"
#include "version.h"  // get the version info (QCN_VERSION_STRING)
#include "filesys.h"
#include "define.h"

#ifndef _DEBUG
//#include "boinc_zip.h"
#define ZIPCMD "\"c:\\program files\\7-Zip\\7z.exe\" a"
#define SFTPCMD "psftp -be -b"    // -be = don't stop batch on errors
#define SFTPBATCH "qcnsftp.txt"
int deploy_qcn(bool bQCNLive = false);  // send exe's to QCN server
#endif

void print_version_xml(const float& fVersion, const char* fname, bool bNCI)
{
	FILE* fileVersion = NULL;
	if (fopen_s(&fileVersion, fname, "w") || !fileVersion) {
	    fprintf(stdout, "Could not create version.xml file!\n");
		return; // error!
	}

	fprintf(fileVersion, 
		"<version>\n"
		"  <file>\n"
		"    <physical_name>qcn_%2.02f_%s%s.exe</physical_name>\n"
		"    <main_program/>\n"
		"  </file>\n"
		"  <file>\n"
		"    <physical_name>qcn_graphics_%2.02f_%s.exe</physical_name>\n"
		"    <logical_name>graphics_app</logical_name>\n"
		"  </file>\n"
		"</version>\n",
		fVersion, BOINC_WIN_SUFFIX, (bNCI ? "__nci" : "")
			,
			fVersion, BOINC_WIN_SUFFIX
	);

	fclose(fileVersion);
}

void printQCNFiles(FILE* fBatch, const char* strRemoteDir)
{

	float fVersion = atof(QCN_VERSION_STRING);
	char fname[32];

#ifdef _WIN64
	sprintf(fname, "version_64_%2.02f__nci.xml", fVersion);
#else
	sprintf(fname, "version_32_%2.02f__nci.xml", fVersion);
#endif

   print_version_xml(fVersion, fname, true);

     // new BOINC handled nci (__nci)
	// CMC note - we are assuming this directory was already created by Mac or Linux build!
	    //fprintf(fBatch, "mkdir %2.02f\n", fVersion);
        fprintf(fBatch, "cd %2.02f\n", fVersion);
        fprintf(fBatch, "mkdir %s__nci\n", BOINC_WIN_SUFFIX);
        fprintf(fBatch, "cd %s__nci\n", BOINC_WIN_SUFFIX);
        fprintf(fBatch, "put qcn_%2.02f_%s__nci.exe\n", fVersion, BOINC_WIN_SUFFIX);
        fprintf(fBatch, "put qcn_graphics_%2.02f_%s.exe\n", fVersion, BOINC_WIN_SUFFIX);
        fprintf(fBatch, "put init/hvt\n");
        fprintf(fBatch, "put init/cbt\n");
        fprintf(fBatch, "put init/earthday4096.jpg\n");
        fprintf(fBatch, "put init/earthnight4096.jpg\n");
        fprintf(fBatch, "put init/xyzaxes.jpg\n");
        fprintf(fBatch, "put init/xyzaxesbl.jpg\n");
        fprintf(fBatch, "put init/logo.jpg\n");
        fprintf(fBatch, "put msvcp100.dll\n");
        fprintf(fBatch, "put msvcr100.dll\n");
        fprintf(fBatch, "put init/logo.jpg\n");

#ifdef _WIN64
		fprintf(fBatch, "put init/phidget21x64.dll\n");
#else
        //fprintf(fBatch, "put init/MotionNodeAccelAPI.dll\n");
		fprintf(fBatch, "put init/phidget21.dll\n");
#endif
        fprintf(fBatch, "put init/%s_%s.exe\n", NTPDATE_EXEC_VERSION, BOINC_WIN_SUFFIX);
        fprintf(fBatch, "put %s version.xml\n", fname);
}

// simple program to rename Windows QCN programs to full BOINC format
int main(int argc, char** argv)
{
	// note qcn_client & qcn_graphics built as Release, Multithreaded (not DLL)
	// QCNLive has to be build with Release Multithreaded DLL

	// pass in 4 args, 1st by default is this execname,
	// 2nd is the program name without the path or suffix, 3rd is path to prog
	// 4th is output path

#ifndef _DEBUG  // never deploy a debug build!
	/*
	if (argc == 2 && !strcmp(argv[1], "deploy-boinc")) {
		// send to the qcn server!
		// this machine has to be setup to seamlessly (i.e. authorized_keys2 etc)
		// ssh/sftp to carlgt1@qcn-web, so the VPN to Stanford must be on
		return deploy_qcn(false);
	} 
	else
	*/
	if (argc == 2 && !strcmp(argv[1], "deploy")) {
		// send to the qcn server!
		// this machine has to be setup to seamlessly (i.e. authorized_keys2 etc)
		// ssh/sftp to carlgt1@qcn-web, so the VPN to Stanford must be on
#ifndef _WIN64
		deploy_qcn(true);
#endif
	return deploy_qcn(false);
	}
#endif

	if (argc != 4) {
		fprintf(stdout, "Usage: setwinver.exe PROGNAME_NO_EXE PROGDIR OUTDIR\n");
		return 1;
	}
	bool bDeploy = false; // deploy if it's graphics i.e. the last exe built for BOINC QCN apps
	char strIn[_MAX_PATH], strOut[2][_MAX_PATH];
	sprintf_s(strIn, _MAX_PATH, "%s\\%s.exe", argv[2], argv[1]);
    if (strstr(argv[1], "qcn_graphics")) {
		bDeploy = true;
	   sprintf_s(strOut[0], _MAX_PATH, "%s\\%s_%s_%s.exe", argv[3], argv[1], QCN_VERSION_STRING, BOINC_WIN_SUFFIX); 
	   sprintf_s(strOut[1], _MAX_PATH, "%s\\%s_%1.02f_%s.exe", argv[3], argv[1], atof(QCN_VERSION_STRING) - .01f, BOINC_WIN_SUFFIX); 
	} else {
	   sprintf_s(strOut[0], _MAX_PATH, "%s\\%s_%s_%s__nci.exe", argv[3], argv[1], QCN_VERSION_STRING, BOINC_WIN_SUFFIX); 
	   sprintf_s(strOut[1], _MAX_PATH, "%s\\%s_%1.02f_%s.exe", argv[3], argv[1], atof(QCN_VERSION_STRING) - .01f, BOINC_WIN_SUFFIX); 
        }

	if (!boinc_file_exists(strIn)) {
		fprintf(stdout, "Input file %s not found!\n", strIn, strOut[0]);
		return 1;
	}
	if (boinc_file_exists(strOut[0])) boinc_delete_file(strOut[0]);
	if (boinc_file_exists(strOut[1])) boinc_delete_file(strOut[1]);
	if (rename(strIn, strOut[0])) {
		fprintf(stdout, "Failed to rename %s to %s!\n", strIn, strOut[0]);
	}
	else {
		fprintf(stdout, "Successfully renamed %s to %s!\n", strIn, strOut[0]);
               // boinc_copy(strOut[0], strOut[1]);  // no need for the non-nci plan class
	}

	// if graphics then we need to deploy the qcn apps to the server
#ifndef _DEBUG
	#ifdef _WIN64
	   if (bDeploy) return deploy_qcn(false);
       else 
	#endif
#endif

	return 0;
}

#ifndef _DEBUG
int deploy_qcn(bool bQCNLive)
{   
	// send exe's to QCN server, just do system to putty etc
    // first create a file of commands similar to qcn/client/bin/deploy
    int iRetVal = 0;
	FILE* fBatch = NULL;
	char* strCmd = new char[1024];
	memset(strCmd, 0x00, 1024);

	_getcwd(strCmd, _MAX_PATH);

	// if not in "bin" move there
	if (!strstr(strCmd, "\\bin")) {
		_chdir("..\\bin");
	}

	// OK directory is bin
	// create the batch file
	boinc_delete_file(SFTPBATCH);
	if (fopen_s(&fBatch, SFTPBATCH, "w") || !fBatch) {
		delete [] strCmd;
	    fprintf(stdout, "Could not create sftp batch file!\n");
		return 1; // error!
	}
	
	// check for qcnlive
	if (bQCNLive) { // deploy separately as it requires a different built i.d. Multithreaded DLL
		memset(strCmd, 0x00, 1024);
	// can't use boinc_zip because it won't do the init subdir for qcnlive!
	// so have the zip & unzip execs in c:\\windows32
#ifdef _WIN64
		const char cstrQCNLive[] = {"qcnlive-win64.zip"};
#else
		const char cstrQCNLive[] = {"qcnlive-win.zip"};
#endif

	    boinc_delete_file(cstrQCNLive);

		sprintf_s(strCmd, 1024, "%s %s "
			"%s %s %s %s %s %s "
			"%s "
			"%s %s %s %s %s %s %s %s %s %s %s %s %s%s%c%s%s",
		 ZIPCMD, cstrQCNLive,
			"qcnlive.exe",
			"readme-win.txt",
			"init/qcnwin.ico",
		    "QtCore4.dll",
		   "QtGui4.dll",
		   "QtOpenGL4.dll",
#ifdef _WIN64
		   "init/phidget21x64.dll",
#else
		   "init/phidget21.dll",
#endif
		   "msvcp100.dll",
		   "msvcr100.dll",
		   "init/hvt",
		   "init/hvtb",
		   "init/cbt",
			"init/earthday4096.jpg",
			"init/qcnlogo.png",
			"init/splash.png",
			"init/xyzaxes.jpg",
			"init/xyzaxesbl.jpg",
			"init/logo.jpg",
			"init/earthnight4096.jpg",
			"init/", NTPDATE_EXEC_VERSION, '_', BOINC_WIN_SUFFIX, ".exe"
		);
		fprintf(stdout, "Executing %s\n", strCmd);

		iRetVal = system(strCmd);
		if (iRetVal) {
			delete [] strCmd;
			return iRetVal;
		}

		 fprintf(stdout, "Created %s archive\n", cstrQCNLive);

		fprintf(fBatch, "cd /var/www/boinc/sensor/download\n");
		fprintf(fBatch, "put %s\n", cstrQCNLive);

		fprintf(fBatch, "exit\n");
		fclose(fBatch);
	} // qcnlive

	else { // regular apps
	// qcn / boinc apps
        // "normal" site
		char strDir[64] = {"/var/www/boinc/sensor/apps/qcnsensor"};
		fprintf(fBatch, "cd %s\n", strDir);
        printQCNFiles(fBatch, strDir);

        // "continual" site
		strcpy(strDir, "/var/www/boinc/continual/apps/qcncontinual");
		fprintf(fBatch, "cd %s\n", strDir);
        printQCNFiles(fBatch, strDir);

		fprintf(fBatch, "exit\n");
		fclose(fBatch);
	}

	memset(strCmd, 0x00, 1024);

	sprintf_s(strCmd, 1024, "%s %s carlgt1@qcn-web", SFTPCMD, SFTPBATCH);
    fprintf(stdout, "Starting sftp session to qcn web server apps dir\n");
    fprintf(stdout, "%s\n", strCmd);
	fflush(stdout);
	iRetVal = system(strCmd);
	if (iRetVal) {
	    fprintf(stdout, "Error with sftp!\n");
	}

/*	if (!bQCNLive) { // clean up
       // get rid of exe files
	   strcpy(strCmd, "del /q *qcn*.exe");
	   iRetVal = system(strCmd);
	}
	*/
        
	delete [] strCmd;

	return iRetVal;
}
#endif // DEBUG build
