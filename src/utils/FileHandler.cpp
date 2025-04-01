/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   FileHandler.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: josfelip <josfelip@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/01 14:10:00 by josfelip          #+#    #+#             */
/*   Updated: 2025/04/01 14:23:26 by josfelip         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "FileHandler.hpp"

/**
 * Default constructor initializes MIME types
 */
FileHandler::FileHandler(void)
{
	initializeMimeTypes();
}

/**
 * Copy constructor
 */
FileHandler::FileHandler(const FileHandler& other) : _mimeTypes(other._mimeTypes)
{
}

/**
 * Destructor
 */
FileHandler::~FileHandler(void)
{
}

/**
 * Assignment operator
 */
FileHandler&	FileHandler::operator=(const FileHandler& other)
{
	if (this != &other)
	{
		_mimeTypes = other._mimeTypes;
	}
	return *this;
}

/**
 * Initialize all MIME types by calling specialized initializers
 */
void	FileHandler::initializeMimeTypes(void)
{
	initializeTextMimeTypes();
	initializeImageMimeTypes();
	initializeApplicationMimeTypes();
	initializeMediaMimeTypes();
}