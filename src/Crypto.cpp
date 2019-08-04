#include "Crypto.h"

using namespace std;

Crypto::Crypto() {
}

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string>

using namespace std;

wchar_t *convertCharArrayToLPCWSTR(const char* charArray) {
    wchar_t* wString=new wchar_t[4096];
    MultiByteToWideChar(CP_ACP, 0, charArray, -1, wString, 4096);
    return wString;
}

Crypto::md5_digest_return Crypto::md5_digest(string s) {
	DWORD dwStatus = 0;
	BOOL bResult = FALSE;
	HCRYPTPROV hProv = 0;
	HCRYPTHASH hHash = 0;
	HANDLE hFile = NULL;
	BYTE rgbFile[MD5_BUFSIZE];
	DWORD cbRead = 0;
	BYTE rgbHash[MD5_LEN];
	DWORD cbHash = 0;
	CHAR rgbDigits[] = "0123456789abcdef";
	//LPCWSTR filename = TEXT(s.c_str());
	//LPCWSTR filename = convertCharArrayToLPCWSTR(s.c_str());
	// Logic to check usage goes here.
	const wchar_t *c = convertCharArrayToLPCWSTR(s.c_str());

	hFile = CreateFile(c, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);

	if (INVALID_HANDLE_VALUE == hFile) {
		dwStatus = GetLastError();

		md5_digest_return ret;

		ret.status = dwStatus;
		memset(ret.val, '\0', MD5_LEN_HEX);

		snprintf(ret.errorMsg, 1000 - 1, "Error opening file");

		return ret;
	}

	// Get handle to the crypto provider
	if (!CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL,
			CRYPT_VERIFYCONTEXT)) {
		dwStatus = GetLastError();

		CloseHandle(hFile);

		md5_digest_return ret;

		ret.status = dwStatus;
		memset(ret.val, '\0', MD5_LEN_HEX);

		snprintf(ret.errorMsg, 1000 - 1, "CryptAcquireContext failed");

		return ret;
	}

	if (!CryptCreateHash(hProv, CALG_MD5, 0, 0, &hHash)) {
		dwStatus = GetLastError();

		CloseHandle(hFile);
		CryptReleaseContext(hProv, 0);

		md5_digest_return ret;

		ret.status = dwStatus;
		memset(ret.val, '\0', MD5_LEN_HEX);

		snprintf(ret.errorMsg, 1000 - 1, "CryptAcquireContext failed");

		return ret;
	}

	while (bResult = ReadFile(hFile, rgbFile, MD5_BUFSIZE, &cbRead, NULL)) {
		if (0 == cbRead) {
			break;
		}

		if (!CryptHashData(hHash, rgbFile, cbRead, 0)) {
			dwStatus = GetLastError();

			CryptReleaseContext(hProv, 0);
			CryptDestroyHash(hHash);
			CloseHandle(hFile);

			md5_digest_return ret;

			ret.status = dwStatus;
			memset(ret.val, '\0', MD5_LEN_HEX);

			snprintf(ret.errorMsg, 1000 - 1, "CryptHashData failed");

			return ret;
		}
	}

	if (!bResult) {
		dwStatus = GetLastError();

		CryptReleaseContext(hProv, 0);
		CryptDestroyHash(hHash);
		CloseHandle(hFile);

		md5_digest_return ret;

		ret.status = dwStatus;
		memset(ret.val, '\0', MD5_LEN_HEX);

		snprintf(ret.errorMsg, 1000 - 1, "ReadFile failed");

		return ret;
	}

	md5_digest_return ret;

	cbHash = MD5_LEN;
	if (CryptGetHashParam(hHash, HP_HASHVAL, rgbHash, &cbHash, 0)) {
		//printf("MD5 hash of file %s is: ", filename);
		for (DWORD i = 0; i < cbHash; i++) {
			//printf("%c%c", rgbDigits[rgbHash[i] >> 4], rgbDigits[rgbHash[i] & 0xf]);

			memset(ret.val + i * 2, rgbDigits[rgbHash[i] >> 4], 1);
			memset(ret.val + i * 2 + 1, rgbDigits[rgbHash[i] & 0xf], 1);
		}
	} else {
		dwStatus = GetLastError();

		memset(ret.val, '\0', MD5_LEN_HEX);

		snprintf(ret.errorMsg, 1000 - 1, "CryptGetHashParam failed");
	}

	CryptDestroyHash(hHash);
	CryptReleaseContext(hProv, 0);
	CloseHandle(hFile);

	ret.status = dwStatus;

	return ret;
}
