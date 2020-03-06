/**
 * @file zte_web_interface.h
 * @brief Public APIs of Sanechips
 *
 * Copyright (C) 2017 Sanechips Technology Co., Ltd.
 * @author Hong Wu <wu.hong@sanechips.com.cn>
 * @defgroup si_ap_app_webserver_id si_ap_app_webserver
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */


#ifndef ZTE_WEB_INTERFACE_H
#define ZTE_WEB_INTERFACE_H

/*******************************************************************************
 *                           Include header files                              *
 ******************************************************************************/
#include <time.h>
#include "../server/webs.h"
#include "message.h"
//#include "errorcode.h"
#include "cfg_nv_def.h"
#include "cfg_api.h"
#include "zte_web_mgmt.h"
#include <sys/ipc.h>
#include <sys/msg.h>
#include "softap_api.h"
#include "netotherapi.h"
#include "net/zte_web_net_lan.h"
#include "net/zte_web_net_wan.h"
#include "net/zte_web_net_other.h"

/*******************************************************************************
 *                             Macro definitions                               *
 ******************************************************************************/
#define cprintf(fmt, args...) do {  \
    FILE *fp = fopen("/dev/console", "w");  \
    if (fp) {   \
        fprintf(fp, fmt, ## args);  \
        fclose(fp); \
    }   \
} while (0)

#define WEBLOG cprintf
#define IFSTREQUAL(str1, str2) (strcmp((str1), (str2))?0:1)
#define STR_EQUAL(a,b) ( strcmp((char*)a, (char*)b)==0 )
#ifndef uint8
#define uint8 unsigned char
#endif

#define ID "id"
#define NAME "name"
#define SITE "site"
#define SUCCESS "success"
#define FAILURE "failure"
#define EXIST "exist"
#define NOEXIST "noexist"
#define PROCESSING "processing"
#define MSG_INVALID_WEB_PARAM    "MSG_INVALID_WEB_PARAM"
#define ZTE_MC_OK_S  (0)

#define CFG_BUF_LENGTH 512
#define CONFIG_DEFAULT_LENGTH  64

#define DIAGLOG_DATA_PATH    "/etc_ro/web/diaglog"

/*** define about NV ***/
#define NV_ITEM_VALUE_DEFAULT_STRING_LEN 128
#define NV_ITEM_VALUE_BOOLEAN_LEN 1
#define NV_ITEM_VALUE_YES_NO_LEN 5
#define NV_ITEM_VALUE_IP_LEN          20
#define NV_ITEM_MODE_STRING_LEN   25
#define NV_ITEM_ADMIN_STRING_LEN 25
#define NV_ITEM_VALUE_APN_STRING_LEN 400
#define NV_ITEM_VALUE_NW_LIST_STRING_LEN 700
#define NV_ITEM_STRING_LEN_5  5
#define NV_ITEM_STRING_LEN_10  10
#define NV_ITEM_STRING_LEN_20  20
#define NV_ITEM_STRING_LEN_50  50
#define NV_ITEM_STRING_LEN_64  64
#define NV_ITEM_STRING_LEN_150  150
#define NV_ITEM_STRING_LEN_200  200
#define NV_FW_RULE_MAX_LEN_V6 400
#define NV_ITEM_VALUE_STRING_LEN 50
#define NVIO_MAX_LEN 1500
#define NVIO_TMP_LEN 512
#define NVIO_DEFAULT_LEN 50
#define NV_ITEM_VALUE_STRING_LEN 50
#define NV_ITEM_VALUE_MAX_LEN        1024
#define zte_web_write(xx_item, xx_value) zte_nvconfig_write(xx_item, xx_value,(int)strlen(xx_value))
#define zte_web_read(xx_item, xx_value) zte_nvconfig_read(xx_item, xx_value,sizeof(xx_value)) //modified,-1 deleted

/*webui帐号*/
#define LOGIN_SUCCESS "0"
#define LOGIN_FAIL "1"
#define LOGIN_DUPLICATE_USER "2"
#define LOGIN_BAD_PASSWORD "3"
#define LOGIN_ALREADY_LOGGED "4"
#define LOGIN_USER_NAME_NOT_EXSIT  "5"
#define LOGIN_TIMEOUT  0x0fffffff //timeout after webui login 
#define LOGIN_PSW_MIN_LEN 1
#define LOGIN_PSW_MAX_LEN 32
#define LOGIN_FAIL_LOCK_TIME 300//the lock time after  login failed
#define LOGIN_FAIL_TIMES "5"
#define LOGIN_RECORD_TIME  32

#define NV_LANGUAGE "Language"
#define NV_LOGINFO "loginfo"
#define NV_USER_IP_ADDR "user_ip_addr"
#define NV_LOGIN_LOCK_TIME "login_lock_time"
#define NV_LAST_LOGIN_TIME "last_login_time"
#define NV_USER_LOGIN_TIMEMARK "user_login_timemark"

/* 快速设置 */
#define MAX_QUICK_SET_NUM 10

/*流量统计*/
#define ZTE_WEB_DATA_STATISTICS_CLEAR_ALL "ALL"
#define ZTE_WEB_DATA_STATISTICS_CLEAR_TOTAL "TOTAL"
#define ZTE_WEB_DATA_STATISTICS_CLEAR_CURRENT "CURRENT"

