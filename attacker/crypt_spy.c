/* LD_PRELOAD shim: logs every crypt(3) call (password length + salt) made
 * by the process it's injected into, then calls through to the real musl
 * crypt(). Used to see exactly what sshd passes to crypt() during a login
 * attempt, without needing to read opaque strace/privsep pipe traffic. */
#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdio.h>
#include <string.h>

// Defines a function that intercepts calls to the crypt() function, logs the key length and salt used, and then calls the real crypt() function. 
// It also logs the result of the crypt() call.
char *crypt(const char *key, const char *salt) {
    static char *(*real_crypt)(const char *, const char *) = NULL;
    if (!real_crypt)
        real_crypt = dlsym(RTLD_NEXT, "crypt");

    FILE *f = fopen("/tmp/crypt_spy.log", "a");
    if (f) {
        fprintf(f, "crypt() called: key_len=%zu salt='%s'\n", strlen(key), salt ? salt : "(null)");
        fflush(f);
        fclose(f);
    }

    char *result = real_crypt(key, salt);

    f = fopen("/tmp/crypt_spy.log", "a");
    if (f) {
        fprintf(f, "crypt() returned: '%s'\n", result ? result : "(null)");
        fflush(f);
        fclose(f);
    }
    return result;
}

// Defines a function that intercepts calls to the strcmp() function, logs the strings being compared and the result of the comparison, 
// and then calls the real strcmp() function.
int strcmp(const char *a, const char *b) {
    static int (*real_strcmp)(const char *, const char *) = NULL;
    if (!real_strcmp)
        real_strcmp = dlsym(RTLD_NEXT, "strcmp");
    int r = real_strcmp(a, b);
    /* Only log comparisons that look relevant (short strings / crypt output)
     * to avoid flooding the log with unrelated libc-internal strcmp calls. */
    if ((a && strlen(a) < 140) && (b && strlen(b) < 140)) {
        FILE *f = fopen("/tmp/crypt_spy.log", "a");
        if (f) {
            fprintf(f, "strcmp('%s', '%s') = %d\n", a, b, r);
            fflush(f);
            fclose(f);
        }
    }
    return r;
}
