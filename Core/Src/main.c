/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "crsf_serial.h"
#include <stdio.h>
#include "servo_control.h"
#include "mpu9250.h"
#include "neo6m.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart6;
extern I2C_HandleTypeDef hi2c1;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
// Voltage monitoring
#define R1		10000.0f	// Ohm
#define R2		2200.0f	// Ohm
#define VREF	3.3f	// Reference voltage of ADC
#define ADC_MAX_VALUE      4095.0f	// 12-bit ADC
float batteryVoltage;
uint16_t millivolts;

uint16_t voltage_dV;

uint8_t sendStatus = 6;

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

// Global CRSF serial instance
CrsfSerial_HandleTypeDef hcrsf;
CrsfSerial_HandleTypeDef crsf_handle_rx;

uint16_t ADC_BUF[1];
uint16_t Buf[64];
uint32_t lastPacketMillis = 0;
uint32_t sendTiming;
<<<<<<< HEAD
uint32_t now;

//Timing variables for the main loop ---
uint32_t lastBatterySendTime = 0;
const uint32_t BATTERY_SEND_INTERVAL_MS = 40; // Send every 1 second
=======

//Timing variables for the main loop ---
uint32_t lastBatterySendTime = 0;
const uint32_t BATTERY_SEND_INTERVAL_MS = 500; // Send every 1 second
>>>>>>> 363fa39 (telemetry send upload)

#define UART_RX_BUFFER_SIZE 128
uint8_t  uartRxBuf[UART_RX_BUFFER_SIZE];
uint16_t oldPos = 0;

bool failsafeActive = false;
int lastValidChannels[CRSF_NUM_CHANNELS];
uint8_t telemetry_Channels[64];
uint8_t telem_queue_status = 6;

<<<<<<< HEAD
volatile bool telemetry_window_open = false;
volatile uint32_t telemetry_window_deadline = 0;
#define TELEMETRY_PERIOD_US   4000   // 4ms
static uint32_t lastTelemetryUs = 0;


=======
>>>>>>> 363fa39 (telemetry send upload)
float A_X, A_Y, A_Z, G_X, G_Y, G_Z, TEMPERATUE;
float Roll, Pitch, Yaw;

uint32_t prev_tick = 0;
uint8_t status_i2c;

// GPS
#define RX_BUF_SIZE 256

uint8_t rx_dma_buf[RX_BUF_SIZE];
uint8_t rx_data[RX_BUF_SIZE];
uint8_t Num_Sats;
uint8_t Fix;
uint16_t old_pos = 0;
float Latitude, Longitude, Altitude, Speed;

NEO6M_Data_t *gps;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

// Called when a valid RC packet is received
void onRCFrameReceived(const int *rcChannels) {
    lastPacketMillis = HAL_GetTick();  // Timestamp of last valid packet
    failsafeActive = false;

    // Store last valid RC values
    for (int i = 0; i < CRSF_NUM_CHANNELS; i++) {
        lastValidChannels[i] = rcChannels[i];
    }

    // Update servos here with lastValidChannels
    ServoControl_Update(lastValidChannels);
}
// This is the function that will be registered as the callback.
// Its name and signature MUST match the pointer in the struct.
void onPacketChannelsReceived(void) {
	// The CRSF library has already updated hcrsf.channels with the new data.
	// We just need to use it.
	lastPacketMillis = HAL_GetTick(); failsafeActive = false;
	// The 'lastValidChannels' buffer might be redundant if you always
	// read from hcrsf.channels, but it's good practice for failsafe.
	for (int i = 0; i < CRSF_NUM_CHANNELS; i++) {
		lastValidChannels[i] = hcrsf.channels[i];
	}
	// Update your servos with the fresh data from the handle.
	ServoControl_Update(hcrsf.channels);
}

