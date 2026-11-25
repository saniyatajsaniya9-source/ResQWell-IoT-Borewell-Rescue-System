package com.aqimonitor.model;

public class SensorReading {

    private final double temperature;
    private final double humidity;
    private final int aqi;

    public SensorReading(double temperature, double humidity, int aqi) {
        this.temperature = temperature;
        this.humidity = humidity;
        this.aqi = aqi;
    }

    public double getTemperature() {
        return temperature;
    }

    public double getHumidity() {
        return humidity;
    }

    public int getAqi() {
        return aqi;
    }
}
