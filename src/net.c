#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>       // read(), write(), close()
#include <sys/socket.h>   // socket(), connect(), recv()
#include <netdb.h>        // getaddrinfo()
#include <arpa/inet.h>    // inet_ntoa(), htons()
#include <fcntl.h>        // open()
#include <sys/stat.h>     // file permissions

#include "net.h"

#define NET_BUF_SIZE 4096 // size of our read buffer

/*
    ----- HTTP downloader -----

    A minimal HTTP client for the getrun feature. Opens a TCP socket,
    fires off a plain HTTP/1.0 GET, skips the headers, writes the body to disk.

    Doesn't do HTTPS, redirects, or chunked encoding because it doesn't need to.
    This code assumes port 80. URLs with custom ports will fail. Feel free to 
    add that functionality if you choose.

    Only http_download() is public, everything else is static to this file.
*/


/*
parse_url() - split a URL into host and path

    "http://example.com/test/file"
                |               |
    host = "example.com"   path = "/test/file"
*/
static void parse_url(const char *url,
                      char *host, size_t host_len,
                      char *path, size_t path_len) {


    const char *p = url;

    // skip "http://" if present
    const char *scheme = strstr(url, "://");
    if (scheme) {
        p = scheme + 3;  // move past "://"
    }

    // find the first slash after the host
    const char *slash = strchr(p, '/');

    if (!slash) {
        // URL has no path, treat as "/"
        strncpy(host, p, host_len - 1);
        host[host_len - 1] = '\0';

        strncpy(path, "/", path_len - 1);
        path[path_len - 1] = '\0';
    } else {
        // copy host portion
        size_t host_size = (size_t)(slash - p);
        if (host_size >= host_len) {
            host_size = host_len - 1;
        }
        memcpy(host, p, host_size);
        host[host_size] = '\0';

        // copy path portion
        strncpy(path, slash, path_len - 1);
        path[path_len - 1] = '\0';
    }
}

/*
http_download() - GET a file over a raw TCP socket and write it to outfile.
Plain HTTP/1.0 only. No HTTPS or redirects.
*/
int http_download(const char *url, const char *outfile) {
    char host[256];
    char path[1024];

    // break URL into host + path
    parse_url(url, host, sizeof(host), path, sizeof(path));

    struct addrinfo hints;
    struct addrinfo *res = NULL;
    int sock = -1;
    int fd = -1;
    int ret = -1;   // assume failure unless we succeed

    
    memset(&hints, 0, sizeof(hints)); // zero out hints structure
    hints.ai_family = AF_INET;        // IPv4 only
    hints.ai_socktype = SOCK_STREAM;  // TCP

    if (getaddrinfo(host, "80", &hints, &res) != 0) {
        perror("getaddrinfo");
        goto cleanup;
    }

    sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sock < 0) {
        perror("socket");
        goto cleanup;
    }

    if (connect(sock, res->ai_addr, res->ai_addrlen) < 0) {
        perror("connect");
        goto cleanup;
    }

    // build a HTTP/1.0 GET request
    char req[2048];
    int req_len = snprintf(req, sizeof(req),
                           "GET %s HTTP/1.0\r\n"
                           "Host: %s\r\n"
                           "User-Agent: nopsh/1.0\r\n"
                           "Connection: close\r\n"
                           "\r\n",
                           path, host);

    if (req_len <= 0 || req_len >= (int)sizeof(req)) {
        fprintf(stderr, "http_download: request too long\n");
        goto cleanup;
    }

    if (send(sock, req, (size_t)req_len, 0) < 0) {
        perror("send");
        goto cleanup;
    }

    fd = open(outfile, O_CREAT | O_WRONLY | O_TRUNC, 0700);
    if (fd < 0) {
        perror("open");
        goto cleanup;
    }

    char buf[NET_BUF_SIZE];
    ssize_t n;
    int header_done = 0;

    while ((n = recv(sock, buf, sizeof(buf), 0)) > 0) {

        if (!header_done) {
            // look for end of HTTP headers: "\r\n\r\n"
            char *hdr_end = strstr(buf, "\r\n\r\n");

            if (hdr_end) {
                header_done = 1;

                // move pointer to start of body
                char *body = hdr_end + 4;
                size_t body_len = (size_t)(n - (body - buf));

                // write body to file
                if (write(fd, body, body_len) < 0) {
                    perror("write");
                    goto cleanup;
                }
            }

            // if no header end found, we ignore this chunk
        } else {
            // headers already skipped,  write raw data
            if (write(fd, buf, (size_t)n) < 0) {
                perror("write");
                goto cleanup;
            }
        }
    }

    if (n < 0) {
        perror("recv");
        goto cleanup;
    }

    ret = 0; // success

cleanup:
    if (fd >= 0) close(fd);
    if (sock >= 0) close(sock);
    if (res) freeaddrinfo(res);

    return ret;
}