// This function is ONLY called by the CRSF library when the link goes down.
void onLinkDown() {
	failsafeActive = true; // Add code here to handle failsafe, e.g., center servos.
	  // Activate failsafe behavior: hold last known values
	  ServoControl_Update(lastValidChannels);
}

// This function packs battery
/*void onPacketBatterysend(){

}*/

// Clalculating battery voltage
float GetBatteryVoltage(void) {
	uint16_t adc_value = ADC_BUF[0];
	// Convert ADC reading to measured divider voltage
	float vout = (adc_value * VREF) / ADC_MAX_VALUE;

	// Claculate battery voltage
	float vbat = vout * ((R1 + R2) / R2);

	return vbat;
}

// This function should use the library's queue, not transmit directly.
/*uint8_t sendBatteryVoltage(uint16_t millivolts) {
    uint16_t voltage_cV = millivolts / 100; // convert to 0.01V units

	uint8_t payload[7] = {0}; // Initialize to zero
//	// Voltage (Little Endian)
//	payload[0] = (uint8_t)(millivolts & 0xFF);
//	payload[1] = (uint8_t)(millivolts >> 8);

    // Voltage (BIG ENDIAN)
    payload[0] = (voltage_cV >> 8) & 0xFF;
    payload[1] = voltage_cV & 0xFF;

    // Current (not measured → 0 mA)
    payload[2] = 0;
    payload[3] = 0;

    // Fuel used (mAh → 0)
    payload[4] = 0;
    payload[5] = 0;

    // Remaining capacity (percent → 0 if unknown)
    payload[6] = 0;

	// Queue the packet for transmission.
	return CrsfSerial_QueuePacket(&hcrsf, CRSF_FRAMETYPE_BATTERY_SENSOR, payload, sizeof(payload));
}*/

static uint8_t sendBatteryTelemetry(float voltage, float current, float capacity, float remaining){
	crsf_sensor_battery_t crsfBat = {0};

	// Values are MSB first (big Endian)
	crsfBat.voltage = htobe32((uint32_t)(voltage*10.0));	// Volts
	crsfBat.current = htobe32((uint32_t)(current*10.0));	// Amps
	crsfBat.capacity = htobe32((uint32_t)(capacity)) << 8;	//mAh (with this implemetation max capacity is 65535mAh)
	crsfBat.remaining = (uint32_t)(remaining);				// Percent
	// Queue the packet for transmission
	return CrsfSerial_QueuePacket(&hcrsf, CRSF_FRAMETYPE_BATTERY_SENSOR, &crsfBat, sizeof(crsfBat));
}
<<<<<<< HEAD

static inline uint32_t micros(void)
{
    return DWT->CYCCNT / (HAL_RCC_GetHCLKFreq() / 1000000);
}
=======
>>>>>>> 363fa39 (telemetry send upload)

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART1_UART_Init();
  MX_ADC1_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_I2C1_Init();
  MX_USART6_UART_Init();
  /* USER CODE BEGIN 2 */

  ServoControl_Init();

  // Initialize CRSF with UART and baud rate
  CrsfSerial_Init(&hcrsf, &huart1, CRSF_BAUDRATE);
  // *** REGISTER THE CALLBACKS ***
  //hcrsf.onRCFrameReceived = onRCFrameReceived;
  hcrsf.onPacketChannels = onPacketChannelsReceived;
//  hcrsf.onPacketBattery = onPacketBatterysend;
  hcrsf.onLinkDown = onLinkDown;

  //CrsfSerial_Begin(&hcrsf, CRSF_BAUDRATE);

  HAL_UART_Receive_DMA(&huart1, uartRxBuf, UART_RX_BUFFER_SIZE);

  __HAL_UART_ENABLE_IT(&huart1, UART_IT_IDLE);

  // Strating ADC
  HAL_ADC_Start_DMA(&hadc1, (uint32_t *)ADC_BUF, 1);

