/**
 * liteprompt is a simple, opinionated prompt to use for zsh. It exists to 
 * only because it cans, and it answers an interest that the developer had.
 *
 * This is not a great C program, but it works.
 *
 * this returns a prompt like: {updates}|{ssh_hostname}|{virtual_env}|{short_path}>
 * or as root, the prompt is in red with a #
 * 
 *
 */
#include <unistd.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <libgen.h>
#include <git2.h>

// This struct is included in the example parts of the libgit2 
// it is intended to also parse command line options, but I don't 
// need those for this. However, I couldn't make the function call work 
// directly from the git_status_options struct.
struct status_opts {
    git_status_options statusopt;
    char *repodir;
    char *pathspec[PATH_MAX];
    int npaths;
    int format;
    int zterm;
    int showbranch;
    int showsubmod;
    int repeat;
};

/**
 * Update the fmt_branch to include the branch name 
 */
void store_branch(git_reference *head, char *fmt_branch) {
    const char *branch = NULL;
    branch = git_reference_shorthand(head);
    if(branch) {
        strcat(fmt_branch, branch);
        strcat(fmt_branch, "%f|");
    }
}


/**
 * update the fmt_branch name to include the branch name and 
 * if it should be colored red or green
 */
void git_details(git_repository *repo, char *fmt_branch) {
    int error = 0;
    git_reference *head = NULL;
    error = git_repository_head(&head, repo);

    git_status_list *status;
    // Make sure the status options are showing the default init values
    struct status_opts o = { GIT_STATUS_OPTIONS_INIT, "." };
    o.statusopt.show  = GIT_STATUS_SHOW_INDEX_AND_WORKDIR;
    o.statusopt.flags = GIT_STATUS_OPT_INCLUDE_UNTRACKED |
         GIT_STATUS_OPT_RENAMES_HEAD_TO_INDEX |
         GIT_STATUS_OPT_SORT_CASE_SENSITIVELY;
 
    git_status_list_new(&status, repo, &o.statusopt);
    size_t i, maxi = git_status_list_entrycount(status);
    //printf("value of i: %li\n", i);

    if (maxi != 0) {
        strcat(fmt_branch, "%F{red}"); 
    } else {
        strcat(fmt_branch, "%F{green}");
    }
    if (!error) {
        store_branch(head, fmt_branch);
    }

    git_reference_free(head);
}


/**
 * update the short_path, if it is in the home directory,
 * otherwise, the function returns an index number to start iterating through
 * the current working directory 
 */
int find_home(char * short_path) {
    char cwd[PATH_MAX];
    char * home = getenv("HOME");

    int index = 0;
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
       return index; 
    }
    for (; index < strlen(home); index++) {
        char h = home[index];
        char w = cwd[index];
        if (w != h) {
            return 0;
        }
    }

    short_path[0] = '~';
    short_path[1] = '/';
    short_path[2] = '\0';
    return index;
}


/**
 * create a short path of the current working directory. 
 * This path will have the home as a tilde, and then will display only 
 * the first letter of base directories. The end of the dirctories will 
 * be the full name
 */
int shorten_path(char * short_path, char *cwd) {
    if(strcmp(cwd, "/") == 0) {
        strcat(short_path, "/");
        return 0; 
    }
    short_path[0] = '\0';
    int index = find_home(short_path);
    if (index == strlen(cwd)) { 
        short_path[1] = '>';
        return 1; 
    }
    char * icwd = &cwd[index];
    if (index == 0) {
        short_path[0] = '/';
        short_path[1] = '\0';
    };
    char * split = strtok(icwd, "/");
    char * last_word;
    while (split != NULL) {
        char * word = split;

        last_word = word;
        char * w = &word[0];
        split = strtok(NULL, "/"); 
        if (split == NULL) {
            continue; 
        }
        int len = strlen(short_path);
        short_path[len] = *w;
        short_path[len+1] = '/';
        short_path[len+2] = '\0';

    }
    if (strlen(last_word) > 0) strcat(short_path, last_word);
    strcat(short_path, ">");
    return 1;
}

/**
 * update the host to be the name of the host machine
 */
int machine_name_prompt(char *host, int max_length, char * color) {
    if (strlen(host) > max_length) return 0;
    char hostname[HOST_NAME_MAX];
    gethostname(hostname, HOST_NAME_MAX);
    int i = sprintf(host, "%%F{%s}%s%%f", color, hostname);
    return i;
}

/**
 * return the line number of the file ~/.updates. 
 * if the file doesn't exist, return zero.
 */
int get_updates() {
    FILE *fp;
    char *home = getenv("HOME");
    char filepath[PATH_MAX];
    strcat(filepath, home);
    strcat(filepath, "/.updates");
    fp=fopen(filepath, "r");
    long int lines =0;

    if ( fp == NULL ) {
        return 0;
    }

    while (EOF != (fscanf(fp, "%*[^\n]"), fscanf(fp,"%*c")))
          ++lines;

    // for arch linux this needs to change
    return lines -1;
}

int main() {
    git_libgit2_init();
    git_buf root = {0};
    char cwd[PATH_MAX];
    char prompt[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        // Root
        char * user = getenv("USER");
        if(strcmp(user, "root") == 0) {
            char short_path[PATH_MAX];
            char host[HOST_NAME_MAX+20];
            machine_name_prompt(host, HOST_NAME_MAX, "yellow");
            int s = shorten_path(short_path, cwd);
            short_path[strlen(short_path)-1] = '#';
            printf("%s:%%F{red}%s%%f", host, short_path);
            return 0;
        }
        // updates
        int updates = get_updates();
        char updatestr[PATH_MAX];
        sprintf(updatestr, "%%F{yellow}%i%%f|", updates);
        if (updates > 0) strcat(prompt, updatestr);
        // ssh hostname
        if (getenv("SSH_TTY") != NULL) {
            char host[HOST_NAME_MAX+13];
            machine_name_prompt(host, HOST_NAME_MAX, "green");
            strcat(prompt, host); 
        }
        // Virtual Env
        char * venvpath = getenv("VIRTUAL_ENV");
        if(venvpath) {
            char *venv = basename(venvpath);
            strcat(venv, "|");
            strcat(prompt, venv);
        }
        // git branch
        int error = git_repository_discover(&root, cwd, 0, NULL);
        if (error == 0) {
            char fmt_branch = '\0';
            git_repository * repo = NULL;
            git_repository_open(&repo, root.ptr);
            //int g = git_branch();
            git_details(repo, &fmt_branch);
            if(fmt_branch) strcat(prompt, &fmt_branch);
        } 
        // path
        char short_path[PATH_MAX];
        int s = shorten_path(short_path, cwd);
        strcat(prompt, "%F{blue}");
        strcat(prompt, short_path);
        strcat(prompt, "%f ");
	}
    printf("%s", prompt);
}


