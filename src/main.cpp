/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: asanni <asanni@student.42sp.org.br>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/26 12:29:42 by josfelip          #+#    #+#             */
/*   Updated: 2025/06/30 17:44:43 by asanni           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <cstdlib>
#include <csignal>
#include "../include/Server.hpp"
#include "../include/Config.hpp"
#include "../include/Logger.hpp"

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
int main(int argc, char **argv)
{
    std::string configPath = "./conf/default.conf";

    if (argc > 2)
    {
        displayUsage(argv[0]);
        return (EXIT_FAILURE);
    }
    else if (argc == 2)
        configPath = argv[1];

    Logger logger;
    logger.setLevel(LOG_DEBUG);

    try
    {
        Config config(configPath);

        setupSignals();

        // Cria o servidor
        Server server(config);

        // Se você alterou o Server para receber Logger, passe ele aqui
        // Por exemplo, server.setLogger(&logger); se tiver método assim

        logger.log(LOG_INFO, "Starting server...");
        server.start();

        while (g_running)
            server.run();

        server.stop();
        logger.log(LOG_INFO, "Server stopped.");

        return (EXIT_SUCCESS);
    }
    catch (const std::exception &e)
    {
        logger.log(LOG_ERROR, std::string("Error: ") + e.what());
        return (EXIT_FAILURE);
    }
}
