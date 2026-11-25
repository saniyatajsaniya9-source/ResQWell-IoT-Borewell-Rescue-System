package com.aqimonitor.service;

import com.aqimonitor.model.SensorReading;

public interface AirQualityService {

    void updateReading(double temperature, double humidity, int aqi);

    boolean hasReading();

    SensorReading getLatestReading();

    String classifyAqi(int aqi);

    String aqiCssClass(int aqi);

    String classifyTemperature(double temperature);

    String classifyHumidity(double humidity);

    String buildAdvice(double temperature, double humidity, int aqi);
}
