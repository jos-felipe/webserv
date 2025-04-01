/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest_dirs.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: josfelip <josfelip@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/02 09:15:00 by josfelip          #+#    #+#             */
/*   Updated: 2025/04/01 16:47:33 by josfelip         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpRequest.hpp"
#include <dirent.h>
#include <sstream>
#include <sys/stat.h>

/**
 * Generate an HTML directory listing
 */
HttpResponse HttpRequest::handleDirectoryListing(const std::string& dirPath,
                                        const std::string& requestPath,
                                        const LocationConfig& location)
{
    HttpResponse response;
    DIR* dir;
    struct dirent* entry;
    
    dir = opendir(dirPath.c_str());
    if (!dir)
    {
        response.setStatus(403);
        return response;
    }
    
    // Generate HTML for directory listing
    std::ostringstream html;
    html << "<!DOCTYPE html>\n"
         << "<html>\n"
         << "<head>\n"
         << "    <title>Index of " << requestPath << "</title>\n"
         << "    <style>\n"
         << "        body { font-family: sans-serif; }\n"
         << "        table { border-collapse: collapse; width: 100%; }\n"
         << "        th, td { padding: 8px; text-align: left; }\n"
         << "        tr:nth-child(even) { background-color: #f2f2f2; }\n"
         << "    </style>\n"
         << "</head>\n"
         << "<body>\n"
         << "    <h1>Index of " << requestPath << "</h1>\n"
         << "    <table>\n"
         << "        <tr>\n"
         << "            <th>Name</th>\n"
         << "            <th>Size</th>\n"
         << "            <th>Type</th>\n"
         << "        </tr>\n";
    
    // Add parent directory link if not at root
    if (requestPath != "/")
    {
        html << "        <tr>\n"
             << "            <td><a href=\"..\">..</a></td>\n"
             << "            <td>-</td>\n"
             << "            <td>Directory</td>\n"
             << "        </tr>\n";
    }
    
    // List directory contents
    while ((entry = readdir(dir)) != NULL)
    {
        std::string name = entry->d_name;
        
        // Skip hidden files and current/parent directory entries
        if (name[0] == '.' && (name.length() == 1 || 
            (name.length() == 2 && name[1] == '.')))
            continue;
        
        // Get file info
        struct stat fileStat;
        std::string fullPath = dirPath + "/" + name;
        stat(fullPath.c_str(), &fileStat);
        
        bool isDir = S_ISDIR(fileStat.st_mode);
        
        // Format file size
        std::string size;
        if (isDir)
            size = "-";
        else
        {
            std::ostringstream sizeStr;
            if (fileStat.st_size < 1024)
                sizeStr << fileStat.st_size << " B";
            else if (fileStat.st_size < 1024 * 1024)
                sizeStr << (fileStat.st_size / 1024) << " KB";
            else
                sizeStr << (fileStat.st_size / (1024 * 1024)) << " MB";
            size = sizeStr.str();
        }
        
        // Add table row
        html << "        <tr>\n"
             << "            <td><a href=\"" 
             << (requestPath == "/" ? "" : requestPath) << "/" << name
             << (isDir ? "/" : "") << "\">" << name << "</a></td>\n"
             << "            <td>" << size << "</td>\n"
             << "            <td>" << (isDir ? "Directory" : "File") 
             << "</td>\n"
             << "        </tr>\n";
    }
    
    closedir(dir);
    
    html << "    </table>\n"
         << "</body>\n"
         << "</html>";
    
    response.setStatus(200);
    response.setBody(html.str());
    response.addHeader("Content-Type", "text/html");
    
    return response;
}