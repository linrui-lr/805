/************************************************************************
* 版权所有 (C)2010, 深圳市中兴通讯股份有限公司。
*
* 文件名称： zte_web_mgmt.c
* 文件标识：
* 内容摘要：
* 其它说明：
* 当前版本： V0.1
* 作    者： zyt
* 完成日期： 2010-11-06
*
* 修改记录1：
* 修改内容：初始版本


*  	EC单号:EC616000235556  开发故障
*   故障主题:开机更新失败后立即重启，重启后仍显示更新失败问题 / 增加webui fota处理log
*   修改时间:20140715
************************************************************************/
#include <sys/time.h>
#include <sys/types.h>
#include <string.h>
#include <pthread.h>
#include <dirent.h>
#include <stdlib.h>

#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "zte_web_interface.h"
#include "zte_web_get_fw_para.h"
#include "zte_web_mgmt_wifi.h"
#include "fota_common.h"
#include "nv_api.h"
/****************************************************************************
* 全局变量定义区
****************************************************************************/
/* password check result*/
typedef enum _psw_check_result_type_t {
	PSW_EMPTY = 0,
	PSW_OK = 1,
	PSW_TOO_LONG = 2,
	PSW_TIME_OUT = 3,
	PSW_OTHER = 4
} psw_check_result_type_t;

typedef enum {
	PIN_MANAGE_DISABLE = 0,
	PIN_MANAGE_ENABLE,
	PIN_MANAGE_MODIFY
} emPIN_MANAGE_ACTION;

typedef struct UNLOCK_PARA {
	char    unlock_code[20];
} UNLOCK_PARA_ST;

//quick setting
typedef void (*funcWPType)(webs_t);

pthread_mutex_t g_login_timemark_mutex = PTHREAD_MUTEX_INITIALIZER;
#define TIME_SEM_KEY_1 0x0A2B
#define TIME_SEM_KEY_2  0x3A4B

/****************************************************************************
* static函数声明区
****************************************************************************/
void deal_quick_setup_apn_ex(webs_t wp);
void deal_quick_setup_wifi_basic(webs_t wp);
void deal_quick_setup_wifi_security(webs_t wp);
int zte_mgmt_handle_account(webs_t wp);
void zte_setLastLoginTime();
int zte_checkLoginTime();
void zte_reduct_login_times();
psw_check_result_type_t zte_password_check(webs_t wp, char* psw);
static void wait_verify(char *wait_name, char *wait_value);
static void zte_mgmt_enable_pin(webs_t wp, char_t *old_pin);
static void zte_mgmt_modify_pin(webs_t wp, char_t *old_pin, char_t *new_pin);
void deal_quick_setup_wifi_basic(webs_t wp);
void deal_quick_setup_wifi_security(webs_t wp);
static void deal_quick_setup_wps(webs_t wp);
static char *split_str_by_sep(char *src, char *sep, char *dst, int len);
static void set_apn_to_cfg(APN_PROFILE *apn_profile, IPV6_APN_PROFILE *ipv6_apn_profile);

/* 快速设置 */
const funcWPType G_ZQUICK_SET[MAX_QUICK_SET_NUM] = {
	deal_quick_setup_apn_ex,
	deal_quick_setup_wifi_basic,
	deal_quick_setup_wifi_security,
	zte_mgmt_handle_account,
	quick_dhcp_set
};

/******************************************************
* Function: void zte_mgmt_login(webs_t wp)
* Description:  deal with the login goform
* Input:  HTTP page info
* Output:
* Return:  none
* Others:
* Modify Date    Version   Author         Modification
* 2010/11/26     V1.0      zyt            create
*******************************************************/
void zte_mgmt_login(webs_t wp)
{
	slog(MISC_PRINT, SLOG_NORMAL, T("UFIx User login!\n"));
	char_t *psw = NULL;
	char_t *user = NULL;
	char_t  *save_flag = NULL;
	char_t *ip_address = NULL;

	int  user_name_len = 0;
	char_t *pUser = NULL;
	char user_name[CONFIG_DEFAULT_LENGTH] = {0};
	int zte_password_len = 0;
	char_t *zte_password = NULL;
	char zte_psw_admin[CONFIG_DEFAULT_LENGTH] = {0};
	psw_check_result_type_t psw_cheak_result = PSW_EMPTY;
	char  buf[CONFIG_DEFAULT_LENGTH] = {0};

	slog(MISC_PRINT, SLOG_DEBUG,"[login] zte_mgmt_login  enter====\n");
	if (NULL == wp) {
		slog(MISC_PRINT, SLOG_ERR,"[login] zte_mgmt_login: wp is null.");/*lint !e26*/
		zte_write_result_to_web(wp, LOGIN_FAIL);
		return;
	}

	psw = websGetVar(wp, T("password"), T(""));
	user = websGetVar(wp, T("username"), NULL);

	if ('\0' == (*psw)) {
		slog(MISC_PRINT, SLOG_ERR,"[login] zte_mgmt_login: psw is empty.");/*lint !e26*/
		zte_write_result_to_web(wp, LOGIN_FAIL);
		return;
	}

	if (user != NULL) {
		slog(MISC_PRINT, SLOG_DEBUG," zte_mgmt_login  user = %s\n", user);
		pUser = (char *)zte_base64_decode((const unsigned char *)user, strlen(user), (unsigned int*)&user_name_len);
		if (NULL == pUser) {
			zte_write_result_to_web(wp, LOGIN_FAIL);
			return;
		}

		strncpy(user_name, pUser, user_name_len);
		free(pUser);

		slog(MISC_PRINT, SLOG_DEBUG," zte_mgmt_login  user_name = %s\n", user_name);
		cfg_get_item("admin_user", buf, sizeof(buf));
		if (0 != strcmp(user_name, buf)) {
			slog(MISC_PRINT, SLOG_ERR," zte_mgmt_login user_name fail \n");
			zte_write_result_to_web(wp, LOGIN_USER_NAME_NOT_EXSIT);
			return;
		}
		slog(MISC_PRINT, SLOG_DEBUG," zte_mgmt_login uername correct \n");
	}
	slog(MISC_PRINT, SLOG_DEBUG,"[login] login1 -> zte_password_encode:%s.\n", psw); /*lint !e26*/
	zte_password = (char *)zte_base64_decode((const unsigned char *)psw, strlen(psw), (unsigned int*)&zte_password_len);
	slog(MISC_PRINT, SLOG_DEBUG,"[login] login2 -> zte_password:%s.\n", zte_password); /*lint !e26*/

	if (NULL == zte_password) {
		slog(MISC_PRINT, SLOG_ERR,"[login] zte_mgmt_login: psw is empty.\n");/*lint !e26*/
		zte_write_result_to_web(wp, LOGIN_FAIL);
		return;
	}
	//zte_password will long than zte_password_len, then strncpy zte_password_len data to zte_psw_admin
	strncpy(zte_psw_admin, zte_password, zte_password_len);
	free(zte_password);
	slog(MISC_PRINT, SLOG_DEBUG,"[login] login3 -> zte_psw_admin:s%.\n", zte_psw_admin); /*lint !e26*/
	psw_cheak_result = zte_password_check(wp, zte_psw_admin);

	if (psw_cheak_result != PSW_OK) {
		slog(MISC_PRINT, SLOG_ERR,"[login] zte_mgmt_login psw_cheak_result != PSW_OK\n");
		zte_write_result_to_web(wp, LOGIN_FAIL);
		return;
	}

	//get request ip addr
	ip_address = websGetRequestIpaddr(wp);
	if (NULL == ip_address) {
		slog(MISC_PRINT, SLOG_ERR,"[login] zte_mgmt_login: ip_address is null.\n");/*lint !e26*/
		zte_write_result_to_web(wp, LOGIN_FAIL);
		return;
	}

	memset(&buf, 0, sizeof(buf));
	cfg_get_item("admin_Password", buf, sizeof(buf));
	if (0 == strcmp(zte_psw_admin, buf)) {
		save_flag = websGetVar(wp, T("save_login"), T(""));
		if (('\0' != (*save_flag)) && (IFSTREQUAL("1", save_flag))) {
			cfg_set("psw_save", zte_psw_admin);
		}
	} else {
		zte_reduct_login_times();
		slog(MISC_PRINT, SLOG_ERR,"[login] zte_mgmt_login: error pass.");/*lint !e26*/
		zte_write_result_to_web(wp, LOGIN_BAD_PASSWORD);
		return;
	}

	if (zte_mgmt_login_timemark_set()) {
		(void)zte_web_write(NV_USER_IP_ADDR, ip_address);
		(void)zte_web_write(NV_LOGINFO, "ok");
		(void)zte_web_write("save_login", save_flag);
		(void)zte_web_write("psw_fail_num_str", LOGIN_FAIL_TIMES);
		zte_write_result_to_web(wp, LOGIN_SUCCESS);
		return;
	} else {
		zte_write_result_to_web(wp, LOGIN_FAIL);
		return;
	}
}
/******************************************************
* Function: int zte_mgmt_login_timemark_set()
* Description:  save the setting operate time
* Input:
* Output:
* Return:  none
* Others:
* Modify Date    Version   Author         Modification
*******************************************************/
int zte_mgmt_login_timemark_set()
{
	char_t login_timemark[NV_ITEM_STRING_LEN_64] = {0};
	long timemark = 0;
	int sem_id = -1;
	sem_id = get_sem(TIME_SEM_KEY_2);
	if (sem_id != -1) {
		sem_p(sem_id);
	}

	timemark = time(0);
	sprintf(login_timemark, "%ld", timemark);
	zte_web_write(NV_USER_LOGIN_TIMEMARK, login_timemark);
	if (sem_id != -1) {
		sem_v(sem_id);
	}
	return TRUE;
}
void zte_mgmt_login_timeout_check()
{
	char_t user_login_timemark[NV_ITEM_STRING_LEN_64] = {0};
	char_t login_info[NV_ITEM_STRING_LEN_20] = {0};

	long timemark_check = 0;
	int sem_id = -1;

	zte_web_read(NV_LOGINFO, login_info);

	if (0 == strcmp(login_info, "ok")) {
		sem_id = get_sem(TIME_SEM_KEY_2);
		if (sem_id != -1) {
			sem_p(sem_id);
		}
		zte_web_read(NV_USER_LOGIN_TIMEMARK, user_login_timemark);
		timemark_check = time(0) - atol(user_login_timemark);

		if (sem_id != -1) {
			sem_v(sem_id);
		}
		if (timemark_check > LOGIN_TIMEOUT) {
			slog(MISC_PRINT, SLOG_DEBUG, "zte_mgmt_login_timemark_check: the login is timeout .");
			(void)zte_web_write(NV_USER_IP_ADDR, "");
			(void)zte_web_write(NV_LOGINFO, "timeout");
			(void)zte_web_write(NV_USER_LOGIN_TIMEMARK, "0");
		}
	}

}
/******************************************************
* Function: void zte_mgmt_logout(webs_t wp)
* Description:  deal with the logout goform
* Input:  HTTP page info
* Output:
* Return:  none
* Others:
* Modify Date    Version   Author         Modification
* 2010/11/26     V1.0      zyt            create
*******************************************************/
void zte_mgmt_logout(webs_t wp)
{
	slog(MISC_PRINT, SLOG_NORMAL, T("UFIx User logout!\n"));

	if (NULL == wp) {
		zte_write_result_to_web(wp, FAILURE);
		return;
	}

	cfg_set(NV_USER_IP_ADDR, "");
	cfg_set(NV_LOGINFO, "");
	zte_write_result_to_web(wp, SUCCESS);
}

/******************************************************
* Function: void zte_mgmt_set_language(webs_t wp)
* Description:  deal with the set language goform
* Input:  HTTP page info
* Output:
* Return:  none
* Others:
* Modify Date    Version   Author         Modification
* 2010/11/26     V1.0      zyt              create
*2012/09/14                  liuyingnan     modification
*******************************************************/
void zte_mgmt_set_language(webs_t wp)
{
	char_t* language = websGetVar(wp, T("Language"), T("en"));

	slog(MISC_PRINT, SLOG_NORMAL, T("UFIx User set language!\n"));


	if ('\0' == (*language)) {
		slog(MISC_PRINT, SLOG_ERR, "zte_mgmt_set_language: web para:[language] is empty string.\n"); /*lint !e26*/
		zte_write_result_to_web(wp, FAILURE);
		return;
	}

	cfg_set(NV_LANGUAGE, language);

	cfg_save();


	zte_write_result_to_web(wp, SUCCESS);
	return;


}


/******************************************************
* Function: void zte_mgmt_set_devicemode(webs_t wp)
* Description:  user/develop mode switch
* Input:  HTTP page info(debug_enable = 1 develop mode)
* Output:
* Return:  none
* Others:
* Modify Date    Version   Author         Modification
* Feb 27, 2014    V1.0      jhy              create
*******************************************************/
void zte_mgmt_set_devicemode(webs_t wp)
{
	char_t* debug_enable = websGetVar(wp, T("debug_enable"), T(""));

	int ret = 0;
	slog(MISC_PRINT, SLOG_NORMAL,"[goahead]debug_enable->%s\n", debug_enable);
	if ('\0' == (*debug_enable)) {
		slog(MISC_PRINT, SLOG_ERR, "zte_mgmt_set_devicemode: web para:[debug_enable] is empty string.\n");
		zte_write_result_to_web(wp, "set_devicemode fail:parameter debug_enable is empty!");
		return;
	}

	if (atoi(debug_enable) < 0 || atoi(debug_enable) > 3) {
		slog(MISC_PRINT, SLOG_ERR, "zte_mgmt_set_devicemode: web para:[debug_enable] is illegal string.\n"); 
		zte_write_result_to_web(wp, "set_devicemode fail:parameter debug_enable is illegal!");
		return;
	}
    char ss[20] = {0};
	switch (atoi(debug_enable)) {
	case 0:
		slog(MISC_PRINT, SLOG_NORMAL,"[goahead]debug_enable->user\n");
		nv_set_item(NV_RO, "usb_modetype", "user", 1);
		break;
	case 1:
		slog(MISC_PRINT, SLOG_NORMAL,"[goahead]debug_enable->debug\n");
		nv_set_item(NV_RO, "usb_modetype", "debug", 1);
		break;
	case 2:
		slog(MISC_PRINT, SLOG_NORMAL,"[goahead]debug_enable->factory\n");
		nv_set_item(NV_RO, "usb_modetype", "factory", 1);
		break;
	case 3:
		slog(MISC_PRINT, SLOG_NORMAL,"[goahead]debug_enable->amt\n");
		nv_set_item(NV_RO, "usb_modetype", "amt", 1);
		break;
	default:
		slog(MISC_PRINT, SLOG_NORMAL,"[goahead]error!\n");
		zte_write_result_to_web(wp, "set_devicemode fail:parameter debug_enable is illegal!");
		return;;
	}
	nv_commit(NV_RO);////default_parameter_ro中的NV保存必须用nv_commit不能用cfg_save
	zte_write_result_to_web(wp, "set_devicemode successfully!");

}

