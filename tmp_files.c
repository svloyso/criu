#include <unistd.h>
#include <limits.h>

#include "tmp_files.h"
#include "cr_options.h"
#include "list.h"
#include "xmalloc.h"
#include "image.h"
#include "util.h"

#define TMP_TAR_NAME "tmpfiles.tar"

struct tmp_file_node {
	struct	list_head list;
	char	filepath[PATH_MAX];
};

int tmp_file_add(const char *filename) {
	struct tmp_file_node *node = xmalloc(sizeof(*node));

	if (!node)
		return -1;
	if (!realpath(filename, node->filepath))
		return -1;
	if (access(node->filepath, R_OK))
		return -1;

	list_add_tail(&node->list, &opts.tmp_files);
	pr_info("Added tmp file: %s", node->filepath);
	return 0;
}

static unsigned long get_tmpfiles_count(void) {
	struct tmp_file_node *pos;
	unsigned long count = 0;

	list_for_each_entry(pos, &opts.tmp_files, list) {
		count += 1;
	}
	return count;
}

int dump_tmp_files(void) {
	int ret = 0;
	unsigned long n;
	char **args;
	static char *default_args[] = {
		"tar",
		"--create",
		"--absolute-names",
		"--gzip",
		"--no-unquote",
		"--no-wildcards",
		"--to-stdout",
		NULL
	};
	struct tmp_file_node *pos;
	struct cr_img *img;

	if (list_empty(&opts.tmp_files))
		return ret;

	args = xmalloc(get_tmpfiles_count() * sizeof(args[0]) +
					sizeof(default_args));
	if (!args)
		return -1;
	n = 0;
	for (; default_args[n]; n++)
		args[n] = default_args[n];
	list_for_each_entry(pos, &opts.tmp_files, list) {
		args[n++] = pos->filepath;
	}
	args[n] = NULL;

	img = open_image(CR_FD_TMP_FILES, O_DUMP);
	if (img) {
		ret = cr_system(-1, img_raw_fd(img), -1, "tar", args, 0);
		close_image(img);
	} else
		ret = -1;

	xfree(args);
	while (!list_empty(&opts.tmp_files)) {
		struct tmp_file_node *node;

		node = list_first_entry(&opts.tmp_files,
					struct tmp_file_node, list);
		list_del(&node->list);
		xfree(node);
	}
	if (ret)
		pr_err("Can't dump tmp files. Please check tar output for details\n");
	return ret;
}

int restore_tmp_files(void) {
	int ret = 0;
	struct cr_img *img;

	img = open_image(CR_FD_TMP_FILES, O_RSTR);
	if (!img || empty_image(img))
		return 0;
	ret = cr_system(img_raw_fd(img), -1, -1, "tar",
			(char *[]) {"tar", "--extract", "--gzip",
				"--no-unquote", "--no-wildcards",
				"--absolute-names", "--directory",
				"/", NULL}, 0);
	close_image(img);
	if (ret)
		pr_err("Can't restore tmp files. Please check tar output for details\n");
	return ret;
}
