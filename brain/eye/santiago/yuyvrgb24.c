/* v4l-utils-0.8.5/lib/libv4lconvert/rgbyuv.c
 * Code from v4l converter project 
 */

#define CLIP(color) (unsigned char)(((color) > 0xFF) ? 0xff : (((color) < 0) ? 0 : (color)))

void v4lconvert_yuyv_to_rgb24(const unsigned char *src, unsigned char *dest, int width, int height)
{
	int j;

	while (--height >= 0) {
		for (j = 0; j < width; j += 2) {
			int u = src[1];
			int v = src[3];
			int u1 = (((u - 128) << 7) +  (u - 128)) >> 6;
			int rg = (((u - 128) << 1) +  (u - 128) +
				((v - 128) << 2) + ((v - 128) << 1)) >> 3;
			int v1 = (((v - 128) << 1) +  (v - 128)) >> 1;

			*dest++ = CLIP(src[0] + u1);
			*dest++ = CLIP(src[0] - rg);
			*dest++ = CLIP(src[0] + v1);

			*dest++ = CLIP(src[2] + u1);
			*dest++ = CLIP(src[2] - rg);
			*dest++ = CLIP(src[2] + v1);
			src += 4;
		}
	}
}
