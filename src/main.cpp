/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: asanni <asanni@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/26 12:29:42 by josfelip          #+#    #+#             */
/*   Updated: 2025/07/26 17:47:20 by asanni           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <cstdlib>
#include <csignal>
#include "Logger.hpp"
#include "Config.hpp"
#include "Server.hpp"

bool	g_running = true;

/**
 * Signal handler to gracefully shutdown the server
 */
void	signalHandler(int signum)
{
	(void)signum;
	g_running = false;
	std::cout << "\nShutting down server..." << std::endl;
}

/**
 * Setup signal handlers for graceful shutdown
 */
void	setupSignals(void)
{
	signal(SIGINT, signalHandler);
	signal(SIGTERM, signalHandler);
}

/**
 * Display usage information
 */
void	displayUsage(const char *programName)
{
	std::cerr << "Usage: " << programName << " [configuration file]" << std::endl;
	std::cerr << "Default configuration: ./conf/default.conf" << std::endl;
}

/**
 * Main function - entry point for the webserver
 */
#include "Logger.hpp"
#include "Config.hpp"
#include "Server.hpp"

int main(int argc, char **argv)
{
	std::string configPath = "./conf/default.conf";

	if (argc > 2) {
		displayUsage(argv[0]);
		return EXIT_FAILURE;
	} else if (argc == 2)
		configPath = argv[1];

	try {
		Logger logger;                     // ✅ Logger local
		logger.setLevel("info");        // ou LOG_DEBUG, dependendo do que quer ver

		Config config(configPath);
		setupSignals();

		Server server(config);    // ✅ Injetando logger local

		logger.log(LOG_INFO, "Starting server...");
		server.start();

		while (g_running)
			server.run();

		server.stop();
		logger.log(LOG_INFO, "Server stopped.");

		return EXIT_SUCCESS;
	}
	catch (const std::exception &e) {
		Logger logger;                    // ✅ Logger local em caso de erro
		logger.error(std::string("Error: ") + e.what());
		return EXIT_FAILURE;
	}
}
