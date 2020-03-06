/************************************************************************
* 版权所有 (C)2010, 深圳市中兴通讯股份有限公司。
*
* 文件名称： zte_web_mgmt.h
* 文件标识：
* 内容摘要：
* 其它说明：
* 当前版本： V0.1
* 作    者： zyt
* 完成日期： 2010-11-24
*
* 修改记录1：
* 修改内容：初始版本
************************************************************************/

#ifndef ZTE_WEB_MGMT_H
#define ZTE_WEB_MGMT_H

#define APNCONFIG_MEMORY 1024
#define PROFILE_MEMBER_LEN 32
#define PROFILE_APN_LEN 65
#define DAYSEC 86400
#define COMMONYEARSEC 31536000
#define LEAPYEARSEC 31622400
#define JAN_2000   946684791
typedef struct tag_APN_PROFILE {
	char profile_name[PROFILE_MEMBER_LEN];
	char apn_name[PROFILE_APN_LEN];
	char apn_select[PROFILE_MEMBER_LEN];
	char dial_num[PROFILE_MEMBER_LEN];
	char ppp_auth_mode[PROFILE_MEMBER_LEN];
	char ppp_username[PROFILE_APN_LEN];
	char ppp_passwd[PROFILE_APN_LEN];
	char pdp_type[PROFILE_MEMBER_LEN];
	char pdp_select[PROFILE_MEMBER_LEN];
	char pdp_addr[PROFILE_MEMBER_LEN];
	char dns_mode[PROFILE_MEMBER_LEN];
	char prefer_dns_manual[PROFILE_MEMBER_LEN];
	char standby_dns_manual[PROFILE_MEMBER_LEN];
} APN_PROFILE;


typedef struct tag_IPv6_APN_PROFILE {
	char profile_name[PROFILE_MEMBER_LEN];
	char apn_name[PROFILE_APN_LEN];
	char apn_select[PROFILE_MEMBER_LEN];
	char dial_num[PROFILE_MEMBER_LEN];
	char ppp_auth_mode[PROFILE_MEMBER_LEN];
	char ppp_username[PROFILE_APN_LEN];
	char ppp_passwd[PROFILE_APN_LEN];
	char pdp_type[PROFILE_MEMBER_LEN];
	char pdp_select[PROFILE_MEMBER_LEN];
	char pdp_addr[PROFILE_MEMBER_LEN];
	char dns_mode[PROFILE_MEMBER_LEN];
	char prefer_dns_manual[PROFILE_MEMBER_LEN];
	char standby_dns_manual[PROFILE_MEMBER_LEN];
} IPV6_APN_PROFILE;


/* management */
extern void zte_mgmt_login(webs_t wp);//11
extern void zte_mgmt_logout(webs_t wp);//11
extern void zte_mgmt_set_language(webs_t wp);//11
extern void zte_mgmt_restore(webs_t wp);//11
extern void zte_mgmt_poweroff(webs_t wp);
extern void zte_mgmt_control_power_on_speed(webs_t wp);
extern void zte_mgmt_change_password(webs_t wp);//11
extern void zte_mgmt_change_account(webs_t wp);//
extern void zte_goform_mgmt_set_not_login_process(webs_t wp);
extern void zte_mgmt_disable_pin(webs_t wp);//11
extern void zte_mgmt_pin_input(webs_t wp);//11
extern void zte_mgmt_puk_input(webs_t wp);//11
extern void zte_mgmt_auto_pin(webs_t wp);
extern void zte_mgmt_pin_enable_or_modify(webs_t wp);//11
extern void zte_mgmt_unlock_network(webs_t wp);//11
extern void zte_goform_mgmt_pin_mgmt_process(webs_t wp);
extern void zte_quick_setup(webs_t wp);//11
//extern void zte_quick_set_first(webs_t wp);//11
void deal_quick_setup_apn_ex(webs_t wp);
extern void zte_mgmt_set_devicemode(webs_t wp);



extern void zte_goform_mgmt_sntp_process(webs_t wp);


extern void zte_goform_mgmt_reboot_process(webs_t wp);
extern void zte_goform_mgmt_syslog_process(webs_t wp);

extern void get_autoapn_profile(APN_PROFILE *apn_profile);
extern void get_apn_profile_by_index(int index, APN_PROFILE *profile);
extern void get_ipv6apn_profile_by_index(int index, IPV6_APN_PROFILE *ipv6profile);
extern void get_ipv4v6apn_profile_by_index(int index, APN_PROFILE *profile, IPV6_APN_PROFILE *ipv6profile);
extern void get_autoapn_profile(APN_PROFILE *profile);
extern void set_apn_profile_by_index(int index, APN_PROFILE *profile);
extern void set_ipv6_apn_profile_by_index(int index, IPV6_APN_PROFILE *ipv6profile);
extern void set_ipv4v6_apn_profile_by_index(int index, APN_PROFILE *profile, IPV6_APN_PROFILE *ipv6profile);
extern void zte_wan_lock_frequency_process(webs_t wp);

extern int manual_set_time();

extern void getdialog_url(int eid, webs_t wp, int argc, char_t **argv);
extern void getddns_status(int eid, webs_t wp, int argc, char_t **argv);
extern char * timei2s(int);
extern void timeen2ch(char *);
extern void datastatic(char *, char *, char *, char *, char *);

extern int zte_fota_notifyPushMsg(int cmd);

extern void zte_fota_get_upgrade_result(webs_t wp);
extern void zte_fota_get_dp_pack_info(webs_t wp);
extern void zte_fota_get_update_info(webs_t wp);
//extern void zte_fota_get_dm_last_check_time(webs_t wp);

#endif

