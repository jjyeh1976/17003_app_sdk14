#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include "nrf_spim.h"
#include "nrf_drv_spi.h"
#include "nrf_delay.h"
#include "nrf_drv_gpiote.h"
#include "sdk_config.h"
#include "gd17003.h"

#include "peripheral_if.h"
#include "bmi160.h"
#include "bmi160_cmd.h"
#include "sysparams.h"

#include "nrf_log.h"


#define AR(x,y,z) ((x <=y) && (y <= z))
static const nrf_drv_spi_t bmi_spi = NRF_DRV_SPI_INSTANCE(1);

static bmi_callback f_cb;
static uint8_t *f_buf;

// bmi160
static struct bmi160_dev bmi160 = {
  .chip_id = 0xD1,
  .id = 0x1,
  .interface = BMI160_SPI_INTF,
  .accel_cfg = {BMI160_ACCEL_NORMAL_MODE,BMI160_ACCEL_ODR_25HZ,BMI160_ACCEL_RANGE_2G,BMI160_ACCEL_BW_NORMAL_AVG4},
  .gyro_cfg = {BMI160_GYRO_NORMAL_MODE,BMI160_GYRO_ODR_25HZ,BMI160_GYRO_RANGE_2000_DPS,BMI160_GYRO_BW_NORMAL_MODE},
  .read = bmi160_read,
  .write = bmi160_write,
  .delay_ms = bmi160_delay
};

static struct bmi160_dev* dev = &bmi160;

//uint8_t bmi_buffer[1200];
struct bmi160_fifo_frame fifo_frame;
struct bmi160_int_settg int_config;

int8_t bmi160_cmd_startfoc(struct bmi160_dev *dev)
{
  int8_t rslt = BMI160_OK;
  struct bmi160_foc_conf foc_conf;
  struct bmi160_offsets offsets;
  
  foc_conf.acc_off_en = BMI160_ENABLE;
  foc_conf.foc_acc_x = BMI160_FOC_ACCEL_0G;
  foc_conf.foc_acc_y = BMI160_FOC_ACCEL_0G;
  foc_conf.foc_acc_z = BMI160_FOC_ACCEL_POSITIVE_G;
  
  foc_conf.gyro_off_en = BMI160_ENABLE;
  foc_conf.foc_gyr_en = BMI160_ENABLE;
  
  rslt = bmi160_start_foc(&foc_conf, &offsets, dev);
  
  if(rslt == BMI160_OK){
    //memcpy((uint8_t*)&moduleParam.bmi160.offsets,(uint8_t*)&offsets,sizeof(struct bmi160_offsets));
  }
  
  return rslt;
}

void bmi160_cmd_testRead(struct bmi160_dev *dev)
{
  int8_t rslt = BMI160_OK;
  struct bmi160_sensor_data accel;
  struct bmi160_sensor_data gyro;

  /* To read only Accel data */
  rslt = bmi160_get_sensor_data(BMI160_ACCEL_SEL, &accel, NULL, dev);

  /* To read only Gyro data */
  rslt = bmi160_get_sensor_data(BMI160_GYRO_SEL, NULL, &gyro, dev);

  /* To read both Accel and Gyro data */
  bmi160_get_sensor_data((BMI160_ACCEL_SEL | BMI160_GYRO_SEL), &accel, &gyro, dev);

  /* To read Accel data along with time */
  rslt = bmi160_get_sensor_data((BMI160_ACCEL_SEL | BMI160_TIME_SEL) , &accel, NULL, dev);

  /* To read Gyro data along with time */
  rslt = bmi160_get_sensor_data((BMI160_GYRO_SEL | BMI160_TIME_SEL), NULL, &gyro, dev);

  /* To read both Accel and Gyro data along with time*/
  bmi160_get_sensor_data((BMI160_ACCEL_SEL | BMI160_GYRO_SEL | BMI160_TIME_SEL), &accel, &gyro, dev);
}