/******************************************************
* Function: void zte_mgmt_restore(webs_t wp)
* Description:  deal with the restore goform
* Input:  HTTP page info
* Output:
* Return:  none
* Others:
* Modify Date    Version   Author         Modification
* 2010/11/26     V1.0      zyt            create
*******************************************************/
void zte_mgmt_restore(webs_t wp)
{
	slog(MISC_PRINT, SLOG_NORMAL,"webui reset send message to blc\n");
	ipc_send_message(MODULE_ID_WEB_CGI, MODULE_ID_MAIN_CTRL, MSG_CMD_RESET_REQUEST, 0, NULL, 0);
	doSystem("sleep 2");//WH:delete?
	zte_write_result_to_web(wp, SUCCESS);
}


/******************************************************
* Function: void zte_mgmt_poweroff(webs_t wp)
* Description:  deal with the poweroff goform
* Input:  HTTP page info
* Output:
* Return:  none
* Others:
* Modify Date    Version   Author         Modification
* 2015/5/26      V1.0      lsl            create
*******************************************************/
void zte_mgmt_poweroff(webs_t wp)
{
	zte_write_result_to_web(wp, SUCCESS);
	slog(MISC_PRINT, SLOG_NORMAL,"webui poweroff send message to blc\n");
	ipc_send_message(MODULE_ID_WEB_CGI, MODULE_ID_MAIN_CTRL, MSG_CMD_POWEROFF_REQUEST, 0, NULL, 0);
}

/******************************************************
* Function: void zte_mgmt_control_power_on_speed(webs_t wp)
* Description:  deal with the power_on_speed goform
* Input:  HTTP page info
* Output:
* Return:  none
* Others:
* Modify Date    Version   Author         Modification
* 2015/5/26      V1.0      lsl            create
*******************************************************/
void zte_mgmt_control_power_on_speed(webs_t wp)
{
	char_t* mgmt_quicken_power_on = websGetVar(wp, T("mgmt_quicken_power_on"), T(""));

	if ('\0' == (*mgmt_quicken_power_on)) {
		zte_write_result_to_web(wp, FAILURE);
		return;
	} else if (strcmp(mgmt_quicken_power_on, "0") != 0 && strcmp(mgmt_quicken_power_on, "1") != 0) {
		zte_write_result_to_web(wp, FAILURE);
		return;
	}

	cfg_set("mgmt_quicken_power_on", mgmt_quicken_power_on);

	zte_write_result_to_web(wp, SUCCESS);
}

/******************************************************
* Function: void zte_goform_mgmt_reboot_process(webs_t wp)
* Description:  deal with the restore goform
* Input:  HTTP page info
* Output:
* Return:  none
* Others:
* Modify Date    Version   Author         Modification
* 2010/11/26     V1.0      zyt            create
*******************************************************/
void zte_goform_mgmt_reboot_process(webs_t wp)
{
	MSG_BUF stMsg         = {0};
	SINT32        iMsgSize      = sizeof(MSG_BUF) - sizeof(SINT32);
	char buf[NV_ITEM_STRING_LEN_20] = {0};

	slog(MISC_PRINT, SLOG_NORMAL, T("UFIx User reboot!\n"));
	ipc_send_message(MODULE_ID_WEB_CGI, MODULE_ID_MAIN_CTRL, MSG_CMD_RESTART_REQUEST, 0, NULL, 0);
}


void zte_goform_mgmt_syslog_process(webs_t wp)
{
	slog(MISC_PRINT, SLOG_DEBUG, "zte_goform_mgmt_syslog_process coming\n"); /*lint !e26*/
	char_t *syslog_mode = NULL;
	/* get value from web page */
	syslog_mode = websGetVar(wp, T("syslog_mode"), T("all"));

	char_t *syslog_flag = NULL;
	/* get value from web page */
	syslog_flag = websGetVar(wp, T("syslog_flag"), T("close"));

	if (strcmp(syslog_flag, "close") == 0) {
		cfg_set("debug_level", "0");
		zte_write_result_to_web(wp, SUCCESS);
	} else if (strcmp(syslog_flag, "delete") == 0) {
		slog(MISC_PRINT, SLOG_NORMAL, T("UFIx User delete syslog!\n"));
		slog(MISC_PRINT, SLOG_DEBUG, "delete syslog====\n"); /*lint !e26*/
		//system("cat /dev/null > /var/log/webshow_messages");
		system("cat /dev/null > /usr/netlog/misc.log");
		system("cat /dev/null > /etc_ro/web/webshow_messages");

		zte_write_result_to_web(wp, SUCCESS);
	} else {
		//cfg_set("debug_level","7");/*added by jhy */
		if (0 == strcmp("ufi", syslog_mode)) {
			slog(MISC_PRINT, SLOG_DEBUG, "syslog_mode=====%s\n", syslog_mode); /*lint !e26*/
			system("mkdir -p /etc_ro/web");
			system("log.sh UFI");
			cfg_set("syslog_mode", "ufi");
		}/*added by jhy */
		else if (0 == strcmp("wan_connect", syslog_mode)) {
			slog(MISC_PRINT, SLOG_DEBUG, "syslog_mode=====%s\n", syslog_mode); /*lint !e26*/
			system("log.sh wan_connect info");
			cfg_set("syslog_mode", "wan_connect");
		} else if (0 == strcmp("voip", syslog_mode)) {
			slog(MISC_PRINT, SLOG_DEBUG, "syslog_mode=====%s\n", syslog_mode); /*lint !e26*/
			system("log.sh voip info");
			cfg_set("syslog_mode", "voip");
		} else if (0 == strcmp("sms", syslog_mode)) {
			slog(MISC_PRINT, SLOG_DEBUG, "syslog_mode=====%s\n", syslog_mode); /*lint !e26*/
			system("log.sh sms info");
			cfg_set("syslog_mode", "sms");
		} else if (0 == strcmp("tr069", syslog_mode)) {
			slog(MISC_PRINT, SLOG_DEBUG, "syslog_mode=====%s\n", syslog_mode); /*lint !e26*/
			system("log.sh tr069 info");
			cfg_set("syslog_mode", "tr069");
		} else if (0 == strcmp("dlna", syslog_mode)) {
			slog(MISC_PRINT, SLOG_DEBUG, "syslog_mode=====%s\n", syslog_mode); /*lint !e26*/
			system("log.sh dlna info");
			cfg_set("syslog_mode", "dlna");
		} else if (0 == strcmp("wlan", syslog_mode)) {
			slog(MISC_PRINT, SLOG_DEBUG, "syslog_mode=====%s\n", syslog_mode); /*lint !e26*/
			system("log.sh wlan info");
			cfg_set("syslog_mode", "wlan");
		} else if (0 == strcmp("router", syslog_mode)) {
			slog(MISC_PRINT, SLOG_DEBUG, "syslog_mode=====%s\n", syslog_mode); /*lint !e26*/
			system("log.sh router info");
			cfg_set("syslog_mode", "router");
		} else {
			slog(MISC_PRINT, SLOG_DEBUG, "syslog_mode=====%s\n", syslog_mode); /*lint !e26*/
			system("log.sh all info");
			cfg_set("syslog_mode", "all");
		}

		zte_write_result_to_web(wp, SUCCESS);
	}

}


/******************************************************
* Function: void zte_mgmt_change_password(webs_t wp)
* Description:  deal with the user account modify goform
* Input:  HTTP page info
* Output:
* Return:  none
* Others:
* Modify Date    Version   Author         Modification
* 2010/11/26     V1.0      zyt            create
*******************************************************/
void zte_mgmt_change_password(webs_t wp)
{
	char_t *old_pass = NULL;
	char_t *new_pass = NULL;
	char_t *current_psw = NULL;
	char_t *new_psw = NULL;
	int current_psw_len = 0;
	int new_psw_len = 0;
	char new_psw_admin[CONFIG_DEFAULT_LENGTH] = {0};
	char old_psw_admin[CONFIG_DEFAULT_LENGTH] = {0};
	char admin_psw_buf[CONFIG_DEFAULT_LENGTH] = {0};

	old_pass = websGetVar(wp, T("oldPassword"), T(""));
	new_pass = websGetVar(wp, T("newPassword"), T(""));

	if (0 == strlen(old_pass) || 0 == strlen(new_pass)) {
		zte_write_result_to_web(wp, FAILURE);
		return;
	}

	slog(MISC_PRINT, SLOG_DEBUG, "user_modify:new_psw_encode:%s\n", new_pass); /*lint !e26*/
	current_psw = (char*)zte_base64_decode((const unsigned char *)old_pass, strlen(old_pass), (unsigned int *)&current_psw_len);
	new_psw = (char*)zte_base64_decode((const unsigned char *)new_pass, strlen(new_pass), (unsigned int *)&new_psw_len);

	if (NULL == current_psw || NULL == new_psw) {
		slog(MISC_PRINT, SLOG_ERR, "current_psw or new_psw is NULL\n"); /*lint !e26*/
		zte_write_result_to_web(wp, FAILURE);
		free(current_psw);
		free(new_psw);
		return;
	}
	slog(MISC_PRINT, SLOG_DEBUG, "user_modify:new_psw:%s\n", new_psw); /*lint !e26*/
	slog(MISC_PRINT, SLOG_DEBUG, "user_modify:new_psw_len:%d\n", new_psw_len); /*lint !e26*/
	strncpy(old_psw_admin, current_psw, current_psw_len);
	strncpy(new_psw_admin, new_psw, new_psw_len);
	free(current_psw);
	free(new_psw);
	slog(MISC_PRINT, SLOG_DEBUG, "user_modify:new_psw_admin:%s\n", new_psw_admin); /*lint !e26*/

	if (LOGIN_PSW_MIN_LEN > new_psw_len || LOGIN_PSW_MAX_LEN < new_psw_len) {
		slog(MISC_PRINT, SLOG_ERR, "new_psw_len is too long\n"); /*lint !e26*/
		cfg_set("data_safe", "failed");
		zte_write_result_to_web(wp, FAILURE);
		return;
	}
	if (DATA_NO_SAFE == zte_Safe_noSpecialChar(old_psw_admin)
	    || DATA_NO_SAFE == zte_Safe_noSpecialChar(new_psw_admin)) {
		slog(MISC_PRINT, SLOG_ERR, "Get Data is no Safe:old_pass:%s,new_pass:%s\n", old_psw_admin, new_psw_admin); /*lint !e26*/
		cfg_set("data_safe", "failed");
		zte_write_result_to_web(wp, FAILURE);
		return;
	}

	cfg_get_item("admin_Password", admin_psw_buf, sizeof(admin_psw_buf));
	if (0 != strcmp(old_psw_admin, admin_psw_buf)) {
		zte_write_result_to_web(wp, FAILURE);
		return;
	}

	slog(MISC_PRINT, SLOG_NORMAL, T("UFIx User change passwd!\n"));

	cfg_set("admin_Password", new_psw_admin);
	//cfg_set("user_save", "");
	cfg_set("psw_save", "");
	cfg_set("save_login", "");

	cfg_save();
	char cmd[64] = {0};
	sprintf(cmd, "mksmbpasswd.sh \'%s\'", new_psw_admin);
	system(cmd);
	system("killall smbd");
	system("killall nmbd");
	system("smbd -D");
	system("nmbd -D");
	zte_write_result_to_web(wp, SUCCESS);
}

int zte_mgmt_check_password(webs_t wp)
{
	char_t *tmp_old_psw = NULL;
	char_t *old_psw = NULL;
	int old_psw_len = 0;
	char old_psw_admin[CONFIG_DEFAULT_LENGTH] = {0};
	char admin_psw_buf[CONFIG_DEFAULT_LENGTH] = {0};

	tmp_old_psw = websGetVar(wp, T("oldPassword"), NULL);
	if (NULL != tmp_old_psw) {
		old_psw = (char*)zte_base64_decode((const unsigned char *)tmp_old_psw, strlen(tmp_old_psw), (unsigned int *)&old_psw_len);
		if (NULL == old_psw) {
			slog(MISC_PRINT, SLOG_DEBUG, "current_psw or new_psw is NULL\n"); /*lint !e26*/
			return -1;
		}
		slog(MISC_PRINT, SLOG_NORMAL,"zte_mgmt_check_password  old_psw:%s!\n", old_psw);

		strncpy(old_psw_admin, old_psw, old_psw_len);
		free(old_psw);

		if (DATA_NO_SAFE == zte_Safe_noSpecialChar(old_psw_admin)) {
			slog(MISC_PRINT, SLOG_ERR,"zte_mgmt_check_password  old_psw_admin Get Data is no Safe!\n");
			cfg_set("data_safe", "failed");
			return -1;
		}

		cfg_get_item("admin_Password", admin_psw_buf, sizeof(admin_psw_buf));
		if (0 != strcmp(old_psw_admin, admin_psw_buf)) {
			slog(MISC_PRINT, SLOG_ERR,"zte_mgmt_check_password  admin_Password fail!\n");
			return -1;
		}
	}

	return 0;
}

