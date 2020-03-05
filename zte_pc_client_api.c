
#include "zte_pc_client_api.h"

//function declaration
static ezxml_t zte_ezxml_web_set_node_txt(ezxml_t node, const char *txt);
static ezxml_t zte_ezxml_web_find_node(ezxml_t root, const char *node_name);

/******************************************************
* Function: zte_ezxml_web_find_node
* Description: to find a node
* Input:  root:parent node pointer;node_name:to find node name
* Output: pointer to finded node
* Return:
* Others:
* Modify Date    Version   Author         Modification
* 2012/03/19        V1.0     chenyi        create
*******************************************************/

static ezxml_t zte_ezxml_web_find_node(ezxml_t root, const char *node_name)
{
	ezxml_t node = NULL;

	if ((NULL == root) || (NULL == node_name)) {
		slog(MISC_PRINT, SLOG_DEBUG, "root or node_name is null pointer.\n"); /*lint !e26*/
		return NULL;
	}

	slog(MISC_PRINT, SLOG_DEBUG, "node_name is [%s].\n", node_name); /*lint !e26*/

	node = ezxml_child(root, node_name);
	if (NULL == node) {
		slog(MISC_PRINT, SLOG_DEBUG, "not find node [%s]\n!", node_name); /*lint !e26*/
		return NULL;
	}

	return node;
}
/******************************************************
* Function: zte_ezxml_web_parse_file
* Description: to converts a given file to xml structure
* Input:  filename:file name
* Output: pointer to root tag
* Return:
* Others:
* Modify Date    Version   Author         Modification
* 2012/03/19        V1.0     chenyi        create
*******************************************************/

ezxml_t zte_ezxml_web_parse_file(const char *filename)
{
	ezxml_t root = NULL;

	if (NULL == filename) {
		printf("filename is null pointer.\n");
		return NULL;
	}

	slog(MISC_PRINT, SLOG_DEBUG, "filename is [%s].\n", filename); /*lint !e26*/

	root = ezxml_parse_file(filename);

	return root;
}
/******************************************************
* Function: zte_ezxml_web_set_node_txt
* Description: sets the character content for the given tag and returns the tag
* Input:  node :pointer to node;txt:node text
* Output: pointer to tag
* Return:
* Others:
* Modify Date    Version   Author         Modification
* 2012/03/19        V1.0     chenyi        create
*******************************************************/

static ezxml_t zte_ezxml_web_set_node_txt(ezxml_t node, const char *txt)
{
	if ((NULL == node) || (NULL == txt)) {
		printf("node or txt is null pointer.\n");
		return NULL;
	}

	slog(MISC_PRINT, SLOG_DEBUG, "txt is [%s].\n", txt); /*lint !e26*/

	return ezxml_set_txt(node, txt);
}