void bmi160_cmd_config_int(struct bmi160_dev *dev, uint8_t st)
{
  struct bmi160_int_settg int_config;
  
//  fifo_frame.data = bmi_buffer;
//  fifo_frame.length = 1200;
//  dev->fifo = &fifo_frame;
//  bmi160_set_fifo_config(BMI160_FIFO_CONFIG_1_MASK, BMI160_DISABLE,dev);
//  bmi160_set_fifo_config(BMI160_FIFO_GYRO | BMI160_FIFO_ACCEL, BMI160_ENABLE,dev);
  
  if(st){
    int_config.int_channel = BMI160_INT_CHANNEL_1;
    int_config.int_type = BMI160_ACC_GYRO_DATA_RDY_INT;
    int_config.int_pin_settg.output_en = BMI160_ENABLE;
    int_config.int_pin_settg.output_mode = BMI160_DISABLE;
    int_config.int_pin_settg.output_type = BMI160_DISABLE;
    int_config.int_pin_settg.edge_ctrl = BMI160_ENABLE;
    int_config.int_pin_settg.input_en = BMI160_DISABLE;
    int_config.int_pin_settg.latch_dur = BMI160_LATCH_DUR_NONE;
    bmi160_set_int_config(&int_config,dev);
    
//    int_config.int_type = BMI160_ACC_ORIENT_INT;
//    int_config.int_type_cfg.acc_orient_int.orient_en = BMI160_ENABLE;
//    int_config.int_type_cfg.acc_orient_int.axes_ex = 1; // axes remapping
//    int_config.int_type_cfg.acc_orient_int.orient_ud_en = BMI160_DISABLE;
//    int_config.int_type_cfg.acc_orient_int.orient_theta = 1;
//    int_config.int_type_cfg.acc_orient_int.orient_hyst = 1; // 1-lsb = 62.5mg for 2g range
//    int_config.int_type_cfg.acc_orient_int.orient_blocking = 0; // no blocking
//    int_config.int_type_cfg.acc_orient_int.orient_mode = 0; //0: sym, 1: hi-asym, 2: low asym
//    bmi160_set_int_config(&int_config,dev);
  }
  else{
    int_config.int_channel = BMI160_INT_CHANNEL_NONE;
    int_config.int_type = BMI160_ACC_GYRO_DATA_RDY_INT;
    bmi160_set_int_config(&int_config,dev);

//    int_config.int_type = BMI160_ACC_ORIENT_INT;
//    bmi160_set_int_config(&int_config,dev);
  }
}
/*
  config to anymotion interrupt
*/
void bmi160_cmd_config_int_am(struct bmi160_dev *dev,uint8_t st)
{
  struct bmi160_int_settg int_config;
  if(st){
    int_config.int_channel = BMI160_INT_CHANNEL_1;
    int_config.int_type = BMI160_ACC_ANY_MOTION_INT;
    int_config.int_pin_settg.output_en = BMI160_ENABLE;
    int_config.int_pin_settg.output_mode = BMI160_DISABLE;
    int_config.int_pin_settg.output_type = BMI160_DISABLE;
    int_config.int_pin_settg.edge_ctrl = BMI160_ENABLE;
    int_config.int_pin_settg.input_en = BMI160_DISABLE;
    int_config.int_pin_settg.latch_dur = BMI160_LATCH_DUR_NONE;
    int_config.int_type_cfg.acc_any_motion_int.anymotion_en = BMI160_ENABLE;
    int_config.int_type_cfg.acc_any_motion_int.anymotion_x = BMI160_ENABLE;
    int_config.int_type_cfg.acc_any_motion_int.anymotion_y = BMI160_ENABLE;
    int_config.int_type_cfg.acc_any_motion_int.anymotion_z = BMI160_ENABLE;
    int_config.int_type_cfg.acc_any_motion_int.anymotion_dur = 0;
    int_config.int_type_cfg.acc_any_motion_int.anymotion_thr = 20;
  }
  else{
    int_config.int_channel = BMI160_INT_CHANNEL_NONE;
    int_config.int_type = BMI160_ACC_ANY_MOTION_INT;
  }
  
  
  bmi160_set_int_config(&int_config,dev);
}

void bmi160_cmd_deconfig_int(struct bmi160_dev *dev)
{
  struct bmi160_int_settg int_config;
  int_config.int_channel = BMI160_INT_CHANNEL_NONE;
  int_config.int_type = BMI160_ACC_GYRO_FIFO_FULL_INT;
  bmi160_set_int_config(&int_config,dev);
}


