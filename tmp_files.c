#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include "asm/types.h"

#include "tmp_files.h"
#include "cr_options.h"
#include "list.h"
#include "xmalloc.h"
#include "unistd.h"
#include "string.h"
#include "limits.h"
#include "fcntl.h"
#include "image.h"
#include "errno.h"
#include "util.h"

#define TMP_TAR_NAME "tmpfiles.tar"

struct tmp_file_node {
	struct	list_head list;
	char	filepath[PATH_MAX];
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

static char* make_append_cmd(const char* filepath, const char* tarname) {
	char cmd_template[] = "tar -f \"%s\" -C / -r \"%s\" 2> /dev/null";
	char* cmd = xmalloc(strlen(filepath) + sizeof(cmd_template) + strlen(tarname));
	sprintf(cmd, cmd_template, tarname, filepath);
	return cmd;
}

static int get_tarname(char* buf, size_t buf_size) {
	int ret = 0;
	char fdfile[64];
	int imgdir_fd = get_service_fd(IMG_FD_OFF);
	sprintf(fdfile, "/proc/self/fd/%d", imgdir_fd);
	ret = readlink(fdfile, buf, buf_size);
	if (ret == -1) {
		return errno;
	}
	ret = 0;
	strcat(buf, "/" TMP_TAR_NAME);
	return ret;
}

static int make_empty_tar() {
	int ret = 0;
	char* fullpath = xmalloc(PATH_MAX);
	ret = get_tarname(fullpath, PATH_MAX);
	if (ret) {
		goto out;
	}
	char cmd_template[] = "tar -cf \"%s\" -T /dev/null 2> /dev/null";
	char cmd[PATH_MAX + sizeof(cmd_template)];
	sprintf(cmd, cmd_template, fullpath);
	ret = system(cmd);
out:
	xfree(fullpath);
	return ret;
}

int dump_tmp_files(void) {
	int ret = 0;
	if (list_empty(&opts.tmp_files)) {
		return ret;
	}
	ret = make_empty_tar();
	if (ret) {
		return ret;
	}
	char tarname[PATH_MAX];
	get_tarname(tarname, PATH_MAX);
	if (ret) {
		return ret;
	}
	struct tmp_file_node* pos;
	list_for_each_entry(pos, &opts.tmp_files, list) {
		const char* tmp_filepath = pos->filepath;
		char* cmd = make_append_cmd(tmp_filepath, tarname);
		ret = system(cmd);
		xfree(cmd);
		if (ret) {
			return ret;
		}
	}
	return ret;
}

int restore_tmp_files(void) {
	int ret = -1;
	char tarname[PATH_MAX];
	get_tarname(tarname, PATH_MAX);
	char cmd_template[] = "tar -xf \"%s\" -C / 2> /dev/null";
	char cmd[PATH_MAX + sizeof(cmd_template)];
	sprintf(cmd, cmd_template, tarname);
	if (access(tarname, F_OK) != -1) {
		ret = system(cmd);
	}
	return ret;
}

