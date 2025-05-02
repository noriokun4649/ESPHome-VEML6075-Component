#include "veml6075.h"
#include "esphome/core/log.h"

namespace esphome {
namespace veml6075 {

static const char *const TAG = "veml6075.sensor";

void VEML6075Component::setup() {
  ESP_LOGCONFIG(TAG, "Setting up VEML6075");
  this->configure();
  state_ = State::READY;
}

void VEML6075Component::configure() {
  ESP_LOGV(TAG, "Configure");

  _commandRegister.reg = 0;

  uint8_t data[2] = {0x00};

  this->read_register(REG_ID, data, 2, false);

  if (data[0] != 0x26) {
    ESP_LOGE(TAG, "Wrong manufacturer id");
    return;
  }

  this->shutdown(true);

  this->setForcedMode(false);

  this->setIntegrationTime(VEML6075_100MS);

  this->setHighDynamic(false);

  this->shutdown(false);

  return;
}

void VEML6075Component::shutdown(bool sd) {
  _commandRegister.SD = sd;
  auto err = this->write_register(REG_UV_CONF, _commandRegister.reg_bytes, 2);
  if (err != i2c::ERROR_OK) {
    ESP_LOGW(TAG, "Shutdown failed ");
  }
}

void VEML6075Component::setForcedMode(bool flag) {
  _commandRegister.UV_AF = flag;
  auto err = this->write_register(REG_UV_CONF, _commandRegister.reg_bytes, 2);
  if (err != i2c::ERROR_OK) {
    ESP_LOGW(TAG, "Forced failed ");
  }
}

void VEML6075Component::setIntegrationTime(veml6075_integrationtime_t itime) {
  // Set integration time
  _commandRegister.UV_IT = (uint8_t) itime;
  auto err = this->write_register(REG_UV_CONF, _commandRegister.reg_bytes, 2);
  if (err != i2c::ERROR_OK) {
    ESP_LOGW(TAG, "Set integration failed ");
  }

  _read_delay = 0;
  switch (itime) {
    case VEML6075_50MS:
      _read_delay = 50;
      break;
    case VEML6075_100MS:
      _read_delay = 100;
      break;
    case VEML6075_200MS:
      _read_delay = 200;
      break;
    case VEML6075_400MS:
      _read_delay = 400;
      break;
    case VEML6075_800MS:
      _read_delay = 800;
      break;
  }
}

void VEML6075Component::setHighDynamic(bool hd) {
  _commandRegister.UV_HD = hd;
  auto err = this->write_register(REG_UV_CONF, _commandRegister.reg_bytes, 2);
  if (err != i2c::ERROR_OK) {
    ESP_LOGW(TAG, "High dynamic failed ");
  }
}

void VEML6075Component::takeReading(void) {
  uint8_t uva_data[2] = {0x00};
  uint8_t uvb_data[2] = {0x00};
  uint8_t uvcomp1_data[2] = {0x00};
  uint8_t uvcomp2_data[2] = {0x00};

  // take the reading immediately
  auto err1 = this->read_register(REG_UVA_DATA, uva_data, 2, false);
  auto err2 = this->read_register(REG_UVB_DATA, uvb_data, 2, false);
  auto err3 = this->read_register(REG_UVCOMP1_DATA, uvcomp1_data, 2, false);
  auto err4 = this->read_register(REG_UVCOMP2_DATA, uvcomp2_data, 2, false);

  if (err1 != i2c::ERROR_OK) {
    ESP_LOGW(TAG, "Read1 failed ");
    return;
  }
  if (err2 != i2c::ERROR_OK) {
    ESP_LOGW(TAG, "Read2 failed ");
    return;
  }
  if (err3 != i2c::ERROR_OK) {
    ESP_LOGW(TAG, "Read3 failed ");
    return;
  }
  if (err4 != i2c::ERROR_OK) {
    ESP_LOGW(TAG, "Read4 failed ");
    return;
  }

  uint16_t rawUVA = (uva_data[0] & 0x00FF) | ((uva_data[1] & 0x00FF) << 8);
  uint16_t rawUVB = (uvb_data[0] & 0x00FF) | ((uvb_data[1] & 0x00FF) << 8);
  uint16_t comp1 = (uvcomp1_data[0] & 0x00FF) | ((uvcomp1_data[1] & 0x00FF) << 8);
  uint16_t comp2 = (uvcomp2_data[0] & 0x00FF) | ((uvcomp2_data[1] & 0x00FF) << 8);

  float uvaCalc =
      (float) rawUVA - ((UVA_A_COEF * UV_ALPHA * comp1) / UV_GAMMA) - ((UVA_B_COEF * UV_ALPHA * comp2) / UV_DELTA);

  float uvbCalc =
      (float) rawUVB - ((UVB_C_COEF * UV_BETA * comp1) / UV_GAMMA) - ((UVB_D_COEF * UV_BETA * comp2) / UV_DELTA);

  float uvia = uvaCalc * (1.0 / UV_ALPHA) * _aResponsivity;
  float uvib = uvbCalc * (1.0 / UV_BETA) * _bResponsivity;
  float uvIndex = (uvia + uvib) / 2.0;
  
  if (_hdEnabled) {
    uvIndex *= HD_SCALAR;
  }


  ESP_LOGI(TAG, "Read success with UVAcalc %f and UVBcalc %f ", uvaCalc, uvbCalc);
  ESP_LOGI(TAG, "UV index: %f ", uvIndex);


  // Publish
  if (this->uva_sensor_ != nullptr) {
    this->uva_sensor_->publish_state(uvaCalc);
  }
  if (this->uvb_sensor_ != nullptr) {
    this->uvb_sensor_->publish_state(uvbCalc);
  }
  if (this->uv_index_sensor_ != nullptr) {
    this->uv_index_sensor_->publish_state(uvIndex);
  }

  if (this->raw1_sensor_ != nullptr) {
    this->raw1_sensor_->publish_state(rawUVA);
  }
  if (this->raw2_sensor_ != nullptr) {
    this->raw2_sensor_->publish_state(rawUVB);
  }
  if (this->raw3_sensor_ != nullptr) {
    this->raw3_sensor_->publish_state(comp1);
  }
  if (this->raw4_sensor_ != nullptr) {
    this->raw4_sensor_->publish_state(comp2);
  }

  
}

void VEML6075Component::dump_config() {
  ESP_LOGCONFIG(TAG, "Hello");
  ESP_LOGCONFIG(TAG, "Hello");
}

void VEML6075Component::update() {
  if (state_ == State::READY) {
    takeReading();

    uint8_t data[2] = {0x00};
    this->read_register(REG_ID, data, 2, false);
    ESP_LOGI(TAG, "Manufacturer id %i,%i", data[0], data[1]);

    data[2] = {0x00};
    this->read_register(REG_UV_CONF, data, 2, false);
    ESP_LOGI(TAG, "Conf reg %i,%i", data[0], data[1]);

  } else {
    ESP_LOGI(TAG, "Not ready for reading");
  }
}

}  // namespace veml6075
}  // namespace esphome
