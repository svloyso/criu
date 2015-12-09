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

static int get_tmpfiles_count() {
	struct tmp_file_node* pos;
	int count = 0;
	list_for_each_entry(pos, &opts.tmp_files, list) {
		count += 1;
	}
	return count;
}

int dump_tmp_files(void) {
	int ret = 0;
	if (list_empty(&opts.tmp_files)) {
		return ret;
	}
	char** args = xmalloc((get_tmpfiles_count() + 7) * sizeof(char*));
	if (!args) {
		return -1;
	}
	args[0] = "tar";
	args[1] = "--create";
	args[2] = "--absolute-names";
	args[3] = "--gzip";
	args[4] = "--no-unquote";
	args[5] = "--no-wildcards";
	struct tmp_file_node* pos;
	int i = 6;
	list_for_each_entry(pos, &opts.tmp_files, list) {
		args[i++] = pos->filepath;
	}
	args[i] = NULL;
	
	struct cr_img* img;
	img = open_image(CR_FD_TMP_FILES, O_DUMP);
	if(!img)
		goto out;
	ret = cr_system(-1, img_raw_fd(img), -1, "tar", args);
	close_image(img);
out:
	xfree(args);

	return ret;
}

int restore_tmp_files(void) {
	int ret = 0;
	struct cr_img *img;
	img = open_image(CR_FD_TMP_FILES, O_RSTR);
	if (!img || empty_image(img)) 
		return -1;
	ret = cr_system(img_raw_fd(img), -1, -1, "tar", 
			(char *[]) {"tar", "--extract", "--gzip", 
				"--no-unquote", "--no-wildcards",
				"--absolute-names", "--directory", "/", NULL});
	close_image(img);
	if (ret) {
		pr_err("Can't restore tmp files\n");
		return -1;
	}

	return 0;
}

