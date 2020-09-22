#include <stdlib.h>
#include <string.h>
#include "ch.h"
#include "hal.h"
#include "test_radio.h"
#include "chprintf.h"

static SI41XXConfig synthcfg = {
    .sen = LINE_LO_SEN,
    .sclk = LINE_LO_SCLK,
    .sdata = LINE_LO_SDATA,
    .ref_freq = 16000000,
    .if_div = SI41XX_IFDIV_DIV1,
    .if_n = 1616,
    .if_r = 32,
};

static const SPIConfig lband_spicfg = {
    false,
    NULL,                                   // Operation complete callback
    LINE_LBAND_CS,
    // SPI cr1 data                            (see 446 ref man.)
    SPI_CR1_SPE     |                       // SPI enable
    SPI_CR1_MSTR    |                       // Master
    //SPI_CR1_BR_2    |
    SPI_CR1_BR_1    |
    SPI_CR1_BR_0   |                        // fpclk/16  approx 5Mhz? BR = 0x011
    SPI_CR1_SSM,
    0, // SPI_CR2_SSOE,
};

static const SPIConfig uhf_spicfg = {
    false,
    NULL,                                   // Operation complete callback
    LINE_UHF_CS,
    // SPI cr1 data                         (see 446 ref man.)
    SPI_CR1_SPE     |                       // SPI enable
    SPI_CR1_MSTR    |                       // Master
    //SPI_CR1_BR_2    |
    SPI_CR1_BR_1    |
    SPI_CR1_BR_0   |                        // fpclk/16  approx 5Mhz? BR = 0x011
    SPI_CR1_SSM,
    0, // SPI_CR2_SSOE,
};

static const ax5043_regval_t lband_regs[] = {
    {AX5043_REG_PINFUNCSYSCLK, AX5043_PFSYSCLK_OUT_XTAL_DIV1},
    {AX5043_REG_0xF00, AX5043_0xF00_DEFVAL},
    {AX5043_REG_0xF0C, AX5043_0xF0C_DEFVAL},
    {AX5043_REG_0xF0D, AX5043_0xF0D_DEFVAL},
    {AX5043_REG_0xF10, AX5043_0xF10_TCXO},
    {AX5043_REG_0xF11, AX5043_0xF11_TCXO},
    {AX5043_REG_0xF1C, AX5043_0xF1C_DEFVAL},
    {AX5043_REG_0xF21, AX5043_0xF21_DEFVAL},
    {AX5043_REG_0xF22, AX5043_0xF22_DEFVAL},
    {AX5043_REG_0xF23, AX5043_0xF23_DEFVAL},
    {AX5043_REG_0xF26, AX5043_0xF26_DEFVAL},
    {AX5043_REG_0xF30, AX5043_0xF30_DEFVAL},
    {AX5043_REG_0xF31, AX5043_0xF31_DEFVAL},
    {AX5043_REG_0xF32, AX5043_0xF32_DEFVAL},
    {AX5043_REG_0xF33, AX5043_0xF33_DEFVAL},
    {AX5043_REG_0xF34, AX5043_0xF34_RFDIV},
    {AX5043_REG_0xF35, AX5043_0xF35_XTALDIV1},
    {AX5043_REG_0xF44, AX5043_0xF44_DEFVAL},
    {0, 0}
};
static const ax5043_regval_t uhf_regs[] = {
    {AX5043_REG_0xF00, AX5043_0xF00_DEFVAL},
    {AX5043_REG_0xF0C, AX5043_0xF0C_DEFVAL},
    {AX5043_REG_0xF0D, AX5043_0xF0D_DEFVAL},
    {AX5043_REG_0xF10, AX5043_0xF10_TCXO},
    {AX5043_REG_0xF11, AX5043_0xF11_TCXO},
    {AX5043_REG_0xF1C, AX5043_0xF1C_DEFVAL},
    {AX5043_REG_0xF21, AX5043_0xF21_DEFVAL},
    {AX5043_REG_0xF22, AX5043_0xF22_DEFVAL},
    {AX5043_REG_0xF23, AX5043_0xF23_DEFVAL},
    {AX5043_REG_0xF26, AX5043_0xF26_DEFVAL},
    {AX5043_REG_0xF30, AX5043_0xF30_DEFVAL},
    {AX5043_REG_0xF31, AX5043_0xF31_DEFVAL},
    {AX5043_REG_0xF32, AX5043_0xF32_DEFVAL},
    {AX5043_REG_0xF33, AX5043_0xF33_DEFVAL},
    {AX5043_REG_0xF34, AX5043_0xF34_RFDIV},
    {AX5043_REG_0xF35, AX5043_0xF35_XTALDIV1},
    {AX5043_REG_0xF44, AX5043_0xF44_DEFVAL},
    {0, 0}
};