//wifi
#define NVIO_WIFI_MAX_LEN  200 /*write or read nv length*/
#define WIFI_PSW_DEFAULT_LENGTH  65
#define WIFI_STATUS_LEN 2
#define WIFI_PSW_MIN_LEN 1
#define WIFI_PSW_MAN_LEN 64
#define WIFI_CONFIG_DEFAULT_LENGTH  128
#define WF_WPS_KEY_INDEX_LEN  2
#define WF_KEY_MODE_LEN  3
#define WF_WEP_KEY_MAX_LEN  27
#define WF_WPA_KEY_MAX_LEN  65
#define WF_ENCRY_TYPE_LEN  16
#define WF_AU_MODE_LEN  16
#define WPS_MODE_PIN "PIN"      //wps mode 
#define WPS_MODE_PBC  "PBC"
#define WPS_MODE_AP_PIN   "APPIN"
#define WF_AU_OPEN            "OPEN"            //wifi auth mode?
#define WF_AU_SHARE           "SHARED"
#define WF_AU_WEPAUTO         "WEPAUTO"
#define WF_AU_WPA             "WPAPSK"
#define WF_AU_WPA2            "WPA2PSK"
#define WF_AU_WPA_WPA2  	  "WPAPSKWPA2PSK"
#define WF_AU_WAPIPSK	      "WAPIPSK"
#define WF_ENCRY_NONE      "NONE"   //encrypt
#define WF_ENCRY_WEP       "WEP"
#define WF_ENCRY_TKIP      "TKIP"
#define WF_ENCRY_CCMP      "CCMP"
#define WF_ENCRY_AES       "AES"
#define WF_ENCRY_TKIP_CCMP "TKIPCCMP"
#define WF_ENCRY_TKIP_AES  "TKIPAES"
#define WIFI_SSID_INDEX "ssid_index"
#define WIFI_STATION_MAC "mac_addr"
#define WIFI_STATION_HOSTNAME "hostname"
#define WIFI_STATION_IPADDR "ip_addr"
#define WIFI_STATION_CONNECTTIME "connect_time"
#define WIFI_STATION_VALIDTIME "valid_time"
#define WIFI_STATION_IPTYPE "ip_type"
#define WIFI_STATION_DEVTYPE "dev_type"
#define WIFI_NV_ITEM_WIFI_SET_FLAGS "wifi_set_flags"
#define NV_WIFI_SCAN_FINISH         "scan_finish"
#define NV_WIFI_WPA_PASS            "WPAPSK1"
#define NV_WIFI_WPA_PASS_ENCODE     "WPAPSK1_encode"
#define NV_WIFI_WPA_PASS_M          "m_WPAPSK1"
#define NV_WIFI_WPA_PASS_M_ENCODE   "m_WPAPSK1_encode"
#define NV_WIFI_COVERAGE  "wifi_coverage"
#define NV_WIFI_WPS_STATE  "WscModeOption"
#define NV_WIFI_WPS_MODE      "wps_mode"
#define NV_WIFI_WPS_SSID       "WPS_SSID"
#define NV_WIFI_WPS_INDEX   "wifi_wps_index"
#define NV_WIFI_WPS_PIN   "wps_pin"
#define NV_WIFI_WPS_AP_PIN   "wifi_ap_pin"
#define NV_WIFI_WPS_DEF_PIN   "wifi_def_pin"
#define CMD_WIFI_STATION_LIST "station_list"
#define CMD_HOSTNAME_LIST "hostNameList"
#define CMD_WIFI_WPS_AP_PIN "wifi_ap_pin"
#define CMD_WIFI_WPS_AP_DEF_PIN "wifi_def_pin"

//wan module
#define ZTE_DAILNUM_LEN 8
#define ZTE_ADDR_LEN 16
#define ZTE_SEL_TYPE 8
#define ZTE_PDP_TYPE_LEN 12
#define ZTE_AUTH_TYPE_LEN 8
#define ZTE_WAN_UMTS_MAX_PROFILE_NAME_LEN 32
#define ZTE_WAN_UMTS_MAX_APN_STRING_LEN 104
#define ZTE_WAN_UMTS_MAX_USERNAME_STRING_LEN 65
#define ZTE_WAN_UMTS_MAX_PASSWD_STRING_LEN 65
#define ZTE_USSD_DATA_TO_WEB_LEN  900
#define CMD_CONNECTION_MODE  "ConnectionMode"

//pbm module
#define CMD_PBM_DATA_INFO "pbm_data_info"
#define CMD_PBM_DATA_TOTAL "pbm_data_total"
#define CMD_PBM_CAPACITY_INFO "pbm_capacity_info"

//sms module
#define CMD_SMS_PAGE_DATA "sms_page_data"
#define CMD_SMS_PARAMETER_INFO "sms_parameter_info"
#define CMD_SMS_STATUS_INFO "sms_cmd_status_info"
#define CMD_SMS_CAPACITY_INFO "sms_capacity_info"
#define CMD_SMS_STATUS_RPT_DATA "sms_status_rpt_data"
#define CMD_SMS_DATA_TOTAL "sms_data_total"
#define CMD_SMS_UNREAD_COUNT "sms_unread_count"
#define CMD_BROADCAST_DATA "broadcast_data"
#define NV_SMS_UNREAD_NUM  "sms_unread_num"

//APN
#define APN_SAVE_AND_SET_DEFAULT "0"
#define APN_ONLY_SET_DEFAULT "1"
#define ZTE_WEB_ACT_AUTO  "auto"
#define ZTE_WEB_ACT_MANUAL "manual"
#define ZTE_WEB_ACT_SAVE "save"
#define ZTE_WEB_ACT_SETDEFAULT "set_default"
#define ZTE_WEB_ACT_SAVE_AND_SETDEFAULT "set_save_and_default"
#define ZTE_WEB_ACT_DELETE "delete"
#define ZTE_WEB_DIAL_MODE_AUTO "auto_dial"
#define ZTE_WEB_DIAL_MODE_MANUAL "manual_dial"
#define ZTE_WEB_DIAL_MODE_DEMAND  "demand_dial"
#define ZTE_WEB_MANUAL_DIAL_ACTION_CONN "connect"
#define ZTE_WEB_MANUAL_DIAL_ACTION_DISCONN "disconnect"

