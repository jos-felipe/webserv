/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   FileHandler.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: josfelip <josfelip@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/01 14:00:00 by josfelip          #+#    #+#             */
/*   Updated: 2025/04/01 14:15:39 by josfelip         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef FILE_HANDLER_HPP
# define FILE_HANDLER_HPP

# include <string>
# include <map>
# include "Config.hpp"

/**
 * @class FileHandler
 * @brief Handles static file operations for the HTTP server
 * 
 * This class is responsible for mapping request URIs to filesystem paths,
 * reading file content, and determining MIME types.
 */
class FileHandler
{
private:
	std::map<std::string, std::string> _mimeTypes;
	
	/**
	 * Initialize text MIME types
	 */
	void				initializeTextMimeTypes(void);
	
	/**
	 * Initialize image MIME types
	 */
	void				initializeImageMimeTypes(void);
	
	/**
	 * Initialize application MIME types
	 */
	void				initializeApplicationMimeTypes(void);
	
	/**
	 * Initialize media MIME types
	 */
	void				initializeMediaMimeTypes(void);
	
	/**
	 * Initialize all MIME types
	 */
	void				initializeMimeTypes(void);
	
public:
	/**
	 * Default constructor
	 */
	FileHandler(void);
	
	/**
	 * Copy constructor
	 */
	FileHandler(const FileHandler& other);
	
	/**
	 * Destructor
	 */
	~FileHandler(void);
	
	/**
	 * Assignment operator
	 */
	FileHandler&		operator=(const FileHandler& other);
	
	/**
	 * Map a request path to a filesystem path
	 * 
	 * @param requestPath The path from the HTTP request
	 * @param location The location configuration
	 * @return The corresponding filesystem path
	 */
	std::string			mapPath(const std::string& requestPath, 
							const LocationConfig& location) const;
	
	/**
	 * Check if a file exists and is readable
	 * 
	 * @param filePath The path to the file
	 * @return True if the file exists and is readable, false otherwise
	 */
	bool				fileExists(const std::string& filePath) const;
	
	/**
	 * Read the content of a file
	 * 
	 * @param filePath The path to the file
	 * @return The content of the file
	 * @throw std::runtime_error if the file cannot be read
	 */
	std::string			readFile(const std::string& filePath) const;
	
	/**
	 * Get the MIME type for a file based on its extension
	 * 
	 * @param filePath The path to the file
	 * @return The MIME type of the file
	 */
	std::string			getMimeType(const std::string& filePath) const;
	
	/**
	 * Check if a path is a directory
	 * 
	 * @param path The path to check
	 * @return True if the path is a directory, false otherwise
	 */
	bool				isDirectory(const std::string& path) const;
};

#endif