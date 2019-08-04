#ifndef CRYPTO_H_
#define CRYPTO_H_

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string>

#include <windows.h>
#include <Wincrypt.h>

using namespace std;

class Crypto {
public:
	Crypto();

	static const int MD5_BUFSIZE = 1024;
	static const int MD5_LEN = 16;
	static const int MD5_LEN_HEX = 2 * MD5_LEN;

	struct md5_digest_return {
		DWORD status;
		char errorMsg[1000];
		char val[MD5_LEN_HEX];
	};

	//typedef struct md5_digest_return md5_digest_return;

	md5_digest_return md5_digest(string s);
};

#endif /* CRYPTO_H_ */
