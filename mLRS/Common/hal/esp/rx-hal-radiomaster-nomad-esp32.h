//*******************************************************
// Copyright (c) MLRS project
// GPL3
// https://www.gnu.org/licenses/gpl-3.0.de.html
// OlliW @ www.olliw.eu
//*******************************************************
// hal
//*******************************************************

//-------------------------------------------------------
// ESP32, Radiomaster Nomad used as RX, LR1121 2400 & 900
//-------------------------------------------------------
// Receiver firmware for the RadioMaster Nomad hardware.
// Derived from tx-hal-radiomaster-nomad-esp32.h: same 2x LR1121 dual-band
// radio, SPI, power table, LED, button and fan. Differences vs the TX HAL:
//   - UARTB (GPIO1 TX / GPIO3 RX) is the main MAVLink serial to the FC
//   - UART -> GPIO4 is the CRSF/SBus output to the FC
//   - the ESP32-C3 WiFi-bridge backpack is DISABLED (no DEVICE_HAS_ESP_WIFI_BRIDGE_*,
//     no UARTD/Serial2, no esp_* control pins)
//   - no JRPIN5, IN or SERIAL_OR_COM (TX-only features)

#define DEVICE_HAS_OUT
#define DEVICE_HAS_NO_DEBUG
#define DEVICE_HAS_DIVERSITY_SINGLE_SPI
// Nomad has 2x LR1121 with band-capable RF front ends — enable simultaneous 2.4 GHz + sub-GHz operation
#define DEVICE_HAS_DUAL_LR11xx
#define DEVICE_HAS_SINGLE_LED_RGB
#define DEVICE_HAS_FAN_ONOFF


//-- UARTS
// UARTB = serial port (MAVLink to FC)
// UART  = output port, CRSF/SBus to FC
// UARTF = debug port (disabled)

#define UARTB_USE_SERIAL // serial, main MAVLink port (main TX/RX pads)
#define UARTB_BAUD                RX_SERIAL_BAUDRATE
#define UARTB_USE_TX_IO           IO_P1
#define UARTB_USE_RX_IO           IO_P3
#define UARTB_TXBUFSIZE           RX_SERIAL_TXBUFSIZE
#define UARTB_RXBUFSIZE           RX_SERIAL_RXBUFSIZE

#define UART_USE_SERIAL1 // output port, CRSF/SBus out to FC
#define UART_BAUD                 416666   // receiver-side CRSF baud rate
#define UART_USE_TX_IO            IO_P4
#define UART_USE_RX_IO            -1       // no rx pin needed
#define UART_TXBUFSIZE            256


//-- SX1: LR11xx & SPI

#define SPI_CS_IO                 IO_P27
#define SPI_MISO                  IO_P33
#define SPI_MOSI                  IO_P32
#define SPI_SCK                   IO_P25
#define SPI_FREQUENCY             16000000L
#define SX_RESET                  IO_P15
#define SX_BUSY                   IO_P36
#define SX_DIO1                   IO_P37

#define SX_USE_REGULATOR_MODE_DCDC

IRQHANDLER(void SX_DIO_EXTI_IRQHandler(void);)

void sx_init_gpio(void)
{
    gpio_init(SX_DIO1, IO_MODE_INPUT_ANALOG);
    gpio_init(SX_BUSY, IO_MODE_INPUT_ANALOG);
    gpio_init(SX_RESET, IO_MODE_OUTPUT_PP_LOW);
}

IRAM_ATTR bool sx_busy_read(void) { return (gpio_read_activehigh(SX_BUSY)) ? true : false; }

IRAM_ATTR void sx_amp_transmit(void) {}

IRAM_ATTR void sx_amp_receive(void) {}

void sx_dio_enable_exti_isr(void) { attachInterrupt(SX_DIO1, SX_DIO_EXTI_IRQHandler, RISING); }

void sx_dio_init_exti_isroff(void) { detachInterrupt(SX_DIO1); }

void sx_dio_exti_isr_clearflag(void) {}


//-- SX2: LR11xx & SPI

#define SX2_CS_IO                 IO_P13
#define SX2_BUSY                  IO_P39
#define SX2_DIO1                  IO_P34
#define SX2_RESET                 IO_P21

#define SX2_USE_REGULATOR_MODE_DCDC

IRQHANDLER(void SX2_DIO_EXTI_IRQHandler(void);)

void sx2_init_gpio(void)
{
    gpio_init(SX2_CS_IO, IO_MODE_OUTPUT_PP_HIGH);
    gpio_init(SX2_DIO1, IO_MODE_INPUT_ANALOG);
    gpio_init(SX2_BUSY, IO_MODE_INPUT_ANALOG);
    gpio_init(SX2_RESET, IO_MODE_OUTPUT_PP_LOW);
}

IRAM_ATTR void spib_select(void) { gpio_low(SX2_CS_IO); }

IRAM_ATTR void spib_deselect(void) { gpio_high(SX2_CS_IO); }

