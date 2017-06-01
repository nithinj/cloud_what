/*
 * clouddetect.c
 *
 *  Created on: May 29, 2017
 *      Author: nithinj
 */

/**
 * @brief
 * 	find whether we are on cloud.
 * 	There is no standard way of telling whether we runs on cloud.
 * 	So we are checking the popular cloud vendors.
 *
 * @param[out]	cloudvendor	-	returns cloud vendor.
 * "No" if we couldn't find any vendor.
 *
 * @return void
 */

#include <stdio.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>

void detect_cloud(char *cloudvendor) {
	FILE *fd;
	char *cmd = "sudo dmidecode -s bios-version";
	char buf[1024];
	char vendor[32];
	int i = 0;
	int len = 0;
	int is_cloud = 0;
	struct dirent *dent;
	char procname[32];
	char *leasefile = NULL;
	DIR *pdir = NULL;
	char *pbuf = NULL;

	/* find whether we are on Amazon/Google/Oracle using dmidecode */
	fd = popen(cmd, "r");
	if (fd == NULL) {
		fprintf( stderr, "could not run %s\n", cmd);
	} else {
		(void) fread(buf, 1, sizeof(buf), fd);
		if (strcasestr(buf, "amazon") != '\0') {
			strcpy(cloudvendor, "AWS");
			is_cloud = 1;
		} else if (strcasestr(buf, "google") != '\0') {
			strcpy(cloudvendor, "GoogleCloud");
			is_cloud = 1;
		} else if (strstr(buf, "OVM") != NULL) {
			strcpy(cloudvendor, "OracleCloud");
			is_cloud = 1;
		}
	}
	if (fd != NULL)
		pclose(fd);
	if (!is_cloud) {
		/* The option “unknown-245” is an Azure-proprietary option
		 * which only gets issued by an Azure DHCP server. */
		if ((pdir = opendir("/proc")) == NULL) {
			fprintf( stderr, "%d: opendir", errno);
			return;
		}
		while (errno = 0, (dent = readdir(pdir)) != NULL) {
			/* Check to see if we have /proc/pid or /proc/.pid */
			if (!isdigit(dent->d_name[0])
					&& !(dent->d_name[0] == '.' && isdigit(dent->d_name[1])))
				continue;
			sprintf(procname, "/proc/%s/cmdline", dent->d_name);
			if ((fd = fopen(procname, "r")) == NULL) {
				continue;
			}
			i = 0;
			while (i < 1024) {
				buf[i++] = fgetc(fd);
				if (feof(fd)) {
					fclose(fd);
					break;
				}
			}
			len = i;
			if (strstr(buf, "dhclient") != NULL) {
				pbuf = buf;
				for (i = 0; i < len; i++) {
					leasefile = strstr(pbuf + i, "-lf");
					if (leasefile != NULL) {
						leasefile += 4;
						if ((fd = fopen(leasefile, "r")) == NULL) {
							fprintf( stderr, "could not open %s\n", buf);
							fclose(fd);
							break;
						}
						while (fgets(buf, sizeof(buf), fd) != NULL) {
							if (strstr(buf, "unknown-245") != NULL) {
								strcpy(cloudvendor, "Azure");
								is_cloud = 1;
								break;
							}
						}
						fclose(fd);
						break;
					}
				}
			}
		}
		if (pdir && closedir(pdir) != 0)
			fprintf( stderr, "%d: closedir", errno);
	}
	if (!is_cloud)
		strcpy(cloudvendor, "None");
}

int main() {
	char cloudvendor[32];
	detect_cloud(cloudvendor);
	printf("%s", cloudvendor);
	return 0;
}
