/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest_files.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: josfelip <josfelip@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/02 09:00:00 by josfelip          #+#    #+#             */
/*   Updated: 2025/04/01 17:11:18 by josfelip         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpRequest.hpp"
#include "FileHandler.hpp"
#include <dirent.h>
#include <sstream>

/**
 * Check if the file is an index file
 */
bool HttpRequest::isIndex(const std::string& path, 
                        const LocationConfig& location,
                        FileHandler& fileHandler)
{
    if (location.index.empty())
        return false;
    
    std::string indexPath = path;
    
    // Ensure path ends with a slash
    if (!indexPath.empty() && indexPath[indexPath.length() - 1] != '/')
        indexPath += '/';
    
    indexPath += location.index;
    
    return fileHandler.fileExists(indexPath);
}

/**
 * Handle GET request for static files
 */
HttpResponse HttpRequest::handleStaticFile(const LocationConfig& location,
                                    const ServerConfig& server,
                                    const Config& config)
{
    (void)server;
	HttpResponse response;
    FileHandler fileHandler;
    
    // Map the request path to a filesystem path
    std::string filePath = fileHandler.mapPath(_path, location);
    
    // Check if the path is a directory
    if (fileHandler.isDirectory(filePath))
    {
        // Check if an index file exists
        if (isIndex(filePath, location, fileHandler))
        {
            // Append the index file to the path
            if (filePath[filePath.length() - 1] != '/')
                filePath += '/';
            filePath += location.index;
        }
        else if (location.autoindex)
        {
            // Generate directory listing
            return handleDirectoryListing(filePath, _path, location);
        }
        else
        {
            // Directory listing disabled and no index file
            response.setStatus(403);
            response.setBody(config.getDefaultErrorPage(403));
            return response;
        }
    }
    
    // Check if the file exists and is readable
    if (!fileHandler.fileExists(filePath))
    {
        response.setStatus(404);
        response.setBody(config.getDefaultErrorPage(404));
        return response;
    }
    
    try
    {
        // Read file content
        std::string content = fileHandler.readFile(filePath);
        
        // Set response
        response.setStatus(200);
        response.setBody(content);
        response.addHeader("Content-Type", fileHandler.getMimeType(filePath));
    }
    catch (const std::exception& e)
    {
        // Handle file reading errors
        response.setStatus(500);
        response.setBody(config.getDefaultErrorPage(500));
    }
    
    return response;
}