int zte_mgmt_handle_account(webs_t wp)
{
	char_t *tmp_new_user = NULL;
	char_t *tmp_new_psw = NULL;
	char_t *new_user = NULL;
	char_t *new_psw = NULL;
	int new_psw_len = 0;
	int new_user_len = 0;
	char new_user_admin[CONFIG_DEFAULT_LENGTH] = {0};
	char new_psw_admin[CONFIG_DEFAULT_LENGTH] = {0};

	tmp_new_psw = websGetVar(wp, T("newPassword"), NULL);
	tmp_new_user = websGetVar(wp, T("newUserName"), NULL);

	if (NULL == tmp_new_psw || NULL == tmp_new_user) {
		return -1;
	}

	new_psw = (char*)zte_base64_decode((const unsigned char *)tmp_new_psw, strlen(tmp_new_psw), (unsigned int *)&new_psw_len);
	new_user = (char*)zte_base64_decode((const unsigned char *)tmp_new_user, strlen(tmp_new_user), (unsigned int *)&new_user_len);

	if (NULL == new_psw || NULL == new_user) {
		slog(MISC_PRINT, SLOG_ERR, "current_psw or new_psw is NULL\n"); /*lint !e26*/
		return -1;
	}

	slog(MISC_PRINT, SLOG_DEBUG,"zte_mgmt_handle_account  new_psw:%s!\n", new_psw);
	slog(MISC_PRINT, SLOG_DEBUG,"zte_mgmt_handle_account  new_user:%s!\n", new_user);

	strncpy(new_psw_admin, new_psw, new_psw_len);
	strncpy(new_user_admin, new_user, new_user_len);
	free(new_psw);
	free(new_user);

	if (LOGIN_PSW_MIN_LEN > new_psw_len || LOGIN_PSW_MAX_LEN < new_psw_len) {
		slog(MISC_PRINT, SLOG_ERR,"zte_mgmt_handle_account  new_psw_len is too long!\n");
		cfg_set("data_safe", "failed");
		return -1;
	}

	if (LOGIN_PSW_MIN_LEN > new_user_len || LOGIN_PSW_MAX_LEN < new_user_len) {
		slog(MISC_PRINT, SLOG_ERR,"zte_mgmt_handle_account  new_user_len is too long!\n");
		cfg_set("data_safe", "failed");
		return -1;
	}

	if (DATA_NO_SAFE == zte_Safe_noSpecialChar(new_psw_admin)
	    || DATA_NO_SAFE == zte_Safe_noSpecialChar(new_user_admin)) {
		slog(MISC_PRINT, SLOG_ERR,"zte_mgmt_handle_account  Get Data is no Safe!\n");
		cfg_set("data_safe", "failed");
		return -1;
	}

	slog(MISC_PRINT, SLOG_NORMAL, T("UFIx User change account!\n"));

	cfg_set("admin_user", new_user_admin);
	cfg_set("admin_Password", new_psw_admin);
	//cfg_set("user_save", "");
	cfg_set("psw_save", "");
	cfg_set("save_login", "");
	cfg_save();
	char cmd[64] = {0};
	sprintf(cmd, "mksmbpasswd.sh \'%s\'", new_psw_admin);
	system(cmd);
	system("killall smbd");
	system("killall nmbd");
	system("smbd -D");
	system("nmbd -D");
	slog(MISC_PRINT, SLOG_DEBUG,"zte_mgmt_handle_account  success!\n");
	return 0;
}

void zte_mgmt_change_account(webs_t wp)
{
	slog(MISC_PRINT, SLOG_NORMAL,"zte_mgmt_change_account ====================!\n");

	if (0 != zte_mgmt_check_password(wp)) {
		zte_write_result_to_web(wp, FAILURE);
		return;
	}

	if (0 != zte_mgmt_handle_account(wp)) {
		zte_write_result_to_web(wp, FAILURE);
		return;
	}

	zte_write_result_to_web(wp, SUCCESS);
}


/******************************************************
* Function: void zte_mgmt_pin_input(webs_t wp)
* Description:  deal with the pin code input goform
* Input:  HTTP page info
* Output:
* Return:  none
* Others:
* Modify Date    Version   Author         Modification
* 2010/11/26     V1.0      zyt            create
*******************************************************/
void zte_mgmt_pin_input(webs_t wp)
{
	char                *pPinCode   = NULL;
	UINT32              length      = 0;
	T_zAt_CpinPukSet    para        = {0};
	UINT32              ret         = 0;
	char                modem_main_state[NV_ITEM_STRING_LEN_50] = {0};

	pPinCode    = websGetVar(wp, T("PinNumber"), T(""));
	length      = strlen(pPinCode);
	if (0 == length || length >= sizeof(para.pin)) {
		zte_write_result_to_web(wp, FAILURE);
		return;
	}

	strcpy(para.pin, pPinCode);
	cfg_set(NV_PIN_PUK_PROCESS, "begin");
	ret = ipc_send_message(MODULE_ID_WEB_CGI, MODULE_ID_AT_CTL, MSG_CMD_VERIFY_PIN_REQ, sizeof(T_zAt_CpinPukSet), (UCHAR *)&para, 0);
	if (0 != ret) {
		cfg_set(NV_PIN_PUK_PROCESS, "");
		zte_write_result_to_web(wp, FAILURE);
		return;
	}

	wait_verify(NV_PIN_PUK_PROCESS, "end");

	(void)sleep(1);
	(void)zte_web_read(NV_MODEM_MAIN_STATE, modem_main_state);

	if (0 != strcmp(modem_main_state, "modem_waitpin")) {
		zte_write_result_to_web(wp, SUCCESS);
	} else {
		zte_write_result_to_web(wp, FAILURE);
	}
}
/******************************************************
* Function: void zte_mgmt_puk_input(webs_t wp)
* Description:  deal with the puk code input goform
* Input:  HTTP page info
* Output:
* Return:  none
* Others:
* Modify Date    Version   Author         Modification
* 2010/11/26     V1.0      zyt            create
*******************************************************/
void zte_mgmt_puk_input(webs_t wp)
{
	char                *pPukCode   = NULL;
	char                *pNewPin    = NULL;
	int                 length      = 0;
	int                 ret         = 0;
	T_zAt_CpinPukSet    para        = {0};
	CHAR modem_main_state[NV_ITEM_STRING_LEN_50] = {0};

	pPukCode    = websGetVar(wp, T("PUKNumber"), T(""));
	pNewPin     = websGetVar(wp, T("PinNumber"), T(""));

	length = strlen(pPukCode);
	if (0 == length || length >= sizeof(para.pin)) {
		zte_write_result_to_web(wp, FAILURE);
		return;
	}

	length = strlen(pNewPin);
	if (0 == length || length >= sizeof(para.newpin)) {
		zte_write_result_to_web(wp, FAILURE);
		return;
	}

	strcpy(para.pin, pPukCode);
	strcpy(para.newpin, pNewPin);
	cfg_set(NV_PIN_PUK_PROCESS, "begin");
	ret = ipc_send_message(MODULE_ID_WEB_CGI, MODULE_ID_AT_CTL, MSG_CMD_VERIFY_PUK_REQ, sizeof(T_zAt_CpinPukSet), (UCHAR *)&para, 0);
	if (0 != ret) {
		cfg_set(NV_PIN_PUK_PROCESS, "");
		zte_write_result_to_web(wp, FAILURE);
		return;
	}

	wait_verify(NV_PIN_PUK_PROCESS, "end");

	(void)sleep(1);
	(void)zte_web_read("pinset_result", modem_main_state);

	if (0 != strcmp(modem_main_state, "fail")) {
		zte_write_result_to_web(wp, SUCCESS);
		cfg_save();
	} else {
		zte_write_result_to_web(wp, FAILURE);
	}
}

void zte_mgmt_auto_pin(webs_t wp)
{
	char                *pPinEable   = NULL;
	char                *pPinCode    = NULL;
	int                 length      = 0;

	pPinEable    = websGetVar(wp, T("auto_simpin"), T(""));
	pPinCode     = websGetVar(wp, T("auto_simpin_code"), T(""));

	length = strlen(pPinCode);
	if (0 == length || length > 8) {
		zte_write_result_to_web(wp, FAILURE);
		return;
	}

	length = strlen(pPinEable);
	if (1 != length) {
		zte_write_result_to_web(wp, FAILURE);
		return;
	}

	cfg_set("auto_simpin", pPinEable);
	cfg_set("auto_simpin_code", pPinCode);
	cfg_save();
	zte_write_result_to_web(wp, SUCCESS);

}
/******************************************************
* Function: void zte_mgmt_unlock_network(webs_t wp)
* Description:  deal with the unlock code input goform
* Input:  HTTP page info
* Output:
* Return:  none
* Others:
* Modify Date    Version   Author         Modification
* 2010/11/26     V1.0      zyt            create
*******************************************************/
void zte_mgmt_unlock_network(webs_t wp)
{
	char *unlock_code = NULL;
	UNLOCK_PARA_ST para;
	int length = 0;
	int ret = 0;

	memset(&para, 0, sizeof(UNLOCK_PARA_ST));

	unlock_code = websGetVar(wp, T("unlock_network_code"), T(""));
	slog(MISC_PRINT, SLOG_DEBUG, "unlock_code=%s", unlock_code); /*lint !e26*/
	length = strlen(unlock_code);
	if (0 == length || length >= sizeof(para.unlock_code)) {
		zte_write_result_to_web(wp, FAILURE);
		return;
	}

	strcpy(para.unlock_code, unlock_code);
	ret = ipc_send_message(MODULE_ID_WEB_CGI, MODULE_ID_AT_CTL, MSG_CMD_UNLOCK_REQ, sizeof(UNLOCK_PARA_ST), (UCHAR *)&para, 0);
	if (0 != ret) {
		zte_write_result_to_web(wp, FAILURE);
		return;
	}

	wait_verify("unlock_at_wait", "0");
	zte_write_result_to_web(wp, SUCCESS);
}


/*PIN码禁用*/
void zte_mgmt_disable_pin(webs_t wp)
{
	char            *pOldPin    = NULL;
	T_zAt_PinManage para        = {0};
	int             length      = 0;
	int             ret         = 0;
	char            pin_manage_result[NV_ITEM_STRING_LEN_5] = {0};

	pOldPin = websGetVar(wp, T("OldPinNumber"), T(""));
	length = strlen(pOldPin);
	if (0 == length || length >= sizeof(para.oldPin)) {
		zte_write_result_to_web(wp, FAILURE);
		return;
	}
	strcpy(para.oldPin, pOldPin);
	para.action = PIN_MANAGE_DISABLE;

	slog(MISC_PRINT, SLOG_NORMAL, T("UFIx User disable pin!\n"));

	(void)zte_web_write("pin_manage_process", "begin");
	slog(MISC_PRINT, SLOG_NORMAL, "zte_goform_mgmt_pin_mgmt_process send message: ZUFI_MODULE_ID_AT_MAIN, MSG_CMD_WEB_REQ_PIN_MANAGE");
	ret = ipc_send_message(MODULE_ID_WEB_CGI, MODULE_ID_AT_CTL, MSG_CMD_PIN_MANAGE_REQ, sizeof(T_zAt_PinManage), (UCHAR *)&para, 0);
	if (0 != ret) {
		cfg_set("pin_manage_process", "");
		zte_write_result_to_web(wp, FAILURE);
		return;
	}

	/*设置等待pin码管理动作AT处理结束*/
	wait_verify("pin_manage_process", "end");

	(void)zte_web_read("pin_manage_result", pin_manage_result);
	(void)zte_web_write("pin_manage_result", "");

	if (0 == strcmp(pin_manage_result, "0")) {
		zte_write_result_to_web(wp, SUCCESS);
		cfg_save();
	} else {
		zte_write_result_to_web(wp, FAILURE);
	}
}

/*PIN码启用或修改*/
void zte_mgmt_pin_enable_or_modify(webs_t wp)
{
	CHAR *pOldPin = NULL;
	CHAR *pNewPin = NULL;

	if (NULL == wp) {
		zte_write_result_to_web(wp, FAILURE);
		return;
	}

	pOldPin = websGetVar(wp, T("OldPinNumber"), T(""));
	pNewPin = websGetVar(wp, T("NewPinNumber"), T(""));

	slog(MISC_PRINT, SLOG_DEBUG, "web para:[OldPinNumber] is [%s].", pOldPin);
	slog(MISC_PRINT, SLOG_DEBUG, "web para:[NewPinNumber] is [%s].", pNewPin);

	if ('\0' == (*pOldPin)) {
		zte_write_result_to_web(wp, FAILURE);
		return;
	}

	if (0 == strcmp(pNewPin, "")) {
		zte_mgmt_enable_pin(wp, pOldPin);
	} else {
		zte_mgmt_modify_pin(wp, pOldPin, pNewPin);
	}


}
static void zte_mgmt_enable_pin(webs_t wp, CHAR *pOldPin)
{
	T_zAt_PinManage para                                    = {0};
	int             ret                                     = 0;
	char            pin_manage_result[NV_ITEM_STRING_LEN_5] = {0};


	if ((NULL == wp) || (NULL == pOldPin)) {
		zte_write_result_to_web(wp, FAILURE);
		return;
	}

	strcpy(para.oldPin, pOldPin);
	para.action = PIN_MANAGE_ENABLE;
	if ('\0' == (*pOldPin)) {
		zte_write_result_to_web(wp, FAILURE);
		return;
	}
	slog(MISC_PRINT, SLOG_NORMAL, T("UFIx User enable pin!\n"));

	(void)zte_web_write("pin_manage_process", "begin");
	slog(MISC_PRINT, SLOG_NORMAL, "zte_mgmt_enable_pin send message : ZUFI_MODULE_ID_AT_MAIN, MSG_CMD_WEB_REQ_PIN_MANAGE.");
	ret = ipc_send_message(MODULE_ID_WEB_CGI, MODULE_ID_AT_CTL, MSG_CMD_PIN_MANAGE_REQ, sizeof(T_zAt_PinManage), (UCHAR *)&para, 0);
	if (0 != ret) {
		cfg_set("pin_manage_process", "");
		zte_write_result_to_web(wp, FAILURE);
		return;
	}

	/*设置等待pin码管理动作AT处理结束*/
	wait_verify("pin_manage_process", "end");

	(void)zte_web_read("pin_manage_result", pin_manage_result);
	(void)zte_web_write("pin_manage_result", "");

	if (0 == strcmp(pin_manage_result, "0")) {
		zte_write_result_to_web(wp, SUCCESS);
		cfg_save();
	} else {
		zte_write_result_to_web(wp, FAILURE);
	}
}

/**********************************************************************
* Function:         zte_mgmt_modify_pin
* Description:      to modify pin
* Input:           wp: the web para;old_pin: old pin number; new_pin:new pin number
* Output:
* Return:
* Others:
* Modify Date   Version     Author          Modification
* -----------------------------------------------
* 2011/11/16    V1.0        chenyi       first version
**********************************************************************/

