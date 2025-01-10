struct SensorData {
    std::string device_id;
    time_t timestamp;
    double temperature;
    double humidity;
    double co2;
    double pm25;
    double noise;
    double light;
    std::string area;
    AreaType area_type;
    
    // 聚合数据相关字段
    bool has_aggregated_data = false;  // 标记是否为聚合数据
    bool has_min_max = false;          // 标记是否有最大最小值
    bool is_hourly = false;            // 标记是否为小时聚合数据
    
    // 最大最小值（仅温度）
    double max_temperature = 0;
    double min_temperature = 0;
    
    // 样本数量
    int samples_count = 0;
}; 