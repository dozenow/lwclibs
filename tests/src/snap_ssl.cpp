#include "snap_ssl.hxx"

#include <iostream>

#include <openssl/err.h>

/* openSSL documentation is the fucking worst. What is their problem? */

#define CERT_DIR "/home/litton/snap/tests/certs/"

#define SERVER_CERT CERT_DIR "ca/certs/server.cert.pem"
#define SERVER_KEY CERT_DIR "ca/private/server.key.pem"

#define CLIENT_CERT CERT_DIR "ca/certs/client.cert.pem"
#define CLIENT_KEY CERT_DIR "ca/private/client.key.pem"

#define CA_CERT CERT_DIR "ca/certs/ca.cert.pem"

#include "shared_malloc.h"

using namespace std;

namespace snapper {
namespace ssl {

static void ssl_init() {
	static bool initialized(false);
	if (!initialized) {

		if (CRYPTO_set_mem_functions(sh_malloc, sh_realloc, sh_free) != 1) {
			cerr << "already did allocations with ssl?" << endl;
			throw "up";
		}

		SSL_library_init();
		OpenSSL_add_all_algorithms();
		SSL_load_error_strings();
	}
	initialized = true;
}

server::server() : m_ctx(nullptr) {
	ssl_init();

	m_ctx = SSL_CTX_new(TLSv1_method());

	if (SSL_CTX_load_verify_locations(m_ctx, CA_CERT, nullptr) != 1) {
		goto fail;
	}

	if (SSL_CTX_use_certificate_file(m_ctx, SERVER_CERT, SSL_FILETYPE_PEM) != 1) {
		goto fail;
	}

	if (SSL_CTX_use_PrivateKey_file(m_ctx, SERVER_KEY, SSL_FILETYPE_PEM) != 1) {
		goto fail;
	}

	SSL_CTX_set_verify(m_ctx, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT | SSL_VERIFY_CLIENT_ONCE, nullptr);
	return;

 fail:
	SSL_CTX_free(m_ctx);
	m_ctx = nullptr;
	ERR_print_errors_fp(stderr);
	throw "up";
	
}

client::client() : m_ctx(nullptr) {
	ssl_init();

	m_ctx = SSL_CTX_new(TLSv1_method());

	if (SSL_CTX_load_verify_locations(m_ctx, CA_CERT, nullptr) != 1) {
		goto fail;
	}

	if (SSL_CTX_use_certificate_file(m_ctx, CLIENT_CERT, SSL_FILETYPE_PEM) != 1) {
		goto fail;
	}

	if (SSL_CTX_use_PrivateKey_file(m_ctx, CLIENT_KEY, SSL_FILETYPE_PEM) != 1) {
		goto fail;
	}

	SSL_CTX_set_verify(m_ctx, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, nullptr);
	return;

 fail:
	SSL_CTX_free(m_ctx);
	m_ctx = nullptr;
	ERR_print_errors_fp(stderr);
	throw "up";
	
}


static void print_ssl_err(int ssl_err) {
	switch(ssl_err) {
	case SSL_ERROR_NONE:
		cerr << "SSL_ERROR_NONE" << endl;
		break;
	case SSL_ERROR_ZERO_RETURN:
		cerr << "SSL_ERROR_ZERO_RETURN" << endl;
		break;
	case SSL_ERROR_WANT_READ:
		cerr << "SSL_ERROR_WANT_READ" << endl;
		break;
	case SSL_ERROR_WANT_WRITE:
		cerr << "SSL_ERROR_WANT_WRITE" << endl;
		break;
	case SSL_ERROR_WANT_CONNECT:
		cerr << "SSL_ERROR_WANT_CONNECT" << endl;
		break;
	case SSL_ERROR_WANT_ACCEPT:
		cerr << "SSL_ERROR_WANT_ACCEPT" << endl;
		break;
	case SSL_ERROR_WANT_X509_LOOKUP:
		cerr << "SSL_ERROR_WANT_X509_LOOKUP" << endl;
		break;
	case SSL_ERROR_SYSCALL:
		cerr << "SSL_ERROR_SYSCALL" << endl;
		break;
	case SSL_ERROR_SSL:
		cerr << "SSL_ERROR_SSL" << endl;
		break;
	default:
		cerr << "Unknown error: " << ssl_err << endl;
		break;
	}

}

unique_ptr<cxn> server::accept(int fd) {
	SSL *ssl(nullptr);
	BIO *bio(nullptr);

	ssl = SSL_new(m_ctx);

	bio = BIO_new(BIO_s_socket());
	BIO_set_fd(bio, fd, BIO_NOCLOSE);
	SSL_set_bio(ssl, bio, bio);

	//BIO_free(bio); /* refcnt incremented in set bio? */
	//bio = nullptr;

	int ret = SSL_accept(ssl);
	if (ret == 1) {
		return unique_ptr<cxn>(new cxn(ssl));
	} else {
		print_ssl_err(SSL_get_error(ssl, ret));
		goto fail;
	}
		
 fail:
	SSL_free(ssl);
	ssl = nullptr;
	BIO_free(bio);
	bio = nullptr;
	return unique_ptr<cxn>(nullptr);

}

unique_ptr<cxn> client::connect(int fd) {
	SSL *ssl(nullptr);
	BIO *bio(nullptr);

	ssl = SSL_new(m_ctx);

	bio = BIO_new(BIO_s_socket());
	BIO_set_fd(bio, fd, BIO_NOCLOSE);
	SSL_set_bio(ssl, bio, bio);

	//BIO_free(bio); /* refcnt incremented in set bio? */
	//bio = nullptr;

	int ret = SSL_connect(ssl);
	if (ret == 1) {
		return unique_ptr<cxn>(new cxn(ssl));
	} else {
		print_ssl_err(SSL_get_error(ssl, ret));
		goto fail;
	}
		
 fail:
	SSL_free(ssl);
	ssl = nullptr;
	BIO_free(bio);
	bio = nullptr;
	return unique_ptr<cxn>(nullptr);

}

int cxn::read(uint8_t *rbuf, size_t len) {
	int ret = SSL_read(m_ssl, rbuf, len);
	if (ret < 0) {
		print_ssl_err(SSL_get_error(m_ssl, ret));
	}
	return ret;
}

int cxn::write(const uint8_t *wbuf, size_t len) {
	int ret = SSL_write(m_ssl, wbuf, len);
	if (ret < 0) {
		print_ssl_err(SSL_get_error(m_ssl, ret));
	}
	return ret;
}



int dumb_cxn::read(uint8_t *rbuf, size_t len) {
	int ret = ::read(m_fd, rbuf, len);
	if (ret < 0) {
		perror("bad dumb read: ");
	}
	return ret;
}

int dumb_cxn::write(const uint8_t *wbuf, size_t len) {
	int ret = ::write(m_fd, wbuf, len);
	if (ret < 0) {
		perror("bad dumb write: ");
	}
	return ret;
}


};
};

