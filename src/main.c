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

    const uint8_t initcmd_3[] = {0x23, 0x81, 0x1C, 0xA4, 0x40, 0xA6, 0xAF};
    spi_write_blocking(spi, initcmd_3, sizeof(initcmd_3));

    gpio_put(CS_PIN, true);

    // 表示RAMをクリア
    gpio_put(CS_PIN, false);
    for (uint8_t page = 0; page < 8; page++) {
        // ページ左端に移動
        gpio_put(RS_PIN, false);
        const uint8_t pageAddressSet[] = {0xB0 + page, 0x10, 0x00};
        spi_write_blocking(spi, pageAddressSet, sizeof(pageAddressSet));

        // 0x00を書き込んでクリアする
        gpio_put(RS_PIN, true);
        for (uint8_t i = 0; i < 132; i++) {
            const uint8_t empty = 0x00;
            spi_write_blocking(spi, &empty, 1);
        }
    }
    gpio_put(CS_PIN, true);

    // メインループ
    while (true) {
    }
}