static void zte_mgmt_modify_pin(webs_t wp, CHAR *pOldPin, CHAR *pNewPin)
{
	int             length                                  = 0;
	int             ret                                     = 0;
	T_zAt_PinManage para                                    = {0};
	char            pin_manage_result[NV_ITEM_STRING_LEN_5] = {0};

	if ((NULL == wp) || (NULL == pOldPin) || (NULL == pNewPin)) {
		zte_write_result_to_web(wp, FAILURE);
		return;
	}

	if (('\0' == (*pOldPin)) || ('\0' == (*pNewPin))) {
		zte_write_result_to_web(wp, FAILURE);
		return;
	}
	length = strlen(pNewPin);
	if (0 == length || length >= sizeof(para.newPin)) {
		zte_write_result_to_web(wp, FAILURE);
		return;
	}


	slog(MISC_PRINT, SLOG_NORMAL, T("UFIx User modify pin!\n"));

	para.action = PIN_MANAGE_MODIFY;
	strcpy(para.oldPin, pOldPin);
	strcpy(para.newPin, pNewPin);
	(void)zte_web_write("pin_manage_process", "begin");
	slog(MISC_PRINT, SLOG_NORMAL, "zte_mgmt_modify_pin send message : ZUFI_MODULE_ID_AT_MAIN, MSG_CMD_WEB_REQ_PIN_MANAGE.");
	ret = ipc_send_message(MODULE_ID_WEB_CGI, MODULE_ID_AT_CTL, MSG_CMD_PIN_MANAGE_REQ, sizeof(T_zAt_PinManage), (UCHAR *)&para, 0);
	if (0 != ret) {
		cfg_set("pin_manage_process", "");
		zte_write_result_to_web(wp, FAILURE);
		return;
	}

	/*设置等待pin码管理动作AT处理结束*/
	wait_verify("pin_manage_process", "end");


	(void)zte_web_read("pin_manage_result", pin_manage_result);
	(void)zte_web_write("pin_manage_result", "");

	if (0 == strcmp(pin_manage_result, "0")) {
		zte_write_result_to_web(wp, SUCCESS);
		cfg_save();
	} else {
		zte_write_result_to_web(wp, FAILURE);
	}
}
/******************************************************
* Function: void zte_quick_setup(webs_t wp)
* Description:  deal with the quick setup goform
* Input:  HTTP page info
* Output:
* Return:  none
* Others:
* Modify Date    Version   Author         Modification
* 2010/11/26     V1.0      zyt            create
*******************************************************/
void zte_quick_setup(webs_t wp)
{
	int iFunc = 0;

	slog(MISC_PRINT, SLOG_NORMAL, T("UFIx User quick setup!\n"));

	for (iFunc = 0; iFunc < MAX_QUICK_SET_NUM; iFunc++) {
		if (G_ZQUICK_SET[iFunc] != NULL) {
			G_ZQUICK_SET[iFunc](wp);
		}
	}
	cfg_save();
	zte_write_result_to_web(wp, SUCCESS);
}

/******************************************************
* Function: void zte_goform_mgmt_sntp_process(webs_t wp)
* Description:  deal with the sntp
* Input:  HTTP page info
* Output:
* Return:  none
* Others:
* Modify Date    Version   Author         Modification
* 2011/04/13     V1.0      mengyuan            create
*******************************************************/
void zte_goform_mgmt_sntp_process(webs_t wp)
{
	MSG_BUF msg;
	int lTgtMsgID = 0;
	char *mode = NULL;
	char *sntp_server0 = NULL;
	char *sntp_server1 = NULL;
	char *sntp_server2 = NULL;
	int result = 0;
	LONG msgSize = sizeof(MSG_BUF) - sizeof(LONG);
	char *ntp_server = NULL;
	int sem_id = 0;
	char state[NV_ITEM_STRING_LEN_20] = {0};

	memset(&msg, 1, sizeof(MSG_BUF));
	mode = websGetVar(wp, T("manualsettime"), T(""));
	cfg_set("sntp_time_set_mode", mode);

	sntp_server0 = websGetVar(wp, T("sntp_server1_ip"), T(""));
	sntp_server1 = websGetVar(wp, T("sntp_server2_ip"), T(""));
	sntp_server2 = websGetVar(wp, T("sntp_server3_ip"), T(""));
	cfg_set("sntp_server0", sntp_server0);
	cfg_set("sntp_server1", sntp_server1);
	cfg_set("sntp_server2", sntp_server2);


	slog(MISC_PRINT, SLOG_NORMAL, T("UFIx User set sntp time!\n"));

	if (strcmp(sntp_server0, "Other") == 0) {
		cfg_set("sntp_other_server0", websGetVar(wp, T("sntp_other_server0"), T("")));
	}
	if (strcmp(sntp_server1, "Other") == 0) {
		cfg_set("sntp_other_server1", websGetVar(wp, T("sntp_other_server1"), T("")));
	}
	if (strcmp(sntp_server2, "Other") == 0) {
		cfg_set("sntp_other_server2", websGetVar(wp, T("sntp_other_server2"), T("")));
	}
	cfg_set("sntp_timezone_index", websGetVar(wp, T("sntp_timezone_index"), T("")));
	cfg_set("sntp_timezone", websGetVar(wp, T("timezone"), T("")));
	cfg_set("sntp_dst_enable", websGetVar(wp, T("DaylightEnabled"), T("")));

	cfg_set("manual_time_year", websGetVar(wp, T("time_year"), T("")));
	cfg_set("manual_time_month", websGetVar(wp, T("time_month"), T("")));
	cfg_set("manual_time_day", websGetVar(wp, T("time_day"), T("")));
	cfg_set("manual_time_hour", websGetVar(wp, T("time_hour"), T("")));
	cfg_set("manual_time_minute", websGetVar(wp, T("time_minute"), T("")));
	cfg_set("manual_time_second", websGetVar(wp, T("time_second"), T("")));

	if (strcmp(mode, "manual") == 0) {
		sem_id = get_sem(TIME_SEM_KEY_1);
		if (sem_id != -1) {
			sem_p(sem_id);
			slog(MISC_PRINT, SLOG_DEBUG,"[MANUAL] manual_set_time, sem_p, sem_id = %d\r\n", sem_id);
		}

		result = manual_set_time();

		if (sem_id != -1) {
			sem_v(sem_id);
			slog(MISC_PRINT, SLOG_DEBUG,"[MANUAL] manual_set_time, sem_v, sem_id = %d\r\n", sem_id);
		}

		cfg_set("sntp_year", websGetVar(wp, T("time_year"), T("")));
		cfg_set("sntp_month", websGetVar(wp, T("time_month"), T("")));
		cfg_set("sntp_day", websGetVar(wp, T("time_day"), T("")));
		cfg_set("sntp_hour", websGetVar(wp, T("time_hour"), T("")));
		cfg_set("sntp_minute", websGetVar(wp, T("time_minute"), T("")));
		cfg_set("sntp_second", websGetVar(wp, T("time_second"), T("")));
		cfg_set("sntp_process_result", "success");
		if (result < 0) {
			zte_write_result_to_web(wp, FAILURE);
		} else {
			if (!zte_mgmt_login_timemark_set()) {
				slog(MISC_PRINT, SLOG_ERR, "[ERROR]zte_goform_whitelist_check -> timemark set error .\n"); /*lint !e26*/
			}
			zte_write_result_to_web(wp, SUCCESS);
		}
	} else if (strcmp(mode, "auto") == 0) {
		cfg_get_item("sntp_process_state", state, sizeof(state));
		if (strcmp(state, "over") != 0) {
			zte_write_result_to_web(wp, PROCESSING);
			slog(MISC_PRINT, SLOG_DEBUG,"[SNTP] already runing return \n");
			cfg_save();
			return;
		}
		cfg_set("sntp_process_state", "idle");
		//cfg_set("systime_mode", "auto");
		//cfg_set("sntp_cmd_from", "WEBUI");
		cfg_set("sntp_process_result", "");
		ipc_send_message(MODULE_ID_WEB_CGI, MODULE_ID_SNTP, MSG_CMD_SNTP_START, 0, NULL, 0);
		zte_write_result_to_web(wp, SUCCESS);
	} else {
		sem_id = get_sem(TIME_SEM_KEY_1);
		if (sem_id != -1) {
			sem_p(sem_id);
			slog(MISC_PRINT, SLOG_DEBUG,"[MANUAL] manual_set_time, sem_p, sem_id = %d\r\n", sem_id);
		}

		result = manual_set_time();

		if (sem_id != -1) {
			sem_v(sem_id);
			slog(MISC_PRINT, SLOG_DEBUG,"[MANUAL] manual_set_time, sem_v, sem_id = %d\r\n", sem_id);
		}

		cfg_set("sntp_year", websGetVar(wp, T("time_year"), T("")));
		cfg_set("sntp_month", websGetVar(wp, T("time_month"), T("")));
		cfg_set("sntp_day", websGetVar(wp, T("time_day"), T("")));
		cfg_set("sntp_hour", websGetVar(wp, T("time_hour"), T("")));
		cfg_set("sntp_minute", websGetVar(wp, T("time_minute"), T("")));
		cfg_set("sntp_second", websGetVar(wp, T("time_second"), T("")));

		if (result < 0) {
			zte_write_result_to_web(wp, FAILURE);
		} else {
			if (!zte_mgmt_login_timemark_set()) {
				slog(MISC_PRINT, SLOG_ERR, "[ERROR]zte_goform_whitelist_check -> timemark set error .\n"); /*lint !e26*/
			}
			zte_write_result_to_web(wp, SUCCESS);
		}
	}

	cfg_save();
}

static void wait_verify(char *wait_name, char *wait_value)
{
	int i = 0;
	char buf[NV_ITEM_STRING_LEN_200] = {0};

	if (NULL == wait_name || NULL == wait_value) {
		return;
	}

	for (i = 0; i < 10; i++) {
		sleep(2);
		cfg_get_item(wait_name, buf, sizeof(buf));
		if (0 == strcmp(wait_value, buf)) {
			break;
		}
	}
	return;
}


void deal_quick_setup_wifi_basic(webs_t wp)
{
	deal_quick_setup_wifi_basic_mgmt(wp);
}
static int deal_quick_setup_auto_apn_set()
{
	APN_PROFILE  newProfile = { 0 };
	char ppp_status[NV_ITEM_STRING_LEN_50] = {0};

	get_autoapn_profile(&newProfile);
	set_apn_to_cfg(&newProfile, NULL);
	cfg_set("ipv6_wan_apn", newProfile.apn_name); 

	cfg_get_item("ppp_status", ppp_status, sizeof(ppp_status));
	if (0 == strcmp("ppp_disconnected", ppp_status) || 0 == strcmp("ppp_ready", ppp_status)) {
		ipc_send_message(MODULE_ID_WEB_CGI, MODULE_ID_AT_CTL, MSG_CMD_APN_SET_REQ, 0, NULL, 0);
	}
	return 0;
}
static void set_apn_to_cfg(APN_PROFILE *apn_profile, IPV6_APN_PROFILE *ipv6_apn_profile)
{
	if (apn_profile != NULL) {
		cfg_set("m_profile_name", apn_profile->profile_name);
		cfg_set("wan_apn", apn_profile->apn_name);
		cfg_set("apn_select", apn_profile->apn_select);
		cfg_set("wan_dial", apn_profile->dial_num);
		cfg_set("ppp_auth_mode", apn_profile->ppp_auth_mode);
		cfg_set("ppp_username", apn_profile->ppp_username);
		cfg_set("ppp_passwd", apn_profile->ppp_passwd);
		cfg_set("pdp_type", apn_profile->pdp_type);
		cfg_set("ipv6_pdp_type", apn_profile->pdp_type);
		cfg_set("pdp_select", apn_profile->pdp_select);
		cfg_set("pdp_addr", apn_profile->pdp_addr);
	}

	if (ipv6_apn_profile != NULL) {
		cfg_set("m_profile_name", ipv6_apn_profile->profile_name);
		cfg_set("ipv6_wan_apn", ipv6_apn_profile->apn_name);
		cfg_set("apn_select", ipv6_apn_profile->apn_select);
		cfg_set("wan_dial", ipv6_apn_profile->dial_num);
		cfg_set(NV_IPV6_PPP_AUTH_MODE, ipv6_apn_profile->ppp_auth_mode);
		cfg_set("ipv6_ppp_username", ipv6_apn_profile->ppp_username);
		cfg_set("ipv6_ppp_passwd", ipv6_apn_profile->ppp_passwd);
		cfg_set("pdp_type", ipv6_apn_profile->pdp_type);
		cfg_set("ipv6_pdp_type", ipv6_apn_profile->pdp_type);
		cfg_set("pdp_select", ipv6_apn_profile->pdp_select);
		cfg_set("pdp_addr", ipv6_apn_profile->pdp_addr);
	}
}


static void deal_quick_setup_manual_apn_set(webs_t wp)
{
	IPV6_APN_PROFILE newIpv6Profile = { 0 };
	APN_PROFILE      newProfile     = { 0 };
	int              index_apn      = atoi(websGetVar(wp, "index", T("")));
	char            *pdp_type       = websGetVar(wp, "pdp_type", T(""));
	char            *apn_name      = websGetVar(wp, "wan_apn", T(""));
	char            *ipv6_wan_apn      = websGetVar(wp, "ipv6_wan_apn", T(""));
	char ppp_status[NV_ITEM_STRING_LEN_50] = {0};
	char            *ppp_auth_mode      = websGetVar(wp, "ppp_auth_mode", T(""));
	char            *ppp_username      = websGetVar(wp, "ppp_username", T(""));
	char            *ppp_passwd      = websGetVar(wp, "ppp_passwd", T(""));
	char            *ipv6_pdp_auth_mode      = websGetVar(wp, NV_IPV6_PPP_AUTH_MODE, T(""));
	char            *ipv6_ppp_username      = websGetVar(wp, "ipv6_ppp_username", T(""));
	char            *ipv6_ppp_passwd      = websGetVar(wp, "ipv6_ppp_passwd", T(""));

	slog(MISC_PRINT, SLOG_NORMAL,"[goahead] deal_quick_setup_manual_apn_set wan_apn %s. %s.\n", apn_name,ppp_username);

	slog(MISC_PRINT, SLOG_NORMAL, T("UFIx User manual apn set!\n"));
	if (0 == strcmp(pdp_type, "IPv6")) {
		get_ipv6apn_profile_by_index(index_apn, &newIpv6Profile);
		strcpy(newIpv6Profile.apn_name, ipv6_wan_apn);
		strcpy(newIpv6Profile.ppp_auth_mode, ipv6_pdp_auth_mode);
		strcpy(newIpv6Profile.ppp_username, ipv6_ppp_username);
		strcpy(newIpv6Profile.ppp_passwd, ipv6_ppp_passwd);
		set_ipv6_apn_profile_by_index(index_apn, &newIpv6Profile);

		set_apn_to_cfg(NULL, &newIpv6Profile);
	} else if (0 == strcmp(pdp_type, "IPv4v6")) {
		get_ipv4v6apn_profile_by_index(index_apn, &newProfile, &newIpv6Profile);
		strcpy(newProfile.apn_name, apn_name);
		strcpy(newProfile.ppp_auth_mode, ppp_auth_mode);
		strcpy(newProfile.ppp_username, ppp_username);
		strcpy(newProfile.ppp_passwd, ppp_passwd);

		strcpy(newIpv6Profile.apn_name, ipv6_wan_apn);
		strcpy(newIpv6Profile.ppp_auth_mode, ipv6_pdp_auth_mode);
		strcpy(newIpv6Profile.ppp_username, ipv6_ppp_username);
		strcpy(newIpv6Profile.ppp_passwd, ipv6_ppp_passwd);
		set_ipv4v6_apn_profile_by_index(index_apn, &newProfile, &newIpv6Profile);

		set_apn_to_cfg(&newProfile, &newIpv6Profile);
	} else {
		get_apn_profile_by_index(index_apn, &newProfile);
		strcpy(newProfile.apn_name, apn_name);
		strcpy(newProfile.ppp_auth_mode, ppp_auth_mode);
		strcpy(newProfile.ppp_username, ppp_username);
		strcpy(newProfile.ppp_passwd, ppp_passwd);
		set_apn_profile_by_index(index_apn, &newProfile);

		set_apn_to_cfg(&newProfile, NULL);
	}

	cfg_get_item("ppp_status", ppp_status, sizeof(ppp_status));
	if (0 == strcmp("ppp_disconnected", ppp_status) || 0 == strcmp("ppp_ready", ppp_status)) {
		ipc_send_message(MODULE_ID_WEB_CGI, MODULE_ID_AT_CTL, MSG_CMD_APN_SET_REQ, 0, NULL, 0);
	}
}

