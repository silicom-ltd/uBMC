/* pam_tacplus.c - PAM interface for TACACS+ protocol.
 *
 * Copyright (C) 2010, Pawel Krawczyk <pawel.krawczyk@hush.com> and
 * Jeroen Nijhof <jeroen@jeroennijhof.nl>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program - see the file COPYING.
 *
 * See `CHANGES' file for revision history.
 */

#include "pam_tacplus.h"
#include "support.h"

#include <stdlib.h>     /* malloc */
#include <stdio.h>
#include <syslog.h>
#include <netdb.h>      /* gethostbyname */
#include <sys/socket.h> /* in_addr */
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdarg.h>     /* va_ */
#include <signal.h>
#include <string.h>     /* strdup */
#include <ctype.h>
#include <time.h>
#include <unistd.h>
#include <strings.h>

#ifdef HAVE_CONFIG_H
  #include "config.h"
#endif

/* address of server discovered by pam_sm_authenticate */
static tacplus_server_t active_server;

/* accounting task identifier */
static short int task_id = 0;


static int do_template_user(pam_handle_t * pamh, char* template_user, int ctrl)
{
	int retval;
	const char *tmpuser, *finaluser;
	char ubuf[128];

	retval = pam_get_item(pamh, PAM_USER, (const void **) &tmpuser);
    if (retval != PAM_SUCCESS) {
		return (retval);
    }
    pam_putenv(pamh, "AUTH_PROTOCOL=tacacs");
    if (tmpuser) {
		ubuf[0] = '\0';
		snprintf(ubuf, sizeof(ubuf), "LOGIN_USER=%s", tmpuser);
		pam_putenv(pamh, ubuf);
    }

    if (template_user[0]) {
		if (ctrl & PAM_TAC_DEBUG) {
		    syslog(LOG_DEBUG, "%s: Trying template user: %s",
					__FUNCTION__, template_user);
		}

		/*
		 * If the given user name doesn't exist in the local password
		 * database, change it to the value given in the "template_user"
		 * option.
		 */
		if (pam_modutil_getpwnam(pamh, template_user) == NULL) {
			_pam_log(LOG_ERR, "template user %s doesn't exist", template_user);
			return PAM_PERM_DENIED;
		}
	    retval = pam_set_item(pamh, PAM_USER, template_user);
	    if (ctrl & PAM_TAC_DEBUG) {
			syslog(LOG_DEBUG, "Using template user");
	    }
    }

	retval = pam_get_item(pamh, PAM_USER, (const void **) &finaluser);
	if (retval != PAM_SUCCESS) {
		return (retval);
    }
    if (finaluser) {
		ubuf[0] = '\0';
		snprintf(ubuf, sizeof(ubuf), "AUTH_USER=%s", finaluser);
		pam_putenv(pamh, ubuf);
    }

	return (retval);
}

/* Helper functions */
int _pam_send_account(int tac_fd, int type, const char *user, char *tty,
    char *r_addr, char *cmd) {

    char buf[64];
    struct tac_attrib *attr;
    int retval;

    attr=(struct tac_attrib *)xcalloc(1, sizeof(struct tac_attrib));

    sprintf(buf, "%lu", (unsigned long)time(NULL));

    if (type == TAC_PLUS_ACCT_FLAG_START) {
        tac_add_attrib(&attr, "start_time", buf);
    } else if (type == TAC_PLUS_ACCT_FLAG_STOP) {
        tac_add_attrib(&attr, "stop_time", buf);
    }
    sprintf(buf, "%hu", task_id);
    tac_add_attrib(&attr, "task_id", buf);
    tac_add_attrib(&attr, "service", tac_service);
    if(tac_protocol != NULL && tac_protocol[0] != '\0')
      tac_add_attrib(&attr, "protocol", tac_protocol);
    if (cmd != NULL) {
        tac_add_attrib(&attr, "cmd", cmd);
    }

    retval = tac_acct_send(tac_fd, type, user, tty, r_addr, attr);

    /* this is no longer needed */
    tac_free_attrib(&attr);

    if(retval < 0) {
        _pam_log (LOG_WARNING, "%s: send %s accounting failed (task %hu)",
            __FUNCTION__,
            tac_acct_flag2str(type),
            task_id);
        close(tac_fd);
        return -1;
    }

    struct areply re;
    if( tac_acct_read(tac_fd, &re) != TAC_PLUS_ACCT_STATUS_SUCCESS ) {
        _pam_log (LOG_WARNING, "%s: accounting %s failed (task %hu)",
            __FUNCTION__,
            tac_acct_flag2str(type),
            task_id);

        if(re.msg != NULL)
            free(re.msg);

        close(tac_fd);
        return -1;
    }

    if(re.msg != NULL)
        free(re.msg);

    close(tac_fd);
    return 0;
}

