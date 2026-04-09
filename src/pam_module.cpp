#include <security/pam_appl.h>
#include <security/pam_modules.h>
#include <syslog.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#define PAM_SM_AUTH

// Path to our face recognition binary
#define FACE_AUTH_BIN "/usr/local/bin/face-auth-verify"

PAM_EXTERN int pam_sm_setcred(pam_handle_t *pamh, int flags, int argc, const char **argv) {
    return PAM_SUCCESS;
}

PAM_EXTERN int pam_sm_acct_mgmt(pam_handle_t *pamh, int flags, int argc, const char **argv) {
    return PAM_SUCCESS;
}

PAM_EXTERN int pam_sm_authenticate(pam_handle_t *pamh, int flags, int argc, const char **argv) {
    const char *username = NULL;
    int retval;
    
    // Get username
    retval = pam_get_user(pamh, &username, NULL);
    if (retval != PAM_SUCCESS || username == NULL) {
        syslog(LOG_ERR, "face-auth: Failed to get username");
        return PAM_AUTH_ERR;
    }
    
    syslog(LOG_INFO, "face-auth: Attempting face authentication for user: %s", username);
    
    // Fork and execute face recognition program
    pid_t pid = fork();
    
    if (pid == 0) {
        // Child process - execute face verification
        execl(FACE_AUTH_BIN, "face-auth-verify", username, NULL);
        exit(1); // If execl fails
    } else if (pid > 0) {
        // Parent process - wait for result
        int status;
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            syslog(LOG_INFO, "face-auth: Face authentication SUCCESS for %s", username);
            return PAM_SUCCESS;
        } else {
            syslog(LOG_WARNING, "face-auth: Face authentication FAILED for %s", username);
            return PAM_AUTH_ERR;
        }
    } else {
        syslog(LOG_ERR, "face-auth: Failed to fork process");
        return PAM_AUTH_ERR;
    }
}
