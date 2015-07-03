/*
 * This file is part of the libCEC(R) library.
 *
 * libCEC(R) is Copyright (C) 2011-2013 Pulse-Eight Limited.  All rights reserved.
 * libCEC(R) is an original work, containing original code.
 *
 * libCEC(R) is a trademark of Pulse-Eight Limited.
 *
 * This program is dual-licensed; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *
 * Alternatively, you can license this library under a commercial license,
 * please contact Pulse-Eight Licensing for more information.
 *
 * For more information contact:
 * Pulse-Eight Licensing       <license@pulse-eight.com>
 *     http://www.pulse-eight.com/
 *     http://www.pulse-eight.net/
 */

#include "env.h"
#ifdef HAS_DRM_EDID_PARSER

#include "platform/os.h"
#include "drm-edid.h"
#include <dirent.h>
#include <fstream>

using namespace PLATFORM;

uint16_t CDRMEdidParser::GetPhysicalAddress(void)
{
  uint16_t iPA(0);

  #if defined(HAS_DRM_EDID_PARSER)

  // Fisrt we look for all DRM subfolder
  std::string baseDir = "/sys/class/drm/";
 
  DIR *dir = opendir(baseDir.c_str());

  struct dirent *entry = readdir(dir);
  std::string enablededid;
  std::string line;

  while (entry != NULL)
  {
    // We look if the element is a symlinl
    if (entry->d_type == DT_LNK)
    {
      // We look for the file enable to find the active video card
      std::string path, enabledPath, edidPath;
      path = baseDir + entry->d_name;
      enabledPath = path + "/enabled"; 
      edidPath = path + "/edid";
      
      std::ifstream f(enabledPath.c_str());
      if (f.is_open()) 
      {
        if (f.good() && !f.eof())
        {
          getline(f, line);
          
          // We look if the card is "Enabled"
          if (line == "enabled")
          {
            // We look if the directory have an edid file
            std::ifstream edid(edidPath.c_str());
            if (edid.is_open()) 
            {
              if (edid.good()) {
                enablededid = edidPath;
              }
              edid.close();
            }
          }
        }
        f.close();
      }
    }
    entry = readdir(dir);
  }

  closedir(dir);

  if (!enablededid.empty())
  {
    FILE *fp = fopen(enablededid.c_str(), "r");
  
    if (fp)
    {
      char buf[4096];
      memset(buf, 0, sizeof(buf));
      int iPtr(0);
      int c(0);
      while (c != EOF)
      {
        c = fgetc(fp);
        if (c != EOF)
          buf[iPtr++] = c;
      }
  
      iPA = CEDIDParser::GetPhysicalAddressFromEDID(buf, iPtr);
      fclose(fp);
    }
  }

  #endif

  return iPA;
}

#endif
