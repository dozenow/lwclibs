#ifndef SNAP_SSL_HXX
#define SNAP_SSL_HXX

#include <memory>

#include <openssl/ssl.h>


namespace snapper {
namespace ssl {

class server;

class cxn {
public:
	cxn(SSL *ssl) : m_ssl(ssl) {}
	int read(uint8_t *rbuf, size_t len);
	int write(const uint8_t *wbuf, size_t len);
private:
	SSL *m_ssl;
};

class dumb_cxn {
public:
	dumb_cxn(int fd) : m_fd(fd) {}
	int read(uint8_t *rbuf, size_t len);
	int write(const uint8_t *wbuf, size_t len);
private:
	int m_fd;
};


class server {
public:
	server();
	::std::unique_ptr<cxn> accept(int fd); // assume fd points to a live socket
private:
	SSL_CTX *m_ctx;
};

class client {
public:
	client();
	::std::unique_ptr<cxn> connect(int fd); //assumes fd points to a live socket
private:
	SSL_CTX *m_ctx;
};

};
};

#endif
