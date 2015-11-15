#include <string.h>
#include <stdio.h>

#include "tmp_files.h"
#include "cr_options.h"
#include "list.h"
#include "xmalloc.h"
#include "image.h"
#include "util.h"
#include "string.h"
#include "unistd.h"


int tmp_file_add(char* filename) {
	struct tmp_file_node* node = xmalloc(sizeof(struct tmp_file_node));

	if(node == NULL) {
		return -1;
	}
	realpath(filename, node->filepath);
	list_add_tail(&node->list, &opts.tmp_files);
	return 0;
}

int dump_tmp_files(void) {
	if (list_empty(&opts.tmp_files)) {
		return 0;
	}
	struct tmp_file_node* pos;
	int ret = 0;
	ret = system("tar -cf " TMP_TAR_NAME " -T /dev/null");
	char cmd_prefix[] = "tar -f " TMP_TAR_NAME " -C / -r ";
	char cmd_postfix[] = " 2> /dev/null";
	char cmd[PATH_MAX + sizeof(cmd_prefix) + sizeof(cmd_postfix) + 1] = {0};
	list_for_each_entry(pos, &opts.tmp_files, list) {
		strcpy(cmd, cmd_prefix);
		sprintf(cmd + sizeof(cmd_prefix) - 1, "\"%s\" ", pos->filepath);
		strcat(cmd, cmd_postfix);
		ret = system(cmd);
		if(ret != 0) {
			pr_perror("Can not append file to tar");
			return ret;
		}
	}
	return ret;
}


int restore_tmp_files(void) {
	int ret = 0;
	
	if (access(TMP_TAR_NAME, F_OK) != -1) {
		system("tar -xf " TMP_TAR_NAME " -C / 2> /dev/null");
	}

	return ret;
}

