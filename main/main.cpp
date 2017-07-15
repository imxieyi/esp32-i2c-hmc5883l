#include "esp_log.h"
#include "hmc5883l.hpp"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <cstdio>

HMC5883L compass(GPIO_NUM_19, GPIO_NUM_22);

void myTask(void *pvParameters) {

  ESP_LOGI("TASK", "task created");
  
  // Set measurement range
  // +/- 0.88 Ga: HMC5883L_RANGE_0_88GA
  // +/- 1.30 Ga: HMC5883L_RANGE_1_3GA (default)
  // +/- 1.90 Ga: HMC5883L_RANGE_1_9GA
  // +/- 2.50 Ga: HMC5883L_RANGE_2_5GA
  // +/- 4.00 Ga: HMC5883L_RANGE_4GA
  // +/- 4.70 Ga: HMC5883L_RANGE_4_7GA
  // +/- 5.60 Ga: HMC5883L_RANGE_5_6GA
  // +/- 8.10 Ga: HMC5883L_RANGE_8_1GA
  compass.setRange(HMC5883L_RANGE_1_3GA);

  // Set measurement mode
  // Idle mode:              HMC5883L_IDLE
  // Single-Measurement:     HMC5883L_SINGLE
  // Continuous-Measurement: HMC5883L_CONTINOUS (default)
  compass.setMeasurementMode(HMC5883L_CONTINOUS);
 
  // Set data rate
  //  0.75Hz: HMC5883L_DATARATE_0_75HZ
  //  1.50Hz: HMC5883L_DATARATE_1_5HZ
  //  3.00Hz: HMC5883L_DATARATE_3HZ
  //  7.50Hz: HMC5883L_DATARATE_7_50HZ
  // 15.00Hz: HMC5883L_DATARATE_15HZ (default)
  // 30.00Hz: HMC5883L_DATARATE_30HZ
  // 75.00Hz: HMC5883L_DATARATE_75HZ
  compass.setDataRate(HMC5883L_DATARATE_15HZ);

  // Set number of samples averaged
  // 1 sample:  HMC5883L_SAMPLES_1 (default)
  // 2 samples: HMC5883L_SAMPLES_2
  // 4 samples: HMC5883L_SAMPLES_4
  // 8 samples: HMC5883L_SAMPLES_8
  compass.setSamples(HMC5883L_SAMPLES_1);

	Vector raw;
	Vector norm;
  char str[128];
	while (1) {
		raw = compass.readRaw();
		norm = compass.readNormalize();
    sprintf(str, "Raw: %.2f, %.2f, %.2f", raw.XAxis, raw.YAxis, raw.ZAxis);
    ESP_LOGI("TASK", "%s", str);
    sprintf(str, "Norm: %.2f, %.2f, %.2f", norm.XAxis, norm.YAxis, norm.ZAxis);
    ESP_LOGI("TASK", "%s", str);
		vTaskDelay(500 / portTICK_PERIOD_MS);
	}

}

extern "C" void app_main() {

	if (compass.start()) {
		ESP_LOGI("HMC", "hmc5883l started");
		xTaskCreatePinnedToCore(&myTask, "hmctask", 2048, NULL, 5, NULL, 1);
	} else {
		ESP_LOGE("HMC", "hmc5883l init failed");
	}
}
