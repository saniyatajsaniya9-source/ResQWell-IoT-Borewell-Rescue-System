package com.aqimonitor.service;

import org.springframework.stereotype.Service;

import com.aqimonitor.model.SensorReading;

@Service
public class AirQualityServiceImpl implements AirQualityService {

    private volatile SensorReading latestReading;

    @Override
    public void updateReading(double temperature, double humidity, int aqi) {
        this.latestReading = new SensorReading(temperature, humidity, aqi);
    }

    @Override
    public boolean hasReading() {
        return latestReading != null;
    }

    @Override
    public SensorReading getLatestReading() {
        return latestReading;
    }

    // ---------------------------------
    // AQI CLASSIFICATION (US EPA breakpoints)
    // ---------------------------------

    @Override
    public String classifyAqi(int aqi) {
        if (aqi < 0 || aqi > 500) {
            return "Invalid AQI reading";
        }
        if (aqi <= 50) return "Good";
        if (aqi <= 100) return "Moderate";
        if (aqi <= 150) return "Unhealthy for Sensitive Groups";
        if (aqi <= 200) return "Unhealthy";
        if (aqi <= 300) return "Very Unhealthy";
        return "Hazardous";
    }

    @Override
    public String aqiCssClass(int aqi) {
        if (aqi < 0 || aqi > 500) return "aqi-invalid";
        if (aqi <= 50) return "aqi-good";
        if (aqi <= 100) return "aqi-moderate";
        if (aqi <= 150) return "aqi-usg";
        if (aqi <= 200) return "aqi-unhealthy";
        if (aqi <= 300) return "aqi-very-unhealthy";
        return "aqi-hazardous";
    }

    // ---------------------------------
    // TEMPERATURE / HUMIDITY CLASSIFICATION
    // ---------------------------------

    @Override
    public String classifyTemperature(double temperature) {
        if (temperature < 0) return "Freezing";
        if (temperature < 15) return "Cold";
        if (temperature < 24) return "Comfortable";
        if (temperature < 30) return "Warm";
        if (temperature < 38) return "Hot";
        return "Extreme Heat";
    }

    @Override
    public String classifyHumidity(double humidity) {
        if (humidity < 30) return "Low / Dry";
        if (humidity <= 60) return "Comfortable";
        if (humidity <= 75) return "Humid";
        return "Very Humid";
    }

    // ---------------------------------
    // RESCUE ADVISORY (most severe condition wins)
    // A trapped child cannot self-regulate exposure, so any single reading
    // outside a safe band is treated as urgent rather than a lifestyle tip.
    // ---------------------------------

    @Override
    public String buildAdvice(double temperature, double humidity, int aqi) {

        if (aqi > 300) {
            return "CRITICAL: Oxygen/gas levels inside the shaft are hazardous. Trigger emergency oxygen supply immediately and alert the rescue team.";
        }
        if (aqi > 200) {
            return "Air quality inside the shaft is very unhealthy. Activate ventilation/oxygen support and expedite rescue operations.";
        }
        if (aqi > 150) {
            return "Air quality is unhealthy for a child in a confined shaft. Increase ventilation and prepare for rapid extraction.";
        }
        if (aqi > 100) {
            return "Air quality may be harmful to a vulnerable child. Monitor continuously and stage ventilation support as a precaution.";
        }

        if (temperature >= 38) {
            return "High shaft temperature detected. Risk of heat stress to the child - expedite rescue and prepare cooling support.";
        }
        if (temperature < 10) {
            return "Low shaft temperature detected. Risk of hypothermia to the child - expedite rescue and prepare warming support.";
        }

        if (humidity > 75) {
            return "High humidity detected inside the shaft. Ensure ventilation is active to maintain breathable air quality.";
        }
        if (humidity < 30) {
            return "Low humidity detected inside the shaft. Continue monitoring for dust or dry-air irritation risk.";
        }

        if (aqi > 50) {
            return "Air quality is acceptable but should stay under continuous monitoring during the rescue operation.";
        }

        return "Conditions inside the shaft are currently stable and within safe limits.";
    }
}

