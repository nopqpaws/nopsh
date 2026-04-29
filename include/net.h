#ifndef NET_H
#define NET_H

// http_download() – GET url and write the body to outfile.
//                   Returns 0 on success, non-zero on error.
int http_download(const char *url, const char *outfile);

#endif