int _pam_account(pam_handle_t *pamh, int argc, const char **argv,
    int type, char *cmd) {

    int retval;
    static int ctrl;
    char *user = NULL;
    char *tty = NULL;
    char *r_addr = NULL;
    char *typemsg;
    int status = PAM_SESSION_ERR;
    int srv_i, tac_fd;

    typemsg = tac_acct_flag2str(type);
    ctrl = _pam_parse (argc, argv);

    if (ctrl & PAM_TAC_DEBUG) {
        syslog (LOG_DEBUG, "%s: [%s] called (pam_tacplus v%u.%u.%u)",
            __FUNCTION__, typemsg, PAM_TAC_VMAJ, PAM_TAC_VMIN, PAM_TAC_VPAT);
        syslog(LOG_DEBUG, "%s: tac_srv_no=%d", __FUNCTION__, tac_srv_no);
    }

    if ((user = _pam_get_user(pamh)) == NULL)
        return PAM_USER_UNKNOWN;

    if (ctrl & PAM_TAC_DEBUG)
        syslog(LOG_DEBUG, "%s: username [%s] obtained", __FUNCTION__, user);

    tty = _pam_get_terminal(pamh);
    if(!strncmp(tty, "/dev/", 5))
        tty += 5;
    if (ctrl & PAM_TAC_DEBUG)
        syslog(LOG_DEBUG, "%s: tty [%s] obtained", __FUNCTION__, tty);

    r_addr = _pam_get_rhost(pamh);
    if (ctrl & PAM_TAC_DEBUG)
        syslog(LOG_DEBUG, "%s: rhost [%s] obtained", __FUNCTION__, r_addr);

    /* checks for specific data required by TACACS+, which should
       be supplied in command line  */
    if(*tac_service == '\0') {
        _pam_log (LOG_ERR, "ACC: TACACS+ service type not configured");
        return PAM_AUTH_ERR;
    }
    if(tac_protocol == NULL || *tac_protocol == '\0') {
    	syslog (LOG_DEBUG, "ACC: TACACS+ protocol type not configured (IGNORED)");
    }

    /* when this module is called from within pppd or other
       application dealing with serial lines, it is likely
       that we will get hit with signal caused by modem hangup;
       this is important only for STOP packets, it's relatively
       rare that modem hangs up on accounting start */
    if(type == TAC_PLUS_ACCT_FLAG_STOP) {
        signal(SIGALRM, SIG_IGN);
        signal(SIGCHLD, SIG_IGN);
        signal(SIGHUP, SIG_IGN);
    }

    status = PAM_SESSION_ERR;
    for(srv_i = 0; srv_i < tac_srv_no; srv_i++) {
        tac_fd = tac_connect_single(tac_srv[srv_i].addr, tac_srv[srv_i].key, NULL);
        if (tac_fd < 0) {
            _pam_log(LOG_WARNING, "%s: error sending %s (fd)",
                __FUNCTION__, typemsg);
            continue;
        }
        if (ctrl & PAM_TAC_DEBUG)
            syslog(LOG_DEBUG, "%s: connected with fd=%d (srv %d)", __FUNCTION__, tac_fd, srv_i);

        retval = _pam_send_account(tac_fd, type, user, tty, r_addr, cmd);
        if (retval < 0) {
            _pam_log(LOG_WARNING, "%s: error sending %s (acct)",
                __FUNCTION__, typemsg);
        } else {
            status = PAM_SUCCESS;
            if (ctrl & PAM_TAC_DEBUG)
                syslog(LOG_DEBUG, "%s: [%s] for [%s] sent", __FUNCTION__, typemsg, user);
        }
        close(tac_fd);

        if ((status == PAM_SUCCESS) && !(ctrl & PAM_TAC_ACCT)) {
            /* do not send acct start/stop packets to _all_ servers */
            break;
        }
    }

    if (type == TAC_PLUS_ACCT_FLAG_STOP) {
        signal(SIGALRM, SIG_DFL);
        signal(SIGCHLD, SIG_DFL);
        signal(SIGHUP, SIG_DFL);
    }
    return status;
}