//nv for data limit settings
#define NV_DATA_VOLUME_LIMIT_SWITCH "data_volume_limit_switch" /*0:disable;1:enable*/
#define NV_DATA_VOLUME_LIMIT_UNIT  "data_volume_limit_unit" /*time/data*/
#define NV_DATA_VOLUME_LIMIT_SIZE "data_volume_limit_size"
#define NV_DATA_VOLUME_ALERT_PERCENT "data_volume_alert_percent"
#define DATA_VOLUME_LIMIT_UNIT_TIME "time"
#define DATA_VOLUME_LIMIT_UNIT_DATA "data"
#define NV_HTTPSHARE_STATUS "HTTP_SHARE_STATUS"
#define NV_HTTPSHARE_WR_AUTH "HTTP_SHARE_WR_AUTH"
#define NV_HTTPSHARE_FILE "HTTP_SHARE_FILE"

//other fluxstat
#define NV_DATA_TRAFFIC_SIM_PROVINCE   "sim_home_location"
#define NV_DATA_TRAFFIC_SWITCH              "is_traffic_aline_on"
#define NV_DATA_TRAFFIC_MONTH_TOTAL   "traffic_month_total"
#define NV_DATA_TRAFFIC_MONTH_USED   "traffic_month_used"

//fota module
#define FOTA_ACTION_CANCEL_DOWNLOAD "0"
#define FOTA_ACTION_CONFIRM_DOWNLOAD "1"
#define FOTA_ACTION_DOWNLOADING_USER_CLICK_CANCEL "2"
//#define FOTA_UPDATE_VERSION "version"
#define FOTA_UPGRADE_RESULT "upgrade_result"
//#define FOTA_PACK_SIZE_INFO "pack_size_info"
//#define FOTA_UPDATE_INFO "update_info"
//#define FOTA_SETTINGS_INFO "GetUpgAutoSetting"
//#define FOTA_NEW_VERSION_STATE "new_version_state"

//NET
#define HOSTANME "hostname"
#define MAC "mac"
#define ADDR "ip"
#define DOMAIN "domain"
#define MAC_ADDR "mac_addr"
#define DEVICES "devices"
#define SITELIST "siteList"
#define CMD_LAN_STATION_LIST "lan_station_list"
#define CMD_CHILDREN_DEVICE_LIST "childGroupList"
#define CMD_WHITE_SITE_LIST "site_white_list"
#define CMD_GET_USER_MAC_ADDR  "get_user_mac_addr"
#define CMD_CURRENT_STATICADDR_LIST "current_static_addr_list"
#define CMD_GET_POWERON_TIME     "get_poweron_time"
#define CMD_GET_LAN_DEV_INFO     "get_lan_dev_info" //you wen ti
#define CMD_GETDDNS_STATUS "getddns_status"
#define CMD_USSD_DATA_INFO "ussd_data_info"
#define CMD_GET_DEVICE_MODE  "get_device_mode"

//SD
#define STR_SDCARD_MODE_OPT "sdcard_mode_option"


/*useradded module start*/
#define CMD_GET_SAMPLE "station_list"
/*useradded module end*/

/****************************All the Goform ID************************************/

#define GOFORM_MGMT_SET_WEB_DATA "SET_WEB_DATA"
#define GOFORM_MGMT_GUEST_USER_CONTROL "GUEST_USER_CONTROL"
#define GOFORM_MGMT_SET_EXTERNAL_NV  "SET_EXTERNAL_NV"
/*management start*/
#define GOFORM_MGMT_SET_WEB_LANGUAGE  "SET_WEB_LANGUAGE"
#define GOFORM_MGMT_SET_DEVICEMODE "SET_DEVICE_MODE"
#define GOFORM_MGMT_LOGIN_IP  "LOGIN"
#define GOFORM_MGMT_LOGOUT_IP "LOGOUT"
#define GOFORM_MGMT_CHANGE_PASSWORD  "CHANGE_PASSWORD"
#define GOFORM_MGMT_CHANGE_ACCOUNT  "CHANGE_ACCOUNT"
#define GOFORM_MGMT_RESTORE_FACTORY_SETTINGS  "RESTORE_FACTORY_SETTINGS"
#define GOFORM_MGMT_REBOOT "REBOOT_DEVICE"
#define GOFORM_MGMT_POWEROFF    "TURN_OFF_DEVICE"
#define GOFORM_MGMT_POWER_ON_SPEED "MGMT_CONTROL_POWER_ON_SPEED"
#define GOFORM_MGMT_QUICK_SETUP "QUICK_SETUP_EX"
#define GOFORM_SET_WORK_TYPE "SET_WORK_TYPE"
#define GOFORM_MGMT_SNTP "SNTP"
#define GOFORM_MGMT_SYSLOG  "SYSLOG"
#define GOFORM_HTTP_REDIRECT "HTTP_REDIRECT"
/*management end*/

