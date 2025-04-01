/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   FileHandler_mime.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: josfelip <josfelip@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/01 14:15:00 by josfelip          #+#    #+#             */
/*   Updated: 2025/04/01 14:30:40 by josfelip         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "FileHandler.hpp"

/**
 * Initialize the map of text MIME types
 */
void	FileHandler::initializeTextMimeTypes(void)
{
	_mimeTypes[".html"] = "text/html";
	_mimeTypes[".htm"] = "text/html";
	_mimeTypes[".css"] = "text/css";
	_mimeTypes[".js"] = "text/javascript";
	_mimeTypes[".txt"] = "text/plain";
	_mimeTypes[".xml"] = "text/xml";
	_mimeTypes[".json"] = "application/json";
	_mimeTypes[".csv"] = "text/csv";
	_mimeTypes[".md"] = "text/markdown";
}

/**
 * Initialize the map of image MIME types
 */
void	FileHandler::initializeImageMimeTypes(void)
{
	_mimeTypes[".gif"] = "image/gif";
	_mimeTypes[".jpg"] = "image/jpeg";
	_mimeTypes[".jpeg"] = "image/jpeg";
	_mimeTypes[".png"] = "image/png";
	_mimeTypes[".bmp"] = "image/bmp";
	_mimeTypes[".ico"] = "image/x-icon";
	_mimeTypes[".svg"] = "image/svg+xml";
	_mimeTypes[".webp"] = "image/webp";
	_mimeTypes[".tiff"] = "image/tiff";
}

/**
 * Initialize the map of application MIME types
 */
void	FileHandler::initializeApplicationMimeTypes(void)
{
	_mimeTypes[".pdf"] = "application/pdf";
	_mimeTypes[".zip"] = "application/zip";
	_mimeTypes[".gz"] = "application/gzip";
	_mimeTypes[".tar"] = "application/x-tar";
	_mimeTypes[".doc"] = "application/msword";
	_mimeTypes[".docx"] = 
		"application/vnd.openxmlformats-officedocument.wordprocessingml.document";
	_mimeTypes[".xls"] = "application/vnd.ms-excel";
	_mimeTypes[".xlsx"] = 
		"application/vnd.openxmlformats-officedocument.spreadsheetml.sheet";
}

/**
 * Initialize the map of audio/video MIME types
 */
void	FileHandler::initializeMediaMimeTypes(void)
{
	_mimeTypes[".mp3"] = "audio/mpeg";
	_mimeTypes[".mp4"] = "video/mp4";
	_mimeTypes[".avi"] = "video/x-msvideo";
	_mimeTypes[".mpeg"] = "video/mpeg";
	_mimeTypes[".webm"] = "video/webm";
	_mimeTypes[".wav"] = "audio/wav";
	_mimeTypes[".ogg"] = "audio/ogg";
	_mimeTypes[".flac"] = "audio/flac";
}