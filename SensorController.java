package com.aqimonitor.controller;

import org.springframework.beans.factory.annotation.Value;
import org.springframework.http.ResponseEntity;
import org.springframework.stereotype.Controller;
import org.springframework.ui.Model;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestParam;

import com.aqimonitor.model.SensorReading;
import com.aqimonitor.service.AirQualityService;

import jakarta.servlet.http.HttpSession;

@Controller
@RequestMapping("/AQI/data")
public class SensorController {

    private static final String SESSION_USER_KEY = "loggedInUser";

    private final AirQualityService airQualityService;

    @Value("${aqi.security.username}")
    private String configuredUsername;

    @Value("${aqi.security.password1}")
    private String configuredPassword;

    public SensorController(AirQualityService airQualityService) {
        this.airQualityService = airQualityService;
    }

    @GetMapping("")
    public String home() {
        return "redirect:/AQI/data/view";
    }

    // ---------------------------------
    // LOGIN
    // ---------------------------------

    @GetMapping("/login")
    public String loginPage(Model model) {
        populateReadoutAttributes(model);
        return "login";
    }

    @PostMapping("/login")
    public String doLogin(
            @RequestParam String username,
            @RequestParam String password,
            HttpSession session,
            Model model) {

        if (configuredUsername.equals(username) && configuredPassword.equals(password)) {
            session.setAttribute(SESSION_USER_KEY, username);
            return "redirect:/AQI/data/view";
        }

        model.addAttribute("error", "Incorrect username or password. Try again.");
        populateReadoutAttributes(model);
        return "login";
    }

    @GetMapping("/logout")
    public String logout(HttpSession session) {
        session.invalidate();
        return "redirect:/AQI/data/login";
    }

    // ---------------------------------
    // RECEIVE DATA FROM SENSOR
    // No credentials here on purpose: the sensor only ever reports
    // readings, it never reads anything back, so there is nothing on this
    // endpoint for a login to protect. Login guards /view instead, where a
    // human actually views/uses the data.
    // ---------------------------------

    @GetMapping("/send")
    public ResponseEntity<String> sendData(
            @RequestParam double temperature,
            @RequestParam double humidity,
            @RequestParam int aqi) {

        airQualityService.updateReading(temperature, humidity, aqi);
        return ResponseEntity.ok("Reading recorded");
    }

    // ---------------------------------
    // VIEW PAGE (requires an authenticated session)
    // ---------------------------------

    @GetMapping("/view")
    public String viewData(HttpSession session, Model model) {

        if (session.getAttribute(SESSION_USER_KEY) == null) {
            return "redirect:/AQI/data/login";
        }

        if (!airQualityService.hasReading()) {
            model.addAttribute("temperature", "-");
            model.addAttribute("humidity", "-");
            model.addAttribute("aqi", "-");
            model.addAttribute("aqiCategory", "Waiting for sensor data...");
            model.addAttribute("aqiCssClass", "aqi-invalid");
            model.addAttribute("temperatureStatus", "-");
            model.addAttribute("humidityStatus", "-");
            model.addAttribute("advice", "Please send sensor data to view a live assessment.");
            model.addAttribute("sensorDataReceived", false);
            return "dashboard";
        }

        SensorReading reading = airQualityService.getLatestReading();
        double temperature = reading.getTemperature();
        double humidity = reading.getHumidity();
        int aqi = reading.getAqi();

        model.addAttribute("temperature", temperature);
        model.addAttribute("humidity", humidity);
        model.addAttribute("aqi", aqi);
        model.addAttribute("aqiCategory", airQualityService.classifyAqi(aqi));
        model.addAttribute("aqiCssClass", airQualityService.aqiCssClass(aqi));
        model.addAttribute("temperatureStatus", airQualityService.classifyTemperature(temperature));
        model.addAttribute("humidityStatus", airQualityService.classifyHumidity(humidity));
        model.addAttribute("advice", airQualityService.buildAdvice(temperature, humidity, aqi));
        model.addAttribute("sensorDataReceived", true);

        return "dashboard";
    }

    // ---------------------------------
    // Small live readout shown on the login screen itself, so a responder
    // can see basic shaft status before signing in.
    // ---------------------------------

    private void populateReadoutAttributes(Model model) {
        if (!airQualityService.hasReading()) {
            model.addAttribute("temperature", "--");
            model.addAttribute("humidity", "--");
            model.addAttribute("aqiCategory", "no data");
            return;
        }
        SensorReading reading = airQualityService.getLatestReading();
        model.addAttribute("temperature", reading.getTemperature() + "°C");
        model.addAttribute("humidity", reading.getHumidity() + "%");
        model.addAttribute("aqiCategory", airQualityService.classifyAqi(reading.getAqi()));
    }
}