<<<<<<< HEAD
  // Enable DWT Cycle Counter
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk; // Enable access to DWT
  DWT->CYCCNT = 0;                                // Reset the counter
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;            // Enable the cycle counter

=======
>>>>>>> 363fa39 (telemetry send upload)
  status_i2c = mpu_init(&hi2c1);

  if (status_i2c == 1) {
	  HAL_GPIO_TogglePin(DPIN_LED_GPIO_Port, DPIN_LED_Pin);
	  HAL_Delay(250);
	  HAL_GPIO_TogglePin(DPIN_LED_GPIO_Port, DPIN_LED_Pin);
	  HAL_Delay(500);
	  HAL_GPIO_TogglePin(DPIN_LED_GPIO_Port, DPIN_LED_Pin);
	  HAL_Delay(250);
	  HAL_GPIO_TogglePin(DPIN_LED_GPIO_Port, DPIN_LED_Pin);

	  MPU_calibrateGyro(&hi2c1, 1500);
  } else {
	  HAL_GPIO_WritePin(DPIN_LED_GPIO_Port, DPIN_LED_Pin, GPIO_PIN_SET);
  }

//  NEO6M_Init(&huart6);

  // Start UART DMA reception in circular mode for NEO
//  HAL_UART_Receive_DMA(&huart6, rx_dma_buf, RX_BUF_SIZE);
//  __HAL_UART_ENABLE_IT(&huart6, UART_IT_IDLE);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

<<<<<<< HEAD
//	  sendTiming = HAL_GetTick();

	  now = micros();

	  CrsfSerial_Loop(&hcrsf);

/*	if (telemetry_window_open &&
		(int32_t)(now - telemetry_window_deadline) >= 0)
	{
		telemetry_window_open = false;
	}*/
    /* Telemetry conditions:
       - RX recently spoke (window open)
       - link is up
       - UART idle
       - 4ms elapsed
    */
    if (telemetry_window_open &&
        hcrsf.linkIsUp &&
        (int32_t)(now - lastTelemetryUs) >= TELEMETRY_PERIOD_US)
    {
        lastTelemetryUs += TELEMETRY_PERIOD_US;

		batteryVoltage = GetBatteryVoltage(); // e.g., returns 12.6f
		millivolts = batteryVoltage * 1000; // Convert to mV
        voltage_dV = millivolts / 100; // CRSF uses 0.1V units
        sendStatus = sendBatteryTelemetry(
        		batteryVoltage,
            0.0f, 0.0f, 0.0f
        );
        telemetry_window_open = false;
    }

      // ====================================================================
      //  PHASE 2: Telemetry Data Queuing
      // ====================================================================

	  // Check if it's time to send the battery voltage
/*	  if ((int32_t)(sendTiming - lastBatterySendTime) >= BATTERY_SEND_INTERVAL_MS){
		  lastBatterySendTime += BATTERY_SEND_INTERVAL_MS;
		  if (CrsfSerial_IsLinkUp(&hcrsf)) {
			  // Get voltage and queue the packet for sending
			  // The CrsfSerial_Loop will handle the actual transmission when it can.
			  batteryVoltage = GetBatteryVoltage(); // e.g., returns 12.6f
			  millivolts = batteryVoltage * 1000; // Convert to mV
	          voltage_dV = millivolts / 100; // CRSF uses 0.1V units

	          sendStatus = sendBatteryTelemetry(batteryVoltage, 3.0, 4.0, 5.0);
		  }
	  }*/
//	  if ((CrsfSerial_IsLinkUp(&hcrsf)) && (HAL_GetTick() - lastBatterySendTime >= BATTERY_SEND_INTERVAL_MS)) {
//	  //if (!CrsfSerial_IsLinkUp(&hcrsf)) {
//		  lastBatterySendTime = HAL_GetTick(); // Update the timestamp
//		  // Get voltage and queue the packet for sending
//		  // The CrsfSerial_Loop will handle the actual transmission when it can.
//		  batteryVoltage = GetBatteryVoltage(); // e.g., returns 12.6f
//		  millivolts = batteryVoltage * 1000; // Convert to mV
//          voltage_dV = millivolts / 100; // CRSF uses 0.1V units
//
//          sendStatus = sendBatteryTelemetry(batteryVoltage, 3.0, 4.0, 5.0);
//	  }

