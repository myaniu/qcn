#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <direct.h>

#include "config.h.win"
#include "version.h"  // get the version info (QCN_VERSION_STRING)
#include "filesys.h"

#ifndef _DEBUG
//#include "boinc_zip.h"
#define ZIPCMD "\"c:\\program files\\7-Zip\\7z.exe\" a"
#define SFTPCMD "psftp -be -b"    // -be = don't stop batch on errors
#define SFTPBATCH "qcnsftp.txt"
int deploy_qcn();  // send exe's to QCN server
#endif

// simple program to rename Windows QCN programs to full BOINC format
int main(int argc, char** argv)
{
	// pass in 4 args, 1st by default is this execname,
	// 2nd is the program name without the path or suffix, 3rd is path to prog
	// 4th is output path

#ifndef _DEBUG  // never deploy a debug build!
	if (argc == 2 && !strcmp(argv[1], "deploy")) {
		// send to the qcn server!
		// this machine has to be setup to seamlessly (i.e. authorized_keys2 etc)
		// ssh/sftp to carlgt1@qcn-web, so the VPN to Stanford must be on
		return deploy_qcn();
	}
#endif

	if (argc != 4) {
		fprintf(stdout, "Usage: setwinver.exe PROGNAME_NO_EXE PROGDIR OUTDIR\n");
		return 1;
	}

	char strIn[_MAX_PATH], strOut[_MAX_PATH];
	sprintf_s(strIn, _MAX_PATH, "%s\\%s.exe", argv[2], argv[1]);
        if (strstr(argv[1], "qcn_graphics"))
	   sprintf_s(strOut, _MAX_PATH, "%s\\%s_%s_windows_intelx86.exe", argv[3], argv[1], QCN_VERSION_STRING); 
	else
           sprintf_s(strOut, _MAX_PATH, "%s\\%s_%s_windows_intelx86__nci.exe", argv[3], argv[1], QCN_VERSION_STRING); 

	if (!boinc_file_exists(strIn)) {
		fprintf(stdout, "Input file %s not found!\n", strIn, strOut);
		return 1;
	}
	if (boinc_file_exists(strOut)) boinc_delete_file(strOut);
	if (rename(strIn, strOut)) {
		fprintf(stdout, "Failed to rename %s to %s!\n", strIn, strOut);
	}
	else {
		fprintf(stdout, "Successfully renamed %s to %s!\n", strIn, strOut);
	}
	return 0;
}

#ifndef _DEBUG
int deploy_qcn()
{   
	// send exe's to QCN server, just do system to putty etc
    // first create a file of commands similar to qcn/client/bin/deploy

	char *strCmd = new char[_MAX_PATH];
	memset(strCmd, 0x00, _MAX_PATH);
	_getcwd(strCmd, _MAX_PATH);
	// if not in "bin" move there
	if (!strstr(strCmd, "\\bin")) {
		_chdir("..\\bin");
	}
	delete [] strCmd;

	// OK directory is bin

	// can't use boinc_zip because it won't do the init subdir for qcnlive!
	// so have the zip & unzip execs in c:\\windows32

    boinc_delete_file("qcnlive-win.zip");

	strCmd = new char[1024];
	memset(strCmd, 0x00, 1024);

		// "init/earthmask.rgb",   // multitexturing mask -- seems to crash for some people though
	sprintf_s(strCmd, 1024, "%s qcnlive-win.zip "
		"%s %s %s %s %s %s %s %s %s", ZIPCMD,
		"qcnlive.exe",
		"init/MotionNodeAccelAPI.dll",
		"init/qcnwin.ico",
		"init/Helvetica.txf",
		"init/earthday4096.jpg",
		"init/splash.png",
		"init/logo.jpg",
		"init/ntpdate_4.2.4p5_windows_intelx86.exe",
		"init/earthnight4096.jpg"
	);
	fprintf(stdout, "Executing %s\n", strCmd);

	int iRetVal = system(strCmd);
	delete [] strCmd;
	if (iRetVal) return iRetVal;

    fprintf(stdout, "Created qcnlive-win.zip archive\n");

	FILE* fBatch;
	boinc_delete_file(SFTPBATCH);
	if (fopen_s(&fBatch, SFTPBATCH, "w") || !fBatch) {
	    fprintf(stdout, "Could not create sftp batch file!\n");
		return 1; // error!
	}

	fprintf(fBatch, "cd /var/www/boinc/qcnalpha/download\n");
	fprintf(fBatch, "put qcnlive-win.zip\n");
	fprintf(fBatch, "cd /var/www/boinc/qcnalpha/apps/qcnalpha\n");
	fprintf(fBatch, "mkdir qcn_%s_%s\n", QCN_VERSION_STRING, "windows_intelx86__nci.exe");
	fprintf(fBatch, "cd qcn_%s_%s\n", QCN_VERSION_STRING, "windows_intelx86__nci.exe");
	fprintf(fBatch, "put qcn_%s_%s\n", QCN_VERSION_STRING, "windows_intelx86__nci.exe");
	fprintf(fBatch, "put graphics_app=qcn_graphics_%s_%s\n", QCN_VERSION_STRING, "windows_intelx86.exe");
	fprintf(fBatch, "put init/Helvetica.txf\n");
	fprintf(fBatch, "put init/earthday4096.jpg\n");
//	fprintf(fBatch, "put init/earthmask.rgb\n");   // mask for multitexturing - but seems to crash some people bad!
	fprintf(fBatch, "put init/earthnight4096.jpg\n");
	fprintf(fBatch, "put init/logo.jpg\n");
	fprintf(fBatch, "put init/ntpdate_4.2.4p5_windows_intelx86.exe\n");
        fprintf(fBatch, "put init/MotionNodeAccelAPI.dll\n");
	fprintf(fBatch, "exit\n");
	fclose(fBatch);

	strCmd = new char[1024];
	memset(strCmd, 0x00, 1024);

	sprintf_s(strCmd, 1024, "%s %s carlgt1@qcn-web", SFTPCMD, SFTPBATCH);
    fprintf(stdout, "Starting sftp session to qcn web server apps dir\n");
    fprintf(stdout, "%s\n", strCmd);
	fflush(stdout);
	iRetVal = system(strCmd);
	if (iRetVal) {
	    fprintf(stdout, "Error with sftp!\n");
	}
	delete [] strCmd;

	return iRetVal;
}
#endif // DEBUG build
