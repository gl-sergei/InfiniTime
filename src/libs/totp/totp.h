#pragma once

#ifdef __cplusplus
extern "C" {
#endif

int generateCode(const uint8_t *secret, uint8_t secret_length, unsigned long tm,
                 int verification_code_modulus);

#ifdef __cplusplus
}
#endif