/*wan module start*/
#define GOFORM_WAN_LOCK_FREQUENCY "LOCK_FREQUENCY"
#define GOFORM_WAN_SET_NETWORK "SET_NETWORK"
#define GOFORM_WAN_SET_CONNECTION_MODE "SET_CONNECTION_MODE"
#define GOFORM_WAN_CONNECT_NETWORK "CONNECT_NETWORK"
#define GOFORM_WAN_DISCONNECT_NETWORK "DISCONNECT_NETWORK"
#define GOFORM_WAN_SCAN_NETWORK "SCAN_NETWORK"
#define GOFORM_WAN_SET_BEARER_PREFERENCE "SET_BEARER_PREFERENCE"
#define GOFORM_WAN_SET_CONN_SETTING "SET_CONN_SETTING"
#define GOFORM_WAN_CANCEL_AUTO_RECONNECT "CANCEL_AUTO_RECONNECT"
#define GOFORM_WAN_UNLOCK_NETWORK "UNLOCK_NETWORK"
//statistics module
#define GOFORM_WAN_RESET_DATA_COUNTER "RESET_DATA_COUNTER"
#define GOFORM_WAN_DATA_LIMIT_SETTING "DATA_LIMIT_SETTING"
#define GOFORM_WAN_DATA_FLOW_CALIBRATION_MANUAL "FLOW_CALIBRATION_MANUAL"
#define GOFORM_SNTP_GETDATASTATIC  "SNTP_Getdatastatic"
//pin,puk module
#define GOFORM_MGMT_ENTER_PIN "ENTER_PIN"
#define GOFORM_MGMT_DISABLE_PIN "DISABLE_PIN"
#define GOFORM_MGMT_ENABLE_PIN "ENABLE_PIN"
#define GOFORM_MGMT_MODIFY_PIN "MODIFY_PIN"
#define GOFORM_MGMT_ENTER_PUK "ENTER_PUK"
#define GOFORM_MGMT_AUTO_PIN "AUTO_PIN"
//pbm
#define GOFORM_PBM_CONTACT_ADD "PBM_CONTACT_ADD"
#define GOFORM_PBM_CONTACT_DEL "PBM_CONTACT_DEL"
//sms module
#define GOFORM_SMS_SET_MSG_CENTER  "SET_MESSAGE_CENTER"
#define GOFORM_SMS_DELETE_SMS  "DELETE_SMS"
#define GOFORM_SMS_DELETE_SMS_ALL  "ALL_DELETE_SMS"
#define GOFORM_SMS_MOVE_TO_SIM  "MOVE_TO_SIM"
#define GOFORM_SMS_SAVE_SMS  "SAVE_SMS"
#define GOFORM_SMS_SEND_SMS  "SEND_SMS"
#define GOFORM_SMS_SET_MSG_READ "SET_MSG_READ"
//ussd module
#define GOFORM_USSD_SEND_CMD "SEND_USSD_CMD"
#define GOFORM_USSD_PROCESS "USSD_PROCESS"
//apn module
#define GOFORM_WAN_APN_PROC_EX  "APN_PROC_EX"
/*wan module end*/

/*wifi module start*/
#define GOFORM_WLAN_SET "SET_WIFI_INFO"
#define GOFORM_WIFI_SET_FOR_SLEEP "SET_WIFI_INFO_FOR_SLEEP"
#define GOFORM_SET_SHOW_SSID_KEY_OLED  "SET_SHOW_SSID_KEY_OLED"
#define GOFORM_WLAN_MAC_FILTER "WIFI_MAC_FILTER"
#define GOFORM_WLAN_WPS_SET "WIFI_WPS_SET"
#define GOFORM_WLAN_SSID1_SET  "SET_WIFI_SSID1_SETTINGS"
#define GOFORM_WLAN_SSID2_SET  "SET_WIFI_SSID2_SETTINGS"
#define GOFORM_WLAN_WIFI_SLEEP_SET "SET_WIFI_SLEEP_INFO"
#define GOFORM_WLAN_WIFI_COVERAGE_SET "SET_WIFI_COVERAGE"
#define GOFORM_WLAN_SET_TSW "SAVE_TSW"
#define GOFORM_PARENT_CONTROL_SET "SAVE_TIME_LIMITED"
//wifi station start
#define GOFORM_WLAN_WIFI_STA_CONTROL "WIFI_STA_CONTROL"
#define GOFORM_WLAN_WIFI_SPOT_PROFILE_UPDATE "WIFI_SPOT_PROFILE_UPDATE"
#define GOFORM_WLAN_SET_STA_CON "WLAN_SET_STA_CON"
#define GOFORM_WLAN_SET_STA_DISCON "WLAN_SET_STA_DISCON"
#define GOFORM_WLAN_SET_STA_REFRESH "WLAN_SET_STA_REFRESH"
/*wifi module end*/

/*router module start*/
#define GOFORM_ROUTER_DEL_IP_PORT_FILETER "DEL_IP_PORT_FILETER"
#define GOFORM_ROUTER_ADD_IP_PORT_FILETER_V4V6 "ADD_IP_PORT_FILETER_V4V6"//设置端口过滤信息
#define GOFORM_ROUTER_DEL_IP_PORT_FILETER_V4V6 "DEL_IP_PORT_FILETER_V4V6"
#define GOFORM_ROUTER_ADD_PORT_FORWARE "FW_FORWARD_ADD"
#define GOFORM_ROUTER_DEL_PORT_FORWARE "FW_FORWARD_DEL"
#define GOFORM_ROUTER_ADD_PORT_MAP "ADD_PORT_MAP"
#define GOFORM_ROUTER_ADD_STATIC_ROUTER "ADD_STATIC_ROUTER" //20180829 lilei add
#define GOFORM_ROUTER_DEL_STATIC_ROUTER "DEL_STATIC_ROUTER"
#define GOFORM_ROUTER_SET_STATIC_PORT "SET_STATIC_PORT"

#define GOFORM_ENABLE_PPTP "ENABLE_PPTP"     //20180907 lilei add
#define GOFORM_SET_PPTP_STATUS "SET_PPTP_STATUS"
#define GOFORM_SAVE_PPTP "SAVE_PPTP"

#define GOFORM_ENABLE_L2TP "ENABLE_L2TP"     //20180907 lilei add
#define GOFORM_SET_L2TP_STATUS "SET_L2TP_STATUS"
#define GOFORM_SAVE_L2TP "SAVE_L2TP"

