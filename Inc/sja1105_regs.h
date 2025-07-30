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

/* ---------------------------------------------------------------------------- */
/* Ethernet Switch Core */
/* ---------------------------------------------------------------------------- */

#define SJA1105_REG_GENERAL_STATUS_1 (0x00000003)

#define SJA1105_L2BUSYS_SHIFT        (0)
#define SJA1105_L2BUSYS_MASK         (0x1 << SJA1105_L2BUSYS_SHIFT)


/* ---------------------------------------------------------------------------- */
/* Auxiliary Configuration Unit */
/* ---------------------------------------------------------------------------- */

#define SJA1105_ACU_REG_CFG_PAD_MIIX_BASE          (0x100800)
#define SJA1105_ACU_REG_CFG_PAD_MIIX_TX(port_num)  (SJA1105_ACU_REG_CFG_PAD_MIIX_BASE + 2 * port_num)
#define SJA1105_ACU_REG_CFG_PAD_MIIX_RX(port_num)  (SJA1105_ACU_REG_CFG_PAD_MIIX_BASE + 2 * port_num + 1)
#define SJA1105_ACU_REG_CFG_PAD_MIIX_ID(port_num)  (SJA1105_ACU_REG_CFG_PAD_MIIX_BASE + 16 + port_num)

#define SJA1105_ACU_REG_CFG_PAD_MISC               (0x100840)
#define SJA1105_ACU_REG_CFG_PAD_SPI                (0x100880)
#define SJA1105_ACU_REG_CFG_PAD_JTAG               (0x100881)

#define SJA1105_ACU_REG_PORT_STATUS_MIIX_BASE      (0x100900)
#define SJA1105_ACU_REG_PORT_STATUS_MIIX(port_num) (SJA1105_ACU_REG_PORT_STATUS_MIIX_BASE + port_num)

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
#define PART_NR_SJA1105_T                          (0x9a82)
#define PART_NR_SJA1105P                           (0x9a84)
#define PART_NR_SJA1105Q                           (0x9a85)
#define PART_NR_SJA1105R                           (0x9a86)
#define PART_NR_SJA1105S                           (0x9a87)

/* ---------------------------------------------------------------------------- */
/* Clock Generation Unit */
/* ---------------------------------------------------------------------------- */






/* ---------------------------------------------------------------------------- */
/* Static Configuration */
/* ---------------------------------------------------------------------------- */

#define SJA1105_STATIC_CONF_ADDR               (0x20000)
#define SJA1105_T_SWITCH_CORE_ID               (0x9f00030e)
#define SJA1105PR_SWITCH_CORE_ID               (0xaf00030e)
#define SJA1105QS_SWITCH_CORE_ID               (0xae00030e)

#define SJA1105_STATIC_CONF_BLOCK_NUM_HEADERS  (2)
#define SJA1105_STATIC_CONF_BLOCK_NUM_CRCS     (2)
#define SJA1105_STATIC_CONF_BLOCK_FIRST_OFFSET (1)
#define SJA1105_STATIC_CONF_BLOCK_LAST_SIZE    (3)  /* Last block contains two 0 words and the global CRC */

#define SJA1105_STATIC_CONF_BLOCK_ID_OFFSET    (0)
#define SJA1105_STATIC_CONF_BLOCK_ID_SHIFT     (24)
#define SJA1105_STATIC_CONF_BLOCK_ID_MASK      (0xff << SJA1105_STATIC_CONF_BLOCK_ID_SHIFT)
#define SJA1105_STATIC_CONF_BLOCK_SIZE_OFFSET  (1)
#define SJA1105_STATIC_CONF_BLOCK_SIZE_SHIFT   (0)
#define SJA1105_STATIC_CONF_BLOCK_SIZE_MASK    (0xffffff)

#define SJA1105_STATIC_CONF_BLOCK_ID_L2ADDR_LU (0x05)
#define SJA1105_STATIC_CONF_BLOCK_ID_CGU       (0x80)
#define SJA1105_STATIC_CONF_BLOCK_ID_ACU       (0x82)

#ifdef __cplusplus
}
#endif

#endif /* SJA1105_INC_SJA1105_REGS_H_ */
