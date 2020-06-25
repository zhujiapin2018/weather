/* SPI Master example: jpeg decoder.

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

/*
The image used for the effect on the LCD in the SPI master example is stored in flash
as a jpeg file. This file contains the decode_image routine, which uses the tiny JPEG
decoder library to decode this JPEG into a format that can be sent to the display.

Keep in mind that the decoder library cannot handle progressive files (will give
``Image decoder: jd_prepare failed (8)`` as an error) so make sure to save in the correct
format if you want to use a different image file.
*/

#include "decode_image.h"
#include "tjpgd.h"
#include "esp_log.h"
#include <string.h>

//Reference the binary-included jpeg file
//extern const uint8_t image_jpg_start[] asm("_binary_image_jpg_start");
//extern const uint8_t image_jpg_end[] asm("_binary_image_jpg_end");
extern const uint8_t image_jpg0_start[] asm("_binary_img0_jpg_start");
extern const uint8_t image_jpg0_end[]   asm("_binary_img0_jpg_end");
extern const uint8_t image_jpg1_start[] asm("_binary_img1_jpg_start");
extern const uint8_t image_jpg1_end[]   asm("_binary_img1_jpg_end");
extern const uint8_t image_jpg2_start[] asm("_binary_img2_jpg_start");
extern const uint8_t image_jpg2_end[]   asm("_binary_img2_jpg_end");
extern const uint8_t image_jpg3_start[] asm("_binary_img3_jpg_start");
extern const uint8_t image_jpg3_end[]   asm("_binary_img3_jpg_end");
extern const uint8_t image_jpg4_start[] asm("_binary_img4_jpg_start");
extern const uint8_t image_jpg4_end[]   asm("_binary_img4_jpg_end");
extern const uint8_t image_jpg5_start[] asm("_binary_img5_jpg_start");
extern const uint8_t image_jpg5_end[]   asm("_binary_img5_jpg_end");
extern const uint8_t image_jpg6_start[] asm("_binary_img6_jpg_start");
extern const uint8_t image_jpg6_end[] asm("_binary_img6_jpg_end");
extern const uint8_t image_jpg7_start[] asm("_binary_img7_jpg_start");
extern const uint8_t image_jpg7_end[] asm("_binary_img7_jpg_end");
extern const uint8_t image_jpg8_start[] asm("_binary_img8_jpg_start");
extern const uint8_t image_jpg8_end[] asm("_binary_img8_jpg_end");
extern const uint8_t image_jpg9_start[] asm("_binary_img9_jpg_start");
extern const uint8_t image_jpg9_end[] asm("_binary_img9_jpg_end");
extern const uint8_t image_jpg10_start[] asm("_binary_img10_jpg_start");
extern const uint8_t image_jpg10_end[] asm("_binary_img10_jpg_end");
extern const uint8_t image_jpg11_start[] asm("_binary_img11_jpg_start");
extern const uint8_t image_jpg11_end[] asm("_binary_img11_jpg_end");
extern const uint8_t image_jpg12_start[] asm("_binary_img12_jpg_start");
extern const uint8_t image_jpg12_end[] asm("_binary_img12_jpg_end");
extern const uint8_t image_jpg13_start[] asm("_binary_img13_jpg_start");
extern const uint8_t image_jpg13_end[] asm("_binary_img13_jpg_end");
extern const uint8_t image_jpg14_start[] asm("_binary_img14_jpg_start");
extern const uint8_t image_jpg14_end[] asm("_binary_img14_jpg_end");
extern const uint8_t image_jpg15_start[] asm("_binary_img15_jpg_start");
extern const uint8_t image_jpg15_end[] asm("_binary_img15_jpg_end");
extern const uint8_t image_jpg16_start[] asm("_binary_img16_jpg_start");
extern const uint8_t image_jpg16_end[] asm("_binary_img16_jpg_end");
extern const uint8_t image_jpg17_start[] asm("_binary_img17_jpg_start");
extern const uint8_t image_jpg17_end[] asm("_binary_img17_jpg_end");
extern const uint8_t image_jpg18_start[] asm("_binary_img18_jpg_start");
extern const uint8_t image_jpg18_end[] asm("_binary_img18_jpg_end");
extern const uint8_t image_jpg19_start[] asm("_binary_img19_jpg_start");
extern const uint8_t image_jpg19_end[] asm("_binary_img19_jpg_end");
extern const uint8_t image_jpg20_start[] asm("_binary_img20_jpg_start");
extern const uint8_t image_jpg20_end[] asm("_binary_img20_jpg_end");
extern const uint8_t image_jpg21_start[] asm("_binary_img21_jpg_start");
extern const uint8_t image_jpg21_end[] asm("_binary_img21_jpg_end");
extern const uint8_t image_jpg22_start[] asm("_binary_img22_jpg_start");
extern const uint8_t image_jpg22_end[] asm("_binary_img22_jpg_end");
extern const uint8_t image_jpg23_start[] asm("_binary_img23_jpg_start");
extern const uint8_t image_jpg23_end[] asm("_binary_img23_jpg_end");
extern const uint8_t image_jpg24_start[] asm("_binary_img24_jpg_start");
extern const uint8_t image_jpg24_end[] asm("_binary_img24_jpg_end");
extern const uint8_t image_jpg25_start[] asm("_binary_img25_jpg_start");
extern const uint8_t image_jpg25_end[] asm("_binary_img25_jpg_end");
extern const uint8_t image_jpg26_start[] asm("_binary_img26_jpg_start");
extern const uint8_t image_jpg26_end[] asm("_binary_img26_jpg_end");
extern const uint8_t image_jpg27_start[] asm("_binary_img27_jpg_start");
extern const uint8_t image_jpg27_end[] asm("_binary_img27_jpg_end");
extern const uint8_t image_jpg28_start[] asm("_binary_img28_jpg_start");
extern const uint8_t image_jpg28_end[] asm("_binary_img28_jpg_end");
extern const uint8_t image_jpg29_start[] asm("_binary_img29_jpg_start");
extern const uint8_t image_jpg29_end[] asm("_binary_img29_jpg_end");
extern const uint8_t image_jpg30_start[] asm("_binary_img30_jpg_start");
extern const uint8_t image_jpg30_end[] asm("_binary_img30_jpg_end");
extern const uint8_t image_jpg31_start[] asm("_binary_img31_jpg_start");
extern const uint8_t image_jpg31_end[] asm("_binary_img31_jpg_end");
extern const uint8_t image_jpg32_start[] asm("_binary_img32_jpg_start");
extern const uint8_t image_jpg32_end[] asm("_binary_img32_jpg_end");
extern const uint8_t image_jpg33_start[] asm("_binary_img33_jpg_start");
extern const uint8_t image_jpg33_end[] asm("_binary_img33_jpg_end");
extern const uint8_t image_jpg34_start[] asm("_binary_img34_jpg_start");
extern const uint8_t image_jpg34_end[] asm("_binary_img34_jpg_end");
extern const uint8_t image_jpg35_start[] asm("_binary_img35_jpg_start");
extern const uint8_t image_jpg35_end[] asm("_binary_img35_jpg_end");
extern const uint8_t image_jpg36_start[] asm("_binary_img36_jpg_start");
extern const uint8_t image_jpg36_end[] asm("_binary_img36_jpg_end");
extern const uint8_t image_jpg37_start[] asm("_binary_img37_jpg_start");
extern const uint8_t image_jpg37_end[] asm("_binary_img37_jpg_end");
extern const uint8_t image_jpg38_start[] asm("_binary_img38_jpg_start");
extern const uint8_t image_jpg38_end[] asm("_binary_img38_jpg_end");
extern const uint8_t image_jpg39_start[] asm("_binary_img99_jpg_start");
extern const uint8_t image_jpg39_end[] asm("_binary_img99_jpg_end");