#define GOFORM_SAVE_NET_LINK "SAVE_NET_LINK" //20181024 lilei

#define GOFORM_ENABLE_RMONITOR "ENABLE_RMONITOR"
#define GOFORM_SAVE_RMONITOR "SAVE_RMONITOR"
#define GOFORM_ENABLE_RUPGRADE "ENABLE_RUPGRADE"
#define GOFORM_SAVE_RUPGRADE "SAVE_RUPGRADE"

#define GOFORM_SAVE_REMOTE_LOG "SAVE_REMOTE_LOG"
#define GOFORM_SAVE_LOCAL_LOG "SAVE_LOCAL_LOG"
#define GOFORM_VIEW_LOG "VIEW_LOG"
#define GOFORM_GEN_LOG_FILE "GEN_LOG_FILE"

#define GOFORM_VIEW_CRON "VIEW_CRON"
#define GOFORM_SAVE_CRON "SAVE_CRON"

#define GOFORM_ROUTER_DEL_PORT_MAP "DEL_PORT_MAP"
#define GOFORM_ROUTER_BASIC_SETTING "BASIC_SETTING"
#define GOFORM_ROUTER_FORWARD_SETTING "VIRTUAL_SERVER"
#define GOFORM_ROUTER_SYSTEM_SECURITY "FW_SYS"
#define GOFORM_ROUTER_DHCP_SETTING "DHCP_SETTING"
#define GOFORM_ROUTER_STATIC_DHCP_SETTING "STATIC_DHCP_SETTING"

#define GOFORM_ROUTER_WAN_SETTING "WAN_SETTING"

#define GOFORM_ROUTER_UPNP_SETTING  "UPNP_SETTING"
#define GOFORM_ROUTER_DMZ_SETTING "DMZ_SETTING"
#define GOFORM_ROUTER_WAN_SETTING "WAN_SETTING"

#define GOFORM_ROUTER_EDIT_HOSTNAME "EDIT_HOSTNAME"
#define GOFORM_BIND_STATIC_ADDRESS_SET "SET_BIND_STATIC_ADDRESS"
#define GOFORM_BIND_STATIC_ADDRESS_ADD "BIND_STATIC_ADDRESS_ADD"
#define GOFORM_BIND_STATIC_ADDRESS_DEL "BIND_STATIC_ADDRESS_DEL"
#define GOFORM_ADD_CHILDREN_DEVICE "ADD_DEVICE"
#define GOFORM_DEL_CHILDREN_DEVICE "DEL_DEVICE"
#define GOFORM_ADD_WHITE_SITE "ADD_WHITE_SITE"
#define GOFORM_REMOVE_WHITE_SITE "REMOVE_WHITE_SITE"
#define GOFORM_URL_FILTER_DELETE "URL_FILTER_DELETE"
#define GOFORM_URL_FILTER_ADD "URL_FILTER_ADD"
#define GOFORM_DDNS "DDNS"
//#define GOFORM_DNS_MODE_SET "SET_DNS_MODE"
//#define GOFORM_DNS_SERVER_SET "SET_DNS_SERVER"

// wan pppoe *
#define GOFORM_SET_OPERATION_MODE    "OPERATION_MODE"
#define GOFORM_SET_WAN_GATEWAYMODE    "WAN_GATEWAYMODE"
#define GOFORM_SET_WAN_GATEWAYMODE_PPPOE    "WAN_GATEWAYMODE_PPPOE"
#define GOFORM_SET_WAN_GATEWAYMODE_DHCP    "WAN_GATEWAYMODE_DHCP"
#define GOFORM_SET_WAN_GATEWAYMODE_STATIC    "WAN_GATEWAYMODE_STATIC"
#define GOFORM_SET_WAN_GATEWAYMODE_AUTO    "WAN_GATEWAYMODE_AUTO"
/*router module end*/

/*httpShare module start*/
#define GOFORM_HTTPSHARE_GETCARD_VAULE "HTTPSHARE_GETCARD_VALUE"
#define GOFORM_HTTPSHARE_ENTERFOLD "HTTPSHARE_ENTERFOLD"
#define GOFORM_HTTPSHARE_NEW "HTTPSHARE_NEW"
#define GOFORM_HTTPSHARE_DEL "HTTPSHARE_DEL"
#define GOFORM_HTTPSHARE_FILE_RENAME "HTTPSHARE_FILE_RENAME"
#define GOFORM_HTTPSHARE_AUTH_SET "HTTPSHARE_AUTH_SET"
#define GOFORM_HTTPSHARE_MODE_SET "HTTPSHARE_MODE_SET"
#define GOFORM_HTTPSHARE_CHECK_FILE "GOFORM_HTTPSHARE_CHECK_FILE"
#define CMD_HTTPSHARE_GETCARD_VAULE "HTTPSHARE_GETCARD_VALUE"
#define CMD_HTTPSHARE_GETCARD_NMEA "HTTPSHARE_GETCARD_NAME"
#define CMD_HTTPSHARE_AUTH_GET "HTTPSHARE_AUTH_GET"
/*httpShare module end*/

/*FOTA module start*/
#define GOFORM_SET_FOTAAUTOUPDATE "IF_UPGRADE"
#define GOFORM_SET_FOTASETTINGS "SetUpgAutoSetting"
/*FOTA module end*/

/*ping test*/
#define GOFORM_PING_DIAGNOSTICS_START "PINT_DIAGNOSTICS_START"
#define GOFORM_PING_DIAGNOSTICS_STOP "PINT_DIAGNOSTICS_STOP"
/*ping test*/

/*useradded module start*/
#define GOFORM_SET_SAMPLE "GOFORM_SET_SAMPLE"
/*useradded module end*/


