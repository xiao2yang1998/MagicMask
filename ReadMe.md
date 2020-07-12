## H5

代码在各版本下的layoutit/src文件夹中

现在上传的版本为v2，对应物联网实验平台的模型{id: 4028c6526f0e53b80170edd30db10003 , name：mask}

### 数据说明

index.html中

```html
o2 = res.SaO2;//血氧
bre = res.BPM;//心率
co2 = res.CO2;//二氧化碳
acl = res.ALC;//酒精
tem = res.TEMP;//体温
bp = res.PRE; //呼吸频率
```

**注：bre与bp需特别注意，中英不对应**

## Hardware

### I2C串口说明

| I2C模块 | Sda  | Scl  |
| :-----: | ---- | ---- |
|   B0    | 1.6  | 1.7  |
|   B1    | 6.4  | 6.5  |
|   B2    | 3.6  | 3.7  |
|   B3    | 6.6  | 6.7  |

### 传感器连接说明

| 传感器   | I2C模块 | 连接位置  | sda  | scl  |
| -------- | ------- | --------- | ---- | ---- |
| SGP30    | B0      | IOT板     | 1.6  | 1.7  |
| BME280   | B1      | IOT板     | 6.4  | 6.5  |
| MAX30100 | B1      | MSP板背面 | 6.4  | 6.5  |

**注：电源切记接正确**

### 采样频率

| 传感器   | 采样频率 |
| -------- | -------- |
| SGP30    | 1HZ      |
| BME280   | 10HZ     |
| MAX30100 | 100HZ    |

### 版本说明

**注：main.v8及以后较重要**

| Version         | 说明                                       |
| --------------- | ------------------------------------------ |
| main.V0         | Bme  ,no iot,:good                         |
| main.V1         | Enable bme sgp                             |
| I2c_driver.v0.c | Sgp30 b0 ; Max30100  b1 ; bme280 b2        |
| I2c_driver.v1.c | bme280  b1  ：**当前使用**                 |
| main.v2         | V0 + Eable send ;  i2c_driver.v1           |
| main.v3         | Max30100  + I2c_driver.v0                  |
| main.v4         | Only  iot                                  |
| main.v5         | Bme,max30100:b1  ; no send                 |
| main.v6         | Bme,max30100:b1  ; with send               |
| main.v7         | Only  sgp30                                |
| main.v8         | 三个传感器 (bme280测得气压，非呼吸频率)    |
| main.v9         | 发送model id                               |
| main.v10        | 更新bme280代码，可测呼吸频率(没验证准确性) |
| main.v11        | 三个传感器 + 更新后bme280                  |
| main.v12        | 可测试app功能：配网+传输数据               |