void deal_quick_setup_apn_ex(webs_t wp)
{
	if (strcmp(ZTE_WEB_ACT_MANUAL, websGetVar(wp, "apn_mode", T(""))) == 0) {
		zte_web_write("apn_mode", ZTE_WEB_ACT_MANUAL);
		deal_quick_setup_manual_apn_set(wp);
	} else {
		zte_web_write("apn_mode", ZTE_WEB_ACT_AUTO);
		deal_quick_setup_auto_apn_set(wp);
	}
}

void deal_quick_setup_wifi_security(webs_t wp)
{
	deal_quick_setup_wifi_security_mgmt(wp);
}

static void deal_quick_setup_wps(webs_t wp)
{
	deal_quick_setup_wps_mgmt(wp);
}
/******************************************************
* Function: void get_apn_profile_by_index(int index, APN_PROFILE *profile)
* Description:  get a apn profile by index from config
* Input:  index:the index of apn profile which to get
* Output: profile:the result of apn profile config after splited
* Return:  none
* Others:
* Modify Date    Version   Author         Modification
* 2010/11/26     V1.0      zyt            create
*******************************************************/
void get_apn_profile_by_index(int index, APN_PROFILE *profile)
{
	char cfg_name[PROFILE_MEMBER_LEN] = {0};
	char cfg_value[APNCONFIG_MEMORY] = {0};
	char *pos_begin = NULL;

	if (NULL == profile) {
		return;
	}

	sprintf(cfg_name, "APN_config%d", index);

	cfg_get_item(cfg_name, cfg_value, sizeof(cfg_value));
	pos_begin = cfg_value;
	pos_begin = split_str_by_sep(pos_begin, "($)", profile->profile_name, PROFILE_MEMBER_LEN);
	pos_begin = split_str_by_sep(pos_begin, "($)", profile->apn_name, PROFILE_APN_LEN);
	pos_begin = split_str_by_sep(pos_begin, "($)", profile->apn_select, PROFILE_MEMBER_LEN);
	pos_begin = split_str_by_sep(pos_begin, "($)", profile->dial_num, PROFILE_MEMBER_LEN);
	pos_begin = split_str_by_sep(pos_begin, "($)", profile->ppp_auth_mode, PROFILE_MEMBER_LEN);
	pos_begin = split_str_by_sep(pos_begin, "($)", profile->ppp_username, PROFILE_APN_LEN);
	pos_begin = split_str_by_sep(pos_begin, "($)", profile->ppp_passwd, PROFILE_APN_LEN);
	pos_begin = split_str_by_sep(pos_begin, "($)", profile->pdp_type, PROFILE_MEMBER_LEN);
	pos_begin = split_str_by_sep(pos_begin, "($)", profile->pdp_select, PROFILE_MEMBER_LEN);
	pos_begin = split_str_by_sep(pos_begin, "($)", profile->pdp_addr, PROFILE_MEMBER_LEN);
	pos_begin = split_str_by_sep(pos_begin, "($)", profile->dns_mode, PROFILE_MEMBER_LEN);
	pos_begin = split_str_by_sep(pos_begin, "($)", profile->prefer_dns_manual, PROFILE_MEMBER_LEN);
	pos_begin = split_str_by_sep(pos_begin, "($)", profile->standby_dns_manual, PROFILE_MEMBER_LEN);
	return;
}

void get_ipv6apn_profile_by_index(int index, IPV6_APN_PROFILE *profile)
{
	char cfg_name[PROFILE_MEMBER_LEN] = {0};
	char cfg_value[APNCONFIG_MEMORY] = {0};
	char *pos_begin = NULL;

	if (NULL == profile) {
		return;
	}

	sprintf(cfg_name, "ipv6_APN_config%d", index);

	cfg_get_item(cfg_name, cfg_value, sizeof(cfg_value));
	slog(MISC_PRINT, SLOG_DEBUG, "get_ipv6apn_profile_by_index  cfg_value=%s", cfg_value); /*lint !e26*/

	pos_begin = cfg_value;
	pos_begin = split_str_by_sep(pos_begin, "($)", profile->profile_name, PROFILE_MEMBER_LEN);
	slog(MISC_PRINT, SLOG_DEBUG, "========= profile->profile_name=%s=======", profile->profile_name); /*lint !e26*/
	pos_begin = split_str_by_sep(pos_begin, "($)", profile->apn_name, PROFILE_APN_LEN);
	slog(MISC_PRINT, SLOG_DEBUG, "=========profile->apn_name=%s=======", profile->apn_name); /*lint !e26*/

	pos_begin = split_str_by_sep(pos_begin, "($)", profile->apn_select, PROFILE_MEMBER_LEN);
	pos_begin = split_str_by_sep(pos_begin, "($)", profile->dial_num, PROFILE_MEMBER_LEN);
	pos_begin = split_str_by_sep(pos_begin, "($)", profile->ppp_auth_mode, PROFILE_MEMBER_LEN);
	pos_begin = split_str_by_sep(pos_begin, "($)", profile->ppp_username, PROFILE_APN_LEN);
	pos_begin = split_str_by_sep(pos_begin, "($)", profile->ppp_passwd, PROFILE_APN_LEN);
	pos_begin = split_str_by_sep(pos_begin, "($)", profile->pdp_type, PROFILE_MEMBER_LEN);
	pos_begin = split_str_by_sep(pos_begin, "($)", profile->pdp_select, PROFILE_MEMBER_LEN);
	pos_begin = split_str_by_sep(pos_begin, "($)", profile->pdp_addr, PROFILE_MEMBER_LEN);
	pos_begin = split_str_by_sep(pos_begin, "($)", profile->dns_mode, PROFILE_MEMBER_LEN);
	pos_begin = split_str_by_sep(pos_begin, "($)", profile->prefer_dns_manual, PROFILE_MEMBER_LEN);
	pos_begin = split_str_by_sep(pos_begin, "($)", profile->standby_dns_manual, PROFILE_MEMBER_LEN);
	return;
}


void get_ipv4v6apn_profile_by_index(int index, APN_PROFILE *profile, IPV6_APN_PROFILE *ipv6profile)
{
	char cfg_name[PROFILE_MEMBER_LEN] = {0};
	char cfg_value[APNCONFIG_MEMORY] = {0};
	char *pos_begin = NULL;

	char ipv6_cfg_name[PROFILE_MEMBER_LEN] = {0};
	char ipv6_cfg_value[APNCONFIG_MEMORY] = {0};
	char *ipv6_pos_begin = NULL;


	if (NULL == profile) {
		return;
	}
	if (NULL == ipv6profile) {
		return;
	}

	sprintf(cfg_name, "APN_config%d", index);

	cfg_get_item(cfg_name, cfg_value, sizeof(cfg_value));
	pos_begin = cfg_value;
	pos_begin = split_str_by_sep(pos_begin, "($)", profile->profile_name, PROFILE_MEMBER_LEN);
	pos_begin = split_str_by_sep(pos_begin, "($)", profile->apn_name, PROFILE_APN_LEN);
	pos_begin = split_str_by_sep(pos_begin, "($)", profile->apn_select, PROFILE_MEMBER_LEN);
	pos_begin = split_str_by_sep(pos_begin, "($)", profile->dial_num, PROFILE_MEMBER_LEN);
	pos_begin = split_str_by_sep(pos_begin, "($)", profile->ppp_auth_mode, PROFILE_MEMBER_LEN);
	pos_begin = split_str_by_sep(pos_begin, "($)", profile->ppp_username, PROFILE_APN_LEN);
	pos_begin = split_str_by_sep(pos_begin, "($)", profile->ppp_passwd, PROFILE_APN_LEN);
	pos_begin = split_str_by_sep(pos_begin, "($)", profile->pdp_type, PROFILE_MEMBER_LEN);
	pos_begin = split_str_by_sep(pos_begin, "($)", profile->pdp_select, PROFILE_MEMBER_LEN);
	pos_begin = split_str_by_sep(pos_begin, "($)", profile->pdp_addr, PROFILE_MEMBER_LEN);
	pos_begin = split_str_by_sep(pos_begin, "($)", profile->dns_mode, PROFILE_MEMBER_LEN);
	pos_begin = split_str_by_sep(pos_begin, "($)", profile->prefer_dns_manual, PROFILE_MEMBER_LEN);
	pos_begin = split_str_by_sep(pos_begin, "($)", profile->standby_dns_manual, PROFILE_MEMBER_LEN);


	sprintf(ipv6_cfg_name, "ipv6_APN_config%d", index);

	cfg_get_item(ipv6_cfg_name, ipv6_cfg_value, sizeof(ipv6_cfg_value));
	ipv6_pos_begin = ipv6_cfg_value;
	ipv6_pos_begin = split_str_by_sep(ipv6_pos_begin, "($)", ipv6profile->profile_name, PROFILE_MEMBER_LEN);
	ipv6_pos_begin = split_str_by_sep(ipv6_pos_begin, "($)", ipv6profile->apn_name, PROFILE_APN_LEN);
	ipv6_pos_begin = split_str_by_sep(ipv6_pos_begin, "($)", ipv6profile->apn_select, PROFILE_MEMBER_LEN);
	ipv6_pos_begin = split_str_by_sep(ipv6_pos_begin, "($)", ipv6profile->dial_num, PROFILE_MEMBER_LEN);
	ipv6_pos_begin = split_str_by_sep(ipv6_pos_begin, "($)", ipv6profile->ppp_auth_mode, PROFILE_MEMBER_LEN);
	ipv6_pos_begin = split_str_by_sep(ipv6_pos_begin, "($)", ipv6profile->ppp_username, PROFILE_APN_LEN);
	ipv6_pos_begin = split_str_by_sep(ipv6_pos_begin, "($)", ipv6profile->ppp_passwd, PROFILE_APN_LEN);
	ipv6_pos_begin = split_str_by_sep(ipv6_pos_begin, "($)", ipv6profile->pdp_type, PROFILE_MEMBER_LEN);
	ipv6_pos_begin = split_str_by_sep(ipv6_pos_begin, "($)", ipv6profile->pdp_select, PROFILE_MEMBER_LEN);
	ipv6_pos_begin = split_str_by_sep(ipv6_pos_begin, "($)", ipv6profile->pdp_addr, PROFILE_MEMBER_LEN);
	ipv6_pos_begin = split_str_by_sep(ipv6_pos_begin, "($)", ipv6profile->dns_mode, PROFILE_MEMBER_LEN);
	ipv6_pos_begin = split_str_by_sep(ipv6_pos_begin, "($)", ipv6profile->prefer_dns_manual, PROFILE_MEMBER_LEN);
	ipv6_pos_begin = split_str_by_sep(ipv6_pos_begin, "($)", ipv6profile->standby_dns_manual, PROFILE_MEMBER_LEN);


	return;
}



void get_autoapn_profile(APN_PROFILE *profile)
{
	char cfg_name[PROFILE_MEMBER_LEN] = {0};
	char cfg_value[APNCONFIG_MEMORY] = {0};
	char *pos_begin = NULL;
	char buf[NV_ITEM_STRING_LEN_20] = {0};

	int auto_apn_index = 0;
	slog(MISC_PRINT, SLOG_DEBUG, "get_autoapn_profile\n"); /*lint !e26*/

	cfg_get_item("auto_apn_index", buf, sizeof(buf));
	auto_apn_index = atoi(buf);
	slog(MISC_PRINT, SLOG_DEBUG, "auto_apn_index=%d\n", auto_apn_index); /*lint !e26*/
	if (NULL == profile) {
		return;
	}

	if (0 != auto_apn_index) {
		sprintf(cfg_name, "apn_auto_config%d", auto_apn_index);
	} else {
		sprintf(cfg_name, "apn_auto_config");
	}
	slog(MISC_PRINT, SLOG_DEBUG, "cfg_name=%s\n", cfg_name); /*lint !e26*/

	cfg_get_item(cfg_name, cfg_value, sizeof(cfg_value));
	pos_begin = cfg_value;
	pos_begin = split_str_by_sep(pos_begin, "($)", profile->profile_name, PROFILE_MEMBER_LEN);
	pos_begin = split_str_by_sep(pos_begin, "($)", profile->apn_name, PROFILE_APN_LEN);
	pos_begin = split_str_by_sep(pos_begin, "($)", profile->apn_select, PROFILE_MEMBER_LEN);
	pos_begin = split_str_by_sep(pos_begin, "($)", profile->dial_num, PROFILE_MEMBER_LEN);
	pos_begin = split_str_by_sep(pos_begin, "($)", profile->ppp_auth_mode, PROFILE_MEMBER_LEN);
	pos_begin = split_str_by_sep(pos_begin, "($)", profile->ppp_username, PROFILE_APN_LEN);
	pos_begin = split_str_by_sep(pos_begin, "($)", profile->ppp_passwd, PROFILE_APN_LEN);
	pos_begin = split_str_by_sep(pos_begin, "($)", profile->pdp_type, PROFILE_MEMBER_LEN);
	pos_begin = split_str_by_sep(pos_begin, "($)", profile->pdp_select, PROFILE_MEMBER_LEN);
	pos_begin = split_str_by_sep(pos_begin, "($)", profile->pdp_addr, PROFILE_MEMBER_LEN);
	pos_begin = split_str_by_sep(pos_begin, "($)", profile->dns_mode, PROFILE_MEMBER_LEN);
	pos_begin = split_str_by_sep(pos_begin, "($)", profile->prefer_dns_manual, PROFILE_MEMBER_LEN);
	pos_begin = split_str_by_sep(pos_begin, "($)", profile->standby_dns_manual, PROFILE_MEMBER_LEN);
	return;
}

