/**
 * epm3256.c - PIO-based SPI keyboard+mouse output driver (EPM3256 CPLD)
 *
 * Serializes ZX keyboard matrix and Kempston mouse state into two
 * 32-bit words and shifts them out via PIO with sideset CLK+latch.
 *
 * Data format (MSB first):
 *   Word 0 (29 data bits + 3 pad): mouse_keys[3] | mouse_x[8] | mouse_y[8] |
 *                                   row7[5] | row6[5]
 *   Word 1 (30 data bits + 2 pad): row5[5] | row4[5] | row3[5] | row2[5] |
 *                                   row1[5] | row0[5]
 */

#include "hardware/clocks.h"

#include "g_config.h"
#include "epm3256.h"
#include "kbd.pio.h"

#ifdef SPI_KB_ENABLE

void epm3256_init(void)
{
    uint offset = pio_add_program(PIO_SPI, &pio_epm3256_program);
    pio_sm_config c = pio_get_default_sm_config();
    sm_config_set_wrap(&c, offset, offset + pio_epm3256_program.length - 1);
    sm_config_set_out_shift(&c, false, false, 32); // MSB first, no autopull
    sm_config_set_fifo_join(&c, PIO_FIFO_JOIN_TX);

    // Data pin (MOSI)
    pio_gpio_init(PIO_SPI, SPI_PIN_MOSI);
    pio_sm_set_consecutive_pindirs(PIO_SPI, SM_SPI, SPI_PIN_MOSI, 1, true);
    sm_config_set_out_pin_base(&c, SPI_PIN_MOSI);
    sm_config_set_out_pin_count(&c, 1);

    // Sideset pins (SCK, CS/latch)
    pio_gpio_init(PIO_SPI, SPI_PIN_SCK);
    pio_gpio_init(PIO_SPI, SPI_PIN_CS);
    pio_sm_set_pindirs_with_mask(PIO_SPI, SM_SPI,
                                 (3u << SPI_PIN_SCK) | (1u << SPI_PIN_MOSI),
                                 (3u << SPI_PIN_SCK) | (1u << SPI_PIN_MOSI));
    sm_config_set_sideset_pins(&c, SPI_PIN_SCK);
    sm_config_set_sideset(&c, 2, false, false);

    // Target ~2.5 MHz PIO clock, safe for shift registers
    float fdiv = (float)clock_get_hz(clk_sys) / 2500000.0f;
    sm_config_set_clkdiv(&c, fdiv);

    pio_sm_init(PIO_SPI, SM_SPI, offset, &c);
    pio_sm_set_enabled(PIO_SPI, SM_SPI, true);
}

void __not_in_flash_func(epm3256_send)(uint8_t mouse_keys, uint8_t mouse_x,
                                       uint8_t mouse_y,
                                       zx_kbd_state_t *zx_keys_matrix)
{
    // Word 0: mouse_keys[3] | mouse_x[8] | mouse_y[8] | row7[5] | row6[5] | pad[3]
    uint32_t data0 = 0;
    data0 |= mouse_keys & 0x07;
    data0 <<= 8;
    data0 |= mouse_x;
    data0 <<= 8;
    data0 |= mouse_y;
    data0 <<= 5;
    data0 |= zx_keys_matrix->a[7] & 0x1F;
    data0 <<= 5;
    data0 |= zx_keys_matrix->a[6] & 0x1F;
    data0 <<= 3; // pad

    // Word 1: row5[5] | row4[5] | row3[5] | row2[5] | row1[5] | row0[5] | pad[2]
    uint32_t data1 = 0;
    data1 |= zx_keys_matrix->a[5] & 0x1F;
    data1 <<= 5;
    data1 |= zx_keys_matrix->a[4] & 0x1F;
    data1 <<= 5;
    data1 |= zx_keys_matrix->a[3] & 0x1F;
    data1 <<= 5;
    data1 |= zx_keys_matrix->a[2] & 0x1F;
    data1 <<= 5;
    data1 |= zx_keys_matrix->a[1] & 0x1F;
    data1 <<= 5;
    data1 |= zx_keys_matrix->a[0] & 0x1F;
    data1 <<= 2; // pad

    // Wait for FIFO space and send
    while (PIO_SPI->fstat & (1u << (PIO_FSTAT_TXFULL_LSB + SM_SPI)))
        ;

    PIO_SPI->txf[SM_SPI] = data0;

    while (PIO_SPI->fstat & (1u << (PIO_FSTAT_TXFULL_LSB + SM_SPI)))
        ;

    PIO_SPI->txf[SM_SPI] = data1;
}

#endif // SPI_KB_ENABLE