=======
	  sendTiming = HAL_GetTick();

	  CrsfSerial_Loop(&hcrsf);

      // ====================================================================
      //  PHASE 2: Telemetry Data Queuing
      // ====================================================================

	  // Check if it's time to send the battery voltage
	  if ((CrsfSerial_IsLinkUp(&hcrsf)) && (HAL_GetTick() - lastBatterySendTime >= BATTERY_SEND_INTERVAL_MS)) {
	  //if (!CrsfSerial_IsLinkUp(&hcrsf)) {
		  lastBatterySendTime = HAL_GetTick(); // Update the timestamp
		  // Get voltage and queue the packet for sending
		  // The CrsfSerial_Loop will handle the actual transmission when it can.
		  batteryVoltage = GetBatteryVoltage(); // e.g., returns 12.6f
		  millivolts = batteryVoltage * 1000; // Convert to mV
          voltage_dV = millivolts / 100; // CRSF uses 0.1V units

/*          crsf_sensor_battery_t battery_payload = {0};
          battery_payload.voltage = voltage_dV;
          // Other fields (current, capacity, remaining_percent) can be set here if available

          // Queue the battery telemetry packet. It will be sent when polled by the TX.
          telem_queue_status = Crsf_QueueTelemetryPacket(&crsf_handle_rx, CRSF_FRAMETYPE_BATTERY_SENSOR, (uint8_t*)&battery_payload, sizeof(crsf_sensor_battery_t));
          for (int i = 0; i < 64; i++){
        	  telemetry_Channels[i] = hcrsf.telemetry_tx_buffer[i];
          }*/
		  // Use the provided function to queue the packet.
		  // Do not check the return status here in the main loop.
		  // The library is responsible for managing the transmission.
//		  sendStatus = sendBatteryVoltage(millivolts);
          sendStatus = sendBatteryTelemetry(batteryVoltage, 10.0, 45.0, 50);
	  }

      // ====================================================================
      //  PHASE 3: Telemetry Response to Polls
      // ====================================================================
      // This flag is set by Crsf_ProcessByte when a poll packet is received.
/*      if (crsf_handle_rx.telemetry_poll_received) {
          crsf_handle_rx.telemetry_poll_received = false; // Clear the flag

          // Check if there's a telemetry packet queued and the UART is not busy.
          if (!crsf_handle_rx.tx_busy && crsf_handle_rx.telemetry_tx_len > 0) {
              // Transmit the queued telemetry packet.
              Crsf_TransmitPacket(&crsf_handle_rx, crsf_handle_rx.telemetry_tx_buffer, crsf_handle_rx.telemetry_tx_len);
              crsf_handle_rx.telemetry_tx_len = 0; // Clear the queue after sending
          }
      }*/
>>>>>>> 363fa39 (telemetry send upload)
//	  if(sendStatus == 0) {
//		  HAL_GPIO_TogglePin(DPIN_LED_GPIO_Port, DPIN_LED_Pin);
//	  }

	  MPU_calcAttitude(&hi2c1);

	  Roll = attitude.r;
	  Pitch = attitude.p;
	  Yaw = attitude.y;

//	  gps = NEO6M_GetData();
//	  if (gps->fix)
//	  {
//		  Fix = gps->fix;
//		  Latitude = gps->latitude;
//		  Longitude = gps->longitude;
//		  Altitude = gps->altitude;
//		  Speed = gps->speed;
//		  Num_Sats = gps->num_sats;
//	  }


  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 12;
  RCC_OscInitStruct.PLL.PLLN = 96;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