/******************************************************
* Function: void set_apn_profile_by_index(int index, APN_PROFILE *profile)
* Description:  set a apn profile into config by index
* Input:  index:the index of apn profile which to set
*         profile:the struct of apn profile.
* Output:
* Return:  none
* Others:
* Modify Date    Version   Author         Modification
* 2010/11/26     V1.0      zyt            create
*******************************************************/
void set_apn_profile_by_index(int index, APN_PROFILE *profile)
{
	char cfg_name[PROFILE_MEMBER_LEN] = {0};
	char cfg_value[APNCONFIG_MEMORY] = {0};

	if (NULL == profile) {
		return;
	}


	sprintf(cfg_name, "APN_config%d", index);

	if (0 == strcmp(profile->pdp_type, "IPv6")) {
		snprintf(cfg_value, APNCONFIG_MEMORY,
		         "%s($)%s($)%s($)%s($)%s($)%s($)%s($)%s($)%s($)%s($)%s($)%s($)%s($)",
		         profile->profile_name,
		         "",
		         "",
		         "",
		         "",
		         "",
		         "",
		         "",
		         "",
		         "",
		         "",
		         "",
		         "");
	} else {
		snprintf(cfg_value, APNCONFIG_MEMORY,
		         "%s($)%s($)%s($)%s($)%s($)%s($)%s($)%s($)%s($)%s($)%s($)%s($)%s($)",
		         profile->profile_name,
		         profile->apn_name,
		         profile->apn_select,
		         profile->dial_num,
		         profile->ppp_auth_mode,
		         profile->ppp_username,
		         profile->ppp_passwd,
		         profile->pdp_type,
		         profile->pdp_select,
		         profile->pdp_addr,
		         profile->dns_mode,
		         profile->prefer_dns_manual,
		         profile->standby_dns_manual);
	}
	cfg_set(cfg_name, cfg_value);

	return;
}

void set_ipv6_apn_profile_by_index(int index, IPV6_APN_PROFILE *profile)
{
	char cfg_name[PROFILE_MEMBER_LEN] = {0};
	char cfg_value[APNCONFIG_MEMORY] = {0};

	if (NULL == profile) {
		return;
	}

	sprintf(cfg_name, "ipv6_APN_config%d", index);


	if (0 == strcmp(profile->pdp_type, "IP")) {
		slog(MISC_PRINT, SLOG_DEBUG, "pdp_type=IP  cpsnprintf "); /*lint !e26*/
		snprintf(cfg_value, APNCONFIG_MEMORY,
		         "%s($)%s($)%s($)%s($)%s($)%s($)%s($)%s($)%s($)%s($)%s($)%s($)%s($)",
		         profile->profile_name,
		         "",
		         "",
		         "",
		         "",
		         "",
		         "",
		         "",
		         "",
		         "",
		         "",
		         "",
		         "");
	} else {
		slog(MISC_PRINT, SLOG_DEBUG, "pdp_type=else  cpsnprintf "); /*lint !e26*/
		slog(MISC_PRINT, SLOG_DEBUG, "profile->profile_name=%s ", profile->profile_name); /*lint !e26*/
		snprintf(cfg_value, APNCONFIG_MEMORY,
		         "%s($)%s($)%s($)%s($)%s($)%s($)%s($)%s($)%s($)%s($)%s($)%s($)%s($)",
		         profile->profile_name,
		         profile->apn_name,
		         profile->apn_select,
		         profile->dial_num,
		         profile->ppp_auth_mode,
		         profile->ppp_username,
		         profile->ppp_passwd,
		         profile->pdp_type,
		         profile->pdp_select,
		         profile->pdp_addr,
		         profile->dns_mode,
		         profile->prefer_dns_manual,
		         profile->standby_dns_manual);
	}
	slog(MISC_PRINT, SLOG_DEBUG, "cfg_value=%s", cfg_value); /*lint !e26*/
	cfg_set(cfg_name, cfg_value);

	return;
}

void set_ipv4v6_apn_profile_by_index(int index, APN_PROFILE *profile, IPV6_APN_PROFILE *ipv6profile)
{
	char cfg_name[PROFILE_MEMBER_LEN] = {0};
	char cfg_value[APNCONFIG_MEMORY] = {0};

	if (NULL == profile) {
		return;
	}

	sprintf(cfg_name, "APN_config%d", index);
	snprintf(cfg_value, APNCONFIG_MEMORY,
	         "%s($)%s($)%s($)%s($)%s($)%s($)%s($)%s($)%s($)%s($)%s($)%s($)%s($)",
	         profile->profile_name,
	         profile->apn_name,
	         profile->apn_select,
	         profile->dial_num,
	         profile->ppp_auth_mode,
	         profile->ppp_username,
	         profile->ppp_passwd,
	         profile->pdp_type,
	         profile->pdp_select,
	         profile->pdp_addr,
	         profile->dns_mode,
	         profile->prefer_dns_manual,
	         profile->standby_dns_manual);

	cfg_set(cfg_name, cfg_value);

	sprintf(cfg_name, "ipv6_APN_config%d", index);
	snprintf(cfg_value, APNCONFIG_MEMORY,
	         "%s($)%s($)%s($)%s($)%s($)%s($)%s($)%s($)%s($)%s($)%s($)%s($)%s($)",
	         ipv6profile->profile_name,
	         ipv6profile->apn_name,
	         ipv6profile->apn_select,
	         ipv6profile->dial_num,
	         ipv6profile->ppp_auth_mode,
	         ipv6profile->ppp_username,
	         ipv6profile->ppp_passwd,
	         ipv6profile->pdp_type,
	         ipv6profile->pdp_select,
	         ipv6profile->pdp_addr,
	         ipv6profile->dns_mode,
	         ipv6profile->prefer_dns_manual,
	         ipv6profile->standby_dns_manual);

	cfg_set(cfg_name, cfg_value);

	return;
}

static char *split_str_by_sep(char *src, char *sep, char *dst, int len)
{
	char *pos_begin = NULL;
	char *pos_end = NULL;

	if (NULL == src || NULL == sep || NULL == dst || 0 == len) {
		return NULL;
	}

	pos_begin = src;
	pos_end = strstr(pos_begin, sep);

	if (NULL == pos_end) {
		return NULL;
	}

	if (len <= (pos_end - pos_begin)) {
		return NULL;
	}

	strncpy(dst, pos_begin, pos_end - pos_begin);
	pos_end += strlen(sep);
	return pos_end;
}

void zte_mgmt_set_login_timemark()
{
	char login_timemark[NV_ITEM_STRING_LEN_20] = {0};
	long now_timemark = time(0);

	sprintf(login_timemark, "%ld", now_timemark);
	zte_web_write(NV_USER_LOGIN_TIMEMARK, login_timemark);
	slog(MISC_PRINT, SLOG_NORMAL,"[SNTP] [manual] timemark, user_login_timemark=%ld\n", now_timemark);
}

int manual_set_time()
{
	struct timeval tp;
	time_t nowtime;
	time_t SynBeforeTime = 0;
	int ltime = 0, mtime = 0, ntime = 0;
	char SynSystemTotal[16] = {0};
	char SynPppTotal[16] = {0};
	char SynSecond[16] = {0};
	int year = 0, month = 0, day = 0;
	int hour = 0, minute = 0, second = 0;
	int days = 0;
	int LeapYear = 0;
	int CommonYear = 0;
	int YearTotal = 0;
	int count;
	int flag = 0;
	int i;
	int ret = 0;
	char buf[NV_ITEM_STRING_LEN_10] = {0};
	char ppp_status[NV_ITEM_STRING_LEN_50] = {0};

	cfg_get_item("manual_time_year", buf, sizeof(buf));
	year = atoi(buf);

	memset(&buf, 0, sizeof(buf));
	cfg_get_item("manual_time_month", buf, sizeof(buf));
	month = atoi(buf);

	memset(&buf, 0, sizeof(buf));
	cfg_get_item("manual_time_day", buf, sizeof(buf));
	day = atoi(buf);

	memset(&buf, 0, sizeof(buf));
	cfg_get_item("manual_time_hour", buf, sizeof(buf));
	hour = atoi(buf);

	memset(&buf, 0, sizeof(buf));
	cfg_get_item("manual_time_minute", buf, sizeof(buf));
	minute = atoi(buf);

	memset(&buf, 0, sizeof(buf));
	cfg_get_item("manual_time_second", buf, sizeof(buf));
	second = atoi(buf);

	YearTotal = year - 1970;
	for (count = 0; count < YearTotal; count++) {
		i = 1971 + count;
		if (i % 4 == 0 && i % 100 != 0) {
			if (YearTotal == count + 1) {
				CommonYear++;
			} else {
				LeapYear++;
			}
			continue;
		} else if (i % 400 == 0) {
			if (YearTotal == count + 1) {
				CommonYear++;
			} else {
				LeapYear++;
			}
			continue;
		} else {
			CommonYear++;
		}
	}

	/****************flag==1表示当年为闰年*************************/
	if (year % 4 == 0 && year % 100 != 0) {
		flag = 1;
	} else if (year % 400 == 0) {
		flag = 1;
	} else {
		flag = 0;
	}

	switch (month - 1) {
	case 0:
		days = day;
		break;
	case 1:
		days = 31 + day;
		break;
	case 2:
		days = 31 + 28 + day;
		break;
	case 3:
		days = 31 + 28 + 31 + day;
		break;
	case 4:
		days = 31 + 28 + 31 + 30 + day;
		break;
	case 5:
		days = 31 + 28 + 31 + 30 + 31 + day;
		break;
	case 6:
		days = 31 + 28 + 31 + 30 + 31 + 30 + day;
		break;
	case 7:
		days = 31 + 28 + 31 + 30 + 31 + 30 + 31 + day;
		break;
	case 8:
		days = 31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + day;
		break;
	case 9:
		days = 31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + day;
		break;
	case 10:
		days = 31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + day;
		break;
	case 11:
		days = 31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30 + day;
		break;
	}
	if ((month - 1) >= 2 && 1 == flag) {
		days++;
	}


	nowtime = COMMONYEARSEC * CommonYear + LEAPYEARSEC * LeapYear + (days - 1) * DAYSEC + hour * 3600 + minute * 60 + second;
	tp.tv_usec = 0;
	tp.tv_sec = nowtime;

	time(&SynBeforeTime);
	slog(MISC_PRINT, SLOG_DEBUG, "Now time  is %d", SynBeforeTime); /*lint !e26*/

	cfg_get_item("syn_after_time", SynSecond, sizeof(SynSecond));
	sscanf(SynSecond, "%d", &ltime);

	cfg_get_item("syn_system_total", SynSystemTotal, sizeof(SynSystemTotal));

	sscanf(SynSystemTotal, "%d", &mtime);
	if (0 != mtime) {
		mtime += (SynBeforeTime - ltime);
	} else {
		mtime = (SynBeforeTime - JAN_2000);
	}
	sprintf(SynSystemTotal, "%d", mtime);
	cfg_set("syn_system_total", SynSystemTotal);

	cfg_get_item("ppp_status", ppp_status, sizeof(ppp_status));
	if ((0 == strcmp(ppp_status, "ppp_connected")) || (0 == strcmp(ppp_status, "ipv6_connected")) || (0 == strcmp(ppp_status, "ipv4_ipv6_connected"))) {
		cfg_get_item("syn_ppp_total", SynPppTotal, sizeof(SynPppTotal));

		sscanf(SynPppTotal, "%d", &mtime);
		if (0 != mtime) {
			mtime += (SynBeforeTime - ltime);
		} else {
			memset(&SynSecond, 0, sizeof(SynSecond));
			cfg_get_item("ppp_start_time", SynSecond, sizeof(SynSecond));

			sscanf(SynSecond, "%d", &ntime);
			mtime = SynBeforeTime - ntime;
		}
		sprintf(SynPppTotal, "%d", mtime);
		cfg_set("syn_ppp_total", SynPppTotal);
		cfg_set("syn_order_flag", "ppp_on");
	} else {
		cfg_set("syn_order_flag", "ppp_off");
		cfg_set("syn_ppp_total", "0");
	}

	if (0 != settimeofday(&tp, NULL)) {
		slog(MISC_PRINT, SLOG_ERR, "set time of system wrong"); /*lint !e26*/
		return -1;
	}
	system("zte-rtc-clock");

	ret = ipc_send_message(MODULE_ID_WEB_CGI, MODULE_ID_AT_CTL, MSG_CMD_SYCTIME_SET_REQ, 0, NULL, 0);
	if (0 != ret) {
		slog(MISC_PRINT, SLOG_ERR, "sync time to cp fail!");
	}

	memset(&buf, 0, sizeof(buf));
	cfg_get_item("outdate_delete", buf, sizeof(buf));
	if (0 == strcmp(buf, "1")) {
		ipc_send_message(MODULE_ID_WEB_CGI, MODULE_ID_SMS, MSG_CMD_SMS_OUTDATE_CHECK, 0, NULL, 0);
	}

	sprintf(SynSecond, "%d", (int)(time((time_t *)NULL)));
	cfg_set("syn_after_time", SynSecond);
	//cfg_set("systime_mode", "manual");
	cfg_set("need_clear_traffic_data", "yes");
	zte_mgmt_set_login_timemark();
	return 0;
}
char * timei2s(int itime)
{
	static char buf[64];
	int hours, min, sec;
	memset(buf, 0, 64);

	hours = itime / 3600;
	min   = (itime / 60) % 60;
	sec   = itime % 60;

	sprintf(buf, "%d:%02d:%02d", hours, min, sec);
	return buf ;
}
void timeen2ch(char *time)
{
	char timetmp[128] = {0};
	char year[5] = {0};
	char month[4] = {0};
	char day[3] = {0};
	char weekday[4] = {0};
	char nowtime[16] = {0};
	char hour[4] = {0};
	char second[4] = {0};
	char minute[4] = {0};
	//int imonth;
	char imonth[4] = {0};
	strcpy(timetmp, time);
	char *loc = timetmp;
	char *tmp = NULL;
	memset(time, 0, 128);
	char str_buf[10] = {0};
	int temp;
	int month_temp;

	while (*loc == ' ') {
		loc++;
	}
	tmp = strtok(loc, " ");
	strcpy(weekday, tmp);
	tmp = strtok(NULL, " ");
	strcpy(month, tmp);
	tmp = strtok(NULL, " ");
	strcpy(day, tmp);
	slog(MISC_PRINT, SLOG_DEBUG, "strlen(tmp)=%d", strlen(tmp)); /*lint !e26*/
	if (strlen(tmp) == 1) {
		temp = atoi(tmp);
		slog(MISC_PRINT, SLOG_DEBUG, "====temp=%d", temp); /*lint !e26*/
		sprintf(day, "0%d", temp);
		slog(MISC_PRINT, SLOG_DEBUG, "====day=%s", day); /*lint !e26*/
	} else {
		strcpy(day, tmp);
	}
	tmp = strtok(NULL, " ");
	strcpy(nowtime, tmp);
	tmp = strtok(NULL, " ");
	strncpy(year, tmp, 4);
	if (0 == strcmp("Jan", month)) {
		strcpy(imonth, "01");
	} else if (0 == strcmp("Feb", month)) {
		strcpy(imonth, "02");
	} else if (0 == strcmp("Mar", month)) {
		strcpy(imonth, "03");
	} else if (0 == strcmp("Apr", month)) {
		strcpy(imonth, "04");
	} else if (0 == strcmp("May", month)) {
		strcpy(imonth, "05");
	} else if (0 == strcmp("Jun", month)) {
		strcpy(imonth, "06");
	} else if (0 == strcmp("Jul", month)) {
		strcpy(imonth, "07");
	} else if (0 == strcmp("Aug", month)) {
		strcpy(imonth, "08");
	} else if (0 == strcmp("Sep", month)) {
		strcpy(imonth, "09");
	} else if (0 == strcmp("Oct", month)) {
		strcpy(imonth, "10");
	} else if (0 == strcmp("Nov", month)) {
		strcpy(imonth, "11");
	} else if (0 == strcmp("Dec", month)) {
		strcpy(imonth, "12");
	}
	sprintf(time, "%s-%s-%s&nbsp;&nbsp;&nbsp;%s", year, imonth, day, nowtime);
	slog(MISC_PRINT, SLOG_DEBUG, "====nowtime=%s", nowtime); /*lint !e26*/


	tmp = strtok(nowtime, ":");
	strcpy(hour, tmp);
	slog(MISC_PRINT, SLOG_DEBUG, "====hour=%s", hour); /*lint !e26*/
	tmp = strtok(NULL, ":");
	strcpy(minute, tmp);
	slog(MISC_PRINT, SLOG_DEBUG, "====minute=%s", minute); /*lint !e26*/
	tmp = strtok(NULL, ":");
	strcpy(second, tmp);
	slog(MISC_PRINT, SLOG_DEBUG, "====second=%s", second); /*lint !e26*/
	cfg_set("sntp_hour", hour);
	cfg_set("sntp_minute", minute);
	cfg_set("sntp_second", second);

	cfg_set("sntp_year", year);
	cfg_set("sntp_month", imonth);
	cfg_set("sntp_day", day);
}
void datastatic(char *rec, char *send, char *onlinetime, char *runtime, char *localtime)
{
	char         Receive_Amount[20] = {0};
	char		 Transmit_Amount[20] = {0};
	char         syn_system_total[20] = {0};
	char         syn_after_time[20] = {0};
	char         syn_ppp_total[20] = {0};
	char         ppp_start_time[20] = {0};
	char		 timestr[128] = {0};
	int  	     mtime;
	int			 ltime;
	int			 ntime;
	int          otime;
	time_t 		 timenow;
	char  ppp_status[NV_ITEM_STRING_LEN_50] = {0};
	char  buf[NV_ITEM_STRING_LEN_20] = {0};

	cfg_get_item("syn_system_total", syn_system_total, sizeof(syn_system_total));

	sscanf(syn_system_total, "%d", &ltime);
	cfg_get_item("syn_after_time", syn_after_time, sizeof(syn_after_time));

	sscanf(syn_after_time, "%d", &ntime);
	cfg_get_item("ppp_start_time", ppp_start_time, sizeof(ppp_start_time));

	sscanf(ppp_start_time, "%d", &otime);

	cfg_get_item("ppp_status", ppp_status, sizeof(ppp_status));
	if ((0 != strcmp(ppp_status, "ppp_connected")) && (0 != strcmp(ppp_status, "ipv6_connected")) && (0 != strcmp(ppp_status, "ipv4_ipv6_connected"))) {
		cfg_set("syn_ppp_total", "0");
	}

	cfg_get_item("syn_ppp_total", syn_ppp_total, sizeof(syn_ppp_total));

	sscanf(syn_ppp_total, "%d", &mtime);

	time(&timenow);
	if (ntime == 0) {
		strcpy(onlinetime, timei2s(timenow - otime));
		strcpy(runtime, timei2s(timenow - JAN_2000));
	} else {
		cfg_get_item("syn_order_flag", buf, sizeof(buf));
		if (0 == strcmp(buf, "ppp_on")) {
			strcpy(onlinetime, timei2s(timenow - ntime + mtime));
		} else {
			strcpy(onlinetime, timei2s(timenow - otime));
		}
		strcpy(runtime, timei2s(timenow - ntime + ltime));
	}
	slog(MISC_PRINT, SLOG_DEBUG, "----runtime=%s", runtime); /*lint !e26*/
	strcpy(timestr, ctime(&timenow));
	slog(MISC_PRINT, SLOG_DEBUG, "----timestr=%s", timestr); /*lint !e26*/
	timeen2ch(timestr);
	strcpy(localtime, timestr);
	slog(MISC_PRINT, SLOG_DEBUG, "----localtime=%s", localtime); /*lint !e26*/

}