void bmi160_cmd_acquire_one(struct bmi160_dev *dev, uint8_t *ptr)
{
  int8_t rslt = BMI160_OK;
  struct bmi160_sensor_data accel;
  struct bmi160_sensor_data gyro;
  bmi160_get_sensor_data((BMI160_ACCEL_SEL | BMI160_GYRO_SEL), &accel, &gyro, dev);
  //memcpy(ptr, (uint8_t*)&accel,6);
  //ptr += 6;
  memcpy(ptr, (uint8_t*)&gyro, 6);
}

int8_t bmi160_update_sensor_config(struct bmi160_dev *dev)
{
  return bmi160_set_sens_conf(dev);
}

int8_t bmi160_cmd_pwdn(struct bmi160_dev *dev)
{
  dev->accel_cfg.power = BMI160_ACCEL_LOWPOWER_MODE;
  dev->accel_cfg.odr = BMI160_ACCEL_ODR_0_78HZ;
  dev->accel_cfg.range = BMI160_ACCEL_RANGE_2G;
  dev->accel_cfg.bw = BMI160_ACCEL_BW_NORMAL_AVG4;
  
  dev->gyro_cfg.power = BMI160_GYRO_SUSPEND_MODE;
  dev->gyro_cfg.odr = BMI160_GYRO_ODR_25HZ;
  dev->gyro_cfg.range = BMI160_GYRO_RANGE_2000_DPS;
  dev->gyro_cfg.bw = BMI160_GYRO_BW_NORMAL_MODE;
  return bmi160_update_sensor_config(dev);
  //return 0;
}

int8_t bmi160_cmd_pwup(struct bmi160_dev *dev)
{
  dev->accel_cfg.power = BMI160_ACCEL_NORMAL_MODE;
  dev->accel_cfg.odr = moduleParam.imu_rate_code;
  //dev->accel_cfg.odr = 0xa;
  dev->accel_cfg.range = moduleParam.imu_acc_range;
//  dev->accel_cfg.bw = BMI160_ACCEL_BW_NORMAL_AVG4;
  dev->accel_cfg.bw = BMI160_ACCEL_BW_OSR4_AVG1;
//  
  dev->gyro_cfg.power = BMI160_GYRO_NORMAL_MODE;
  dev->gyro_cfg.odr = moduleParam.imu_rate_code;
  //dev->gyro_cfg.odr = 0xa;
  dev->gyro_cfg.range = moduleParam.imu_gyro_range;
  dev->gyro_cfg.bw = BMI160_GYRO_BW_NORMAL_MODE;
  return bmi160_update_sensor_config(dev);
}
int8_t bmi160_cmd_init(bmi_callback f, uint8_t *buf)
{
  //NRF_LOG_INFO("%s buffer=%d",__func__,buf);
  int8_t rslt = BMI160_OK;
  
    // spim1
  nrf_drv_spi_config_t spibmi160_config = NRF_DRV_SPI_DEFAULT_CONFIG;
  spibmi160_config.ss_pin = BMI160_CS_PIN;
  spibmi160_config.miso_pin = BMI160_MISO_PIN;
  spibmi160_config.mosi_pin = BMI160_MOSI_PIN;
  spibmi160_config.sck_pin = BMI160_SCK_PIN;
  spibmi160_config.frequency = NRF_DRV_SPI_FREQ_8M;
  spibmi160_config.mode = NRF_DRV_SPI_MODE_3;
  APP_ERROR_CHECK(nrf_drv_spi_init(&bmi_spi, &spibmi160_config, NULL, NULL));
  
  nrf_gpio_cfg_output(BMI160_CS_PIN);
  nrf_gpio_pin_set(BMI160_CS_PIN);

  // gpio for bmi
  //nrf_gpio_cfg_input(BMI160_INT1_PIN,NRF_GPIO_PIN_PULLUP);
  //nrf_gpio_cfg_input(BMI160_INT2_PIN,NRF_GPIO_PIN_PULLUP);
  //nrf_gpio_cfg_sense_input(BMI160_INT1_PIN, NRF_GPIO_PIN_PULLUP, GPIO_PIN_CNF_SENSE_Low);
  //nrf_gpio_cfg_sense_input(BMI160_INT2_PIN, NRF_GPIO_PIN_PULLUP, GPIO_PIN_CNF_SENSE_Low);

  nrf_drv_gpiote_in_config_t in_cfg_bmi160 = GPIOTE_CONFIG_IN_SENSE_HITOLO(false);
  in_cfg_bmi160.pull = NRF_GPIO_PIN_PULLUP;
  APP_ERROR_CHECK(nrf_drv_gpiote_in_init(BMI160_INT1_PIN,&in_cfg_bmi160,bmi160_int_isr));
  //in_cfg_bmi160.hi_accuracy = false;
  //APP_ERROR_CHECK(nrf_drv_gpiote_in_init(BMI160_INT2_PIN,&in_cfg_bmi160,bmi160_int_isr2));
  
  
  struct bmi160_foc_conf foc_config;
  foc_config.acc_off_en = BMI160_ENABLE;
  foc_config.gyro_off_en = BMI160_ENABLE;
  bmi160.interface = BMI160_SPI_INTF;
  bmi160.read = bmi160_read;
  bmi160.write = bmi160_write;
  bmi160.delay_ms = bmi160_delay;
  
  //nrf_delay_ms(50);
  
  rslt = bmi160_init(dev);
  if(rslt != BMI160_OK){
    NRF_LOG_INFO("BMI160 Init Fail %x",rslt);
    return rslt;
  }
  
  // previous process will set accel & gyro into suspend mode
  // reconfig power mode
  dev->accel_cfg.power = BMI160_ACCEL_LOWPOWER_MODE;
  dev->accel_cfg.odr = BMI160_ACCEL_ODR_0_78HZ;
  dev->accel_cfg.range = BMI160_ACCEL_RANGE_2G;
  dev->accel_cfg.bw = BMI160_ACCEL_BW_NORMAL_AVG4;
  dev->gyro_cfg.power = BMI160_GYRO_NORMAL_MODE;
  dev->gyro_cfg.odr = BMI160_GYRO_ODR_25HZ;
  dev->gyro_cfg.range = BMI160_GYRO_RANGE_2000_DPS;
  dev->gyro_cfg.bw = BMI160_GYRO_BW_NORMAL_MODE;
  rslt = bmi160_update_sensor_config(dev);
  //rslt = bmi160_set_offsets(&foc_config,&moduleParam.bmi160.offsets,dev);
  //bmi160_cmd_config_int(cfg);
  bmi160_cmd_testRead(dev);

  f_cb = f;
  f_buf = buf;
  
  //NRF_LOG_INFO("%s buffer=%d",__func__,f_buf);
  
  nrf_drv_gpiote_in_event_enable(BMI160_INT1_PIN,true);  
  return rslt;
}


