/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   FileHandler_io.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: josfelip <josfelip@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/01 14:25:00 by josfelip          #+#    #+#             */
/*   Updated: 2025/04/01 15:51:27 by josfelip         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "FileHandler.hpp"
#include <fstream>
#include <sstream>

/**
 * Read the content of a file
 */
std::string FileHandler::readFile(const std::string& filePath) const
{
	std::ifstream file(filePath.c_str(), std::ios::binary);
	
	if (!file.is_open())
		throw std::runtime_error("Failed to open file: " + filePath);
	
	std::ostringstream content;
	content << file.rdbuf();
	
	if (file.fail())
	{
		file.close();
		throw std::runtime_error("Failed to read file: " + filePath);
	}
	
	file.close();
	return content.str();
}

/**
 * Get the MIME type for a file based on its extension
 */
std::string FileHandler::getMimeType(const std::string& filePath) const
{
	// Find the last dot in the filename
	size_t dotPos = filePath.find_last_of('.');
	
	if (dotPos != std::string::npos)
	{
		std::string extension = filePath.substr(dotPos);
		
		// Check if we have a MIME type for this extension
		std::map<std::string, std::string>::const_iterator it = 
			_mimeTypes.find(extension);
		
		if (it != _mimeTypes.end())
			return it->second;
	}
	
	// Default MIME type if the extension is not recognized
	return "application/octet-stream";
}