/* Main PAM functions */

/* authenticates user on remote TACACS+ server
 * returns PAM_SUCCESS if the supplied username and password
 * pair is valid
 */
PAM_EXTERN
int pam_sm_authenticate (pam_handle_t * pamh, int flags,
    int argc, const char **argv) {

    int ctrl, retval;
    char *user;
    char *pass;
    char *tty;
    char *r_addr;
    int srv_i;
    int tac_fd, status, msg, communicating;

    user = pass = tty = r_addr = NULL;

    ctrl = _pam_parse(argc, argv);

    if (ctrl & PAM_TAC_DEBUG)
        syslog(LOG_DEBUG, "%s: called (pam_tacplus v%u.%u.%u)",
            __FUNCTION__, PAM_TAC_VMAJ, PAM_TAC_VMIN, PAM_TAC_VPAT);

    if ((user = _pam_get_user(pamh)) == NULL)
        return PAM_USER_UNKNOWN;

    if (ctrl & PAM_TAC_DEBUG)
        syslog(LOG_DEBUG, "%s: user [%s] obtained", __FUNCTION__, user);

    retval = tacacs_get_password (pamh, flags, ctrl, &pass);
    if (retval != PAM_SUCCESS || pass == NULL || *pass == '\0') {
        _pam_log(LOG_ERR, "unable to obtain password");
        return PAM_CRED_INSUFFICIENT;
    }

    retval = pam_set_item (pamh, PAM_AUTHTOK, pass);
    if (retval != PAM_SUCCESS) {
        _pam_log(LOG_ERR, "unable to set password");
        return PAM_CRED_INSUFFICIENT;
    }

    if (ctrl & PAM_TAC_DEBUG)
        syslog(LOG_DEBUG, "%s: password obtained", __FUNCTION__);

    tty = _pam_get_terminal(pamh);
    if (!strncmp(tty, "/dev/", 5))
        tty += 5;
    if (ctrl & PAM_TAC_DEBUG)
        syslog(LOG_DEBUG, "%s: tty [%s] obtained", __FUNCTION__, tty);

    r_addr = _pam_get_rhost(pamh);
    if (ctrl & PAM_TAC_DEBUG)
        syslog(LOG_DEBUG, "%s: rhost [%s] obtained", __FUNCTION__, r_addr);

    status = PAM_AUTHINFO_UNAVAIL;
    for (srv_i = 0; srv_i < tac_srv_no; srv_i++) {
        if (ctrl & PAM_TAC_DEBUG)
            syslog(LOG_DEBUG, "%s: trying srv %d", __FUNCTION__, srv_i );

        tac_fd = tac_connect_single(tac_srv[srv_i].addr, tac_srv[srv_i].key, NULL);
        if (tac_fd < 0) {
            _pam_log(LOG_ERR, "connection failed srv %d: %m", srv_i);
            continue;
        }
        if (tac_authen_send(tac_fd, user, pass, tty, r_addr) < 0) {
            close(tac_fd);
            _pam_log(LOG_ERR, "error sending auth req to TACACS+ server");
            continue;
        }
        communicating = 1;
        while (communicating) {
            msg = tac_authen_read(tac_fd);

            /* talk the protocol */
            switch (msg) {
                case TAC_PLUS_AUTHEN_STATUS_PASS:
                    /* success */
                    if (ctrl & PAM_TAC_DEBUG)
                        syslog(LOG_DEBUG, "tacacs status: TAC_PLUS_AUTHEN_STATUS_PASS");

                    status = do_template_user(pamh, tac_template_user, ctrl);;
                    communicating = 0;
                    active_server.addr = tac_srv[srv_i].addr;
                    active_server.key = tac_srv[srv_i].key;

                    if (ctrl & PAM_TAC_DEBUG)
                        syslog(LOG_DEBUG, "%s: active srv %d", __FUNCTION__, srv_i);
                    break;

                case TAC_PLUS_AUTHEN_STATUS_FAIL:
                    /* forget it */
                    if (ctrl & PAM_TAC_DEBUG)
                        syslog(LOG_DEBUG, "tacacs status: TAC_PLUS_AUTHEN_STATUS_FAIL");

                    status = PAM_AUTH_ERR;
                    communicating = 0;
                    syslog(LOG_DEBUG, "auth failed: %d", msg);
                    break;

                case TAC_PLUS_AUTHEN_STATUS_GETDATA:
                    /* not implemented */
                    if (ctrl & PAM_TAC_DEBUG)
                        syslog(LOG_DEBUG, "tacacs status: TAC_PLUS_AUTHEN_STATUS_GETDATA");

                    communicating = 0;
                    break;

                case TAC_PLUS_AUTHEN_STATUS_GETUSER:
                    /* not implemented */
                    if (ctrl & PAM_TAC_DEBUG)
                        syslog(LOG_DEBUG, "tacacs status: TAC_PLUS_AUTHEN_STATUS_GETUSER");

                    communicating = 0;
                    break;

                case TAC_PLUS_AUTHEN_STATUS_GETPASS:
                    if (ctrl & PAM_TAC_DEBUG)
                        syslog(LOG_DEBUG, "tacacs status: TAC_PLUS_AUTHEN_STATUS_GETPASS");

                    if (ctrl & PAM_TAC_DEBUG)
                        syslog(LOG_DEBUG, "%s: tac_cont_send called", __FUNCTION__);

                    if (tac_cont_send(tac_fd, pass) < 0) {
                        _pam_log (LOG_ERR, "error sending continue req to TACACS+ server");
                        communicating = 0;
                        break;
                    }
                    /* continue the while loop; go read tac response */
                    break;

                case TAC_PLUS_AUTHEN_STATUS_RESTART:
                    /* try it again */
                    if (ctrl & PAM_TAC_DEBUG)
                        syslog(LOG_DEBUG, "tacacs status: TAC_PLUS_AUTHEN_STATUS_RESTART");

                    /*
                     * not implemented
                     * WdJ: I *think* you can just do tac_authen_send(user, pass) again
                     *      but I'm not sure
                     */
                    communicating = 0;
                    break;

                case TAC_PLUS_AUTHEN_STATUS_ERROR:
                    /* server has problems */
                    if (ctrl & PAM_TAC_DEBUG)
                        syslog(LOG_DEBUG, "tacacs status: TAC_PLUS_AUTHEN_STATUS_ERROR");

                    communicating = 0;
                    break;

                case TAC_PLUS_AUTHEN_STATUS_FOLLOW:
                    /* server tells to try a different server address */
                    /* not implemented */
                    if (ctrl & PAM_TAC_DEBUG)
                        syslog(LOG_DEBUG, "tacacs status: TAC_PLUS_AUTHEN_STATUS_FOLLOW");

                    communicating = 0;
                    break;

                default:
                    if (msg < 0) {
                        /* connection error */
                        communicating = 0;
                        if (ctrl & PAM_TAC_DEBUG)
                            syslog(LOG_DEBUG, "error communicating with tacacs server");
                        break;
                    }

                    /* unknown response code */
                    communicating = 0;
                    if (ctrl & PAM_TAC_DEBUG)
                        syslog(LOG_DEBUG, "tacacs status: unknown response 0x%02x", msg);
            }
        }    /* end while(communicating) */
        close(tac_fd);

        if (status == PAM_SUCCESS)
            break;
    }
    if (status != PAM_SUCCESS && status != PAM_AUTH_ERR)
        _pam_log(LOG_ERR, "no more servers to connect");

    if (ctrl & PAM_TAC_DEBUG)
        syslog(LOG_DEBUG, "%s: exit with pam status: %d", __FUNCTION__, status);

    bzero(pass, strlen (pass));
    free(pass);
    pass = NULL;

    return status;
}    /* pam_sm_authenticate */