static const AX5043Config lbandcfg = {
    .spip       = &SPID1,
    .spicfg     = &lband_spicfg,
    .miso       = LINE_SPI1_MISO,
    .irq        = LINE_LBAND_IRQ,
    .xtal_freq  = 16000000,
    .reg_values = lband_regs,
};

static const AX5043Config uhfcfg = {
    .spip       = &SPID1,
    .spicfg     = &uhf_spicfg,
    .miso       = LINE_SPI1_MISO,
    .irq        = LINE_UHF_IRQ,
    .xtal_freq  = 16000000,
    .reg_values = uhf_regs,
};

AX5043Driver lband;
AX5043Driver uhf;
SI41XXDriver synth;

void radio_init(void)
{
    ax5043ObjectInit(&lband);
    ax5043ObjectInit(&uhf);
    si41xxObjectInit(&synth);

    ax5043Start(&lband, &lbandcfg);
    ax5043Start(&uhf, &uhfcfg);
    si41xxStart(&synth, &synthcfg);
}

/*===========================================================================*/
/* OreSat Radio Control                                                      */
/*===========================================================================*/
void cmd_radio(BaseSequentialStream *chp, int argc, char *argv[])
{
    static AX5043Driver *devp = NULL;
    static const AX5043Config *cfgp = NULL;
    if (argc < 1) {
        goto radio_usage;
    }

    if (!strcmp(argv[0], "setdev") && argc > 1) {
        if (argv[1][0] == 'l') {
            devp = &lband;
            cfgp = &lbandcfg;
        } else if (argv[1][0] == 'u') {
            devp = &uhf;
            cfgp = &uhfcfg;
        } else {
            goto radio_usage;
        }
        return;
    }

    if (devp == NULL) {
        chprintf(chp, "Error: Set the device via 'setdev' first\r\n");
        goto radio_usage;
    }

    if (!strcmp(argv[0], "start")) {
        chprintf(chp, "Starting %s AX5043 driver...", (devp == &uhf ? "UHF" : "L-Band"));
        ax5043Start(devp, cfgp);
        if (devp->state != AX5043_READY) {
            chprintf(chp, "Error: Failed to start driver. Error code %d.\r\n", devp->error);
        } else {
            chprintf(chp, "OK\r\n");
        }
    } else if (!strcmp(argv[0], "stop")) {
        chprintf(chp, "Stopping %s AX5043 driver...", (devp == &uhf ? "UHF" : "L-Band"));
        ax5043Stop(devp);
        chprintf(chp, "OK\r\n");
    } else if (!strcmp(argv[0], "setfreq") && argc > 1) {
        uint32_t freq = strtoul(argv[1], NULL, 0);
        uint8_t vcor = (argc > 2 ? strtoul(argv[2], NULL, 0) : 0);
        bool chan_b = (argc > 3 && !strcmp(argv[3], "true") ? true : false);

        if (devp->state != AX5043_READY) {
            chprintf(chp, "Error: Please start the AX5043 driver first\r\n");
            goto radio_usage;
        }

        chprintf(chp, "Setting %s %s frequency to %u...", (devp == &uhf ? "UHF" : "L-Band"), (chan_b ? "B" : "A"), freq);
        ax5043SetFreq(devp, freq, vcor, chan_b);
        if (devp->error != AX5043_ERR_NOERROR) {
            chprintf(chp, "Error: Failed to set frequency. Error code %d.\r\n", devp->error);
        } else {
            chprintf(chp, "OK\r\n");
        }
    } else if (!strcmp(argv[0], "readreg") && argc > 2) {
        uint16_t reg = strtoul(argv[1], NULL, 0);

        if (devp->state != AX5043_READY) {
            chprintf(chp, "Error: Please start the AX5043 driver first\r\n");
            goto radio_usage;
        }

        if (!strcmp(argv[2], "u8")) {
            chprintf(chp, "0x%02X\r\n", ax5043ReadU8(devp, reg));
        } else if (!strcmp(argv[2], "u16")) {
            chprintf(chp, "0x%04X\r\n", ax5043ReadU16(devp, reg));
        } else if (!strcmp(argv[2], "u24")) {
            chprintf(chp, "0x%06X\r\n", ax5043ReadU24(devp, reg));
        } else if (!strcmp(argv[2], "u32")) {
            chprintf(chp, "0x%08X\r\n", ax5043ReadU32(devp, reg));
        } else {
            goto radio_usage;
        }
    } else if (!strcmp(argv[0], "writereg") && argc > 3) {
        uint16_t reg = strtoul(argv[1], NULL, 0);

        if (!strcmp(argv[3], "u8")) {
            uint8_t value = strtoul(argv[2], NULL, 0);
            ax5043WriteU8(devp, reg, value);
        } else if (!strcmp(argv[3], "u16")) {
            uint16_t value = strtoul(argv[2], NULL, 0);
            ax5043WriteU16(devp, reg, value);
        } else if (!strcmp(argv[3], "u24")) {
            uint32_t value = strtoul(argv[2], NULL, 0);
            ax5043WriteU24(devp, reg, value);
        } else if (!strcmp(argv[3], "u32")) {
            uint32_t value = strtoul(argv[2], NULL, 0);
            ax5043WriteU32(devp, reg, value);
        } else {
            goto radio_usage;
        }
    } else {
        goto radio_usage;
    }

    return;

radio_usage:
    chprintf(chp, "\r\n"
                  "Usage: radio <cmd>\r\n"
                  "    setdev <dev>:\r\n"
                  "                 Set the device to (l)band or (u)hf\r\n"
                  "\r\n"
                  "    start:       Start AX5043 device\r\n"
                  "    stop:        Stop AX5043 device\r\n"
                  "    setfreq <freq> [vcor] [chan_b]:\r\n"
                  "                 Sets frequency of channel A/B to <freq>\r\n"
                  "                 Optionally provide VCOR. [chan_b] specifies channel B\r\n"
                  "\r\n"
                  "    readreg <reg> <type>:\r\n"
                  "                 Read <reg> where <type> is u8|u16|u24|u32\r\n"
                  "\r\n"
                  "    writereg <reg> <value> <type>:\r\n"
                  "                 Write <reg> with <value> where <type> is u8|u16|u24|u32\r\n"
                  "\r\n");
    return;
}

