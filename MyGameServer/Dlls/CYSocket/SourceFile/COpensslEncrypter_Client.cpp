#include "../stdafx.h"
#include "COpensslEncrypter_Client.h"

COpensslEncrypter_Client::COpensslEncrypter_Client() :
_running(false)
{
}

COpensslEncrypter_Client::~COpensslEncrypter_Client()
{
	if (_running)
	{
		RSA_free(_clientPrivateRsaKey);
		_running = false;
	}
	
}

bool COpensslEncrypter_Client::SetAndGenerateRsaKey(unsigned char *publicRsaBufferAndBuffer, int nLen)
{
	if (_running)
	{
		RSA_free(_clientPrivateRsaKey);
		_running = false;
	}

	int nPublicRsaKeyLen = nLen - RSA_DECRY_LEN;
	if (nLen <= 0)
		return false;

	unsigned char *pBuffer = publicRsaBufferAndBuffer;
	_clientPrivateRsaKey = d2i_RSAPublicKey(NULL, (const unsigned char**)&pBuffer, nPublicRsaKeyLen);

	pBuffer = publicRsaBufferAndBuffer + nPublicRsaKeyLen;
	RSA_public_decrypt(RSA_DECRY_LEN, (const unsigned char*)pBuffer, _aesSeed, _clientPrivateRsaKey, RSA_NO_PADDING);

	AES_set_encrypt_key(_aesSeed, 256, &_aesEncrypterKey);
	AES_set_decrypt_key(_aesSeed, 256, &_aesDecrypterKey);

	_running = true;
	return true;
}

bool COpensslEncrypter_Client::Encrypter(const char * inBuffer,
	char * outBuffer,
	int inLen,
	int & outLen)
{
	if (!_running)
		return false;

	if (inBuffer == NULL || outBuffer == NULL || inLen <= 0)
		return false;

	const unsigned char *pInBufferBegin = (const unsigned char *)inBuffer;
	unsigned char *pOutBufferBegin = (unsigned char *)outBuffer;

	outLen = 0;
	for (int i = 0; i < inLen; i += AES_ENCRY_LEN)
	{
		pInBufferBegin += outLen;
		pOutBufferBegin += outLen;
		AES_encrypt(pInBufferBegin, pOutBufferBegin, &_aesEncrypterKey);
		outLen += AES_ENCRY_LEN;
// 		if (nEndLen < AES_ENCRY_LEN)
// 		{
// 			outLen += nEndLen;
// 		}
	}

	return true;
}

bool COpensslEncrypter_Client::Decrypter(const char * inBuffer,
	char * outBuffer,
	int inLen,
	int & outLen)
{
	if (!_running)
		return false;

	if (inBuffer == NULL || outBuffer == NULL || inLen <= 0 || inLen % AES_ENCRY_LEN != 0)
		return false;

	const unsigned char *pInBufferBegin = (const unsigned char *)inBuffer;
	unsigned char *pOutBufferBegin = (unsigned char *)outBuffer;

	for (int i = 0; i < inLen; i += AES_ENCRY_LEN)
	{
		pInBufferBegin += outLen;
		pOutBufferBegin += outLen;
		AES_decrypt(pInBufferBegin, pOutBufferBegin, &_aesDecrypterKey);
		outLen += AES_ENCRY_LEN;
	}

	//assert(inLen == outLen + nEndLen);
	return true;
}
