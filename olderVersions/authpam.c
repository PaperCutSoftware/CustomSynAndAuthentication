/*
 * (c) Copyright 1999-2018 PaperCut Software International Pty Ltd
 *
 * A quick Unix auth provider based on PAM.
 */

#ifdef __APPLE_CC__
#include <security/pam_appl.h>
#else
#include <security/pam_appl.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>

#include "../../../version.h"

/* Function Prototypes */
static int read_line(char* dest, int max_chars);
static int password_conversation(int num_msg, const struct pam_message **msg, 
        struct pam_response **resp, void *appdata_ptr);

static struct pam_conv conv = {&password_conversation, NULL};


int
main(int argc, char *argv[]) {

    pam_handle_t *pamh  = NULL;
    int retval = 0;
    char username[1024];
    char password[1024];
    struct passwd *pwent;
    int debug = 0;

    if (argc > 1 && strcmp("-d", argv[1]) == 0) {
        debug = 1;
    }

    /* Read username and password from stdin */
    if (read_line(username, sizeof(username)) < 0) {
        goto error;
    }
    if (read_line(password, sizeof(password)) < 0) {
        goto error;
    }

    if (*username == '\0') {
        fprintf(stderr, "Username was blank\n");
        goto error;
    }
    if (*password == '\0') {
        fprintf(stderr, "Password was blank\n");
        goto error;
    }

    conv.appdata_ptr = password;
    retval = pam_start(STR_PAM_SERVERNAME, username, &conv, &pamh);

    if (retval == PAM_SUCCESS) {
        retval = pam_authenticate(pamh, 0);    /* is user really user? */

        if (debug) {
            fprintf(stderr, "pam_authenticate: %s\n", pam_strerror(pamh, retval));
        }
    }

    if (retval == PAM_SUCCESS) {
        retval = pam_acct_mgmt(pamh, 0);       /* permitted access? */
        if (debug) {
            fprintf(stderr, "pam_acct_mgmt: %s\n", pam_strerror(pamh, retval));
        }
    }

    if (retval == PAM_SUCCESS) {
        /* Check to make sure the user as authenticated using their username */
        pwent = getpwnam(username);
        if (pwent != NULL && pwent->pw_name != (char *) NULL) {
            if (strcasecmp(username, pwent->pw_name) != 0) {
                fprintf(stderr, "Must log in using username - not full/real name\n");
                retval = -1;
            }
        } 
    }

    /* This is where we have been authorized or not. */
    if (retval == PAM_SUCCESS) {
        fprintf(stdout, "OK\n");
    } else {
error:
        fprintf(stdout, "ERROR\n");
        fprintf(stderr, "Invalid username or password\n");
    }

    if (pamh != NULL && pam_end(pamh, retval) != PAM_SUCCESS) {
        pamh = NULL;
    }

    exit(0);

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

/* The PAM conversation function */
static
int
password_conversation(int num_msg, 
                const struct pam_message **msg, 
                struct pam_response **resp, 
                void *appdata_ptr) {

    if (num_msg != 1 || 
            msg[0]->msg_style != PAM_PROMPT_ECHO_OFF) {
        fprintf(stderr, "Error: %s (%d)\n", msg[0]->msg, msg[0]->msg_style);
        return PAM_CONV_ERR;
    }

    if (!appdata_ptr) {
        fprintf(stderr, "Error: No password set.\n");
        return PAM_CONV_ERR;
    }

    *resp = calloc(num_msg, sizeof(struct pam_response));
    if (!*resp) {
        fprintf(stderr, "Error: Out of memory\n");
        return PAM_CONV_ERR;
    }
    (*resp)[0].resp = strdup((char *) appdata_ptr);
    (*resp)[0].resp_retcode = 0;

    return ((*resp)[0].resp ? PAM_SUCCESS : PAM_CONV_ERR);
}

