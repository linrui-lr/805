#include "mxml.h"
#include "zte_phone_limit_xml.h"
#include "zte_web_interface.h"

mxml_node_t *phone_limit_num_XML_tree;   /*存储modem状态*/

INT phone_limit_Xml_Init()
{
	system("rm -rf /etc_rw/config/phone_limit.xml > /dev/null");

	mxml_node_t *parent = NULL;
	phone_limit_num_XML_tree = mxmlNewXML("1.0");
	parent = mxmlNewElement(phone_limit_num_XML_tree, "phone_limit"); /*建立父节点*/
	if (parent == NULL)
		return -1;

	FILE *fp = NULL;
	system("touch /etc_rw/config/phone_limit.xml");
	fp = fopen("/etc_rw/config/phone_limit.xml", "w+");
	if (fp == NULL) {
		return -1;
	}
	/*将内存中的modem状态树保存到文件中 */
	mxmlSaveFile(phone_limit_num_XML_tree, fp, MXML_NO_CALLBACK);
	fclose(fp);

	return 1;
}

INT phone_limit_InsertStatesToXml(PHONE_LIMIT_NUM_T *phone_limit_num)
{
	mxml_node_t *tree = NULL;
	mxml_node_t *parent = NULL;
	mxml_node_t *phone_limit_num_node = NULL;
	FILE *ptrFile = NULL;
	char tmpStatus[68] = {0};

	tree = phone_limit_num_XML_tree;

	parent = mxmlFindElement(tree, tree, "phone_limit", NULL, NULL, MXML_DESCEND_FIRST);/*找到父节点phone_limit*/
	if (parent == NULL || phone_limit_num == NULL) {
		return -1;
	}

	phone_limit_num_node = mxmlNewElement(parent, "phone_limit_detail");/*以limit_detail为父节点建立一个节点*/

	sprintf(tmpStatus, "%d", phone_limit_num->uIndex);
	mxmlElementSetAttr(phone_limit_num_node, "index", tmpStatus);/*插入index属性*/
	slog(MISC_PRINT, SLOG_DEBUG, "limit_InsertStatesToXml index=%s\n", tmpStatus);

	memset(tmpStatus, 0, sizeof(tmpStatus));
	sprintf(tmpStatus, "%s", phone_limit_num->phone_limit_num1);
	mxmlElementSetAttr(phone_limit_num_node, "phone_limit_num1", tmpStatus);/*插入limit_num属性*/
	slog(MISC_PRINT, SLOG_DEBUG, "phone_limit_InsertStatesToXml phone_limit_num1=%s\n", tmpStatus);

	memset(tmpStatus, 0, sizeof(tmpStatus));
	sprintf(tmpStatus, "%s", phone_limit_num->phone_limit_num_from1);
	mxmlElementSetAttr(phone_limit_num_node, "phone_limit_num_from1", tmpStatus);/*插入limit_num_from属性*/
	slog(MISC_PRINT, SLOG_DEBUG, "phone_limit_InsertStatesToXml phone_limit_num_from1=%s\n", tmpStatus);

	memset(tmpStatus, 0, sizeof(tmpStatus));
	sprintf(tmpStatus, "%s", phone_limit_num->phone_limit_num_to1);
	mxmlElementSetAttr(phone_limit_num_node, "phone_limit_num_to1", tmpStatus);/*插入limit_num_to属性*/
	slog(MISC_PRINT, SLOG_DEBUG, "phone_limit_InsertStatesToXml phone_limit_num_to1=%s\n", tmpStatus);


	memset(tmpStatus, 0, sizeof(tmpStatus));
	sprintf(tmpStatus, "%s", phone_limit_num->phone_limit_num2);
	mxmlElementSetAttr(phone_limit_num_node, "phone_limit_num2", tmpStatus);/*插入limit_num属性*/
	slog(MISC_PRINT, SLOG_DEBUG, "phone_limit_InsertStatesToXml phone_limit_num2=%s\n", tmpStatus);

	memset(tmpStatus, 0, sizeof(tmpStatus));
	sprintf(tmpStatus, "%s", phone_limit_num->phone_limit_num_from2);
	mxmlElementSetAttr(phone_limit_num_node, "phone_limit_num_from2", tmpStatus);/*插入limit_num_from属性*/
	slog(MISC_PRINT, SLOG_DEBUG, "phone_limit_InsertStatesToXml phone_limit_num_from2=%s\n", tmpStatus);

	memset(tmpStatus, 0, sizeof(tmpStatus));
	sprintf(tmpStatus, "%s", phone_limit_num->phone_limit_num_to2);
	mxmlElementSetAttr(phone_limit_num_node, "phone_limit_num_to2", tmpStatus);/*插入limit_num_to属性*/
	slog(MISC_PRINT, SLOG_DEBUG, "phone_limit_InsertStatesToXml phone_limit_num_to2=%s\n", tmpStatus);




	memset(tmpStatus, 0, sizeof(tmpStatus));
	sprintf(tmpStatus, "%s", phone_limit_num->phone_limit_num3);
	mxmlElementSetAttr(phone_limit_num_node, "phone_limit_num3", tmpStatus);/*插入limit_num属性*/
	slog(MISC_PRINT, SLOG_DEBUG, "phone_limit_InsertStatesToXml phone_limit_num3=%s\n", tmpStatus);

	memset(tmpStatus, 0, sizeof(tmpStatus));
	sprintf(tmpStatus, "%s", phone_limit_num->phone_limit_num_from3);
	mxmlElementSetAttr(phone_limit_num_node, "phone_limit_num_from3", tmpStatus);/*插入limit_num_from属性*/
	slog(MISC_PRINT, SLOG_DEBUG, "phone_limit_InsertStatesToXml phone_limit_num_from3=%s\n", tmpStatus);

	memset(tmpStatus, 0, sizeof(tmpStatus));
	sprintf(tmpStatus, "%s", phone_limit_num->phone_limit_num_to3);
	mxmlElementSetAttr(phone_limit_num_node, "phone_limit_num_to3", tmpStatus);/*插入limit_num_to属性*/
	slog(MISC_PRINT, SLOG_DEBUG, "phone_limit_InsertStatesToXml phone_limit_num_to3=%s\n", tmpStatus);


	ptrFile = fopen("/etc_rw/config/phone_limit.xml", "w+");
	if (ptrFile == NULL) {
		return -1;
	}
	/*将内存中的modem状态树保存到文件中*/
	mxmlSaveFile(tree, ptrFile, MXML_NO_CALLBACK);
	fclose(ptrFile);
	return 0;
}