IRAM_ATTR bool sx2_busy_read(void) { return (gpio_read_activehigh(SX2_BUSY)) ? true : false; }

IRAM_ATTR void sx2_amp_transmit(void) {}

IRAM_ATTR void sx2_amp_receive(void) {}

void sx2_dio_init_exti_isroff(void) { detachInterrupt(SX2_DIO1); }

void sx2_dio_enable_exti_isr(void) { attachInterrupt(SX2_DIO1, SX2_DIO_EXTI_IRQHandler, RISING); }

void sx2_dio_exti_isr_clearflag(void) {}


//-- Out port

void out_init_gpio(void) {}

void out_set_normal(void)
{
    // https://github.com/espressif/esp-idf/blob/release/v4.4/components/esp_rom/include/esp32/rom/gpio.h#L228-L242
    gpio_matrix_out((gpio_num_t)UART_USE_TX_IO, U1TXD_OUT_IDX, false, false);
}

void out_set_inverted(void)
{
    gpio_matrix_out((gpio_num_t)UART_USE_TX_IO, U1TXD_OUT_IDX, true, false);
}


//-- Button

#define BUTTON                    IO_P14

void button_init(void) { gpio_init(BUTTON, IO_MODE_INPUT_PU); }

IRAM_ATTR bool button_pressed(void) { return (gpio_read_activelow(BUTTON)) ? true : false; }


//-- LEDs

#define LED_RGB                   IO_P22
#define LED_RGB_PIXEL_NUM         2
#include "../esp-hal-led-rgb.h"


//-- Cooling Fan

#define FAN_IO                    IO_P2

void fan_init(void) { gpio_init(FAN_IO, IO_MODE_OUTPUT_PP_LOW); }

IRAM_ATTR void fan_set_power(int8_t power_dbm)
{
    if (power_dbm >= POWER_23_DBM) { gpio_high(FAN_IO); }
    else { gpio_low(FAN_IO); }
}


//-- POWER

#include "../../setup_types.h" // needed for frequency band condition in rfpower calc
#define SX_USE_LP_PA  // Nomad uses the low power amplifier for the 900 side
#define SX_PA_DAC_IO  IO_P26

void lr11xx_rfpower_calc(const int8_t power_dbm, int8_t* sx_power, int8_t* actual_power_dbm, const uint8_t frequency_band)
{
    if (frequency_band == SX_FHSS_FREQUENCY_BAND_2P4_GHZ) {
        if (power_dbm >= POWER_30_DBM) { // -> 30
            *sx_power = 5;
            *actual_power_dbm = 30;
        } else if (power_dbm >= POWER_27_DBM) { // -> 27
            *sx_power = 3;
            *actual_power_dbm = 27;
        } else if (power_dbm >= POWER_24_DBM) { // -> 24
            *sx_power = 2;
            *actual_power_dbm = 24;
        } else if (power_dbm >= POWER_20_DBM) { // -> 20
            *sx_power = -6;
            *actual_power_dbm = 20;
        } else if (power_dbm >= POWER_17_DBM) { // -> 17
            *sx_power = -8;
            *actual_power_dbm = 17;
        } else if (power_dbm >= POWER_14_DBM) { // -> 14
            *sx_power = -14;
            *actual_power_dbm = 14;
        } else {
            *sx_power = -18;
            *actual_power_dbm = 10; // measures about 11 dBm
        }
    } else {
        uint8_t dac = 120;
        if (power_dbm >= POWER_30_DBM) { // -> 30
            dac = 95;
            *sx_power = 5;
            *actual_power_dbm = 30;
        } else if (power_dbm >= POWER_27_DBM) { // -> 27
            *sx_power = -3;
            *actual_power_dbm = 27;
        } else if (power_dbm >= POWER_24_DBM) { // -> 24
            *sx_power = -7;
            *actual_power_dbm = 24;
        } else if (power_dbm >= POWER_20_DBM) { // -> 20
            *sx_power = -11;
            *actual_power_dbm = 20;
        } else if (power_dbm >= POWER_17_DBM) { // -> 17
            *sx_power = -14;
            *actual_power_dbm = 17;
        } else if (power_dbm >= POWER_14_DBM) { // -> 14
            *sx_power = -16;
            *actual_power_dbm = 14;
        } else {
            dac = 150;
            *sx_power = -17;
            *actual_power_dbm = 10; // measures about 11 dBm
        }
        dacWrite(SX_PA_DAC_IO, dac);
    }
}

#define RFPOWER_DEFAULT           0 // index into rfpower_list array

const rfpower_t rfpower_list[] = {
    { .dbm = POWER_10_DBM, .mW = 10 },
    { .dbm = POWER_14_DBM, .mW = 25 },
    { .dbm = POWER_17_DBM, .mW = 50 },
    { .dbm = POWER_20_DBM, .mW = 100 },
    { .dbm = POWER_24_DBM, .mW = 250 },
    { .dbm = POWER_27_DBM, .mW = 500 },
    { .dbm = POWER_30_DBM, .mW = 1000 },
};
