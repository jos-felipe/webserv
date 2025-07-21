/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: josfelip <josfelip@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/26 12:29:42 by josfelip          #+#    #+#             */
/*   Updated: 2025/03/26 12:42:52 by josfelip         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <cstdlib>
#include <csignal>
#include "../include/Server.hpp"
#include "../include/Config.hpp"

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
int	main(int argc, char **argv)
{
	std::string configPath = "./conf/default.conf";

	// Check if a configuration file was provided
	if (argc > 2)
	{
		displayUsage(argv[0]);
		return (EXIT_FAILURE);
	}
	else if (argc == 2)
		configPath = argv[1];

	try
	{
		// Parse configuration file
		Config config(configPath);
		
		// Setup signal handlers
		setupSignals();
		
		// Create and start the server
		Server server(config);
		server.start();
		
		// Main server loop
		while (g_running)
			server.run();

		return (EXIT_SUCCESS);
	}
	catch (const std::exception &e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		return (EXIT_FAILURE);
	}
}