INT Deletephone_limit_detailFromXml(UINT index)
{
	mxml_node_t *tree = NULL;
	mxml_node_t *phone_limit_num_node = NULL;
	char tmpStatus[68] = {0};
	FILE *ptrFile = NULL;

	tree = phone_limit_num_XML_tree;

	sprintf(tmpStatus, "%d", index);
	/*查找节点名为limit_detail属性名为index属性值为tmpStatus存储的索引号的节点*/
	phone_limit_num_node = mxmlFindElement(tree, tree, "phone_limit_detail", "index", tmpStatus, MXML_DESCEND);
	if (phone_limit_num_node == NULL) {
		return -1;
	}
	mxmlDelete(phone_limit_num_node);/*删除找到的节点*/

	ptrFile = fopen("/etc_rw/config/phone_limit.xml", "w+");
	if (ptrFile == NULL) {
		return -1;
	}
	mxmlSaveFile(tree, ptrFile, MXML_NO_CALLBACK);
	fclose(ptrFile);
	return 0;
}

INT phone_limit_ModifyStatesAttr(PHONE_LIMIT_NUM_T *modify_value)
{
	mxml_node_t *tree = NULL;
	mxml_node_t *phone_limit_num_node = NULL;
	char tmpStatus[68] = {0};
	FILE *ptrFile = NULL;

	if (modify_value == NULL) {
		return -1;
	}

	tree = phone_limit_num_XML_tree;

	sprintf(tmpStatus, "%d", modify_value->uIndex);
	/*查找节点名为states属性名为index属性值为tmpStatus存储的索引号的节点*/
	phone_limit_num_node = mxmlFindElement(tree, tree, "phone_limit_detail", "index", tmpStatus, MXML_DESCEND);
	if (phone_limit_num_node == NULL) {
		return -1;
	}


	memset(tmpStatus, 0, sizeof(tmpStatus));
	sprintf(tmpStatus, "%s", modify_value->phone_limit_num1);
	mxmlElementSetAttr(phone_limit_num_node, "phone_limit_num1", tmpStatus);/*插入limit_num1属性*/
	slog(MISC_PRINT, SLOG_DEBUG, "phone_limit_InsertStatesToXml phone_limit_num1=%s\n", tmpStatus);

	memset(tmpStatus, 0, sizeof(tmpStatus));
	sprintf(tmpStatus, "%s", modify_value->phone_limit_num_from1);
	mxmlElementSetAttr(phone_limit_num_node, "phone_limit_num_from1", tmpStatus);/*插入limit_num_from1属性*/
	slog(MISC_PRINT, SLOG_DEBUG, "phone_limit_InsertStatesToXml limit_num_from1=%s\n", tmpStatus);

	memset(tmpStatus, 0, sizeof(tmpStatus));
	sprintf(tmpStatus, "%s", modify_value->phone_limit_num_to1);
	mxmlElementSetAttr(phone_limit_num_node, "phone_limit_num_to1", tmpStatus);/*插入limit_num_to1属性*/
	slog(MISC_PRINT, SLOG_DEBUG, "phone_limit_InsertStatesToXml phone_limit_num_to1=%s\n", tmpStatus);


	memset(tmpStatus, 0, sizeof(tmpStatus));
	sprintf(tmpStatus, "%s", modify_value->phone_limit_num2);
	mxmlElementSetAttr(phone_limit_num_node, "phone_limit_num2", tmpStatus);/*插入limit_num2属性*/
	slog(MISC_PRINT, SLOG_DEBUG, "phone_limit_InsertStatesToXml phone_limit_num2=%s\n", tmpStatus);

	memset(tmpStatus, 0, sizeof(tmpStatus));
	sprintf(tmpStatus, "%s", modify_value->phone_limit_num_from2);
	mxmlElementSetAttr(phone_limit_num_node, "phone_limit_num_from2", tmpStatus);/*插入limit_num_from2属性*/
	slog(MISC_PRINT, SLOG_DEBUG, "phone_limit_InsertStatesToXml phone_limit_num_from2=%s\n", tmpStatus);

	memset(tmpStatus, 0, sizeof(tmpStatus));
	sprintf(tmpStatus, "%s", modify_value->phone_limit_num_to2);
	mxmlElementSetAttr(phone_limit_num_node, "phone_limit_num_to2", tmpStatus);/*插入limit_num_to2属性*/
	slog(MISC_PRINT, SLOG_DEBUG, "phone_limit_InsertStatesToXml phone_limit_num_to2=%s\n", tmpStatus);


	memset(tmpStatus, 0, sizeof(tmpStatus));
	sprintf(tmpStatus, "%s", modify_value->phone_limit_num3);
	mxmlElementSetAttr(phone_limit_num_node, "phone_limit_num3", tmpStatus);/*插入limit_num3属性*/
	slog(MISC_PRINT, SLOG_DEBUG, "phone_limit_InsertStatesToXml phone_limit_num3=%s\n", tmpStatus);

	memset(tmpStatus, 0, sizeof(tmpStatus));
	sprintf(tmpStatus, "%s", modify_value->phone_limit_num_from3);
	mxmlElementSetAttr(phone_limit_num_node, "phone_limit_num_from3", tmpStatus);/*插入limit_num_from3属性*/
	slog(MISC_PRINT, SLOG_DEBUG, "phone_limit_InsertStatesToXml phone_limit_num_from3=%s\n", tmpStatus);

	memset(tmpStatus, 0, sizeof(tmpStatus));
	sprintf(tmpStatus, "%s", modify_value->phone_limit_num_to3);
	mxmlElementSetAttr(phone_limit_num_node, "phone_limit_num_to3", tmpStatus);/*插入limit_num_to3属性*/
	slog(MISC_PRINT, SLOG_DEBUG, "phone_limit_InsertStatesToXml phone_limit_num_to3=%s\n", tmpStatus);


	ptrFile = fopen("/etc_rw/config/phone_limit.xml", "w+");

	if (ptrFile == NULL) {
		return -1;
	}
	mxmlSaveFile(tree, ptrFile, MXML_NO_CALLBACK);
	fclose(ptrFile);
	return 0;
}

