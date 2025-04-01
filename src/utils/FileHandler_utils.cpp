/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   FileHandler_utils.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: josfelip <josfelip@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/01 14:20:00 by josfelip          #+#    #+#             */
/*   Updated: 2025/04/01 14:34:12 by josfelip         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "FileHandler.hpp"
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <unistd.h>

/**
 * Map a request path to a filesystem path
 */
std::string FileHandler::mapPath(const std::string& requestPath, 
							const LocationConfig& location) const
{
	std::string result;
	
	// Get the part of the path that comes after the location path
	std::string relativePath = requestPath;
	
	if (location.path != "/" && requestPath.find(location.path) == 0)
		relativePath = requestPath.substr(location.path.length());
	
	// If the path starts with /, remove it to avoid double slashes
	if (!relativePath.empty() && relativePath[0] == '/')
		relativePath = relativePath.substr(1);
	
	// Combine the root directory with the relative path
	result = location.root;
	
	// Make sure the root ends with a slash
	if (!result.empty() && result[result.length() - 1] != '/')
		result += '/';
	
	result += relativePath;
	
	return result;
}

/**
 * Check if a file exists and is readable
 */
bool	FileHandler::fileExists(const std::string& filePath) const
{
	struct stat fileStat;
	
	// Check if the file exists and is a regular file
	if (stat(filePath.c_str(), &fileStat) != 0)
		return false;
	
	// Check if the file is a regular file (not a directory)
	if (!S_ISREG(fileStat.st_mode))
		return false;
	
	// Check if the file is readable
	if (access(filePath.c_str(), R_OK) != 0)
		return false;
	
	return true;
}

/**
 * Check if a path is a directory
 */
bool	FileHandler::isDirectory(const std::string& path) const
{
	struct stat fileStat;
	
	// Check if the path exists
	if (stat(path.c_str(), &fileStat) != 0)
		return false;
	
	// Check if it's a directory
	return S_ISDIR(fileStat.st_mode);
}