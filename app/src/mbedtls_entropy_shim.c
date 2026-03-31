#include <stdint.h>
#include <stddef.h>

#include <zephyr/random/random.h>
#include <zephyr/sys/util.h>

#include <mbedtls/entropy.h>

int mbedtls_hardware_poll(void *data, unsigned char *output, size_t len, size_t *olen)
{
	ARG_UNUSED(data);

	if ((output == NULL) || (olen == NULL) || (len == 0U)) {
		return MBEDTLS_ERR_ENTROPY_SOURCE_FAILED;
	}

	if (sys_csrand_get(output, len) != 0) {
		return MBEDTLS_ERR_ENTROPY_SOURCE_FAILED;
	}

	*olen = len;
	return 0;
}
