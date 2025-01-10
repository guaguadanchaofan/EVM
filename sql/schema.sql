-- 实时数据表（保存最近24小时的原始数据）
CREATE TABLE IF NOT EXISTS sensor_data_realtime (
    id BIGINT AUTO_INCREMENT PRIMARY KEY,
    device_id VARCHAR(50) NOT NULL,
    timestamp TIMESTAMP NOT NULL,
    temperature FLOAT NOT NULL,
    humidity FLOAT NOT NULL,
    co2 FLOAT NOT NULL,
    pm25 FLOAT NOT NULL,
    noise FLOAT NOT NULL,
    light FLOAT NOT NULL,
    area VARCHAR(50) NOT NULL,
    area_type TINYINT NOT NULL,
    INDEX idx_device_time (device_id, timestamp)
);

-- 小时聚合表（保存最近30天的小时平均值）
CREATE TABLE IF NOT EXISTS sensor_data_hourly (
    id BIGINT AUTO_INCREMENT PRIMARY KEY,
    device_id VARCHAR(50) NOT NULL,
    hour_timestamp TIMESTAMP NOT NULL,
    avg_temperature FLOAT NOT NULL,
    avg_humidity FLOAT NOT NULL,
    avg_co2 FLOAT NOT NULL,
    avg_pm25 FLOAT NOT NULL,
    avg_noise FLOAT NOT NULL,
    avg_light FLOAT NOT NULL,
    max_temperature FLOAT NOT NULL,
    min_temperature FLOAT NOT NULL,
    samples_count INT NOT NULL,
    area VARCHAR(50) NOT NULL,
    area_type TINYINT NOT NULL,
    INDEX idx_device_hour (device_id, hour_timestamp)
);

-- 天聚合表（保存历史数据的天平均值）
CREATE TABLE IF NOT EXISTS sensor_data_daily (
    id BIGINT AUTO_INCREMENT PRIMARY KEY,
    device_id VARCHAR(50) NOT NULL,
    date_timestamp DATE NOT NULL,
    avg_temperature FLOAT NOT NULL,
    avg_humidity FLOAT NOT NULL,
    avg_co2 FLOAT NOT NULL,
    avg_pm25 FLOAT NOT NULL,
    avg_noise FLOAT NOT NULL,
    avg_light FLOAT NOT NULL,
    max_temperature FLOAT NOT NULL,
    min_temperature FLOAT NOT NULL,
    samples_count INT NOT NULL,
    area VARCHAR(50) NOT NULL,
    area_type TINYINT NOT NULL,
    INDEX idx_device_date (device_id, date_timestamp)
); 