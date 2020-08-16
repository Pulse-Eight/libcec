#pragma once

#include "env.h"

#if defined(HAVE_MACOS_API)

#include "p8-platform/threads/mutex.h"
#include <IOKit/IOKitLib.h>

// TODO: Linux copyright
#define CEC_DEFAULT_PADDR   0x2000
#define DP_CEC_LOGICAL_ADDRESS_MASK            0x300E

#define DP_CEC_TUNNELING_CONTROL               0x3001
# define DP_CEC_TUNNELING_ENABLE                (1 << 0)
# define DP_CEC_SNOOPING_ENABLE                 (1 << 1)

#define DP_CEC_RX_MESSAGE_INFO                 0x3002
# define DP_CEC_RX_MESSAGE_LEN_MASK             (0xf << 0)
# define DP_CEC_RX_MESSAGE_LEN_SHIFT            0
# define DP_CEC_RX_MESSAGE_HPD_STATE            (1 << 4)
# define DP_CEC_RX_MESSAGE_HPD_LOST             (1 << 5)
# define DP_CEC_RX_MESSAGE_ACKED                (1 << 6)
# define DP_CEC_RX_MESSAGE_ENDED                (1 << 7)

#define DP_CEC_TX_MESSAGE_INFO                 0x3003
# define DP_CEC_TX_MESSAGE_LEN_MASK             (0xf << 0)
# define DP_CEC_TX_MESSAGE_LEN_SHIFT            0
# define DP_CEC_TX_RETRY_COUNT_MASK             (0x7 << 4)
# define DP_CEC_TX_RETRY_COUNT_SHIFT            4
# define DP_CEC_TX_MESSAGE_SEND                 (1 << 7)

#define DP_CEC_TUNNELING_IRQ_FLAGS             0x3004
# define DP_CEC_RX_MESSAGE_INFO_VALID           (1 << 0)
# define DP_CEC_RX_MESSAGE_OVERFLOW             (1 << 1)
# define DP_CEC_TX_MESSAGE_SENT                 (1 << 4)
# define DP_CEC_TX_LINE_ERROR                   (1 << 5)
# define DP_CEC_TX_ADDRESS_NACK_ERROR           (1 << 6)
# define DP_CEC_TX_DATA_NACK_ERROR              (1 << 7)

#define DP_CEC_RX_MESSAGE_BUFFER               0x3010
#define DP_CEC_TX_MESSAGE_BUFFER               0x3020
#define DP_CEC_MESSAGE_BUFFER_LENGTH             0x10

//class io_service_t;
class IOI2CRequest;

class DisplayPortAux {
 public:
  DisplayPortAux();
  virtual ~DisplayPortAux();

  bool Write(uint16_t address, uint8_t data) { return Write(address, &data, 1); }
  bool Write(uint16_t address, uint8_t *data, uint32_t len);

  bool Read(uint16_t address, uint8_t *byte) { return Read(address, byte, 1); }
  bool Read(uint16_t address, uint8_t *data, uint32_t len);

 private:
  bool DisplayRequest(IOI2CRequest *request);

  io_service_t m_framebuffer;
  mutable P8PLATFORM::CMutex m_mutex;
};
#endif
