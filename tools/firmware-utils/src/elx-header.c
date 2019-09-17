#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#include <endian.h>	/* for __BYTE_ORDER */
#include <zlib.h>	/* for crc32 */
#include "md5.h"	/* for md5sum */

static int image_type = 3;
static int hw_id = 0;
static size_t data_len = 0;
static int is_hex_pattern;

struct kernel {
	uint32_t space1;
	uint32_t image_type;
	uint8_t unknown1[34];	/* not required */
	uint32_t hw_id;
	char ver_id[16];		/* not required */
	uint32_t data_len;
	uint32_t space2;
	uint8_t md5sum[16];
	uint8_t unknown2[34];	/* not required */
	uint32_t header_crc;	/* unknown, not required */
	uint32_t magic_key;		/* unknown, not required */
} __attribute ((packed));

struct kernel_elecom1701 {
	char id[8];
	char product[32];		/* not required */
	char fw_ver1[16];
	uint32_t magic;
	uint32_t data_len;
	uint16_t image_type;
	uint16_t comp_type;
	uint32_t hw_id;
	uint32_t timestamp;
	uint32_t space1;
	uint32_t space2;
	char fw_ver2[16];		/* not required */
	char code_ver[16];		/* not required */
	char config_ver[32];	/* not required */
	char model_name[16];	/* not required */
	uint32_t data_crc;
	uint32_t header_crc;	/* unknown, not required */
} __attribute ((packed));

#define ELECOM1701_ID	"ELECOM"
#define ELECOM1701_MAGIC	0x031d6129
#define DEF_COMP_TYPE	0x0
#define DEF_FW_VER		"0.00"

#if (__BYTE_ORDER == __LITTLE_ENDIAN)
#	define HOST_TO_LE16(x)	(x)
#	define HOST_TO_LE32(x)	(x)
#	define HOST_TO_BE32(x)	bswap_32(x)
#else
#	define HOST_TO_LE16(x)	bswap_16(x)
#	define HOST_TO_LE32(x)	bswap_32(x)
#	define HOST_TO_BE32(x)	(x)
#endif

static void
xor_data(uint8_t *data, size_t len, const char *pattern, int p_len, int p_off)
{
	int offset = 0;
	while (len--) {
		*data ^= pattern[offset];
		data++;
		offset = (offset + 1) % p_len;
	}
}

static void add_kernel_hdr(unsigned char *buf, size_t data_len)
{
	MD5_CTX ctx;
	struct kernel header = {
		0,
		HOST_TO_BE32(3),
		"",
		HOST_TO_BE32(hw_id),
		"",
		HOST_TO_BE32(data_len),
	};

	MD5_Init(&ctx);
	MD5_Update(&ctx, buf + sizeof(struct kernel), data_len);
	MD5_Final(header.md5sum, &ctx);

	memcpy(buf, &header, sizeof(struct kernel));
}

static void add_elecom1701_hdr(unsigned char *buf, size_t data_len)
{
	unsigned long crc;
	struct kernel_elecom1701 header = {
		ELECOM1701_ID,
		"",
		DEF_FW_VER,
		HOST_TO_BE32(ELECOM1701_MAGIC),
		HOST_TO_BE32(data_len),
		HOST_TO_LE16(6),
		DEF_COMP_TYPE,
		HOST_TO_BE32(hw_id),
		HOST_TO_LE32(time(NULL)),
		0, 0,
		DEF_FW_VER,
	};

	printf("timestamp: %x\n", header.timestamp);

	crc = crc32(0, buf + sizeof(struct kernel_elecom1701), data_len);
	printf("crc32: %lx\n", crc);
	header.data_crc = HOST_TO_BE32(crc);

	memcpy(buf, &header, sizeof(struct kernel_elecom1701));
}

int main(int argc, char **argv)
{
	int c, tmp, p_len;
	unsigned int hex_buf;
	char *e, *ifn = NULL, *ofn = NULL, *pattern = NULL;
	char hex_pattern[128];
	unsigned char *buf;
	size_t hdr_len; 
	struct stat st;
	FILE *in, *out;

	while ((c = getopt(argc, argv, "i:o:p:t:w:x")) != -1) {
		switch (c) {
		case 'i':
			ifn = optarg;
			break;
		case 'o':
			ofn = optarg;
			break;
		case 'p':
			pattern = optarg;
			break;
		case 't':
			tmp = strtol(optarg, NULL, 10);
			if (tmp == 3 || tmp == 6) {
				image_type = tmp;
			} else {
				fprintf(stderr, "invalid image type\n");
				return 1;
			}
			break;
		case 'w':
			tmp = strtol(optarg, &e, 16);
			if (strlen(optarg) != 8 || *e != '\0'){
				fprintf(stderr, "invalid hardware ID\n");
				return 1;
			}
			hw_id = tmp;
			break;
		case 'x':
			is_hex_pattern = true;
			printf("hex pattern mode\n");
			break;
		}
	}

	in = fopen(ifn, "r");
	if (!in) {
		perror("can not open input file");
		//usage();
	}

	out = fopen(ofn, "w");
	if (!out) {
		perror("can not open output file");
		return 1;
		//usage();
	}

	if (stat(ifn, &st)) {
		fprintf(stderr, "stat failed on %s\n", ifn);
		return 1;
	}

	data_len = st.st_size;
//	printf("data_len: 0x%lx\n", data_len);
	if (image_type == 3)
		hdr_len = sizeof(struct kernel);
	else if (image_type == 6)
		hdr_len = sizeof(struct kernel_elecom1701);
//	printf("hdr_len: 0x%lx\n", hdr_len);
	buf = malloc(hdr_len + data_len);

	size_t read =
		fread(buf + hdr_len, 1, data_len, in);
//	printf("read: %lx\n", read);
	if (read != data_len){
		fprintf(stderr, "read err\n");
		return 1;
	}

	p_len = strlen(pattern);
	if (p_len == 0) {
		fprintf(stderr, "pattern cannot be empty\n");
		//usage();
		return 1;
	}

	if (is_hex_pattern) {
		int i;

		if ((p_len / 2) > sizeof(hex_pattern)) {
			fprintf(stderr, "provided hex pattern is too long\n");
			//usage();
			return 1;
		}

		if (p_len % 2 != 0) {
			fprintf(stderr, "the number of characters (hex) is incorrect\n");
			//usage();
			return 1;
		}

		for (i = 0; i < (p_len / 2); i++) {
			if (sscanf(pattern + (i * 2), "%2x", &hex_buf) < 0) {
				fprintf(stderr, "invalid hex digit around %d\n", i * 2);
				return 1;
			}
			hex_pattern[i] = (char)hex_buf;
		}
		printf("hex_pattern: %s\n", hex_pattern);
		printf("pattern: %s\n", pattern);
	}

	if (image_type == 3) {
		add_kernel_hdr(buf, data_len);

		if (is_hex_pattern)
			xor_data(buf + hdr_len, data_len, hex_pattern, (p_len / 2), 0);
		else
			xor_data(buf + hdr_len, data_len, pattern, p_len, 0);

	} else if (image_type == 6) {
		if (is_hex_pattern)
			xor_data(buf + hdr_len, data_len, hex_pattern, (p_len / 2), 0);
		else
			xor_data(buf + hdr_len, data_len, pattern, p_len, 0);

		add_elecom1701_hdr(buf, data_len);
	}

	fwrite(buf, 1, hdr_len + data_len, out);

	fclose(in);
	fclose(out);
}