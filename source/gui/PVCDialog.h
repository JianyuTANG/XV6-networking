#ifndef PVCDIALOG_H
#define PVCDIALOG_H

#include "PVCObject.h"

PHdlg pvcCreateDialog(PHwnd caller, char * title, PHwnd parent, int x, int y, int width, int height, void * data);

bool pvcDlgProc(PHdlg hdlg, PMessage msg);

void pvcDlgExec(PHdlg hdlg, bool (*dlgProc)(PHdlg, PMessage));

void pvcDestroyDialog(PHdlg hdlg);

bool pvcMessageBoxProc(PHdlg hdlg, PMessage msg);

int pvcMessageBox(PHwnd caller, char * title, char * content);

bool pvcGetInputDialogProc(PHdlg hdlg, PMessage msg);

int pvcGetInputDialog(PHwnd caller, char * title, char * buf, int len);

#endif