/******************************************************
* Function: zte_ezxml_web_save_xml_file
* Description: open a file and write xml tree into file, close file
* Input:  root :pointer to node;filename:output file name
* Output:
* Return:
* Others:
* Modify Date    Version   Author         Modification
* 2012/03/19        V1.0     chenyi        create
*******************************************************/
void zte_ezxml_web_save_xml_file(ezxml_t root, const char *filename)
{
	char *body = NULL;
	FILE *fd;
	char header[] = "<?xml version=\"1.0\" encoding=\"utf-8\"?>";
	//int len=0;

	if ((NULL == root) || (NULL == filename)) {
		slog(MISC_PRINT, SLOG_ERR, "root or filename is null pointer.\n"); /*lint !e26*/
		return ;
	}

	body = ezxml_toxml(root);

	//fd = drv_fs_Open(filename, O_WRONLY | O_CREAT | O_TRUNC, S_IREAD | S_IWRITE);
	fd = fopen(filename, "w+");
	if (fd == NULL) {
		slog(MISC_PRINT, SLOG_ERR, "open file [%s]failed.\n", filename); /*lint !e26*/
		return ;
	} else {
		/*Fristly, write header */
		//len=drv_fs_Write(fd, header, strlen(header));   /*not sizeof !!*/
		fwrite(header, 1, strlen(header), fd);
		//printf("header len=[%d]",len);

		/*Secondly, write body*/
		// len=drv_fs_Write(fd, body, strlen(body));   /*not  strlen +1 !! */
		fwrite(body, 1, strlen(body), fd);
		// printf("body len=[%d]",len);

		/*Thirdly, close file*/
		//drv_fs_Close(fd);
		fclose(fd);
	}

	/*Finally, free body and root*/
	free(body);

	if (NULL != root) {
		ezxml_free(root);
		root = NULL;
	}

}
/******************************************************
* Function: zte_ezxml_web_find_and_update_node
* Description: find given node, then update its text
* Input:  root :pointer to node;node_name:node name;txt:node text
* Output:
* Return:
* Others:
* Modify Date    Version   Author         Modification
* 2012/03/19        V1.0     chenyi        create
*******************************************************/
void zte_ezxml_web_find_and_update_node(ezxml_t root, const char *node_name, const char *txt)
{
	ezxml_t node = NULL;

	if ((NULL == root) || (NULL == node_name) || (NULL == txt)) {
		slog(MISC_PRINT, SLOG_ERR, "input paras is null pointer.\n"); /*lint !e26*/
		return ;
	}

	node = zte_ezxml_web_find_node(root, CMD_INFO_NODE);
	if (NULL != node) {
		node = zte_ezxml_web_find_node(node, node_name);
		zte_ezxml_web_set_node_txt(node, txt);
	}
}
/******************************************************
* Function: zte_write_xml_file_to_client
* Description: to write given file to client
* Input:  wp:http request info;filename:file name
* Output:
* Return:
* Others:
* Modify Date    Version   Author         Modification
* 2012/07/27        V1.0     liuyingnan        create
*******************************************************/

void zte_write_xml_file_to_client(webs_t wp, const char *filename)
{
	struct stat s;/*lint !e565 !1080 */
	FILE *fd;
	int len, wrote, written;
	char	*buf = NULL;

	//data initialization
	len = 0;
	written = 0;
	wrote = 0;

	memset(&s, 0, sizeof(struct stat));

	if ((NULL == wp) || (NULL == filename)) {
		return;
	}
	/*
	    if (drv_fs_Lstat(filename, &s) < 0) {
	        printf("stat [%s] fail.\n", filename);
	        return;
	    }
	    */

	if (lstat(filename, &s) < 0) {
		slog(MISC_PRINT, SLOG_ERR, "stat [%s] fail.\n", filename); /*lint !e26*/
		return;
	}

	written = s.st_size;
	slog(MISC_PRINT, SLOG_DEBUG, "file size : [%d].\n", written); /*lint !e26*/

	//malloc the read buf
	if ((buf = malloc(XML_READ_BUF_SIZE)) == NULL) {
		slog(MISC_PRINT, SLOG_ERR, "malloc buf fail.\n"); /*lint !e26*/
		return;
	}
	memset(buf, 0, XML_READ_BUF_SIZE);

	fd = fopen(filename, "w+");
	if (fd == NULL) {
		slog(MISC_PRINT, SLOG_ERR, "open file [%s]failed.\n", filename); /*lint !e26*/
		if (NULL != buf) {
			free(buf);
		}
		return ;
	}

	while ((len = fread(buf, XML_READ_BUF_SIZE - 1, 1, fd)) > 0) {
		if ((wrote = websWriteDataNonBlock(wp, buf, len)) < 0) {
			break;
		}
		slog(MISC_PRINT, SLOG_ERR, " read[%d],wrote[%d].\n", len, wrote); /*lint !e26*/
		if (wrote != len) {
			lseek(fd, - (len - wrote), SEEK_CUR);
		}
		//reset
		memset(buf, 0, XML_READ_BUF_SIZE);
	}

	fclose(fd);
	//drv_fs_Close(fd);
	free(buf);

}

