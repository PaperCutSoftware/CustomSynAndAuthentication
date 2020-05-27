/*
 * (c) Copyright 1999-2018 PaperCut Software International Pty Ltd
 *
 * A user dir information provider for Unix systems.  This uses the standard
 * C API's (getpwnam, etc) and hence will work with systems using nsswitch.conf.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>

/* Function Prototypes */
static int list_all_users();
static int list_all_groups();
static int list_group_members(char *groupname, int user_names_only);
static int is_user_in_group(char *username, char *groupname);
static int get_user_details();
static int is_valid();
static int output_bool(int b);


int
main(int argc, char *argv[]) {

    int ret = 0;

    if (argc <= 2) {
        fprintf(stderr, "Error: invalid command\n");
        fprintf(stderr, "Usage: options command [arguments...]\n");
        exit(1);
    }

    if (strcmp(argv[2], "is-valid") == 0) {
        ret = is_valid();
    } else if (strcmp(argv[2], "all-users") == 0) {
        ret = list_all_users();
    } else if (strcmp(argv[2], "all-groups") == 0) {
        ret = list_all_groups();
    } else if (strcmp(argv[2], "group-member-names") == 0) {
        if (argc != 4) {
            fprintf(stderr, "Error: need a group name as an argument\n");
            exit(1);
        }
        ret = list_group_members(argv[3], 1);
    } else if (strcmp(argv[2], "group-members") == 0) {
        if (argc != 4) {
            fprintf(stderr, "Error: need a group name as an argument\n");
            exit(1);
        }
        ret = list_group_members(argv[3], 0);
    } else if (strcmp(argv[2], "is-user-in-group") == 0) {
        if (argc != 5) {
            fprintf(stderr, "Error: need a group and user name as an arguments\n");
            exit(1);
        }
        ret = is_user_in_group(argv[3], argv[4]);
    } else if (strcmp(argv[2], "get-user-details") == 0) {
        ret = get_user_details();
    } else {
        fprintf(stderr, "Error: unknown command\n");
        exit(1);
    }

    if (ret < 0) {
        fprintf(stderr, "An error has occured.\n");
        exit(1);
    }

    /* normal exit */
    exit(0);
    return(0);
}

static
int
is_valid() {
    struct passwd *p;
    struct group *g;

    /* Check that a few of our key API's work */

    g = getgrent();
    if (g == NULL) {
        output_bool(0);
        return 0;
    }
    
    p = getpwent();
    if (p == NULL) {
        output_bool(0);
        return 0;
    }

    /* Valid */
    output_bool(1);
    return 0;
}


static
int
is_user_in_group(char *username, char *groupname) {

    struct passwd *p;
    struct group *g;

    if ((p = getpwnam(username)) == NULL) {
        /* The user does not exist in the /etc/passwd */
        output_bool(0);
        return 0;
    }

    /* Check to see if the this is the primary user group */
    if ((g = getgrgid(p->pw_gid)) != NULL) {
        if ((strcmp(groupname, g->gr_name)) == 0) {
            output_bool(1);
            return 0;
        }
    }

    /* Check to see if is a member of the group */
    if ((g = getgrnam(groupname)) == NULL) {
        /* group name does not exist */
        fprintf(stderr, "Group %s does not exist\n", groupname);
        return -1;
    }

    while (*(g->gr_mem) != NULL) {
        if (strcmp(*((g->gr_mem)++), username) == 0) {
            output_bool(1);
            return 0;
        }
    }

    /* If we are here we have not found a match */
    output_bool(0);
    return 0;
}


static
int
list_group_members(char *groupname, int user_names_only) {

    struct group *g;
    struct passwd *pwent;
    char *name = (char *) NULL;

    if ((g = getgrnam(groupname)) == NULL) {
        fprintf(stderr, "Error: The group %s does not exist.", groupname);
        return -1;
    }

    /* Get standard group members */
    while (*(g->gr_mem) != NULL) {
        name = *(g->gr_mem);
        if (user_names_only) {
            printf("%s\n", name);
        } else {
            /* extract fullname from passwd */
            pwent = getpwnam(name);
            if (pwent != NULL && pwent->pw_gecos != (char *) NULL) {
                printf("%s\t%s\t\n", name, pwent->pw_gecos);
            } else {
                printf("%s\t\t\n", name);
            }
        }
        g->gr_mem++;
    }

    /* We'll also walk through all users checking their primary group. */
    while ((pwent = getpwent()) != NULL) {

        if (g->gr_gid == pwent->pw_gid) {
            if (pwent->pw_name != (char *) NULL) {
                if (user_names_only) {
                    printf("%s\n", pwent->pw_name);
                } else {
                    if (pwent->pw_gecos == (char *) NULL) {
                        printf("%s\t\t\n", pwent->pw_name);
                    } else {
                        printf("%s\t%s\t\n", pwent->pw_name, pwent->pw_gecos);
                    }
                }
            } 
        }
    }
    return 0;
}

static
int
list_all_groups() {

    struct group *grpent;

    while ( (grpent = getgrent()) != NULL) {
        if (grpent->gr_name != (char *) NULL) {
            printf("%s\n", grpent->gr_name);
        }
    }
    return 0;
}

static
int
list_all_users() {

    struct passwd *pwent;

    while ( (pwent = getpwent()) != NULL) {

        if (pwent->pw_name != (char *) NULL
                && pwent->pw_gecos != (char *) NULL) {
            printf("%s\t%s\t\n", pwent->pw_name, pwent->pw_gecos);
        }
    }
    return 0;
}

static
int
read_line(char* dest, int max_chars) {

    char *p;

    /* Read the line */
    if (fgets(dest, max_chars, stdin) == NULL) {
        fprintf(stderr, "No input received\n");
        return -1;
    }

    if (NULL == strstr(dest, "\n")) {
        fprintf(stderr, "Oversized input\n");
        return -1;
    }

    /* Strip off the new line chars */
    if ((p = strstr(dest, "\n")) != NULL) {
        *p = '\0';
    }

    if ((p = strstr(dest, "\r")) != NULL) {
        *p = '\0';
    }

    return 0;
}

static
int
get_user_details() {
    struct passwd *p;
    char username[1024];

    if (read_line(username, sizeof(username)) < 0) {
        return -1;
    }

    p = getpwnam(username);
    if (p != NULL && p->pw_gecos != (char *) NULL) {
        printf("%s\t%s\n", username, p->pw_gecos);
    }

    return 0;
}


static
int
output_bool(int b) {
    if (b) 
        printf("Y\n");
    else
        printf("N\n");
    return 0;
}

