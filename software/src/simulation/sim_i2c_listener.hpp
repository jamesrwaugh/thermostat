#pragma once

#include <cstdint>
#include <optional>
#include <simavr-toolbox/sim_i2c_base.hpp>
#include <string_view>
#include <variant>

using I2cStructMessageCallback = std::function<void(avr_twi_msg_irq_t*)>;

class SimI2CListener {
 public:
  SimI2CListener(avr_t* avr);

  void HandleI2CMessage(const avr_twi_msg_t& msg);

  enum class RegWriteMethod { RegisterThenData = 0, RawDataOnly };

  struct DeviceConfig {
    const uint8_t Address;
    std::string_view Name;
    RegWriteMethod WriteMethod;
  };

  struct ReadMessage {
    ReadMessage(uint8_t address) : I2cAddress(address) {}
    const uint8_t I2cAddress;
    uint8_t Register{0};
    bool Acknowledged{false};
    std::vector<uint8_t> BytesRead;
  };

  struct WriteMessage {
    WriteMessage(uint8_t address) : I2cAddress(address) {}
    const uint8_t I2cAddress;
    uint8_t Register{0};
    bool Acknowledged{false};
    std::vector<uint8_t> BytesWritten;
  };

  typedef std::variant<ReadMessage, WriteMessage> Message;

 private:
  avr_t* Avr_{nullptr};
  I2cStructMessageCallback OnMessageFromAvr_;
  I2cStructMessageCallback OnMessageToAvr_;
  void OnMessageFromAvr(avr_twi_msg_irq_t* value);
  void OnMessageToAvr(avr_twi_msg_irq_t* value);

  std::optional<Message> MessageInProgress_;
  std::vector<Message> Messages_;
};