/* no-op function to satisfy PAM authentication module */
PAM_EXTERN
int pam_sm_setcred (pam_handle_t * pamh, int flags,
    int argc, const char **argv) {

    int ctrl = _pam_parse (argc, argv);

    if (ctrl & PAM_TAC_DEBUG)
        syslog (LOG_DEBUG, "%s: called (pam_tacplus v%u.%u.%u)"
            , __FUNCTION__, PAM_TAC_VMAJ, PAM_TAC_VMIN, PAM_TAC_VPAT);

    return PAM_SUCCESS;
}    /* pam_sm_setcred */


/* authorizes user on remote TACACS+ server, i.e. checks
 * his permission to access requested service
 * returns PAM_SUCCESS if the service is allowed
 */
PAM_EXTERN
int pam_sm_acct_mgmt (pam_handle_t * pamh, int flags,
    int argc, const char **argv) {

    int retval, ctrl, status=PAM_AUTH_ERR;
    const char *user;
    char *tty;
    char *r_addr;
    struct areply arep;
    struct tac_attrib *attr = NULL;
    int tac_fd;

    user = tty = r_addr = NULL;

    /* this also obtains service name for authorization
       this should be normally performed by pam_get_item(PAM_SERVICE)
       but since PAM service names are incompatible TACACS+
       we have to pass it via command line argument until a better
       solution is found ;) */
    ctrl = _pam_parse (argc, argv);

    if (ctrl & PAM_TAC_DEBUG)
        syslog (LOG_DEBUG, "%s: called (pam_tacplus v%u.%u.%u)"
            , __FUNCTION__, PAM_TAC_VMAJ, PAM_TAC_VMIN, PAM_TAC_VPAT);
/*
    if ((user = _pam_get_user(pamh)) == NULL)
     * Get user name given at the prompt (set in do_template_user) 
     * We are assuming LOGIN_USER is what PAM_USER would have been
     * if not for the do_template_user processing
*/
    if ((user = pam_getenv(pamh, "LOGIN_USER")) == NULL)
        return PAM_USER_UNKNOWN;

    if (ctrl & PAM_TAC_DEBUG)
        syslog(LOG_DEBUG, "%s: username obtained [%s]", __FUNCTION__, user);

    tty = _pam_get_terminal(pamh);
    if(!strncmp(tty, "/dev/", 5))
        tty += 5;
    if (ctrl & PAM_TAC_DEBUG)
        syslog(LOG_DEBUG, "%s: tty obtained [%s]", __FUNCTION__, tty);

    r_addr = _pam_get_rhost(pamh);
    if (ctrl & PAM_TAC_DEBUG)
        syslog(LOG_DEBUG, "%s: rhost obtained [%s]", __FUNCTION__, r_addr);

    /* checks if user has been successfully authenticated
       by TACACS+; we cannot solely authorize user if it hasn't
       been authenticated or has been authenticated by method other
       than TACACS+ */
    if(active_server.addr == NULL) {
        _pam_log (LOG_ERR, "user not authenticated by TACACS+");
        return PAM_AUTH_ERR;
    }
    if (ctrl & PAM_TAC_DEBUG)
        syslog (LOG_DEBUG, "%s: active server is [%s]", __FUNCTION__,
            tac_ntop(active_server.addr->ai_addr));

    /* checks for specific data required by TACACS+, which should
       be supplied in command line  */
    if(!*tac_service) {
        _pam_log (LOG_ERR, "SM: TACACS+ service type not configured");
        return PAM_AUTH_ERR;
    }
    if(tac_protocol == NULL || !*tac_protocol) {
    	syslog (LOG_DEBUG, "SM: TACACS+ protocol type not configured (IGNORED)");
    }

    tac_add_attrib(&attr, "service", tac_service);
    if(tac_protocol != NULL && tac_protocol[0] != '\0')
      tac_add_attrib(&attr, "protocol", tac_protocol);

    tac_fd = tac_connect_single(active_server.addr, active_server.key, NULL);
    if(tac_fd < 0) {
        _pam_log (LOG_ERR, "TACACS+ server unavailable");
        if(arep.msg != NULL)
            free (arep.msg);

        close(tac_fd);
        return PAM_AUTH_ERR;
    }

    retval = tac_author_send(tac_fd, user, tty, r_addr, attr);

    tac_free_attrib(&attr);

    if(retval < 0) {
        _pam_log (LOG_ERR, "error getting authorization");
        if(arep.msg != NULL)
            free (arep.msg);

        close(tac_fd);
        return PAM_AUTH_ERR;
    }

    if (ctrl & PAM_TAC_DEBUG)
        syslog(LOG_DEBUG, "%s: sent authorization request", __FUNCTION__);

    tac_author_read(tac_fd, &arep);

    if(arep.status != AUTHOR_STATUS_PASS_ADD &&
        arep.status != AUTHOR_STATUS_PASS_REPL) {

    	syslog (LOG_DEBUG, "TACACS+ authorisation failed for [%s] to [%s]", user, tac_service);
        if(arep.msg != NULL)
            free (arep.msg);

        close(tac_fd);
        return PAM_PERM_DENIED;
    }

    if (ctrl & PAM_TAC_DEBUG)
        syslog(LOG_DEBUG, "%s: user [%s] successfully authorized", __FUNCTION__, user);

    status = PAM_SUCCESS;

    attr = arep.attr;
    while (attr != NULL)  {
        char attribute[attr->attr_len];
        char value[attr->attr_len];
        char *sep;

        sep = index(attr->attr, '=');
        if(sep == NULL)
            sep = index(attr->attr, '*');
        if(sep != NULL) {
            bcopy(attr->attr, attribute, attr->attr_len-strlen(sep));
            attribute[attr->attr_len-strlen(sep)] = '\0';
            bcopy(sep, value, strlen(sep));
            value[strlen(sep)] = '\0';

            size_t i;
            for (i = 0; attribute[i] != '\0'; i++) {
                attribute[i] = toupper(attribute[i]);
                if (attribute[i] == '-')
                    attribute[i] = '_';
            }

            if (ctrl & PAM_TAC_DEBUG)
                syslog(LOG_DEBUG, "%s: returned attribute `%s%s' from server", __FUNCTION__, attribute, value);

            /* make returned attributes available for other PAM modules via PAM environment */
            if (pam_putenv(pamh, strncat(attribute, value, strlen(value))) != PAM_SUCCESS)
                syslog(LOG_WARNING, "%s: unable to set PAM environment", __FUNCTION__);

        } else {
            syslog(LOG_WARNING, "%s: invalid attribute `%s', no separator", __FUNCTION__, attr->attr);
        }
        attr = attr->next;
    }

    /* free returned attributes */
    if(arep.attr != NULL)
        tac_free_attrib(&arep.attr);

    if(arep.msg != NULL)
        free (arep.msg);

    close(tac_fd);

    return status;
}    /* pam_sm_acct_mgmt */