void cmd_synth(BaseSequentialStream *chp, int argc, char *argv[])
{

    if (argc < 1) {
        goto synth_usage;
    }

    if (!strcmp(argv[0], "start")) {
        si41xxStart(&synth, &synthcfg);
    } else if (!strcmp(argv[0], "stop")) {
        si41xxStop(&synth);
    } else if (!strcmp(argv[0], "reg") && argc > 2) {
        uint32_t reg = strtoul(argv[1], NULL, 0);
        uint32_t value = strtoul(argv[2], NULL, 0);
        si41xxWriteRaw(&synth, reg, value);
    } else if (!strcmp(argv[0], "freq") && argc > 1) {
        uint32_t freq = strtoul(argv[1], NULL, 0);
        if (!si41xxSetIF(&synth, freq)) {
            chprintf(chp, "Failed to set frequency\r\n");
            goto synth_usage;
        }
        chprintf(chp, "N=%u R=%u\r\n", synthcfg.if_n, synthcfg.if_r);
    } else if (!strcmp(argv[0], "ifdiv") && argc > 1) {
        uint32_t div = strtoul(argv[1], NULL, 0);
        if (!si41xxSetIFDiv(&synth, div)) {
            chprintf(chp, "Failed to set IF divider value\r\n");
            goto synth_usage;
        }
        chprintf(chp, "IFDIV=%u\r\n", synthcfg.if_div);
    } else if (!strcmp(argv[0], "status")) {
        chprintf(chp, "PLL: %s\r\n", (palReadLine(LINE_LO_PLL) ? "NOT LOCKED" : "LOCKED"));
    } else {
        goto synth_usage;
    }

    return;
synth_usage:
    chprintf(chp, "\r\n"
                  "Usage: synth <cmd>\r\n"
                  "    start:       Start the device\r\n"
                  "    stop:        Stop the device\r\n"
                  "    reg <reg> <value>:\r\n"
                  "                 Write <reg> with <value>\r\n"
                  "\r\n"
                  "    freq <freq>: Sets frequency of IF output to <freq>\r\n"
                  "    ifdiv <div>: Sets IF output divider to <div> (1,2,4,8)\r\n"
                  "    status:      Print PLL lock status\r\n"
                  "\r\n");
    return;
}
