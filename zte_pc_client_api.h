/******************************************************
* Description:  constants, type, interface for pc client
* Modify Date    Version     Author         Modification
* 2012/07.27       V1.0         liuyingnan              modife
*******************************************************/

/********************************header file************************/


#include "ezxml.h"
#include "zte_web_interface.h"

#ifndef SEEK_CUR
#define SEEK_CUR    1
#endif

#ifndef SEEK_END
#define SEEK_END    2
#endif

#ifndef SEEK_SET
#define SEEK_SET    0
#endif
/********************************constants************************/
#define XML_READ_BUF_SIZE (2*1024)
#define DEVCIE_INFO_XML_FILE "/zero-cd/webui/pc_client/device_xml/device_cmd_status.xml"
#define USSD_INFO_XML_FILE "/zero-cd/webui/ussd_xml/ussd.xml"
#define WAN_INFO_XML_FILE "/zero-cd/webui/pc_client/wan_xml/wan_cmd_status.xml"
#define SMS_INFO_XML_FILE "/zero-cd/webui/sms_xml/cmd_status_report.xml"

//for modem and SIM
#define DEVICE_INFO_SET_NO_ERROR "0"
#define DEVICE_INFO_SET_INVALID_PARA "12"
#define DEVICE_INFO_PROCESSING "1"
#define DEVICE_INFO_SET_SUCCESS "3"
#define DEVICE_INFO_SET_FAIL "2"

//for USSD
#define USSD_STATUS_PROCESSING "1"
#define  USSD_STATUS_FAIL "2"
#define  USSD_STATUS_SUCCESS "3"
#define USSD_ERR_CODE_NO_ERROR "0"
#define USSD_ERR_CODE_INVALID_PARA "12"

//for WAN
#define WAN_STATUS_PROCESSING "1"
#define  WAN_STATUS_FAIL "2"
#define  WAN_STATUS_SUCCESS "3"
#define WAN_ERR_CODE_NO_ERROR "0"
#define WAN_ERR_CODE_INVALID_PARA "89"
#define WAN_ERR_CODE_ERROR_MODEM_STATE "90"
#define WAN_ERR_CODE_WRITE_FILE_FAIL "91"

#define CMD_INFO_NODE "cmd_info"
#define CMD_ID_NODE "cmd_id"
#define CMD_STATUS_NODE "status"
#define CMD_ERR_CODE "err_code"

#define PC_CLIENT_FLAG "1"
/********************************function************************/

