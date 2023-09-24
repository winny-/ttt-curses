#include <errno.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <limits.h>

#include <bsd/string.h>

#include "log.h"

char* ensure_dir(char* path) {
	for (char* ptr = path;; ptr++) {
		if (*ptr != '/' && *ptr != '\0') {
			continue;
		}
		char* prefix = strndup(path, ptr-path);
		if (!mkdir(prefix, 0755)) {
			if (errno == EEXIST) {
				struct stat sb;
				if (stat(prefix, &sb) && S_ISDIR(sb.st_mode)) {
					goto mkdirok;
				}
				errno = EEXIST;
			}
			free(prefix);
			return NULL;
		}
mkdirok:
		free(prefix);
		if (!*ptr) {
			break;
		}
	}
	return path;
}


char* get_dir2(char* var, char* def) {
	char* home = getenv("HOME");
	if (!home) {
		free(home);
		return NULL;
	}
	char* path = getenv(var);
	char* full = malloc(sizeof(char) * PATH_MAX);
	strlcpy(full, home, PATH_MAX);
	strlcat(full, "/", PATH_MAX);
	if (path) {
		strlcat(full, path, PATH_MAX),
		free(path);
		strlcat(full, "/ttt-curses", PATH_MAX);
		if (ensure_dir(full)) {
			return full;
		}
	}
	strlcat(full, def, PATH_MAX);
	strlcat(full, "/ttt-curses", PATH_MAX);
	if (ensure_dir(full)) {
		return full;
	}
	free(full);
	return NULL;
}

char* get_state_dir() {
	return get_dir2("XDG_STATE_HOME", ".local/state");
}


char* get_config_dir() {
	return get_dir2("XDG_CONFIG_HOME", ".config");
}