void zte_goform_sntp_getdatastatic_process(webs_t wp)
{
	char receive[30], send[30], onlinetime[64], runtime[64], localtime[128];

	memset(receive, 0, 30);
	memset(send, 0, 30);
	memset(onlinetime, 0, 64);
	memset(runtime, 0, 64);
	memset(localtime, 0, 128);

	datastatic(receive, send, onlinetime, runtime, localtime);

	websWrite(wp, T("{\"receive\":\"%s\",\"send\":\"%s\",\"onlinetime\":\"%s\",\"runtime\":\"%s\",\"localtime\":\"%s\"}"), receive, send, onlinetime, runtime, localtime);
}


typedef struct LOG_FILES {
	char filename[64];
} LOG_FILES;

int start_diaglog()
{
	int ret = 0;
	slog(MISC_PRINT, SLOG_NORMAL, "start_diaglog"); /*lint !e26*/
	system("/sbin/diaglog &");
	return ret;
}

int stop_diaglog()
{
	int ret = 0;
	slog(MISC_PRINT, SLOG_NORMAL, "get killall SIGINT"); /*lint !e26*/
	system("killall -9 diaglog");//SIGINT=2
	return ret;
}

int del_diaglog(char *fllename)
{
	slog(MISC_PRINT, SLOG_NORMAL, "del log name=%s", fllename); /*lint !e26*/
	int ret = 0;
	char cmd[128];
	sprintf(cmd, "rm %s", fllename);
	system(cmd);
	return ret;
}


int getDiaglogFile(LOG_FILES *files, int n_files)
{
	char ext[] = ".dlf";
	int i = 0;

	DIR *dp;
	struct dirent *entry;

	dp = opendir(DIAGLOG_DATA_PATH);
	if (dp == NULL) {
		slog(MISC_PRINT, SLOG_ERR, "dp==NULL");
		return 0;
	}

	i = 0;
	while ((entry = readdir(dp)) != NULL && (i < n_files)) {
		{
			int len = strlen(entry->d_name);
			if ((len > strlen(ext)) && !strcmp(entry->d_name + len - strlen(ext), ext)) { //*.dlf *.zm
				strcpy(files[i].filename, entry->d_name);
				slog(MISC_PRINT, SLOG_DEBUG, "entry->d_name=%s", entry->d_name);
				i++;
			}
		}
	}
	closedir(dp);

	return i;

}


void getdialog_url(int eid, webs_t wp, int argc, char_t **argv)
{
	char filename[128] = {0};
	LOG_FILES files[10];
	int n, i;
	char *p = filename;
	n = getDiaglogFile(files, 10);
	i = 0;
	if (n > 0) {
		while (i < n) {
			slog(MISC_PRINT, SLOG_DEBUG, "i=%d", i); /*lint !e26*/
			strcpy(p, files[i].filename);
			p += strlen(files[i].filename);
			*p = ';';
			p++;
			slog(MISC_PRINT, SLOG_DEBUG, "filename=%s", files[i].filename); /*lint !e26*/
			i++;
		}
		websWrite(wp, T(filename));
	} else {
		websWrite(wp, T(""));
	}
}

/******************************************************
* Function: zte_fota_get_upgrade_result
* Description: get fota upgrade result
* Input:  http request info
* Output:
* Return:
* Others:
* Modify Date    Version   Author         Modification
* 2013/03/14        V1.0     chenyi        create
*******************************************************/
void zte_fota_get_upgrade_result(webs_t wp)
{
	zte_topsw_state_e_type nv_result = ZTE_NVIO_MAX;

	char nv_upgrade_result[NV_ITEM_STRING_LEN_50] = {0};

	nv_result = zte_web_read(NV_FOTA_UPGRADE_RESULT, nv_upgrade_result);
	if (ZTE_NVIO_DONE != nv_result) {
		slog(MISC_PRINT, SLOG_ERR,"read nv [%s] fail.\n", NV_FOTA_UPGRADE_RESULT);
	}

	web_feedback_header(wp);
	(void)websWrite(wp, T("{\"%s\":\"%s\"}"), FOTA_UPGRADE_RESULT, nv_upgrade_result);

	if ((0 == strcmp("success", nv_upgrade_result)) || (0 == strcmp("fail", nv_upgrade_result))) {
		(void)zte_web_write(NV_FOTA_UPGRADE_RESULT, ""); //reset

		// 重置NV后保存该NV参数，避免因快速重启，NV新值未保存导致的更新结果重复上报问题
		//cfg_save();
	}

}

#if 0
/******************************************************
* Function: zte_fota_get_dp_pack_info
* Description: get fota dp info
* Input:  http request info
* Output:
* Return:
* Others:
* Modify Date    Version   Author         Modification
* 2013/01/06        V1.0     chenyi        create
*******************************************************/
void zte_fota_get_dp_pack_info(webs_t wp)
{
	char dp_total_size[NV_ITEM_STRING_LEN_50] = {0};
	char dp_download_size[NV_ITEM_STRING_LEN_50] = {0};

	//get dp pack info
	(void)zte_web_read(NV_FOTA_PKG_TOTAL_SIZE, dp_total_size);
	(void)zte_web_read(NV_FOTA_PKG_DL_SIZE, dp_download_size);

	web_feedback_header(wp);
	(void)websWrite(wp, T("{\"%s\":\"%s\",\"%s\":\"%s\"}"), \
	                NV_FOTA_PKG_TOTAL_SIZE, dp_total_size, NV_FOTA_PKG_DL_SIZE, dp_download_size);
}
#endif
/**********************************************************************
* Function:         zte_fota_update
* Description:      send msg to fota update module
* Input:            wp:the web para;
* Output:
* Return:
* Others:         add from uFi
* Modify Date   Version     Author          Modification
* -----------------------------------------------
*
**********************************************************************/
int zte_fota_SendMsg2DM(int cmd)
{
	slog(MISC_PRINT, SLOG_NORMAL,"goahead  send cmd=% to fota\n", cmd);
	return ipc_send_message(MODULE_ID_WEB_CGI, MODULE_ID_DM_WEBUI_AT, cmd, 0, NULL,  0);
}

void zte_fota_update(webs_t wp)
{
	slog(MISC_PRINT, SLOG_NORMAL, T("UFIx User check new version!\n"));
	char *update_action = NULL;
	zte_topsw_state_e_type nv_result = ZTE_NVIO_MAX;
	char new_version_state[NV_ITEM_STRING_LEN_150] = {0};
	BOOL new_state_version_check = FALSE;
	int iDlPara = -1;

	if (NULL == wp) {
		return;
	}

	update_action = websGetVar(wp, T("select_op"), T(""));
	slog(MISC_PRINT, SLOG_DEBUG,"websGetVar FOTA upgrade action[%s].\n", update_action);

	if ('\0' == *update_action) {
		zte_write_result_to_web(wp, FAILURE);
		return;
	}


	//与deviceUi的互斥保护处理
	zte_web_read(NV_FOTA_NEW_VERSION_STATE, new_version_state);

	if ((0 == strcmp(update_action, FOTA_ACTION_CONFIRM_DOWNLOAD)) || (0 == strcmp(update_action, FOTA_ACTION_CANCEL_DOWNLOAD))) {
		if ((0 == strcmp(new_version_state, HAS_OPTIONAL)) || (0 == strcmp(new_version_state, HAS_CRITICAL))) {
			new_state_version_check = TRUE;
		}
		if (!new_state_version_check) {
			zte_write_result_to_web(wp, FAILURE);
			return;
		}
	}

	if (0 == strcmp(update_action, FOTA_ACTION_CONFIRM_DOWNLOAD)) {
		zte_web_write(NV_FOTA_UPGRADE_SELECTOR, "accept");
		//zte_web_write(NV_FOTA_NEED_USER_CONFIRM,"0");
		slog(MISC_PRINT, SLOG_DEBUG,"web :zte_fota_update. update_action = FOTA_ACTION_CONFIRM_DOWNLOAD, set NV_FOTA_UPGRADE_SELECTOR accept!\n");
		//zte_fota_SendMsg2DM(MSG_CMD_FOTA_WEBUI_START_DOWNLOAD);
		//ipc_send_message(MODULE_ID_WEB_CGI, MODULE_ID_DM_WEBUI_AT,MSG_CMD_FOTA_WEBUI_START_DOWNLOAD, 2, "1", 0);
		iDlPara = 1;
		ipc_send_message(MODULE_ID_AT_CTL, MODULE_ID_DM_WEBUI_AT, MSG_CMD_FOTADL_REQ, sizeof(iDlPara), (UCHAR *)(&iDlPara), 0);
	} else if (0 == strcmp(update_action, FOTA_ACTION_CANCEL_DOWNLOAD)) {
		(void)zte_web_write(NV_FOTA_UPGRADE_SELECTOR, "cancel");
		zte_web_write(NV_FOTA_NEW_VERSION_STATE, IDLE);
		zte_web_write(NV_FOTA_CURR_UPGRADE_STATE, IDLE);

		slog(MISC_PRINT, SLOG_DEBUG,"web :zte_fota_update. update_action = FOTA_ACTION_CANCEL_DOWNLOAD, set NV_FOTA_UPGRADE_SELECTOR cancel!\n");
		zte_write_result_to_web(wp, SUCCESS);
		return; //no need to send msg to ota module
	} else if (0 == strcmp(update_action, "check")) {
		slog(MISC_PRINT, SLOG_DEBUG,"goahead :zte_fota_update.  begin to check!!\n");
		zte_fota_SendMsg2DM(MSG_CMD_FOTA_WEBUI_START_FOTA);
	}

	zte_write_result_to_web(wp, SUCCESS);
}

