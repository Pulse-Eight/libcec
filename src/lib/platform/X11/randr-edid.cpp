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
 * Pulse-Eight Licensing     <license@pulse-eight.com>
 *     http://www.pulse-eight.com/
 *     http://www.pulse-eight.net/
 */

#include "env.h"

#include "lib/platform/os.h"
#include "randr-edid.h"

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/extensions/Xrandr.h>

#include <stdlib.h>

using namespace PLATFORM;

static const char * const edid_names[] = 
{ 
#if (RANDR_MAJOR > 1) || (RANDR_MAJOR == 1 && RANDR_MINOR >2) 
  RR_PROPERTY_RANDR_EDID,
#else
  "EDID",
#endif
  "EDID_DATA",
  "XFree86_DDC_EDID1_RAWDATA"
};

#define EDID_NAME_COUNT (sizeof(edid_names)/sizeof(*edid_names))

uint16_t CRandrEdidParser::GetPhysicalAddress(void)
{
  uint16_t physical_address = 0;

  /* open default X11 DISPLAY */
  Display *disp = XOpenDisplay(NULL);
  if( disp )
  {
    int event_base, error_base;
    int maj, min;

    if( XRRQueryExtension(disp, &event_base, &error_base)
     && XRRQueryVersion(disp, &maj, &min) )
    {
      int version = (maj << 8) | min;
        
      if( version >= 0x0102 )
      {
        size_t atom_avail = 0;
        Atom edid_atoms[EDID_NAME_COUNT];

        if( XInternAtoms(disp, (char **)edid_names, EDID_NAME_COUNT, True, edid_atoms) )
        {
          /* remove missing some atoms */
          atom_avail = 0;
          for(size_t atom_count=0; atom_count<EDID_NAME_COUNT; ++atom_count)
          {
            Atom edid_atom = edid_atoms[atom_count];
            if( None != edid_atom )
            {
              if( atom_avail < atom_count )
              {
                edid_atoms[atom_avail] = edid_atom;
              }
              ++atom_avail;
            }
          }
        }

        if( atom_avail > 0 )
        {
          int scr_count = ScreenCount(disp);
          int screen;

          for(screen=0; screen<scr_count; ++screen)
          {
            XRRScreenResources *rsrc = NULL;
            Window root = RootWindow(disp, screen);

#if (RANDR_MAJOR > 1) || (RANDR_MAJOR == 1 && RANDR_MINOR >=3) 
            if( version >= 0x0103 )
            {
              /* get cached resources if they are available */
              rsrc = XRRGetScreenResourcesCurrent(disp, root);
            }

            if( NULL == rsrc )
#endif
              rsrc = XRRGetScreenResources(disp, root);

            if( NULL != rsrc )
            {
              int output_id;
              for( output_id=0; 0 == physical_address && output_id < rsrc->noutput; ++output_id )
              {
                RROutput rr_output_id = rsrc->outputs[output_id];
                XRROutputInfo *output_info = XRRGetOutputInfo(disp, rsrc, rr_output_id);
                if( NULL != output_info )
                {
                  if( RR_Connected == output_info->connection )
                  {
                    for(size_t atom_count=0; 0 == physical_address && atom_count<atom_avail; ++atom_count)
                    {
                      Atom actual_type;
                      int actual_format;
                      unsigned long nitems;
                      unsigned long bytes_after;
                      unsigned char *data;
                      int status;

                      status = XRRGetOutputProperty(disp, rr_output_id, edid_atoms[atom_count], 0, 128, False, False,
                            AnyPropertyType, &actual_type, &actual_format,
                            &nitems, &bytes_after, &data);
                      if( Success == status )
                      {
                        if((actual_type == XA_INTEGER) && (actual_format == 8) )
                        {
                          physical_address = CEDIDParser::GetPhysicalAddressFromEDID(data, nitems);
                        }
                        XFree(data);
                      }
                    }
                  }
                  XRRFreeOutputInfo(output_info);
                }
                else
                  break;  /* problem ? */
              }
              XRRFreeScreenResources(rsrc);
            }
          }
        }
      }
    }
    XCloseDisplay(disp);
  }
  return physical_address;
}