/* sends START accounting request to the remote TACACS+ server
 * returns PAM error only if the request was refused or there
 * were problems connection to the server
 */
/* accounting packets may be directed to any TACACS+ server,
 * independent from those used for authentication and authorization;
 * it may be also directed to all specified servers
 */
PAM_EXTERN
int pam_sm_open_session (pam_handle_t * pamh, int flags,
    int argc, const char **argv) {

    task_id=(short int) magic();
    return _pam_account(pamh, argc, argv, TAC_PLUS_ACCT_FLAG_START, NULL);
}    /* pam_sm_open_session */

/* sends STOP accounting request to the remote TACACS+ server
 * returns PAM error only if the request was refused or there
 * were problems connection to the server
 */
PAM_EXTERN
int pam_sm_close_session (pam_handle_t * pamh, int flags,
    int argc, const char **argv) {

    return _pam_account(pamh, argc, argv, TAC_PLUS_ACCT_FLAG_STOP, NULL);
}    /* pam_sm_close_session */


#ifdef PAM_SM_PASSWORD
/* no-op function for future use */
PAM_EXTERN
int pam_sm_chauthtok (pam_handle_t * pamh, int flags,
    int argc, const char **argv) {

    int ctrl = _pam_parse (argc, argv);

    if (ctrl & PAM_TAC_DEBUG)
        syslog (LOG_DEBUG, "%s: called (pam_tacplus v%u.%u.%u)"
            , __FUNCTION__, PAM_TAC_VMAJ, PAM_TAC_VMIN, PAM_TAC_VPAT);

    return PAM_SUCCESS;
}    /* pam_sm_chauthtok */
#endif


#ifdef PAM_STATIC
struct pam_module _pam_tacplus_modstruct {
    "pam_tacplus",
    pam_sm_authenticate,
    pam_sm_setcred,
    pam_sm_acct_mgmt,
    pam_sm_open_session,
    pam_sm_close_session,
#ifdef PAM_SM_PASSWORD
    pam_sm_chauthtok
#else
    NULL
#endif
};
#endif

