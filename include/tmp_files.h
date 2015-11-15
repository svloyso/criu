#ifndef __CR_TMP_FILES_H__
#define __CR_TMP_FILES_H__

#include <sys/types.h>
#include "asm/types.h"
#include "limits.h"

#include "list.h"

struct tmp_file_node {
	struct list_head list;
	char filepath[PATH_MAX];
};

int tmp_file_add(char* filename);

int dump_tmp_files(void);
int restore_tmp_files(void);

#define TMP_TAR_NAME "tmpfiles.tar"

#endif // __CR_TMP_FILES_H__
