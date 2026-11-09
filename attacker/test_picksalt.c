/* Minimal reproduction of OpenSSH's pick_salt()+xcrypt() logic (openbsd-compat/xcrypt.c)
 * against the real /etc/shadow on this box, using musl's real crypt(). */
#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <pwd.h>
#include <shadow.h>
#include <unistd.h>

static const char *shadow_pw(const char *name) {
    struct spwd *sp = getspnam(name);
    if (sp) return sp->sp_pwdp;
    return "";
}

static char salt_buf[32];

static const char *pick_salt(void) {
    struct passwd *pw;
    const char *passwd;
    const char *p;
    size_t typelen;

    strcpy(salt_buf, "xx");
    pw = getpwuid(0);
    if (!pw) { printf("pick_salt: getpwuid(0) failed\n"); return salt_buf; }
    printf("pick_salt: root username is '%s'\n", pw->pw_name);
    passwd = shadow_pw(pw->pw_name);
    printf("pick_salt: root shadow field = '%s'\n", passwd);
    if (passwd[0] != '$' || (p = strrchr(passwd + 1, '$')) == NULL) {
        printf("pick_salt: not a $-prefixed hash, falling back to 'xx' (DES)\n");
        return salt_buf;
    }
    typelen = p - passwd + 1;
    if (typelen >= sizeof(salt_buf)) typelen = sizeof(salt_buf) - 1;
    memcpy(salt_buf, passwd, typelen);
    salt_buf[typelen] = '\0';
    printf("pick_salt: using salt prefix '%s'\n", salt_buf);
    return salt_buf;
}

int main(int argc, char **argv) {
    const char *user = argc > 1 ? argv[1] : "alice";
    const char *password = argc > 2 ? argv[2] : "AAAA";

    struct spwd *sp = getspnam(user);
    char pw_password_buf[256];
    strncpy(pw_password_buf, sp ? sp->sp_pwdp : "", sizeof(pw_password_buf) - 1);
    pw_password_buf[sizeof(pw_password_buf) - 1] = '\0';
    const char *pw_password = pw_password_buf;
    printf("target user '%s' shadow field = '%s' (len=%zu)\n", user, pw_password, strlen(pw_password));

    const char *salt = NULL;
    if (pw_password[0] && pw_password[1]) {
        salt = pw_password;
        printf("using target's OWN field as salt: '%s'\n", salt);
    } else {
        salt = pick_salt();
    }

    char *result = crypt(password, salt);
    printf("crypt(password[len=%zu], salt='%s') = '%s'\n", strlen(password), salt, result ? result : "(null)");
    printf("matches stored field '%s'? %s\n", pw_password,
           (result && strcmp(result, pw_password) == 0) ? "YES <-- AUTH BYPASS" : "no");
    return 0;
}