void push_phone_limit_num_to_9x15()
{
	system("adb push /etc_rw/config/phone_limit.xml /usr/zte/zte_conf/config");
	slog(MISC_PRINT, SLOG_DEBUG, "push phone_limit.xml to 9x15\n");
}

void get_phone_limt_insert_XML()
{

	PHONE_LIMIT_NUM_T  phone_limit_num;
	memset(&phone_limit_num, 0, sizeof(PHONE_LIMIT_NUM_T));
	phone_limit_num.uIndex = 1;

	cfg_get_item("phone_limit_num1", &(phone_limit_num.phone_limit_num1), sizeof(phone_limit_num.phone_limit_num1));
	cfg_get_item("phone_limit_num_from1", &(phone_limit_num.phone_limit_num_from1), sizeof(phone_limit_num.phone_limit_num_from1));
	cfg_get_item("phone_limit_num_to1", &(phone_limit_num.phone_limit_num_to1), sizeof(phone_limit_num.phone_limit_num_to1));
	cfg_get_item("phone_limit_num2", &(phone_limit_num.phone_limit_num2), sizeof(phone_limit_num.phone_limit_num2));
	cfg_get_item("phone_limit_num_from2", &(phone_limit_num.phone_limit_num_from2), sizeof(phone_limit_num.phone_limit_num_from2));
	cfg_get_item("phone_limit_num_to2", &(phone_limit_num.phone_limit_num_to2), sizeof(phone_limit_num.phone_limit_num_to2));
	cfg_get_item("phone_limit_num3", &(phone_limit_num.phone_limit_num3), sizeof(phone_limit_num.phone_limit_num3));
	cfg_get_item("phone_limit_num_from3", &(phone_limit_num.phone_limit_num_from3), sizeof(phone_limit_num.phone_limit_num_from3));
	cfg_get_item("phone_limit_num_to3", &(phone_limit_num.phone_limit_num_to3), sizeof(phone_limit_num.phone_limit_num_to3));
	/*
	    strcpy(phone_limit_num.phone_limit_num1, cfg_safe_get("phone_limit_num1"));
	    strcpy(phone_limit_num.phone_limit_num_from1, cfg_safe_get("phone_limit_num_from1"));
	    strcpy(phone_limit_num.phone_limit_num_to1, cfg_safe_get("phone_limit_num_to1"));
	    strcpy(phone_limit_num.phone_limit_num2, cfg_safe_get("phone_limit_num2"));
	    strcpy(phone_limit_num.phone_limit_num_from2, cfg_safe_get("phone_limit_num_from2"));
	    strcpy(phone_limit_num.phone_limit_num_to2, cfg_safe_get("phone_limit_num_to2"));
		strcpy(phone_limit_num.phone_limit_num3, cfg_safe_get("phone_limit_num3"));
	    strcpy(phone_limit_num.phone_limit_num_from3, cfg_safe_get("phone_limit_num_from3"));
	    strcpy(phone_limit_num.phone_limit_num_to3, cfg_safe_get("phone_limit_num_to3"));
	*/
	phone_limit_InsertStatesToXml(&phone_limit_num);

}