/****************************All the web pages' URL************************************/
#define ZTE_WEB_PAGE_LOGIN_NAME   			"index.html"
#define ZTE_WEB_MOBILE_PAGE_LOGIN_NAME      "mobile.html"

#define ZTE_WEB_PAGE_MSG                	"/message.asp"
#define ZTE_WEB_PAGE_NET_CONNECT       		"/air_network/net_connect.asp"
#define ZTE_WEB_PAGE_PPP_CONNECTING  		"/air_network/pppconnect.asp"
#define ZTE_WEB_PAGE_PPP_DISCONNECT  		"/air_network/pppdisconnect.asp"

/*******************************************************************************
 *                             Type definitions                                *
 ******************************************************************************/
/**
 * @brief time
 * @param sec second
 * @param usec Microsecond
 * @note
 * @warning
 */
struct os_time {
	long sec;
	long usec;
};


/**
 * @brief goform/cmd table's struct
 * @param goform_id the message id extract from URL
 * @param proc_func the handler of this goform_id
 * @note
 * @warning
 */
typedef struct web_goform_struct {
	char goform_id[50];
	void (*proc_func)(webs_t wp);
} web_goform_type;

/*******************************************************************************
 *                       Global variable declarations                          *
 ******************************************************************************/
typedef enum _data_safe_result_type_t {
	DATA_NO_SAFE = 0,
	DATA_SAFE = 1
} data_safe_result_type_t;

typedef enum {
	ZTE_NVIO_FAIL = 0,
	ZTE_NVIO_DONE = 1,
	ZTE_NVIO_BUSY = 2,
	ZTE_NVIO_BADCMD = 3,
	ZTE_NVIO_MAX
} zte_topsw_state_e_type;

//wifi
typedef enum {
	ZTE_WLAN_SSID_SET = 0x1,
	ZTE_WLAN_WIRELESS_MODE_SET = 0x2,
	ZTE_WLAN_BROADCAST_SET = 0x4,
	ZTE_WLAN_CHANNEL_SET = 0x8,
	ZTE_WLAN_MAX_ACCESS_NUM_SET = 0x10,
	ZTE_WLAN_AP_ISOLATION_SET = 0x20,
	ZTE_WLAN_COUNTRY_SET = 0x40,
	ZTE_WLAN_DATA_RATE_SET = 0x80,
	ZTE_WLAN_POWER_SET = 0x100,
	ZTE_WLAN_ACL_SET = 0x200,
	ZTE_WLAN_WPS_SET = 0x400,
	ZTE_WLAN_BASIC_SECURITY_SET = 0x1000,
	ZTE_WLAN_SLP_TIME_SET = 0x4000,
	ZTE_WLAN_ON_OFF_SET = 0x8000,
	ZTE_WLAN_SET_AP_MSSID = 0x10000,
	ZTE_WLAN_SET_AP = 0x100000,
	ZTE_WLAN_SET_AP_SLEEPTIMER = 0x200002,
	ZTE_WLAN_SET_STA_REFRESH = 0x400001,
	ZTE_WLAN_SET_STA_C0N = 0x400002,
	ZTE_WLAN_SET_STA_DISCON = 0x400004,
	ZTE_WLAN_PROFILE_SET = 0x400008,
	ZTE_WLAN_WISPR_SET = 0x400010,
	ZTE_WLAN_APSTA_SET = 0x400012,
	ZTE_WLAN_SET_STA_ENABLE = 0x800000,
	ZTE_WLAN_WIFI_BAND_SET = 0X1000000,
} zte_wlan_set_e_flags;

/*******************************************************************************
 *                       Global function declarations                          *
 ******************************************************************************/
/**
* @brief Get current systime
*
* @param
*
* @return currentTime
* @note
* @warning
*/
int zte_web_getCurrentTime();

/**
* @brief the entry of zte code in goahead.
*
* @param
*
* @return currentTime
* @note
* @warning
*/
extern void zte_web_init();

/**
 * @brief Read the NV's value from the nv file.
 *
 * @param item  Pointer to the NV's name.
 * @param data  Pointer to the NV's value.
 * @param dataLen  data's length.
 *
 * @return zte_topsw_state_e_type result number
 * @note
 * @warning
 */
zte_topsw_state_e_type zte_nvconfig_read(char *item, char *data, int dataLen);

/**
* @brief Write the NV's name and value into the nv file.
*
* @param item  Pointer to the NV's name.
* @param data  Pointer to the NV's value.
* @param dataLen  data's length.
*
* @return zte_topsw_state_e_type result number
* @note
* @warning
*/
zte_topsw_state_e_type zte_nvconfig_write(char *item, char *data, int dataLen);

/**
* @brief goform entry from web pages to get fw para, call the related functions according to the cmd
*
* @param wp  HTTP Request Info.
*
* @return
* @note
* @warning
*/
void zte_goform_get_cmd_process(webs_t wp);

/**
* @brief goform entry from web pages to set fw para, call the related functions according to the goformId
*
* @param wp  HTTP Request Info.
*
* @return
* @note
* @warning
*/
void zte_goform_set_cmd_process(webs_t wp);

/**
* @brief Feed back web page at top location.
*
* @param i_wp HTTP Request Info.
* @param i_pageName The page name.
*
* @return
* @note
* @warning
*/
extern void zte_webs_feedback_top(webs_t i_wp, char *i_pageName);

/**
* @brief Check whether the login timeout.
*
* @param i_wp HTTP Request Info.
* @param i_pageName The page name.
*
* @return
* @note
* @warning
*/
extern void zte_mgmt_login_timeout_check();

/**
* @brief Send SMS.
*
* @param wp HTTP Request Info.
*
* @return
* @note
* @warning
*/
extern void zte_sms_send_msg_process(webs_t wp);

