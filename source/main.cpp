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
#include <vitasdk.h>
#include <vitaGL.h>
#include <bzlib.h>
#include <stdio.h>
#include <string>
#include <taihen.h>
#include "unzip.h"
#include "promoter.h"
#include "dialogs.h"
#include "network.h"

int _newlib_heap_size_user = 200 * 1024 * 1024;
static char download_link[512];

void copy_file(const char *src, const char *dst) {
	FILE *fs = fopen(src, "r");
	FILE *fd = fopen(dst, "w");
	size_t fsize = fread(generic_mem_buffer, 1, MEM_BUFFER_SIZE, fs);
	fwrite(generic_mem_buffer, 1, fsize, fd);
	fclose(fs);
	fclose(fd);
}

void recursive_mkdir(char *dir) {
	char *p = dir;
	while (p) {
		char *p2 = strstr(p, "/");
		if (p2) {
			p2[0] = 0;
			sceIoMkdir(dir, 0777);
			p = p2 + 1;
			p2[0] = '/';
		} else break;
	}
}

static char fname[512], ext_fname[512], read_buffer[8192];

void extract_file(char *file, char *dir) {
	init_progressbar_dialog("Extracting SharkF00D"); // Hardcoded for now since it's the sole instance of this function
	FILE *f;
	unz_global_info global_info;
	unz_file_info file_info;
	unzFile zipfile = unzOpen(file);
	unzGetGlobalInfo(zipfile, &global_info);
	unzGoToFirstFile(zipfile);
	uint64_t total_extracted_bytes = 0;
	uint64_t curr_extracted_bytes = 0;
	uint64_t curr_file_bytes = 0;
	int num_files = global_info.number_entry;
	for (int zip_idx = 0; zip_idx < num_files; ++zip_idx) {
		unzGetCurrentFileInfo(zipfile, &file_info, fname, 512, NULL, 0, NULL, 0);
		total_extracted_bytes += file_info.uncompressed_size;
		if ((zip_idx + 1) < num_files) unzGoToNextFile(zipfile);
	}
	unzGoToFirstFile(zipfile);
	for (int zip_idx = 0; zip_idx < num_files; ++zip_idx) {
		unzGetCurrentFileInfo(zipfile, &file_info, fname, 512, NULL, 0, NULL, 0);
		sprintf(ext_fname, "%s/%s", dir, fname);
		const size_t filename_length = strlen(ext_fname);
		if (ext_fname[filename_length - 1] != '/') {
			curr_file_bytes = 0;
			unzOpenCurrentFile(zipfile);
			recursive_mkdir(ext_fname);
			FILE *f = fopen(ext_fname, "wb");
			while (curr_file_bytes < file_info.uncompressed_size) {
				int rbytes = unzReadCurrentFile(zipfile, read_buffer, 8192);
				if (rbytes > 0) {
					fwrite(read_buffer, 1, rbytes, f);
					curr_extracted_bytes += rbytes;
					curr_file_bytes += rbytes;
				}
				sceKernelPowerTick(SCE_KERNEL_POWER_TICK_DEFAULT);
				sceMsgDialogProgressBarSetValue(SCE_MSG_DIALOG_PROGRESSBAR_TARGET_BAR_DEFAULT, ((zip_idx + 1) / num_files) * 100);
				vglSwapBuffers(GL_TRUE);
			}
			fclose(f);
			unzCloseCurrentFile(zipfile);
		}
		if ((zip_idx + 1) < num_files) unzGoToNextFile(zipfile);
	}
	unzClose(zipfile);
	sceMsgDialogClose();
	int status = sceMsgDialogGetStatus();
	do {
		vglSwapBuffers(GL_TRUE);
		status = sceMsgDialogGetStatus();
	} while (status != SCE_COMMON_DIALOG_STATUS_FINISHED);
	sceMsgDialogTerm();
}

