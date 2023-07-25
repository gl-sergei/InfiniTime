#include "components/motor/MotorController.h"
#include <nrf_drv_pwm.h>
#include <hal/nrf_gpio.h>
#include <nrfx_pwm.h>
#include "systemtask/SystemTask.h"
#include "drivers/PinMap.h"

using namespace Pinetime::Controllers;

nrfx_pwm_t m_pwm0 = NRFX_PWM_INSTANCE(0);

#define PERIOD 16000
#define DUTY 25

void init_pwm(void)
{
    nrfx_pwm_config_t config;

    config.output_pins[0] = Pinetime::PinMap::Motor;
    config.output_pins[1] = NRFX_PWM_PIN_NOT_USED;
    config.output_pins[2] = NRFX_PWM_PIN_NOT_USED;
    config.output_pins[3] = NRFX_PWM_PIN_NOT_USED;

    config.irq_priority   = 6;
    config.base_clock     = NRF_PWM_CLK_1MHz;
    config.count_mode     = NRF_PWM_MODE_UP;
    config.top_value      = 1000;
    config.load_mode      = NRF_PWM_LOAD_INDIVIDUAL;
    config.step_mode      = NRF_PWM_STEP_AUTO;

    nrfx_pwm_init(&m_pwm0, &config, NULL);

    uint16_t pulse_width = PERIOD / 4;

    // TODO: Move DMA buffer to global memory.
    volatile static uint16_t pwm_seq[4];

    pwm_seq[0] = PERIOD - pulse_width * 3;
    pwm_seq[1] = PERIOD - pulse_width * 2;

    pwm_seq[2] = PERIOD - pulse_width;
    pwm_seq[3] = PERIOD;

    const nrf_pwm_sequence_t pwm_sequence = {
        .values = {.p_raw = (const uint16_t *)&pwm_seq},
        .length = 4,
        .repeats = 0,
        .end_delay = 0
    };

    nrfx_pwm_simple_playback(&m_pwm0,
                             &pwm_sequence,
                             0, // Loop disabled.
                             0);

}

void deinit_pwm(void) {
    nrfx_pwm_stop(&m_pwm0, true);
    nrfx_pwm_uninit(&m_pwm0);
}

void MotorController::Init() {
  //nrf_gpio_cfg_output(PinMap::Motor);
  //nrf_gpio_pin_set(PinMap::Motor);

  shortVib = xTimerCreate("shortVib", 1, pdFALSE, nullptr, StopMotor);
  longVib = xTimerCreate("longVib", pdMS_TO_TICKS(1000), pdTRUE, this, Ring);
}

void MotorController::Ring(TimerHandle_t xTimer) {
  auto* motorController = static_cast<MotorController*>(pvTimerGetTimerID(xTimer));
  motorController->RunForDuration(250);
}

void MotorController::RunForDuration(uint16_t motorDuration) {
  if (motorDuration > 0 && xTimerChangePeriod(shortVib, pdMS_TO_TICKS(motorDuration), 0) == pdPASS && xTimerStart(shortVib, 0) == pdPASS) {
    init_pwm();
    //nrf_gpio_pin_clear(PinMap::Motor);
  }
}

void MotorController::StartRinging() {
  RunForDuration(400);
  xTimerStart(longVib, 0);
}

void MotorController::StopRinging() {
  xTimerStop(longVib, 0);
  deinit_pwm();
  nrf_gpio_pin_set(PinMap::Motor);
}

void MotorController::StopMotor(TimerHandle_t xTimer) {
  deinit_pwm();
  nrf_gpio_pin_set(PinMap::Motor);
}