/**
* @brief Save SMS.
*
* @param wp HTTP Request Info.
*
* @return
* @note
* @warning
*/
extern void zte_sms_save_msg_process(webs_t wp);

/**
* @brief Delete message.
*
* @param wp HTTP Request Info.
*
* @return
* @note
* @warning
*/
extern void zte_sms_delete_msg_process(webs_t wp);

/**
* @brief Move message to SIM.
*
* @param wp HTTP Request Info.
*
* @return
* @note
* @warning
*/
extern void zte_sms_move_to_sim_msg_process(webs_t wp);

/**
* @brief Delete all messages.
*
* @param wp HTTP Request Info.
*
* @return
* @note
* @warning
*/
extern void zte_goform_sms_alldel_msg_process(webs_t wp);

/**
* @brief Set SMS related parameters.
*
* @param wp HTTP Request Info.
*
* @return
* @note
* @warning
*/
extern void zte_sms_set_message_center(webs_t wp);

/**
* @brief To set the viewed message as read.
*
* @param wp HTTP Request Info.
*
* @return
* @note
* @warning
*/
extern void zte_sms_view_msg_process(webs_t wp);

/**
* @brief Add new contact.
*
* @param wp HTTP Request Info.
*
* @return
* @note
* @warning
*/
extern void zte_goform_pbm_contact_add_process(webs_t wp);

/**
* @brief Delete contact.
*
* @param wp HTTP Request Info.
*
* @return
* @note
* @warning
*/
extern void zte_goform_pbm_contact_del_process(webs_t wp);

/**
* @brief Set WiFi basic parameter.
*
* @param wp HTTP Request Info.
*
* @return
* @note
* @warning
*/
extern void zte_wlan_basic_set(webs_t wp);

/**
* @brief Set WiFi MAC filter parameters.
*
* @param wp HTTP Request Info.
*
* @return
* @note
* @warning
*/
extern void zte_wlan_mac_filter_set(webs_t wp);

/**
* @brief Set WiFi WPS mode.
*
* @param wp HTTP Request Info.
*
* @return
* @note
* @warning
*/
extern void zte_wlan_wps_mode_set(webs_t wp);

/**
* @brief Set WiFi sleep and wake up at regular time.
*
* @param wp HTTP Request Info.
*
* @return
* @note
* @warning
*/
extern void zte_wlan_set_tsw(webs_t wp);

/**
* @brief Set WiFi SSID1 parameters.
*
* @param wp HTTP Request Info.
*
* @return
* @note
* @warning
*/
extern void zte_wlan_web_ssid1_set(webs_t wp);

/**
* @brief Set WiFi SSID2 parameters.
*
* @param wp HTTP Request Info.
*
* @return
* @note
* @warning
*/
extern void zte_wlan_ssid2_set(webs_t wp);

/**
* @brief To set the WiFi sleep mode.
*
* @param wp HTTP Request Info.
*
* @return
* @note
* @warning
*/
extern void zte_wlan_sleep_set(webs_t wp);

/**
* @brief Set WiFi coverage mode.
*
* @param wp HTTP Request Info.
*
* @return
* @note
* @warning
*/
extern void zte_wlan_coverage_set(webs_t wp);

/**
* @brief Set WiFi apstation parameters.
*
* @param wp HTTP Request Info.
*
* @return
* @note
* @warning
*/
extern void zte_wlan_station_set(webs_t wp);

/**
* @brief Update the wifi spot profile.
*
* @param wp HTTP Request Info.
*
* @return
* @note
* @warning
*/
extern void zte_wlan_station_spot_profile_update(webs_t wp);

/**
* @brief Connect to wifi spot.
*
* @param wp HTTP Request Info.
*
* @return
* @note
* @warning
*/
extern void zte_wlan_station_connect(webs_t wp);

/**
* @brief Disconnect wifi spot.
*
* @param wp HTTP Request Info.
*
* @return
* @note
* @warning
*/
extern void zte_wlan_station_disconnect(webs_t wp);

/**
* @brief Scan the wifi spot.
*
* @param wp HTTP Request Info.
*
* @return
* @note
* @warning
*/
extern void zte_wlan_station_refresh(webs_t wp);

/**
* @brief Get the wlan port's information.
*
* @param wp HTTP Request Info.
*
* @return
* @note
* @warning
*/
extern void zte_wlan_get_station_list(webs_t wp);
extern void zte_wlan_get_wps_pin(webs_t wp);
extern void zte_wlan_get_wps_defpin(webs_t wp);



/**
* @brief Register network after manual search.
*
* @param wp HTTP Request Info.
*
* @return
* @note
* @warning
*/
extern void zte_wan_set_network(webs_t wp);

/**
* @brief Set connect mode.
*
* @param wp HTTP Request Info.
*
* @return
* @note
* @warning
*/
extern void zte_wan_set_connection_mode(webs_t wp);

/**
* @brief Connect to the network.
*
* @param wp HTTP Request Info.
*
* @return
* @note
* @warning
*/
extern void zte_wan_connect_network(webs_t wp);

/**
* @brief Disonnect the network.
*
* @param wp HTTP Request Info.
*
* @return
* @note
* @warning
*/
extern void zte_wan_disconnect_network(webs_t wp);

/**
* @brief Scan the network.
*
* @param wp HTTP Request Info.
*
* @return
* @note
* @warning
*/
extern void zte_wan_scan_network(webs_t wp);

/**
* @brief Set the mode of searching network.
*
* @param wp HTTP Request Info.
*
* @return
* @note
* @warning
*/
extern void zte_wan_network_select(webs_t wp);

/**
* @brief Set the management of network flow.
*
* @param wp HTTP Request Info.
*
* @return
* @note
* @warning
*/
extern void zte_wan_data_limit_setting(webs_t wp);