int8_t bmi160_read(uint8_t dev_adr, uint8_t reg_adr, uint8_t *b, uint16_t n)
{
  ret_code_t err_code;
  uint8_t rx[16];
  //nrf_gpio_pin_clear(BMI160_CS_PIN);
  err_code = nrf_drv_spi_transfer(&bmi_spi, &reg_adr, 
                    1, rx, n+1);
  memcpy(b,&rx[1],n);
  //nrf_gpio_pin_set(BMI160_CS_PIN);
  return BMI160_OK;
}

int8_t bmi160_write(uint8_t dev_adr, uint8_t reg_adr, uint8_t *b, uint16_t n)
{
  ret_code_t err_code;
  uint8_t tx[8];
  //nrf_gpio_pin_clear(BMI160_CS_PIN);
  tx[0] = reg_adr;
  memcpy(&tx[1],b,n);
  err_code = nrf_drv_spi_transfer(&bmi_spi, tx, 
                    n+1, NULL, 0);
  //nrf_gpio_pin_set(BMI160_CS_PIN);
  return BMI160_OK;
}

void bmi160_delay(uint32_t period)
{
    nrf_delay_us(period*500);
//    nrf_delay_ms(period);
}

void bmi160_int_isr(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
  union bmi160_int_status interrupt;
  int8_t rslt;
  rslt = bmi160_get_int_status(BMI160_INT_STATUS_ALL,&interrupt,&bmi160);
  if(interrupt.bit.anym){
    NRF_LOG_INFO("Any Motion interrupt");
  }
  
  if(f_buf){
    //NRF_LOG_INFO("%s with Buffer=%d",__func__,f_buf);
    bmi160_get_regs(BMI160_GYRO_DATA_ADDR, f_buf, 12, &bmi160);
//    bool az = true;
//    for(uint8_t i=0;i<12;i++)
//      if(f_buf[i] != 0x0) az = false;
//    if(az) NRF_LOG_INFO("%s, BUFFER ZERO",__func__);
    if(f_cb) f_cb();
  }else{
    NRF_LOG_INFO("%s No Buffer=%d",__func__,f_buf);
  }

}
void bmi160_int_isr2(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
  bmi160_int_isr(pin,action);
}

