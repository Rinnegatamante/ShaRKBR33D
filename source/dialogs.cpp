/*
 * This file is part of ShaRKBR33D
 * Copyright 2022 Rinnegatamante
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation, version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
 
#include <iostream>
#include <string>
#include <locale>
#include <codecvt>
#include <stdio.h>
#include <string>
#include <vitasdk.h>
#include <vitaGL.h>

void early_fatal_error(const char *msg) {
	vglInit(0);
	SceMsgDialogUserMessageParam msg_param;
	sceClibMemset(&msg_param, 0, sizeof(SceMsgDialogUserMessageParam));
	msg_param.buttonType = SCE_MSG_DIALOG_BUTTON_TYPE_OK;
	msg_param.msg = (const SceChar8*)msg;
	SceMsgDialogParam param;
	sceMsgDialogParamInit(&param);
	param.mode = SCE_MSG_DIALOG_MODE_USER_MSG;
	param.userMsgParam = &msg_param;
	sceMsgDialogInit(&param);
	while (sceMsgDialogGetStatus() != SCE_COMMON_DIALOG_STATUS_FINISHED) {
		vglSwapBuffers(GL_TRUE);
	}
	sceKernelExitProcess(0);
}

int init_warning(const char *fmt, ...) {
	va_list list;
	char msg[1024];

	va_start(list, fmt);
	vsnprintf(msg, sizeof(msg), fmt, list);
	va_end(list);
  
	SceMsgDialogUserMessageParam msg_param;
	sceClibMemset(&msg_param, 0, sizeof(msg_param));
	msg_param.buttonType = SCE_MSG_DIALOG_BUTTON_TYPE_NONE;
	msg_param.msg = (SceChar8 *)msg;

	SceMsgDialogParam param;
	sceMsgDialogParamInit(&param);
	_sceCommonDialogSetMagicNumber(&param.commonParam);
	param.mode = SCE_MSG_DIALOG_MODE_USER_MSG;
	param.userMsgParam = &msg_param;

	return sceMsgDialogInit(&param);
}

int init_progressbar_dialog(const char *fmt, ...) {
	vglInit(0);
	va_list list;
	char msg[1024];

	va_start(list, fmt);
	vsnprintf(msg, sizeof(msg), fmt, list);
	va_end(list);

	SceMsgDialogProgressBarParam msg_param;
	sceClibMemset(&msg_param, 0, sizeof(msg_param));
	msg_param.barType = SCE_MSG_DIALOG_PROGRESSBAR_TYPE_PERCENTAGE;
	msg_param.msg = (const SceChar8*)msg;
	
	SceMsgDialogParam param;
	sceMsgDialogParamInit(&param);
	param.mode = SCE_MSG_DIALOG_MODE_PROGRESS_BAR;
	param.progBarParam = &msg_param;
	
	return sceMsgDialogInit(&param);
}