/**
* @brief Calibrate the network flow by manual.
*
* @param wp HTTP Request Info.
*
* @return
* @note
* @warning
*/
extern void zte_wan_data_flow_calibration_manual(webs_t wp);


/**
* @brief Clear the network flow records.
*
* @param wp HTTP Request Info.
*
* @return
* @note
* @warning
*/
extern void zte_wan_data_statistics_clear_process(webs_t wp, char_t *path, char_t *query);//11

/**
* @brief Init the httpshare.
*
* @param
*
* @return
* @note
* @warning
*/
extern void zte_httpshare_init();

/**
* @brief Get file list from httpshare's database.
*
* @param wp HTTP Request Info.
*
* @return
* @note
* @warning
*/
extern void zte_httpShare_enterFold(webs_t wp);

/**
* @brief Creat a new folder.
*
* @param wp HTTP Request Info.
*
* @return
* @note
* @warning
*/
extern void zte_httpShare_new(webs_t wp);

/**
* @brief Delete file or folder from SD card.
*
* @param wp HTTP Request Info.
*
* @return
* @note
* @warning
*/
extern void zte_httpShare_del(webs_t wp);

/**
* @brief Set the httpshare's configs.
*
* @param wp HTTP Request Info.
*
* @return
* @note
* @warning
*/
extern void zte_httpShare_auth_set(webs_t wp);

/**
* @brief Set the SD card's mode:usb mode or httpshare mode.
*
* @param wp HTTP Request Info.
*
* @return
* @note
* @warning
*/
extern void zte_httpShare_modeset(webs_t wp);

/**
* @brief Creat a new folder.
*
* @param wp HTTP Request Info.
*
* @return
* @note
* @warning
*/
extern void zte_httpShare_rename(webs_t wp);

/**
* @brief Check file exists.
*
* @param wp HTTP Request Info.
*
* @return
* @note
* @warning
*/
extern void zte_httpShare_check_file(webs_t wp);

/**
* @brief Get the httpshare's configs.
*
* @param wp HTTP Request Info.
*
* @return
* @note
* @warning
*/
extern void zte_httpShare_auth_get(webs_t wp);

/**
* @brief Get the SD card's name:"MicroSD Card".
*
* @param wp HTTP Request Info.
*
* @return
* @note
* @warning
*/
extern void zte_httpShare_getcard_name(webs_t wp);

/**
* @brief Get the SD card's available capacity and total capacity.
*
* @param wp HTTP Request Info.
*
* @return
* @note
* @warning
*/
extern void zte_httpShare_getcard_value(webs_t wp);

/**
* @brief Ussd's operator process.
*
* @param wp HTTP Request Info.
*
* @return
* @note
* @warning
*/
extern void zte_goform_ussd_process(webs_t wp);

/**
* @brief Handle the auto or manual apn set for ipv4ipv6.
*
* @param wp HTTP Request Info.
*
* @return
* @note
* @warning
*/
extern void zte_form_multi_apn_proc_ex(webs_t wp);

/**
* @brief Set the user's selection:update or cancel.
*
* @param wp HTTP Request Info.
*
* @return
* @note
* @warning
*/
extern void zte_fota_update(webs_t wp);

/**
* @brief Set the fota's configs.
*
* @param wp HTTP Request Info.
*
* @return
* @note
* @warning
*/
extern void zte_fota_settings(webs_t wp);


extern void zte_init_login_psw_time(void);
extern int zte_apn_param_check(webs_t  wp, char * type);
extern void zte_get_login_lock_time(webs_t wp);
extern void zte_get_login_status_value(webs_t wp, char *login_status);
extern void zte_get_ddns_status(webs_t wp);
extern void zte_dhcpv6_state_set(webs_t wp);
extern void zte_mtu_set(webs_t wp);
extern void zte_dns_mode_set(webs_t wp);
//extern void zte_dns_server_set(webs_t wp);
extern void zte_ping_diagnostics_start(webs_t wp);
extern void zte_ping_diagnostics_stop(webs_t wp);
extern void zte_get_poweron_time(webs_t wp);
extern void zte_get_lan_dev_info(webs_t wp);
extern void zte_goform_set_external_nv(webs_t wp);
extern void zte_goform_set_work_type(webs_t wp);

//other sntp + fluxstat + parent_control_set
extern void zte_goform_sntp_getdatastatic_process(webs_t wp);
extern void zte_parent_control_set(webs_t wp);//parent mode time control

/*useradded module start*/
extern void zte_goform_set_sample(webs_t wp);
/*useradded module end*/


/**
* @brief Data to be decoded.
*
* @param src Data to be decoded.
* @param len Length of the data to be decoded.
* @param out_len Pointer to output length variable.
*
* @return Allocated buffer of out_len bytes of decoded data,or NULL on failure
* @note Caller is responsible for freeing the returned buffer.
* @warning
*/
extern unsigned char * zte_base64_decode(const unsigned char *src, size_t len, size_t *out_len);

extern int zte_Safe_valid_SpecialChar(char single_data);
extern int zte_Safe_valid_SpecialChar_other(char single_data);
extern int zte_valid_length_str(char *string_s, int min, int max);

extern data_safe_result_type_t zte_Safe_isMacValid(char *str);
extern data_safe_result_type_t zte_Safe_isIpValid(char *str);
extern data_safe_result_type_t zte_Safe_isNumOnly(char *str);
extern data_safe_result_type_t zte_Safe_noSpecialChar(char *str);
extern data_safe_result_type_t zte_Safe_noSpecialChar_other(char *str);
extern data_safe_result_type_t zte_Safe_isStringOnly(char *str);
extern data_safe_result_type_t zte_Safe_isNumorStringOnly(char *str);


#endif