uint8_t *image_start[]=
{
    image_jpg0_start,image_jpg1_start,image_jpg2_start,image_jpg3_start,image_jpg4_start,image_jpg5_start,image_jpg6_start,image_jpg7_start,image_jpg8_start,image_jpg9_start,
    image_jpg10_start,image_jpg11_start,image_jpg12_start,image_jpg13_start,image_jpg14_start,image_jpg15_start,image_jpg16_start,image_jpg17_start,image_jpg18_start,image_jpg19_start,
    image_jpg20_start,image_jpg21_start,image_jpg22_start,image_jpg23_start,image_jpg24_start,image_jpg25_start,image_jpg26_start,image_jpg27_start,image_jpg28_start,image_jpg29_start,
    image_jpg30_start,image_jpg31_start,image_jpg32_start,image_jpg33_start,image_jpg34_start,image_jpg35_start,image_jpg36_start,image_jpg37_start,image_jpg38_start,image_jpg39_start
};
uint8_t *image_end[]=
{
    image_jpg0_end,image_jpg1_end,image_jpg2_end,image_jpg3_end,image_jpg4_end,image_jpg5_end,image_jpg6_end,image_jpg7_end,image_jpg8_end,image_jpg9_end,
    image_jpg10_end,image_jpg11_end,image_jpg12_end,image_jpg13_end,image_jpg14_end,image_jpg15_end,image_jpg16_end,image_jpg17_end,image_jpg18_end,image_jpg19_end,
    image_jpg20_end,image_jpg21_end,image_jpg22_end,image_jpg23_end,image_jpg24_end,image_jpg25_end,image_jpg26_end,image_jpg27_end,image_jpg28_end,image_jpg29_end,
    image_jpg30_end,image_jpg31_end,image_jpg32_end,image_jpg33_end,image_jpg34_end,image_jpg35_end,image_jpg36_end,image_jpg37_end,image_jpg38_end,image_jpg39_end
};

