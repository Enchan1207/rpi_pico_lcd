//
// グラフィック液晶を制御する
//

#include <hardware/spi.h>
#include <pico/stdlib.h>
#include <stdbool.h>
#include <stdint.h>

// ピン定義
const unsigned int CS_PIN = 17;
const unsigned int SCK_PIN = 18;
const unsigned int MOSI_PIN = 19;
const unsigned int RS_PIN = 20;

const unsigned int INDICATOR_PIN = 21;

// ドットパターン
// created using https://gist.github.com/Enchan1207/f0212f9ea90507fd9e9ab04ce6cfaa50
const uint8_t image[6][48] = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0xF0, 0xF8, 0xFC, 0xFC, 0xFE, 0xFE, 0xFE, 0x9C, 0x8C, 0xC0, 0xE0, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC, 0x7E, 0x3E, 0x1E, 0x8E, 0x80, 0x00, 0x30, 0x70, 0xF0, 0xE2, 0xE7, 0xFF, 0xFE, 0xFC, 0x7F, 0x7F, 0x3F, 0x1F, 0x1F, 0x3F, 0x7F, 0xFF, 0xFF, 0xFF, 0xFC, 0xFC, 0xFE, 0xE7, 0xE3, 0xF0, 0xF8, 0xF8, 0xE0, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF8, 0xFE, 0xFF, 0xFF, 0x0F, 0x03, 0x01, 0xF0, 0xFC, 0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0xFF, 0xFF, 0xFF, 0xEF, 0xEF, 0xE7, 0xE0, 0xE0, 0xE0, 0xE0, 0xE4, 0xEC, 0xFE, 0xFE, 0x7E, 0x7F, 0xFF, 0xEF, 0xCF, 0x0F, 0x1F, 0x1F, 0x0F, 0x07, 0x03, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x07, 0x0F, 0x1F, 0x3F, 0x7E, 0xFC, 0xF8, 0xF1, 0xE7, 0xCF, 0x9F, 0x3F, 0x7F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF7, 0x63, 0x07, 0x0F, 0x8E, 0xC4, 0xC0, 0xC1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x08, 0x18, 0x38, 0x78, 0xF8, 0xF8, 0xF0, 0xF0, 0xF8, 0xFC, 0xFC, 0x7C, 0x39, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7E, 0xFC, 0xF9, 0xF1, 0xE3, 0xC7, 0xC7, 0xC7, 0xC7, 0xE3, 0xE3, 0xF1, 0xF8, 0xFC, 0x7E, 0x3F, 0x1F, 0x0F, 0x07, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
};

int main() {
    // SPI初期化
    spi_inst_t* spi = spi0;
    spi_init(spi, 1000000);
    spi_set_format(spi, 8, SPI_CPOL_1, SPI_CPHA_1, SPI_MSB_FIRST);

    // GPIO初期化
    gpio_init(RS_PIN);
    gpio_set_dir(RS_PIN, GPIO_OUT);
    gpio_put(RS_PIN, false);

    gpio_init(CS_PIN);
    gpio_set_dir(CS_PIN, GPIO_OUT);
    gpio_put(CS_PIN, true);  // 負論理

    gpio_set_function(SCK_PIN, GPIO_FUNC_SPI);
    gpio_set_function(MOSI_PIN, GPIO_FUNC_SPI);

    gpio_init(INDICATOR_PIN);
    gpio_set_dir(INDICATOR_PIN, GPIO_OUT);
    gpio_put(INDICATOR_PIN, false);

    // ディスプレイ初期化
    gpio_put(CS_PIN, false);
    gpio_put(RS_PIN, false);  // コマンド書込み

    const uint8_t initcmd_1[] = {0xAE, 0xA0, 0xC8, 0xA3};
    spi_write_blocking(spi, initcmd_1, sizeof(initcmd_1));

    // 内蔵レギュレータ有効化時は2msずつ待機しなければならないので、1byteずつ送る
    const uint8_t initcmd_2[] = {0x2C, 0x2E, 0x2F};
    for (size_t i = 0; i < sizeof(initcmd_2); i++) {
        spi_write_blocking(spi, initcmd_2 + i, 1);
        sleep_ms(2);
    }

    const uint8_t contrast = 0x1C;  // 00 ~ 3F
    const uint8_t initcmd_3[] = {0x23, 0x81, contrast, 0xA4, 0x40, 0xA6, 0xAF};
    spi_write_blocking(spi, initcmd_3, sizeof(initcmd_3));

    gpio_put(CS_PIN, true);

    // 表示RAMをクリア
    gpio_put(CS_PIN, false);
    for (uint8_t page = 0; page < 8; page++) {
        // ページ左端に移動
        gpio_put(RS_PIN, false);
        const uint8_t moveToPageHead[] = {0xB0 + page, 0x10, 0x00};
        spi_write_blocking(spi, moveToPageHead, sizeof(moveToPageHead));

        // 0x00を書き込んでクリアする
        gpio_put(RS_PIN, true);
        for (uint8_t i = 0; i < 132; i++) {
            const uint8_t empty = 0x00;
            spi_write_blocking(spi, &empty, 1);
        }
    }
    gpio_put(CS_PIN, true);

    // パターンを描画
    gpio_put(CS_PIN, false);
    for (uint8_t page = 0; page < 6; page++) {
        // 表示位置をオフセット
        gpio_put(RS_PIN, false);
        const uint8_t offsetX = 40;
        const uint8_t moveToImageHead[] = {0xB0 + page, 0x10 | (offsetX >> 4), offsetX & 0x0F};
        spi_write_blocking(spi, moveToImageHead, sizeof(moveToImageHead));

        // 対応するデータを書き込み
        gpio_put(RS_PIN, true);
        spi_write_blocking(spi, (const uint8_t*)(image + page), 48);
    }
    gpio_put(CS_PIN, true);

    gpio_put(INDICATOR_PIN, true);
    sleep_ms(1000);

    while (true) {
        gpio_put(INDICATOR_PIN, true);
        sleep_ms(500);
        gpio_put(INDICATOR_PIN, false);
        sleep_ms(500);
    }
}