int main(int argc, char *argv[]) {
	bool use_ur0 = false;
	SceIoStat st1, st2;
	scePowerSetArmClockFrequency(444);
	scePowerSetBusClockFrequency(222);
	
	// Initializing sceNet
	generic_mem_buffer = (uint8_t*)malloc(MEM_BUFFER_SIZE);
	sceSysmoduleLoadModule(SCE_SYSMODULE_NET);
	int ret = sceNetShowNetstat();
	SceNetInitParam initparam;
	if (ret == SCE_NET_ERROR_ENOTINIT) {
		initparam.memory = malloc(141 * 1024);
		initparam.size = 141 * 1024;
		initparam.flags = 0;
		sceNetInit(&initparam);
	}
	
	// Initializing sceCommonDialog
	SceCommonDialogConfigParam cmnDlgCfgParam;
	sceCommonDialogConfigParamInit(&cmnDlgCfgParam);
	sceAppUtilSystemParamGetInt(SCE_SYSTEM_PARAM_ID_LANG, (int *)&cmnDlgCfgParam.language);
	sceAppUtilSystemParamGetInt(SCE_SYSTEM_PARAM_ID_ENTER_BUTTON, (int *)&cmnDlgCfgParam.enterButtonAssign);
	sceCommonDialogSetConfigParam(&cmnDlgCfgParam);
	
	// Checking for network connection
	sceNetCtlInit();
	sceNetCtlInetGetState(&ret);
	if (ret != SCE_NETCTL_STATE_CONNECTED)
		early_fatal_error("Error: You need an Internet connection to run this application.");
	sceNetCtlTerm();
	
	// Checking for libshacccg.suprx existence
	char user_plugin_str[96];
	strcpy(user_plugin_str, "*SHARKF00D\nux0:data/vitadb.suprx\n*NPXS10031\nux0:data/vitadb.suprx\n");
	FILE *fp = fopen("ux0:tai/config.txt", "r");
	if (!fp) {
		fp = fopen("ur0:tai/config.txt", "r");
		use_ur0 = true;
	}
	int cfg_size = fread(generic_mem_buffer, 1, MEM_BUFFER_SIZE, fp);
	fclose(fp);
	if (!strncmp(generic_mem_buffer, user_plugin_str, strlen(user_plugin_str))) {
		if (!(sceIoGetstat("ur0:/data/libshacccg.suprx", &st1) >= 0 || sceIoGetstat("ur0:/data/external/libshacccg.suprx", &st2) >= 0)) { // Step 2: Extract libshacccg.suprx
			sceIoRemove("ux0:/data/Runtime1.00.pkg");
			sceIoRemove("ux0:/data/Runtime2.01.pkg");
			download_file("https://www.rinnegatamante.eu/vitadb/get_hb_url.php?id=567", "Downloading SharkF00D");
			sceIoMkdir("ux0:data/sharkbr33d", 0777);
			extract_file(TEMP_DOWNLOAD_NAME, "ux0:data/sharkbr33d/");
			sceIoRemove(TEMP_DOWNLOAD_NAME);
			makeHeadBin("ux0:data/sharkbr33d");
			init_warning("Installing SharkF00D");
			scePromoterUtilInit();
			scePromoterUtilityPromotePkg("ux0:data/sharkbr33d", 0);
			int state = 0;
			do {
				vglSwapBuffers(GL_TRUE);
				int ret = scePromoterUtilityGetState(&state);
				if (ret < 0)
					break;
			} while (state);
			sceMsgDialogClose();
			int status = sceMsgDialogGetStatus();
			do {
				vglSwapBuffers(GL_TRUE);
				status = sceMsgDialogGetStatus();
			} while (status != SCE_COMMON_DIALOG_STATUS_FINISHED);
			sceMsgDialogTerm();
			scePromoterUtilTerm();
			sceAppMgrLaunchAppByName(0x60000, "SHARKF00D", "");
			sceKernelExitProcess(0);
		} else { // Step 3: Cleanup
			fp = fopen(use_ur0 ? "ur0:tai/config.txt" : "ux0:tai/config.txt", "w");
			fwrite(&generic_mem_buffer[strlen(user_plugin_str)], 1, cfg_size - strlen(user_plugin_str), fp);
			fclose(fp);
			sceIoRemove("ux0:data/vitadb.skprx");
			sceIoRemove("ux0:data/vitadb.suprx");
			scePromoterUtilInit();
			scePromoterUtilityDeletePkg("SHARKF00D");
			int state = 0;
			do {
				int ret = scePromoterUtilityGetState(&state);
				if (ret < 0)
					break;
			} while (state);
			scePromoterUtilTerm();
			sceKernelExitProcess(0);
		}
	} else { // Step 1: Download PSM Runtime and install it
		fp = fopen(use_ur0 ? "ur0:tai/config.txt" : "ux0:tai/config.txt", "w");
		fwrite(user_plugin_str, 1, strlen(user_plugin_str), fp);
		fwrite(generic_mem_buffer, 1, cfg_size, fp);
		fclose(fp);
		download_file("https://archive.org/download/psm-runtime/IP9100-PCSI00011_00-PSMRUNTIME000000.pkg", "Downloading PSM Runtime v.1.00");
		sceIoRename(TEMP_DOWNLOAD_NAME, "ux0:/data/Runtime1.00.pkg");
		download_file("https://archive.org/download/psm-runtime/IP9100-PCSI00011_00-PSMRUNTIME000000-A0201-V0100-e4708b1c1c71116c29632c23df590f68edbfc341-PE.pkg", "Downloading PSM Runtime v.2.01");
		sceIoRename(TEMP_DOWNLOAD_NAME, "ux0:/data/Runtime2.01.pkg");
		copy_file("app0:vitadb.skprx", "ux0:data/vitadb.skprx");
		copy_file("app0:vitadb.suprx", "ux0:data/vitadb.suprx");
		taiLoadStartKernelModule("ux0:data/vitadb.skprx", 0, NULL, 0);
		sceAppMgrLaunchAppByName(0x60000, "NPXS10031", "[BATCH]host0:/package/Runtime1.00.pkg\nhost0:/package/Runtime2.01.pkg");
		sceKernelExitProcess(0);
	}
}