//Define the height and width of the jpeg file. Make sure this matches the actual jpeg
//dimensions.
#define IMAGE_W 50
#define IMAGE_H 50

const char *TAG = "ImageDec";

//Data that is passed from the decoder function to the infunc/outfunc functions.
typedef struct {
    const unsigned char *inData; //Pointer to jpeg data
    uint16_t inPos;              //Current position in jpeg data
    uint16_t **outData;          //Array of IMAGE_H pointers to arrays of IMAGE_W 16-bit pixel values
    int outW;                    //Width of the resulting file
    int outH;                    //Height of the resulting file
} JpegDev;

//Input function for jpeg decoder. Just returns bytes from the inData field of the JpegDev structure.
static uint16_t infunc(JDEC *decoder, uint8_t *buf, uint16_t len)
{
    //Read bytes from input file
    JpegDev *jd = (JpegDev *)decoder->device;
    if (buf != NULL) {
        memcpy(buf, jd->inData + jd->inPos, len);
    }
    jd->inPos += len;
    return len;
}

//Output function. Re-encodes the RGB888 data from the decoder as big-endian RGB565 and
//stores it in the outData array of the JpegDev structure.
static uint16_t outfunc(JDEC *decoder, void *bitmap, JRECT *rect)
{
    JpegDev *jd = (JpegDev *)decoder->device;
    uint8_t *in = (uint8_t *)bitmap;
    for (int y = rect->top; y <= rect->bottom; y++) {
        for (int x = rect->left; x <= rect->right; x++) {
            //We need to convert the 3 bytes in `in` to a rgb565 value.
            uint16_t v = 0;
            v |= ((in[0] >> 3) << 11);
            v |= ((in[1] >> 2) << 5);
            v |= ((in[2] >> 3) << 0);
            //The LCD wants the 16-bit value in big-endian, so swap bytes
//            v = (v >> 8) | (v << 8);
            jd->outData[y][x] = v;
            in += 3;
        }
    }
    return 1;
}

//Size of the work space for the jpeg decoder.
#define WORKSZ 3100

//Decode the embedded image into pixel lines that can be used with the rest of the logic.
esp_err_t decode_image(uint16_t ***pixels,int code)
{
    char *work = NULL;
    int r;
    JDEC decoder;
    JpegDev jd;
    *pixels = NULL;
    esp_err_t ret = ESP_OK;

    //Alocate pixel memory. Each line is an array of IMAGE_W 16-bit pixels; the `*pixels` array itself contains pointers to these lines.
    *pixels = calloc(IMAGE_H, sizeof(uint16_t *));
    if (*pixels == NULL) {
        ESP_LOGE(TAG, "Error allocating memory for lines");
        ret = ESP_ERR_NO_MEM;
        goto err;
    }
    for (int i = 0; i < IMAGE_H; i++) {
        (*pixels)[i] = malloc(IMAGE_W * sizeof(uint16_t));
        if ((*pixels)[i] == NULL) {
            ESP_LOGE(TAG, "Error allocating memory for line %d", i);
            ret = ESP_ERR_NO_MEM;
            goto err;
        }
    }

    //Allocate the work space for the jpeg decoder.
    work = calloc(WORKSZ, 1);
    if (work == NULL) {
        ESP_LOGE(TAG, "Cannot allocate workspace");
        ret = ESP_ERR_NO_MEM;
        goto err;
    }

    //Populate fields of the JpegDev struct.
    uint8_t * image;
    if(code==99)
    {
        image=image_start[39];
    }
    else if(code<39)
    {
        image=image_start[code];
    }
    else
    {
        ret =ESP_ERR_INVALID_STATE;
        goto err;
    }
    
    
    jd.inData = image;
    jd.inPos = 0;
    jd.outData = *pixels;
    jd.outW = IMAGE_W;
    jd.outH = IMAGE_H;

    //Prepare and decode the jpeg.
    r = jd_prepare(&decoder, infunc, work, WORKSZ, (void *)&jd);
    if (r != JDR_OK) {
        ESP_LOGE(TAG, "Image decoder: jd_prepare failed (%d)", r);
        ret = ESP_ERR_NOT_SUPPORTED;
        goto err;
    }
    r = jd_decomp(&decoder, outfunc, 0);
    if (r != JDR_OK && r != JDR_FMT1) {
        ESP_LOGE(TAG, "Image decoder: jd_decode failed (%d)", r);
        ret = ESP_ERR_NOT_SUPPORTED;
        goto err;
    }

    //All done! Free the work area (as we don't need it anymore) and return victoriously.
    free(work);
    return ret;
err:
    //Something went wrong! Exit cleanly, de-allocating everything we allocated.
    if (*pixels != NULL) {
        for (int i = 0; i < IMAGE_H; i++) {
            free((*pixels)[i]);
        }
        free(*pixels);
    }
    free(work);
    return ret;
}
