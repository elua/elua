// eLua-specific definitions for XMC
// Mostly adapted from XMC4500.h

#ifndef __XMC_ELUA_H__
#define __XMC_ELUA_H__

#include "XMC4500.h"

// Generic port definition

typedef struct {                                    /*!< (@ 0x48028000) PORT0 Structure                                        */
  __IO uint32_t  OUT;                               /*!< (@ 0x48028000) Port 0 Output Register                                 */
  __O  uint32_t  OMR;                               /*!< (@ 0x48028004) Port 0 Output Modification Register                    */
  __I  uint32_t  RESERVED0[2];
  __IO uint32_t  IOCRS[4];
  __I  uint32_t  RESERVED1;
  __I  uint32_t  IN;                                /*!< (@ 0x48028024) Port 0 Input Register                                  */
  __I  uint32_t  RESERVED2[6];
  __IO uint32_t  PDR0;                              /*!< (@ 0x48028040) Port 0 Pad Driver Mode 0 Register                      */
  __IO uint32_t  PDR1;                              /*!< (@ 0x48028044) Port 0 Pad Driver Mode 1 Register                      */
  __I  uint32_t  RESERVED3[6];
  __I  uint32_t  PDISC;                             /*!< (@ 0x48028060) Port 0 Pin Function Decision Control Register          */
  __I  uint32_t  RESERVED4[3];
  __IO uint32_t  PPS;                               /*!< (@ 0x48028070) Port 0 Pin Power Save Register                         */
  __IO uint32_t  HWSEL;                             /*!< (@ 0x48028074) Port 0 Pin Hardware Select Register                    */
} PORT_Type;

#define PORT_0                           ((PORT_Type*) PORT0_BASE)
#define PORT_1                           ((PORT_Type*) PORT1_BASE)
#define PORT_2                           ((PORT_Type*) PORT2_BASE)
#define PORT_3                           ((PORT_Type*) PORT3_BASE)
#define PORT_4                           ((PORT_Type*) PORT4_BASE)
#define PORT_5                           ((PORT_Type*) PORT5_BASE)
#define PORT_6                           ((PORT_Type*) PORT6_BASE)
#define PORT_14                          ((PORT_Type*) PORT14_BASE)
#define PORT_15                          ((PORT_Type*) PORT15_BASE)

#endif // #ifndef __XMC_ELUA_H__

