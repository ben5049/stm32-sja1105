/*
 * sja1105_regs.h
 *
 *  Created on: Jul 30, 2025
 *      Author: bens1
 */

#ifndef SJA1105_INC_SJA1105_REGS_H_
#define SJA1105_INC_SJA1105_REGS_H_

#ifdef __cplusplus
extern "C" {
#endif


#include "sja1105.h"
#include "stdint.h"


/* ---------------------------------------------------------------------------- */
/* Ethernet Switch Core */
/* ---------------------------------------------------------------------------- */

enum SJA1105_GeneralReg_Enum{
    SJA1105_REG_DEVICE_ID         = 0x00000000,
    SJA1105_REG_STATIC_CONF_FLAGS = 0x00000001,
    SJA1105_REG_VL_PART_STATUS    = 0x00000002,
    SJA1105_REG_GENERAL_STATUS_1  = 0x00000003,
    SJA1105_REG_GENERAL_STATUS_2  = 0x00000004,
    SJA1105_REG_GENERAL_STATUS_3  = 0x00000005,
    SJA1105_REG_GENERAL_STATUS_4  = 0x00000006,
    SJA1105_REG_GENERAL_STATUS_5  = 0x00000007,
    SJA1105_REG_GENERAL_STATUS_6  = 0x00000008,
    SJA1105_REG_GENERAL_STATUS_7  = 0x00000009,
    SJA1105_REG_GENERAL_STATUS_8  = 0x0000000a,
    SJA1105_REG_GENERAL_STATUS_9  = 0x0000000b,
    SJA1105_REG_GENERAL_STATUS_10 = 0x0000000c,  /* RAM Parity error register (lower) */
    SJA1105_REG_GENERAL_STATUS_11 = 0x0000000d,  /* RAM Parity error register (upper) */
};

enum SJA1105_DeviceID_Enum{
    SJA1105ET_DEVICE_ID = 0x9f00030e,
    SJA1105PR_DEVICE_ID = 0xaf00030e,
    SJA1105QS_DEVICE_ID = 0xae00030e,
};

#define SJA1105_CONFIGS_SHIFT         (31)
#define SJA1105_CONFIGS_MASK          (0x1 << SJA1105_CONFIGS_SHIFT)
#define SJA1105_CRCCHKL_SHIFT         (30)                            /* Local CRC check */
#define SJA1105_CRCCHKL_MASK          (0x1 << SJA1105_CRCCHKL_SHIFT)  /* Local CRC check */
#define SJA1105_IDS_SHIFT             (29)
#define SJA1105_IDS_MASK              (0x1 << SJA1105_IDS_SHIFT)
#define SJA1105_CRCCHKG_SHIFT         (28)                            /* Global CRC check */
#define SJA1105_CRCCHKG_MASK          (0x1 << SJA1105_CRCCHKG_SHIFT)  /* Global CRC check */

#define SJA1105_REGULAR_CHECK_ADDR    (SJA1105_REG_VL_PART_STATUS)
#define SJA1105_REGULAR_CHECK_SIZE    (SJA1105_REG_GENERAL_STATUS_11 - SJA1105_REGULAR_CHECK_ADDR + 1)

#define SJA1105_L2BUSYS_SHIFT         (0)
#define SJA1105_L2BUSYS_MASK          (0x1 << SJA1105_L2BUSYS_SHIFT)

/* ---------------------------------------------------------------------------- */
/* Auxiliary Configuration Unit */
/* ---------------------------------------------------------------------------- */

enum SJA1105_ACUReg_Enum{
    SJA1105_ACU_REG_CFG_PAD_MIIX_BASE = 0x100800,
    SJA1105_ACU_REG_CFG_PAD_MISC      = 0x100840,
    SJA1105_ACU_REG_CFG_PAD_SPI       = 0x100880,
    SJA1105_ACU_REG_CFG_PAD_JTAG      = 0x100881,
    SJA1105_ACU_REG_PORT_STATUS_MII0  = 0x100900,
    SJA1105_ACU_REG_PORT_STATUS_MII1  = 0x100901,
    SJA1105_ACU_REG_PORT_STATUS_MII2  = 0x100902,
    SJA1105_ACU_REG_PORT_STATUS_MII3  = 0x100903,
    SJA1105_ACU_REG_PORT_STATUS_MII4  = 0x100904,
};

#define SJA1105_ACU_REG_CFG_PAD_MIIX_TX(port_num)  (SJA1105_ACU_REG_CFG_PAD_MIIX_BASE +  2 * (port_num))
#define SJA1105_ACU_REG_CFG_PAD_MIIX_RX(port_num)  (SJA1105_ACU_REG_CFG_PAD_MIIX_BASE +  2 * (port_num) + 1)
#define SJA1105_ACU_REG_CFG_PAD_MIIX_ID(port_num)  (SJA1105_ACU_REG_CFG_PAD_MIIX_BASE + 16 + (port_num))

#define SJA1105_ACU_REG_PORT_STATUS_MIIX(port_num) (SJA1105_ACU_REG_PORT_STATUS_MII0 + (port_num))

#define SJA1105_ACU_REG_TS_CONFIG                  (0x100a00)
#define SJA1105_ACU_REG_TS_STATUS                  (0x100a01)
#define SJA1105_ACU_REG_PROD_CFG                   (0x100bc0)
#define SJA1105_ACU_REG_PROD_ID                    (0x100bc3)
#define SJA1105_ACU_REG_ACCESS_DISABLE             (0x100bfd)

#define SJA1105_ACU_PAD_CFG_TX                     (0)
#define SJA1105_ACU_PAD_CFG_RX                     (1)
#define SJA1105_ACU_PAD_CFG_SIZE                   (2)

#define SJA1105_CLK_OS_LOW                         (0 << 3)
#define SJA1105_CLK_OS_MEDIUM                      (1 << 3)
#define SJA1105_CLK_OS_FAST                        (2 << 3)
#define SJA1105_CLK_OS_HIGH                        (3 << 3)
#define SJA1105_CTRL_OS_LOW                        (0 << 11)
#define SJA1105_CTRL_OS_MEDIUM                     (1 << 11)
#define SJA1105_CTRL_OS_FAST                       (2 << 11)
#define SJA1105_CTRL_OS_HIGH                       (3 << 11)
#define SJA1105_D10_OS_LOW                         (0 << 19)
#define SJA1105_D10_OS_MEDIUM                      (1 << 19)
#define SJA1105_D10_OS_FAST                        (2 << 19)
#define SJA1105_D10_OS_HIGH                        (3 << 19)
#define SJA1105_D32_OS_LOW                         (0 << 27)
#define SJA1105_D32_OS_MEDIUM                      (1 << 27)
#define SJA1105_D32_OS_FAST                        (2 << 27)
#define SJA1105_D32_OS_HIGH                        (3 << 27)

#define SJA1105_OS_LOW                             (SJA1105_CLK_OS_LOW    | SJA1105_CTRL_OS_LOW    | SJA1105_D10_OS_LOW    | SJA1105_D32_OS_LOW   )
#define SJA1105_OS_MEDIUM                          (SJA1105_CLK_OS_MEDIUM | SJA1105_CTRL_OS_MEDIUM | SJA1105_D10_OS_MEDIUM | SJA1105_D32_OS_MEDIUM)
#define SJA1105_OS_FAST                            (SJA1105_CLK_OS_FAST   | SJA1105_CTRL_OS_FAST   | SJA1105_D10_OS_FAST   | SJA1105_D32_OS_FAST  )
#define SJA1105_OS_HIGH                            (SJA1105_CLK_OS_HIGH   | SJA1105_CTRL_OS_HIGH   | SJA1105_D10_OS_HIGH   | SJA1105_D32_OS_HIGH  )

#define SJA1105_CLK_IPUD_PU                        (0 << 0)
#define SJA1105_CLK_IPUD_R                         (1 << 0)
#define SJA1105_CLK_IPUD_PI                        (2 << 0)
#define SJA1105_CLK_IPUD_PD                        (3 << 0)
#define SJA1105_CTRL_IPUD_PU                       (0 << 8)
#define SJA1105_CTRL_IPUD_R                        (1 << 8)
#define SJA1105_CTRL_IPUD_PI                       (2 << 8)
#define SJA1105_CTRL_IPUD_PD                       (3 << 8)
#define SJA1105_D10_IPUD_PU                        (0 << 16)
#define SJA1105_D10_IPUD_R                         (1 << 16)
#define SJA1105_D10_IPUD_PI                        (2 << 16)
#define SJA1105_D10_IPUD_PD                        (3 << 16)
#define SJA1105_D32_IPUD_PU                        (0 << 24)
#define SJA1105_D32_IPUD_R                         (1 << 24)
#define SJA1105_D32_IPUD_PI                        (2 << 24)
#define SJA1105_D32_IPUD_PD                        (3 << 24)

#define SJA1105_IPUD_PU                            (SJA1105_CLK_IPUD_PU | SJA1105_CTRL_IPUD_PU | SJA1105_D10_IPUD_PU | SJA1105_D32_IPUD_PU)
#define SJA1105_IPUD_R                             (SJA1105_CLK_IPUD_R  | SJA1105_CTRL_IPUD_R  | SJA1105_D10_IPUD_R  | SJA1105_D32_IPUD_R )
#define SJA1105_IPUD_PI                            (SJA1105_CLK_IPUD_PI | SJA1105_CTRL_IPUD_PI | SJA1105_D10_IPUD_PI | SJA1105_D32_IPUD_PI)
#define SJA1105_IPUD_PD                            (SJA1105_CLK_IPUD_PD | SJA1105_CTRL_IPUD_PD | SJA1105_D10_IPUD_PD | SJA1105_D32_IPUD_PD)

#define SJA1105_CLK_IH_SCHMITT                     (0 << 2)
#define SJA1105_CLK_IH_NON_SCHMITT                 (1 << 2)
#define SJA1105_CTRL_IH_SCHMITT                    (0 << 10)
#define SJA1105_CTRL_IH_NON_SCHMITT                (1 << 10)
#define SJA1105_D10_IH_SCHMITT                     (0 << 18)
#define SJA1105_D10_IH_NON_SCHMITT                 (1 << 18)
#define SJA1105_D32_IH_SCHMITT                     (0 << 26)
#define SJA1105_D32_IH_NON_SCHMITT                 (1 << 26)

#define SJA1105_IH_SCHMITT                         (SJA1105_CLK_IH_SCHMITT     | SJA1105_CTRL_IH_SCHMITT     | SJA1105_D10_IH_SCHMITT     | SJA1105_D32_IH_SCHMITT)
#define SJA1105_IH_NON_SCHMITT                     (SJA1105_CLK_IH_NON_SCHMITT | SJA1105_CTRL_IH_NON_SCHMITT | SJA1105_D10_IH_NON_SCHMITT | SJA1105_D32_IH_NON_SCHMITT)

#define SJA1105_PART_NR_OFFSET                     (4)
#define SJA1105_PART_NR_MASK                       (0xffff << SJA1105_PART_NR_OFFSET)

enum SJA1105_PartNR_Enum{
    PART_NR_SJA1105ET = 0x9a82,
    PART_NR_SJA1105P  = 0x9a84,
    PART_NR_SJA1105Q  = 0x9a85,
    PART_NR_SJA1105R  = 0x9a86,
    PART_NR_SJA1105S  = 0x9a87
};

#define SJA1105_TS_PD                              (1 << 6)
#define SJA1105_TS_THRESHOLD_MASK                  (0x3f)
#define SJA1105_TS_EXCEEDED                        (1)

#define SJA1105_TS_LUT_SIZE                        (40)
static const int16_t SJA1105_TS_LUT[SJA1105_TS_LUT_SIZE] = {
    INT16_MIN, -457, -417, -375, -330, -284, -235, -183,
         -114,  -61,  -21,   21,   65,  110,  157,  206,
          256,  309,  364,  420,  461,  502,  545,  588,
          633,  679,  726,  774,  824,  875,  928,  982,
         1025, 1069, 1114, 1160, 1207, 1255, 1305, 1355};

/* ---------------------------------------------------------------------------- */
/* Clock Generation Unit */
/* ---------------------------------------------------------------------------- */

enum SJA1105_CGUReg_Enum{
    SJA1105_CGU_REG_RFRQ           = 0x100006,
    SJA1105_CGU_REG_XO66M_0_C      = 0x100006,  /* C = Control */
    SJA1105_CGU_REG_PLL_0_S        = 0x100007,  /* S = Status */
    SJA1105_CGU_REG_PLL_0_C        = 0x100008,
    SJA1105_CGU_REG_PLL_1_S        = 0x100009,
    SJA1105_CGU_REG_PLL_1_C        = 0x10000a,
    SJA1105_CGU_REG_IDIV_0_C       = 0x10000b,
    SJA1105_CGU_REG_IDIV_1_C       = 0x10000c,
    SJA1105_CGU_REG_IDIV_2_C       = 0x10000d,
    SJA1105_CGU_REG_IDIV_3_C       = 0x10000e,
    SJA1105_CGU_REG_IDIV_4_C       = 0x10000f,
    SJA1105_CGU_REG_MII_TX_CLK_0   = 0x100013,
    SJA1105_CGU_REG_MII_RX_CLK_0   = 0x100014,
    SJA1105_CGU_REG_RMII_REF_CLK_0 = 0x100015,
    SJA1105_CGU_REG_RGMII_TX_CLK_0 = 0x100016,
    SJA1105_CGU_REG_EXT_TX_CLK_0   = 0x100017,
    SJA1105_CGU_REG_EXT_RX_CLK_0   = 0x100018,
};

#define SJA1105_CGU_REG_IDIV_C(port_num)     (SJA1105_CGU_REG_IDIV_0_C + (port_num))

#define SJA1105_CGU_REG_CLK_NUM              (6)
#define SJA1105_CGU_REG_CLK_BASE(port_num)   (SJA1105_CGU_REG_MII_TX_CLK_0 + (SJA1105_CGU_REG_CLK_NUM * (port_num)))
#define SJA1105_CGU_MII_TX_CLK               (0)
#define SJA1105_CGU_MII_RX_CLK               (1)
#define SJA1105_CGU_RMII_REF_CLK             (2)
#define SJA1105_CGU_RGMII_TX_CLK             (3)
#define SJA1105_CGU_EXT_TX_CLK               (4)
#define SJA1105_CGU_EXT_RX_CLK               (5)

#define SJA1105_CGU_PD                       (1 << 0)
#define SJA1105_CGU_BYPASS                   (1 << 1)
#define SJA1105_CGU_P23EN                    (1 << 2)
#define SJA1105_CGU_FBSEL                    (1 << 6)
#define SJA1105_CGU_DIRECT                   (2 << 7)
#define SJA1105_CGU_PSEL_SHIFT               (8)
#define SJA1105_CGU_PSEL_MASK                (0x3 << SJA1105_CGU_PSEL_SHIFT)
#define SJA1105_CGU_AUTOBLOCK                (11)
#define SJA1105_CGU_NSEL_SHIFT               (12)
#define SJA1105_CGU_NSEL_MASK                (0x3 << SJA1105_CGU_NSEL_SHIFT)
#define SJA1105_CGU_MSEL_SHIFT               (16)
#define SJA1105_CGU_MSEL_MASK                (0xff << SJA1105_CGU_MSEL_SHIFT)
#define SJA1105_CGU_CLKSRC_SHIFT             (24)
#define SJA1105_CGU_CLKSRC_MASK              (0x1f << SJA1105_CGU_CLKSRC_SHIFT)

#define SJA1105_CGU_IDIV_SHIFT               (2)
#define SJA1105_CGU_IDIV_MASK                (0xff << SJA1105_CGU_IDIV_SHIFT)

#define SJA1105_CGU_CLK_SRC_XO66M_0          (0xa)
#define SJA1105_CGU_CLK_SRC_PLL0(port_num)   (0xb + ((port_num) % 3))  /* Use different phase for each port to improve EMC */
#define SJA1105_CGU_CLK_SRC_PLL1(port_num)   (0xe + ((port_num) % 3))  /* Use different phase for each port to improve EMC */
#define SJA1105_CGU_CLK_SRC_IDIV(port_num)   (0x11 + (port_num))
#define SJA1105_CGU_CLK_SRC_TX_CLK(port_num) (2 * (port_num))
#define SJA1105_CGU_CLK_SRC_RX_CLK(port_num) ((2 * (port_num)) + 1)

/* ---------------------------------------------------------------------------- */
/* Static Configuration */
/* ---------------------------------------------------------------------------- */

#define SJA1105_STATIC_CONF_ADDR                                (0x20000)

#define SJA1105_STATIC_CONF_BLOCK_HEADER                        (2)
#define SJA1105_STATIC_CONF_BLOCK_HEADER_CRC                    (1)
#define SJA1105_STATIC_CONF_BLOCK_DATA_CRC                      (1)
#define SJA1105_STATIC_CONF_BLOCK_FIRST_OFFSET                  (1)
#define SJA1105_STATIC_CONF_BLOCK_LAST_SIZE                     (3)  /* Last block contains two empty words and the global CRC */

#define SJA1105_STATIC_CONF_BLOCK_ID_OFFSET                     (0)
#define SJA1105_STATIC_CONF_BLOCK_ID_SHIFT                      (24)
#define SJA1105_STATIC_CONF_BLOCK_ID_MASK                       (0xff << SJA1105_STATIC_CONF_BLOCK_ID_SHIFT)
#define SJA1105_STATIC_CONF_BLOCK_SIZE_OFFSET                   (1)
#define SJA1105_STATIC_CONF_BLOCK_SIZE_SHIFT                    (0)
#define SJA1105_STATIC_CONF_BLOCK_SIZE_MASK                     (0xffffff)

#define SJA1105_STATIC_CONF_BLOCK_ID_L2ADDR_LU                  (0x05)
#define SJA1105_STATIC_CONF_BLOCK_ID_MAC_CONF                   (0x09)
#define SJA1105_STATIC_CONF_BLOCK_ID_GENERAL_PARAMS             (0x11)
#define SJA1105_STATIC_CONF_BLOCK_ID_CGU                        (0x80)
#define SJA1105_STATIC_CONF_BLOCK_ID_ACU                        (0x82)
#define SJA1105_STATIC_CONF_BLOCK_ID_XMII_MODE                  (0x4e)

#define SJA1105_STATIC_CONF_MAC_CONF_ENTRY_SIZE                 (8)
#define SJA1105_STATIC_CONF_MAC_CONF_BASE(port_num)             ((port_num) * SJA1105_STATIC_CONF_MAC_CONF_ENTRY_SIZE)
#define SJA1105_STATIC_CONF_MAC_CONF_WORD(port_num, word)       (SJA1105_STATIC_CONF_MAC_CONF_BASE(port_num) + (word))

#define SJA1105_STATIC_CONF_MAC_CONF_SPEED_OFFSET               (3)  /* [98:97] therefore in the 4th word */
#define SJA1105_STATIC_CONF_MAC_CONF_SPEED_SHIFT                (1)  /* shifted up by 1 */
#define SJA1105_STATIC_CONF_MAC_CONF_SPEED_MASK                 (0x3 << SJA1105_STATIC_CONF_MAC_CONF_SPEED_SHIFT)

#define SJA1105_STATIC_CONF_GENERAL_PARAMS_SIZE                 (11)

#define SJA1105_MAC_FLT_START_OFFSET_W                          (4) /* Starts at bit 152 therefore in the 5th word */
#define SJA1105_MAC_FLT_START_OFFSET_B                          (3) /* Starts at bit 152 therefore offset 3 bytes from the nearest multiple of 32 bits (128 + 3 * 8 = 152) */

#define SJA1105_STATIC_CONF_GENERAL_PARAMS_HOST_PORT_OFFSET     (4)  /* [144:142] therefore in the 5th word */
#define SJA1105_STATIC_CONF_GENERAL_PARAMS_HOST_PORT_SHIFT      (14)  /* shifted up by 14 */
#define SJA1105_STATIC_CONF_GENERAL_PARAMS_HOST_PORT_MASK       (0x7 << SJA1105_STATIC_CONF_GENERAL_PARAMS_HOST_PORT_SHIFT)

#define SJA1105_STATIC_CONF_XMII_MODE_PHY_MAC_SHIFT(port_num)   (19 + ((port_num) * 3))
#define SJA1105_STATIC_CONF_XMII_MODE_PHY_MAC_MASK(port_num)    (1 << SJA1105_STATIC_CONF_XMII_MODE_PHY_MAC_SHIFT(port_num))

#define SJA1105_STATIC_CONF_XMII_MODE_INTERFACE_SHIFT(port_num) (17 + ((port_num) * 3))
#define SJA1105_STATIC_CONF_XMII_MODE_INTERFACE_MASK(port_num)  (0x3 << SJA1105_STATIC_CONF_XMII_MODE_INTERFACE_SHIFT(port_num))

/* ---------------------------------------------------------------------------- */
/* Dynamic Reonfiguration */
/* ---------------------------------------------------------------------------- */

#define SJA1105_GENERAL_PARAMS


#ifdef __cplusplus
}
#endif

#endif /* SJA1105_INC_SJA1105_REGS_H_ */
