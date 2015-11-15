#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include "asm/types.h"

#include "tmp_files.h"
#include "cr_options.h"
#include "list.h"
#include "xmalloc.h"
#include "image.h"
#include "util.h"
#include "string.h"
#include "unistd.h"
#include "limits.h"
#include "list.h"

#define TMP_TAR_NAME "tmpfiles.tar"


struct tmp_file_node {
	struct list_head list;
	char filepath[PATH_MAX];
};



int tmp_file_add(char* filename) {
	struct tmp_file_node* node = xmalloc(sizeof(*node));

	if (!node) {
		return -1;
	}
	if (!realpath(filename, node->filepath)) {
        return -1;
    }
	list_add_tail(&node->list, &opts.tmp_files);
	return 0;
}

char* make_append_cmd(const char* filepath) {
	char cmd_template[] = "tar -f " TMP_TAR_NAME " -C / -r \"%s\" 2> /dev/null";
	char* cmd = xmalloc(strlen(filepath) + sizeof(cmd_template));
	sprintf(cmd, cmd_template, filepath);
	return cmd;
}

int dump_tmp_files(void) {
	int ret = 0;
	if (list_empty(&opts.tmp_files)) {
		return ret;
	}
	ret = system("tar -cf " TMP_TAR_NAME " -T /dev/null");
	if (ret) {
		return ret;
	}
	struct tmp_file_node* pos;
	list_for_each_entry(pos, &opts.tmp_files, list) {
		const char* tmp_file_path = pos->filepath;
		char* cmd = make_append_cmd(tmp_file_path);
		ret = system(cmd);
		xfree(cmd);
		if (ret) {
			return ret;
		}
	}
	return ret;
}


int restore_tmp_files(void) {
	int ret = 0;
	
	if (access(TMP_TAR_NAME, F_OK) != -1) {
		ret = system("tar -xf " TMP_TAR_NAME " -C / 2> /dev/null");
	}

	return ret;
}

