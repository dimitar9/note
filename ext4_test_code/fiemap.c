/*
 * Copyright (C) 2010 Canonical
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 * 
 */

/*
 *  Author Colin Ian King,  colin.king@canonical.com
 *  modified by Dimi
 */



#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#include <linux/fs.h>
#include <linux/fiemap.h>

void syntax(char **argv)
{
	fprintf(stderr, "%s [filename]...\n",argv[0]);
}

struct fiemap *read_fiemap(int fd)
{
	struct fiemap *fiemap;
	int extents_size;

	if ((fiemap = (struct fiemap*)malloc(sizeof(struct fiemap))) == NULL) {
		fprintf(stderr, "Out of memory allocating fiemap\n");	
		return NULL;
	}
	memset(fiemap, 0, sizeof(struct fiemap));


	fiemap->fm_start = 0;
	fiemap->fm_length = ~0;		/* Lazy */
	fiemap->fm_flags = FIEMAP_FLAG_SYNC; //
    /* FIEMAP_FLAG_SYNC
        If this flag is set, the kernel will sync the file before mapping extents.
        
        * FIEMAP_FLAG_XATTR
        If this flag is set, the extents returned will describe the inodes
        extended attribute lookup tree, instead of its data tree.
    */
	fiemap->fm_extent_count = 0;
	fiemap->fm_mapped_extents = 0;

	/* Find out how many extents there are */
	if (ioctl(fd, FS_IOC_FIEMAP, fiemap) < 0) {
		fprintf(stderr, "fiemap ioctl() failed\n");
		return NULL;
	}

	/* Read in the extents */
	extents_size = sizeof(struct fiemap_extent) * 
                              (fiemap->fm_mapped_extents);

	/* Resize fiemap to allow us to read in the extents */
	if ((fiemap = (struct fiemap*)realloc(fiemap,sizeof(struct fiemap) + 
                                         extents_size)) == NULL) {
		fprintf(stderr, "Out of memory allocating fiemap\n");	
		return NULL;
	}

	memset(fiemap->fm_extents, 0, extents_size);
	fiemap->fm_extent_count = fiemap->fm_mapped_extents;
	fiemap->fm_mapped_extents = 0;

	if (ioctl(fd, FS_IOC_FIEMAP, fiemap) < 0) {
		fprintf(stderr, "fiemap ioctl() failed\n");
		return NULL;
	}
	
	return fiemap;
}

void dump_fiemap(struct fiemap *fiemap, char *filename)
{
	int i;

	printf("File %s has %d extents:\n",filename, fiemap->fm_mapped_extents);

	printf("#\tLogical          Physical         Length           Flags reserved64,1, reserved0 1 2\n");
	for (i=0;i<fiemap->fm_mapped_extents;i++) {
		printf("%d:\t%-16.16llx %-16.16llx %-16.16llx %-4.4x %-16.16llx %-16.16llx %-4.4x %-4.4x %-4.4x\n",
			i,
			fiemap->fm_extents[i].fe_logical,
			fiemap->fm_extents[i].fe_physical,
			fiemap->fm_extents[i].fe_length,
			fiemap->fm_extents[i].fe_flags,
            fiemap->fm_extents[i].fe_reserved64[0],
            fiemap->fm_extents[i].fe_reserved64[1],
            fiemap->fm_extents[i].fe_reserved[0],
            fiemap->fm_extents[i].fe_reserved[1],
            fiemap->fm_extents[i].fe_reserved[2]
            );
	}
	printf("\n");
}

int main(int argc, char **argv)
{
	int i;

	if (argc < 2) {
		syntax(argv);
		exit(EXIT_FAILURE);
	}

	for (i=1;i<argc;i++) {
		int fd;

		if ((fd = open(argv[i], O_RDONLY)) < 0) {
			fprintf(stderr, "Cannot open file %s\n", argv[i]);
		}
		else {
			struct fiemap *fiemap;

			if ((fiemap = read_fiemap(fd)) != NULL) 
				dump_fiemap(fiemap, argv[i]);
			close(fd);
		}
	}
	exit(EXIT_SUCCESS);
}
