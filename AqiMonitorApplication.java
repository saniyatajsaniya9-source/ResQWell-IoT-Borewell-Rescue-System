package com.aqimonitor;

import jakarta.servlet.SessionTrackingMode;
import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;
import org.springframework.boot.web.servlet.ServletContextInitializer;
import org.springframework.context.annotation.Bean;

import java.util.Collections;

@SpringBootApplication
public class AqiMonitorApplication {

	public static void main(String[] args) {
		SpringApplication.run(AqiMonitorApplication.class, args);
	}

	@Bean
	public ServletContextInitializer servletContextInitializer() {
		return servletContext -> servletContext.setSessionTrackingModes(
				Collections.singleton(SessionTrackingMode.COOKIE));
	}
}