void bmi160_cmd_start(bool intEn)
{
  bmi160_cmd_pwup(&bmi160);
  if(intEn){
    bmi160_cmd_config_int_am(&bmi160,0);
    bmi160_cmd_config_int(&bmi160,1);
    nrf_drv_gpiote_in_event_enable(BMI160_INT1_PIN,true);
  }
//  nrf_drv_gpiote_in_event_enable(BMI160_INT2_PIN,true);  
}

int8_t bmi160_cms_startConv(void)
{
  bmi160_cmd_config_int_am(&bmi160,0);
  bmi160_cmd_config_int(&bmi160,1);
  //nrf_drv_gpiote_in_event_enable(BMI160_INT1_PIN,true);
  
  bmi160.accel_cfg.power = BMI160_ACCEL_NORMAL_MODE;
  bmi160.accel_cfg.odr = moduleParam.imu_rate_code;
  bmi160.accel_cfg.range = moduleParam.imu_acc_range;
  bmi160.accel_cfg.bw = BMI160_ACCEL_BW_OSR4_AVG1;
  bmi160.gyro_cfg.power = BMI160_GYRO_NORMAL_MODE;
  bmi160.gyro_cfg.odr = moduleParam.imu_rate_code;
  bmi160.gyro_cfg.range = moduleParam.imu_gyro_range;
  bmi160.gyro_cfg.bw = BMI160_GYRO_BW_NORMAL_MODE;
  
  bmi160_update_sensor_config(&bmi160);
  
  //bmi160_cmd_pwup(&bmi160);
  
  NRF_LOG_INFO("BMI160:odr=%d",bmi160.accel_cfg.odr);


  return 0;
}

int8_t bmi160_cmd_start_anymotion(void)
{
  bmi160_cmd_config_int(&bmi160,0); 
  bmi160_cmd_config_int_am(&bmi160,1);
  bmi160.accel_cfg.power = BMI160_ACCEL_LOWPOWER_MODE;
//  bmi160.accel_cfg.power = BMI160_ACCEL_NORMAL_MODE;
  bmi160.accel_cfg.odr = BMI160_ACCEL_ODR_0_78HZ;
  bmi160.accel_cfg.range = BMI160_ACCEL_RANGE_2G;
  bmi160.accel_cfg.bw = BMI160_ACCEL_BW_OSR4_AVG1;
  bmi160.gyro_cfg.power = BMI160_GYRO_SUSPEND_MODE;
//  bmi160.gyro_cfg.power = BMI160_GYRO_NORMAL_MODE;
  bmi160.gyro_cfg.odr = BMI160_GYRO_ODR_25HZ;
  bmi160.gyro_cfg.range = BMI160_GYRO_RANGE_2000_DPS;
  bmi160.gyro_cfg.bw = BMI160_GYRO_BW_NORMAL_MODE;
  
  bmi160_update_sensor_config(&bmi160);
//  bmi160_cmd_pwup(&bmi160);
  //bmi160_cmd_pwdn(&bmi160);
  //nrf_drv_gpiote_in_event_enable(BMI160_INT1_PIN,true);
}

void bmi160_cmd_stop(void)
{
//  nrf_drv_gpiote_in_event_enable(BMI160_INT1_PIN,false);
//  nrf_drv_gpiote_in_event_disable(BMI160_INT1_PIN);
  bmi160_cmd_config_int(&bmi160,0); 
  bmi160_cmd_config_int_am(&bmi160,0);
  bmi160_cmd_pwdn(&bmi160);
}

void bmi160_cmd_set_odr(uint8_t odr)
{
  bmi160.accel_cfg.odr = odr;
  bmi160.gyro_cfg.odr = odr;
}

int8_t bmi160_cmd_singleshot(uint8_t *ptr)
{
  bmi160_get_regs(BMI160_GYRO_DATA_ADDR, ptr, 12, &bmi160);
  return 12;
}

