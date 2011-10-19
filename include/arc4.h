#ifndef _CRYPTO_ARC4_H_
#define _CRYPTO_ARC4_H_
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct rc4_key
{
     unsigned char state[256];
     unsigned char x;
     unsigned char y;
} rc4_key;

void prepare_key(unsigned char *key_data_ptr, int key_data_len, rc4_key *key);
void rc4(unsigned char *buffer_ptr, int buffer_len, rc4_key *key);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _CRYPTO_ARC4_H_ */