/**********************************************************************
* Function:         zte_fota_settings
* Description:      to set fota settings
* Input:            wp:the web para;dm_nextpollingtime;dm_pollingcycle;pollingswitch;
* Output:
* Return:
* Others:         add from uFi
* Modify Date   Version     Author          Modification
* -----------------------------------------------
*
**********************************************************************/
void zte_fota_settings(webs_t wp)
{
	char_t *updateMode = NULL;
	char_t *allowRoamingUpdate = NULL;
	zte_topsw_state_e_type status = ZTE_NVIO_MAX;

	if (NULL == wp) {
		return;
	}

	updateMode = websGetVar(wp, T("UpgMode"), T(""));
	allowRoamingUpdate = websGetVar(wp, T("UpgRoamPermission"), T(""));

	slog(MISC_PRINT, SLOG_DEBUG,"zte_fota_settings web para:[updateMode] is [%s].\n", updateMode);
	slog(MISC_PRINT, SLOG_DEBUG,"zte_fota_settings web para:[allowRoamingUpdate] is [%s].\n", allowRoamingUpdate);

	if ('\0' == (*updateMode)) {
		zte_write_result_to_web(wp, FAILURE);
		return;
	}

	status = zte_web_write("fota_allowRoamingUpdate", allowRoamingUpdate);
	if (ZTE_NVIO_DONE != status) {
		zte_write_result_to_web(wp, FAILURE);
		return;
	}


	status = zte_web_write("fota_updateMode", updateMode);
	if (ZTE_NVIO_DONE != status) {
		zte_write_result_to_web(wp, FAILURE);
		return;
	}
	zte_fota_SendMsg2DM(MSG_CMD_FOTA_WEBUI_CHANGE_PARAMETER);
	zte_write_result_to_web(wp, SUCCESS);
}

#if 0

/**********************************************************************
* Function:         zte_get_fota_settings
* Description:      to get fota  settings
* Input:            wp:the web para;
* Output:
* Return:
* Others:         add from uFi
* Modify Date   Version     Author          Modification
* -----------------------------------------------
*
**********************************************************************/
void zte_get_fota_settings(webs_t wp)
{
	zte_topsw_state_e_type status = ZTE_NVIO_MAX;

	char_t updateMode[NV_ITEM_STRING_LEN_20] = {0};
	char_t updateIntervalDay[NV_ITEM_STRING_LEN_20] = {0};
	char_t allowRoamingUpdate[NV_ITEM_STRING_LEN_20] = {0};

	status = zte_web_read("fota_allowRoamingUpdate", allowRoamingUpdate);
	if (ZTE_NVIO_DONE != status) {
		zte_write_result_to_web(wp, FAILURE);
		return;
	}

	status = zte_web_read("fota_updateIntervalDay", updateIntervalDay);
	if (ZTE_NVIO_DONE != status) {
		zte_write_result_to_web(wp, FAILURE);
		return;
	}

	status = zte_web_read("fota_updateMode", updateMode);
	if (ZTE_NVIO_DONE != status) {
		zte_write_result_to_web(wp, FAILURE);
		return;
	}

	WEBLOG("zte_get_fota_settings:[updateMode] is [%s].\n", updateMode);
	WEBLOG("zte_get_fota_settings:[updateIntervalDay] is [%s].\n", updateIntervalDay);
	WEBLOG("zte_get_fota_settings:[allowRoamingUpdate] is [%s].\n", allowRoamingUpdate);

	web_feedback_header(wp);
	zte_rest_cmd_write_head(wp);

	if ('\0' != *updateMode)
		zte_rest_cmd_write(wp, "UpgMode", updateMode, 1);
	else
		zte_rest_cmd_write(wp, "UpgMode", "0", 1);

	if ('\0' != *updateIntervalDay)
		zte_rest_cmd_write(wp, "UpgIntervalDay", updateIntervalDay, 1);
	else
		zte_rest_cmd_write(wp, "UpgIntervalDay", "1", 1);

	if ('\0' != *allowRoamingUpdate)
		zte_rest_cmd_write(wp, "UpgRoamPermission", allowRoamingUpdate, 0);
	else
		zte_rest_cmd_write(wp, "UpgRoamPermission", "0", 0);

	zte_rest_cmd_write_foot(wp);
}


/******************************************************
* Function: zte_fota_get_update_info
* Description: get fota update info
* Input:  http request info
* Output:
* Return:
* Others:
* Modify Date    Version   Author         Modification
* 2013/01/06        V1.0     chenyi        create
*******************************************************/
void zte_fota_get_update_info(webs_t wp)
{
	char version[NV_ITEM_STRING_LEN_64 + 1] = {0};

	(void)zte_web_read(NV_FOTA_VERSION, version);

	web_feedback_header(wp);
	(void)websWrite(wp, T("{\"%s\":\"%s\"}"), FOTA_UPDATE_VERSION, version);
}
#endif

void zte_wan_lock_frequency_process(webs_t wp)
{
	slog(MISC_PRINT, SLOG_NORMAL, "zte_wan_lock_frequency_process coming");
	CHAR *actionlte = NULL;
	CHAR *uarfcnlte = NULL;
	CHAR *cellParaIdlte = NULL;

	/* get value from web page */
	actionlte = websGetVar(wp, T("actionlte"), T(""));
	uarfcnlte = websGetVar(wp, T("uarfcnlte"), T(""));
	cellParaIdlte = websGetVar(wp, T("cellParaIdlte"), T(""));

	cfg_set("actionlte", actionlte);
	cfg_set("uarfcnlte", uarfcnlte);
	cfg_set("cellParaIdlte", cellParaIdlte);
	ipc_send_message(MODULE_ID_WEB_CGI, MODULE_ID_AT_CTL, MSG_CMD_CELL_LOCK_REQ, 0, NULL, 0);
	zte_write_result_to_web(wp, SUCCESS);
}

void zte_setLastLoginTime()
{
	int curr_time = 0;
	char login_time[LOGIN_RECORD_TIME] = {0};
	curr_time = zte_web_getCurrentTime();
	sprintf(login_time, "%d", curr_time);
	(void)zte_web_write(NV_LAST_LOGIN_TIME, login_time);
}

int zte_checkLoginTime()
{
	char last_record_time[LOGIN_RECORD_TIME] = {0};
	int last_record_time_num = 0;
	int curr_record_time_num = 0;
	int lock_time = 0;
	zte_web_read(NV_LAST_LOGIN_TIME, last_record_time);
	last_record_time_num = atoi(last_record_time);
	curr_record_time_num = zte_web_getCurrentTime();
	lock_time = curr_record_time_num - last_record_time_num;
	if (lock_time < LOGIN_FAIL_LOCK_TIME) {
		return -1;
	} else {
		return 1;
	}
}

void zte_reduct_login_times()
{
	char psw_fail_num_str[10] = {0};
	int login_times = 0;
	zte_web_read("psw_fail_num_str", psw_fail_num_str);
	login_times = atoi(psw_fail_num_str);
	login_times--;
	sprintf(psw_fail_num_str, "%d", login_times);
	(void)zte_web_write("psw_fail_num_str", psw_fail_num_str);
}
/******************************************************
* Function: void zte_password_check(webs_t wp)
* Description:  password check when login
* Input:  HTTP page info
* Output:
* Return:  none
* Others:
* Modify Date    Version   Author         Modification
*
*******************************************************/
psw_check_result_type_t zte_password_check(webs_t wp, char* psw)
{
	int psw_len = 0;
	int psw_fail_num = 0;
	char psw_fail_num_str[10] = {0};
	int check_lock = 0;

	if (NULL == psw) {
		slog(MISC_PRINT, SLOG_DEBUG, "zte_mgmt_login: psw is empty.");
		return PSW_EMPTY;
	}

	slog(MISC_PRINT, SLOG_DEBUG, "zte_mgmt_login:psw:%s", psw);
	//don't foget add the nv psw_fail_num_str
	zte_web_read("psw_fail_num_str", psw_fail_num_str);
	psw_fail_num = atoi(psw_fail_num_str);
	psw_len = strlen(psw);

	if (0 == psw_len) {
		slog(MISC_PRINT, SLOG_ERR, "zte_mgmt_login: psw is empty.");
		return PSW_EMPTY;
	}

	if (psw_fail_num <= 0) {
		check_lock = zte_checkLoginTime(); // check the current time if it is time out
		if (check_lock < 0) {
			slog(MISC_PRINT, SLOG_ERR, "zte_mgmt_login: psw number use out.");
			return PSW_TIME_OUT;
		} else if (check_lock > 0) {
			slog(MISC_PRINT, SLOG_DEBUG, "zte_mgmt_login: login time out, can login.");
			(void)zte_web_write("psw_fail_num_str", LOGIN_FAIL_TIMES);
		}
	}
	zte_setLastLoginTime();  //record current time to nv
	if (LOGIN_PSW_MIN_LEN > psw_len || LOGIN_PSW_MAX_LEN < psw_len) {
		slog(MISC_PRINT, SLOG_ERR, "zte_mgmt_login: psw is too long.");
		return PSW_TOO_LONG;
	} else {
		return PSW_OK;
	}
}

void zte_get_ddns_status(webs_t wp)
{
	char * line = NULL;
	size_t len = 0;
	size_t lsize = 0;
	char path_conf[100] = {0};
	char path_file[500] = {0};
	cfg_get_item("path_conf", path_conf, sizeof(path_conf));
	sprintf(path_file, "%s/inadyn.status", path_conf);
	FILE *proc_file = fopen(path_file, "r");

	if (proc_file == NULL) {
		//websWrite(wp, T("4"));
		websWrite(wp, T("{\"%s\":\"%s\"}"), "getddns_status", "4");
		return;
	}
	if ((lsize = getline(&line, &len, proc_file)) != -1) {
		if (strstr(line, "RC_IP_INVALID_REMOTE_ADDR")) {
			//websWrite(wp, T("2"));
			websWrite(wp, T("{\"%s\":\"%s\"}"), "getddns_status", "2");
		} else if (strstr(line, "RC_DYNDNS_RSP_NOTOK")) {
			//websWrite(wp, T("1"));
			websWrite(wp, T("{\"%s\":\"%s\"}"), "getddns_status", "1");
		} else if (strstr(line, "RC_OK")) {
			//websWrite(wp, T("0"));
			websWrite(wp, T("{\"%s\":\"%s\"}"), "getddns_status", "0");
		} else if (strstr(line, "RC_REGISTERING")) {
			//websWrite(wp, T("3"));
			websWrite(wp, T("{\"%s\":\"%s\"}"), "getddns_status", "3");
		} else {
			//websWrite(wp, T("5"));
			websWrite(wp, T("{\"%s\":\"%s\"}"), "getddns_status", "5");
		}
	} else {
		//websWrite(wp, T("-1"));
		websWrite(wp, T("{\"%s\":\"%s\"}"), "getddns_status", "-1");
	}
	if (line) {
		free(line);
	}
	fclose(proc_file);
	return;

}

void zte_goform_set_work_type(webs_t wp)
{
	char_t *work_type = NULL;
	work_type = websGetVar(wp, T("work_type"), T(""));

	slog(MISC_PRINT, SLOG_DEBUG,"[zyl]zte_goform_set_work_type:work_type->%s\n", work_type);

	if (work_type == NULL) {
		slog(MISC_PRINT, SLOG_ERR,"[zyl]zte_goform_set_work_type:empty!\n");
		zte_write_result_to_web(wp, FAILURE);
		return;
	}
	if ((strncmp(work_type, "0", 1) == 0) || (strncmp(work_type, "1", 1) == 0)) {
		char LanEnable[5] = {0};
		(void)zte_web_read("LanEnable", LanEnable);
		if (strncmp(LanEnable, work_type, 1) == 0) {
			return zte_write_result_to_web(wp, SUCCESS);
		}
		(void)zte_web_write("LanEnable", work_type);
		slog(MISC_PRINT, SLOG_DEBUG,"[zyl]zte_goform_set_work_type:work_type->%s\n", work_type);
		ipc_send_message(MODULE_ID_WEB_CGI, MODULE_ID_MAIN_CTRL, MSG_CMD_RESTART_REQUEST, 0, NULL, 0);
		slog(MISC_PRINT, SLOG_NORMAL,"[zyl]zte_goform_set_work_type:device reboot now\n");
		zte_write_result_to_web(wp, SUCCESS);
		return;
	} else {
		slog(MISC_PRINT, SLOG_ERR,"[zyl]zte_goform_set_work_type:work_type->%s error!\n", work_type);
		zte_write_result_to_web(wp, FAILURE);
		return;
	}
}

/******************************************************
* Function: void zte_goform_set_sample(webs_t wp)
* Description:  example function
* Input:  HTTP page info
* Output:
* Return:  none
* Others:
* Modify Date    Version   Author         Modification
*
*******************************************************/
void zte_goform_set_sample(webs_t wp)
{
	//从WEB请求中获取参数,没有参数则不需要这一步

	// 设置NV或发消息给相应模块
	ipc_send_message(MODULE_ID_WEB_CGI, MODULE_ID_AT_CTL, MSG_CMD_PDP_ACT_REQ, 0, NULL, 0);

	//返回JSON 格式结果，如果需要查询设置结果则需要webui通过获取ppp_status这个NV值来获取
	zte_write_result_to_web(wp, SUCCESS);
	return;
}


